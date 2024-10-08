#include <boost/asio/ip/host_name.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/exception.hpp>
#include <boost/log/trivial.hpp>
#include <boost/program_options.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <chrono>
#include <fstream>
#include <iostream>
#include <set>
#include <thread>
#include <zmq.hpp>

#include "message.h"
#include "utils.h"
#include "worker_exception.h"
#include "work_parser/work_parser.h"
#include "work_processor/result.h"

using Options = struct {
    std::string server_info;
    std::string workfile_name;
    int port_nr;
    std::string out_name;
    std::string err_name;
    std::string log_name;
    long wait_time;
};

using Uuid = boost::uuids::uuid;

void print_to_do(const std::set<size_t>& to_do) {
    std::cerr << "To do: ";
    for (const auto& id: to_do)
        std::cerr << " " << id;
    std::cerr << std::endl;
}

Options get_options(int argc, char* argv[]);

namespace logging = boost::log;
namespace wm = worker::message;
namespace wp = worker::work_parser;
namespace wpr = worker::work_processor;

using namespace logging::trivial;

void write_server_info(const std::string& file_name, const Uuid& id,
        const std::string& info_str);
size_t send_work(zmq::socket_t& socket, const Uuid& dest,
        wp::Work_parser& parser, wm::Message_builder& msg_builder);
void send_stop(zmq::socket_t& socket, const Uuid& dest,
        wm::Message_builder& msg_builder);
void send_ack(zmq::socket_t& socket, const Uuid& dest,
        wm::Message_builder& msg_builder);
void send_ack_stop(zmq::socket_t& socket, const Uuid& dest,
        wm::Message_builder& msg_builder);

int main(int argc, char* argv[]) {
    // determine UUID for this run
    Uuid id = boost::uuids::random_generator()();
    std::string id_str = boost::lexical_cast<std::string>(id);

    // handle command line options
    auto options = get_options(argc, argv);

    // set up logging
    try {
        init_logging(options.log_name);
        BOOST_LOG_TRIVIAL(info) << "server ID " << id;
    } catch (boost::wrapexcept<boost::filesystem::filesystem_error>& err) {
        std::cerr << "### error: can not create log file, " << err.what() << std::endl;
        worker::exit(worker::Error::file);
    }

    // open workfile and create work parser
    std::ifstream ifs(options.workfile_name);
    if (ifs.fail()) {
        BOOST_LOG_TRIVIAL(error) << "could not open workfile '" << options.workfile_name << "'";
        std::cerr << "### error: can not open workfile '" << options.workfile_name << "'" << std::endl;
        worker::exit(worker::Error::file);
    }
    wp::Work_parser parser(ifs);

    // open output file
    std::ofstream ofs;
    if (options.out_name.length() > 0) {
        ofs.open(options.out_name);
        if (ofs.fail()) {
            BOOST_LOG_TRIVIAL(error) << "could not open output '" << options.out_name << "'";
            std::cerr << "### error: can not create output file '" << options.out_name << "'" << std::endl;
            worker::exit(worker::Error::file);
        }
    }
    std::ostream& out_stream(ofs.is_open() ? ofs : std::cout);

    // open error file
    std::ofstream efs(options.err_name);
    if (options.err_name.length() > 0) {
        efs.open(options.err_name);
        if (efs.fail()) {
            BOOST_LOG_TRIVIAL(error) << "could not open error '" << options.err_name << "'";
            std::cerr << "### error: can not create error file '" << options.err_name << "'" << std::endl;
            worker::exit(worker::Error::file);
        }
    }
    std::ostream& err_stream(efs.is_open() ? efs : std::cerr);

    // create socket and bind to it
    const std::string protocol {"tcp"};
    auto hostname = boost::asio::ip::host_name();
    const std::string bind_str {protocol + "://*:" +
        std::to_string(options.port_nr)};

    zmq::context_t context(1);
    zmq::socket_t socket(context, ZMQ_REP);
    try {
        socket.bind(bind_str);
        BOOST_LOG_TRIVIAL(info) << "socket boundi on " << bind_str;
    } catch (zmq::error_t& err) {
        BOOST_LOG_TRIVIAL(error) << "socket binding failed, " << err.what();
        std::cerr << "### error: socket can not bind to " << bind_str << std::endl;
        worker::exit(worker::Error::socket);
    }

    // show server info for use by clients
    const std::string info_str {protocol + "://" + hostname +
        ":" + std::to_string(options.port_nr)};
    BOOST_LOG_TRIVIAL(info) << "server address " << info_str;
    write_server_info(options.server_info, id, info_str);

    wm::Message_builder msg_builder(id);
    // set of open work item IDs, i.e., work items started, but not completed yet
    std::set<size_t> to_do;


    // start message loop
    for (;;) {
        // wait for incoming messages
        zmq::message_t request;
        auto recv_result = socket.recv(request, zmq::recv_flags::none);
        if (!recv_result) {
            BOOST_LOG_TRIVIAL(error) << "server could not receive message";
        }
        auto msg = unpack_message(request, msg_builder);

        // handle incoming message
        if (msg.subject() == wm::Subject::query) {
            // client wants work, if there is work, send it, if not send stop message
            BOOST_LOG_TRIVIAL(info) << "query message from "
                << msg.from();
            if (parser.has_next()) {
                size_t work_id = send_work(socket, msg.from(), parser, msg_builder);
                to_do.insert(work_id);
                BOOST_LOG_TRIVIAL(info) << "workitem " << work_id
                    << " started: " << msg.from();
            } else {
                send_stop(socket, msg.from(), msg_builder);
                BOOST_LOG_TRIVIAL(info) << "stop message to "
                    << msg.from();
            }
        } else if (msg.subject() == wm::Subject::result) {
            // client sent result, handle it, and send acknowledgement
            BOOST_LOG_TRIVIAL(info) << "result message for " << msg.id()
                << " from " << msg.from();
            std::string result_str = msg.content();
            wpr::Result result(result_str);
            out_stream << result.stdout() << std::endl;
            err_stream << result.stderr() << std::endl;
            BOOST_LOG_TRIVIAL(info) << "workitem " << msg.id()
                << " done: " << result.exit_status();
            to_do.erase(msg.id());
            if (parser.has_next()) {
                send_ack(socket, msg.from(), msg_builder);
                BOOST_LOG_TRIVIAL(info) << "ack message to "
                    << msg.from();
            } else {
                send_ack_stop(socket, msg.from(), msg_builder);
                BOOST_LOG_TRIVIAL(info) << "ack_stop message to "
                    << msg.from();
            }
        } else {
            BOOST_LOG_TRIVIAL(fatal) << "invalid message";
            worker::exit(worker::Error::unexpected);
        }
        if (!parser.has_next() && to_do.empty()) {
            BOOST_LOG_TRIVIAL(info) << "processing done";
            break;
        }
    }
    std::this_thread::sleep_for(std::chrono::seconds(options.wait_time));
    BOOST_LOG_TRIVIAL(info) << "exiting normally";
    return 0;
}

Options get_options(int argc, char* argv[]) {
    namespace po = boost::program_options;
    Options options;
    const int default_port {5555};
    std::string default_out_name {""};
    std::string default_err_name {""};
    std::string default_log_name {"server.log"};
    long default_wait_time {3};

    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "produce help message")
        ("version,v", "show software version")
        ("server_info", po::value<std::string>(&options.server_info)->required(),
         "file name to store server info in")
        ("workfile", po::value<std::string>(&options.workfile_name)->required(),
         "work file to use")
        ("port", po::value<int>(&options.port_nr)
         ->default_value(default_port), "port to listen on")
        ("out", po::value<std::string>(&options.out_name)
         ->default_value(default_out_name), "output file name")
        ("err", po::value<std::string>(&options.err_name)
         ->default_value(default_err_name), "error file name")
        ("log", po::value<std::string>(&options.log_name)
         ->default_value(default_log_name),
         "log file name")
        ("wait", po::value<long>(&options.wait_time)
         ->default_value(default_wait_time),
         "wait time before server exit in seconds")
        ;
    po::positional_options_description pos_desc;
    pos_desc.add("workfile", -1);
    po::variables_map vm;
    try {
        po::store(po::command_line_parser(argc, argv)
                .options(desc).positional(pos_desc).run(), vm);
    } catch (boost::wrapexcept<boost::program_options::invalid_option_value>& err) {
        std::cerr << "### error: " << err.what() << std::endl;
        std::cerr << desc << std::endl;
        worker::exit(worker::Error::cli_option);
    } catch (boost::wrapexcept<boost::program_options::ambiguous_option>& err) {
        std::cerr << "### error: " << err.what() << std::endl;
        std::cerr << desc << std::endl;
        worker::exit(worker::Error::cli_option);
    }

    if (vm.count("help")) {
        std::cout << desc << std::endl;
        std::exit(0);
    }

    if (vm.count("version")) {
        print_version_info();
        std::exit(0);
    }

    try {
        po::notify(vm);
    } catch (boost::wrapexcept<boost::program_options::required_option>& err) {
        std::cerr << "### error: " << err.what() << std::endl;
        std::cerr << desc << std::endl;
        worker::exit(worker::Error::cli_option);
    }

    if (options.port_nr < 1 || options.port_nr > 65535) {
        std::cerr << "### error: invalid port number" << std::endl;
        worker::exit(worker::Error::cli_option);
    }

    return options;
}

void write_server_info(const std::string& file_name, const Uuid& id,
        const std::string& info_str) {
    std::ofstream ofs(file_name);
    if (ofs.fail()) {
        BOOST_LOG_TRIVIAL(error) << "could not open server_info file '" << file_name << "'";
        std::cerr << "### error: can not open server_info file '" << file_name << "'" << std::endl;
        worker::exit(worker::Error::file);
    }
    ofs << id << " " << info_str << std::endl;
    BOOST_LOG_TRIVIAL(info) << "created server_info file '" << file_name << "'";
}

size_t send_work(zmq::socket_t& socket, const Uuid& dest,
        wp::Work_parser& parser, wm::Message_builder& msg_builder) {
    std::string work_item = parser.next();
    size_t work_id = parser.nr_items();
    msg_builder.to(dest).subject(wm::Subject::work)
        .id(work_id) .content(work_item);
    auto work_msg = msg_builder.build();
    BOOST_LOG_TRIVIAL(info) << "work message " << work_id
                                << " to " << work_msg.to();
    auto send_result = socket.send(pack_message(work_msg), zmq::send_flags::none);
    if (!send_result) {
        BOOST_LOG_TRIVIAL(error) << "server could not send work messag";
    }
    return work_id;
}

void send_stop(zmq::socket_t& socket, const Uuid& dest,
        wm::Message_builder& msg_builder) {
    msg_builder.to(dest).subject(wm::Subject::stop);
    auto stop_msg = msg_builder.build();
    BOOST_LOG_TRIVIAL(info) << "stop message to "
                                << stop_msg.to();
    auto send_result = socket.send(pack_message(stop_msg), zmq::send_flags::none);
    if (!send_result) {
        BOOST_LOG_TRIVIAL(error) << "server could not send stop message";
    }
}

void send_ack(zmq::socket_t& socket, const Uuid& dest,
        wm::Message_builder& msg_builder) {
    auto ack_msg = msg_builder.to(dest).subject(wm::Subject::ack).build();
    auto send_result = socket.send(pack_message(ack_msg), zmq::send_flags::none);
    if (!send_result) {
        BOOST_LOG_TRIVIAL(error) << "server could not send ack message";
    }
}

void send_ack_stop(zmq::socket_t& socket, const Uuid& dest,
        wm::Message_builder& msg_builder) {
    auto ack_msg = msg_builder.to(dest).subject(wm::Subject::ack_stop).build();
    auto send_result = socket.send(pack_message(ack_msg), zmq::send_flags::none);
    if (!send_result) {
        BOOST_LOG_TRIVIAL(error) << "server could not send ack message";
    }
}
