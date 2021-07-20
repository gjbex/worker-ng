#include <boost/asio/ip/host_name.hpp>
#include <boost/filesystem/exception.hpp>
#define BOOST_LOG_DYN_LINK 1
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/program_options.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <fstream>
#include <iostream>
#include <set>
#include <zmq.hpp>

#include "message.h"
#include "utils.h"
#include "work_parser/work_parser.h"
#include "work_processor/result.h"

enum class Error : int {
    cli_option = 1,
    file = 2,
    socket = 3,
    unexpected = 4
};

void exit_on(Error err) {
    std::exit(static_cast<int>(err));
}

using Options = struct {
    std::string workfile_name;
    int port_nr;
    std::string out_name;
    std::string err_name;
    std::string log_name_prefix;
    std::string log_name_ext;
};

using Uuid = boost::uuids::uuid;

void print_to_do(const std::set<size_t> to_do) {
    std::cerr << "To do: ";
    for (const auto& id: to_do)
        std::cerr << " " << id;
    std::cerr << std::endl;
}

Options get_options(int argc, char* argv[]);

namespace logging = boost::log;
namespace src = boost::log::sources;
namespace wm = worker::message;
namespace wp = worker::work_parser;
namespace wpr = worker::work_processor;
 
using namespace logging::trivial;
using Logger = src::severity_logger<severity_level>;

size_t send_work(zmq::socket_t& socket, const Uuid& dest,
        wp::Work_parser& parser, wm::Message_builder& msg_builder,
        Logger& logger);
void send_stop(zmq::socket_t& socket, const Uuid& dest,
        wm::Message_builder& msg_builder, Logger& logger);
void send_ack(zmq::socket_t& socket, const Uuid& dest,
        wm::Message_builder& msg_builder, Logger& logger);

int main(int argc, char* argv[]) {
    // determine UUID for this run
    Uuid id = boost::uuids::random_generator()();
    std::string id_str = boost::lexical_cast<std::string>(id);

    // handle command line options
    auto options = get_options(argc, argv);

    // set up logging
    Logger logger;
    std::string log_name = options.log_name_prefix + "-" + id_str +
        options.log_name_ext;
    try {
        init_logging(log_name);
        BOOST_LOG_SEV(logger, info) << "server ID " << id;
    } catch (boost::wrapexcept<boost::filesystem::filesystem_error>& err) {
        std::cerr << "### error: can not create log file, " << err.what() << std::endl;
        exit_on(Error::file);
    }

    // open workfile and create work parser
    std::ifstream ifs(options.workfile_name);
    if (ifs.fail()) {
        BOOST_LOG_SEV(logger, error) << "could not open workfile '" << options.workfile_name << "'";
        std::cerr << "### error: can not open workfile '" << options.workfile_name << "'" << std::endl;
        exit_on(Error::file);
    }
    wp::Work_parser parser(ifs);

    // open output file
    const std::string out_file_name = options.out_name + "-" + id_str;
    std::ofstream ofs(out_file_name);
    if (ofs.fail()) {
        BOOST_LOG_SEV(logger, error) << "could not open output '" << out_file_name << "'";
        std::cerr << "### error: can not create output file '" << out_file_name << "'" << std::endl;
        exit_on(Error::file);
    }

    // open error file
    const std::string err_file_name = options.err_name + "-" + id_str;
    std::ofstream efs(err_file_name);
    if (efs.fail()) {
        BOOST_LOG_SEV(logger, error) << "could not open error '" << err_file_name << "'";
        std::cerr << "### error: can not create error file '" << err_file_name << "'" << std::endl;
        exit_on(Error::file);
    }

    // create socket and bind to it
    const std::string protocol {"tcp"};
    auto hostname = boost::asio::ip::host_name();
    const std::string bind_str {protocol + "://*:" +
                                std::to_string(options.port_nr)};

    zmq::context_t context(1);
    zmq::socket_t socket(context, ZMQ_REP);
    try {
        socket.bind(bind_str);
    } catch (zmq::error_t& err) {
        BOOST_LOG_SEV(logger, error) << "socket binding failed, " << err.what();
        std::cerr << "### error: socket can not bind to " << bind_str << std::endl;
        exit_on(Error::socket);
    }

    // show server info for use by clients
    const std::string info_str {protocol + "://" + hostname +
                                ":" + std::to_string(options.port_nr)};
    BOOST_LOG_SEV(logger, info) << "server address " << info_str;
    std::cout << id << " " << info_str << std::endl;

    wm::Message_builder msg_builder(id);
    // set of open work item IDs, i.e., work items started, but not completed yet
    std::set<size_t> to_do;


    // start message loop
    for (size_t msg_nr = 0; ; ++msg_nr) {
        // wait for incoming messages
        zmq::message_t request;
        auto recv_result = socket.recv(request, zmq::recv_flags::none);
        if (!recv_result) {
            BOOST_LOG_SEV(logger, error) << "server could not receive message";
        }
        auto msg = unpack_message(request, msg_builder);

        // handle incoming message
        if (msg.subject() == wm::Subject::query) {
            // client wants work, if there is work, send it, if not send stop message
            BOOST_LOG_SEV(logger, info) << "query message from "
                                        << msg.from();
            if (parser.has_next()) {
                size_t work_id = send_work(socket, msg.from(), parser, msg_builder, logger);
                to_do.insert(work_id);
            } else {
                send_stop(socket, msg.from(), msg_builder, logger);
                if (to_do.empty()) {
                    BOOST_LOG_SEV(logger, info) << "processing done";
                    break;
                }
            }
        } else if (msg.subject() == wm::Subject::result) {
            // client sent result, handle it, and send acknowledgement
            BOOST_LOG_SEV(logger, info) << "result message for " << msg.id()
                                        << " from " << msg.from();
            std::string result_str = msg.content();
            wpr::Result result(result_str);
            ofs << result.stdout() << std::endl;
            efs << result.stderr() << std::endl;
            BOOST_LOG_SEV(logger, info) << "work item " << msg.id()
                                        << ": " << result.exit_status();
            to_do.erase(msg.id());
            send_ack(socket, msg.from(), msg_builder, logger);
        } else {
            BOOST_LOG_SEV(logger, fatal) << "invalid message";
            exit_on(Error::unexpected);
        }
    }
    BOOST_LOG_SEV(logger, info) << "exiting normally";
    return 0;
}

Options get_options(int argc, char* argv[]) {
    namespace po = boost::program_options;
    Options options;
    const int default_port {5555};
    std::string default_out_name {"out"};
    std::string default_err_name {"err"};
    std::string default_log_name_prefix {"server"};
    std::string default_log_name_ext {".log"};

    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "produce help message")
        ("version,v", "show software version")
        ("workfile", po::value<std::string>(&options.workfile_name)->required(),
         "work file to use")
        ("port", po::value<int>(&options.port_nr)
            ->default_value(default_port), "port to listen on")
        ("out,o", po::value<std::string>(&options.out_name)
            ->default_value(default_out_name), "output file name prefix")
        ("err,e", po::value<std::string>(&options.err_name)
            ->default_value(default_err_name), "error file name prefix")
        ("log_prefix", po::value<std::string>(&options.log_name_prefix)
            ->default_value(default_log_name_prefix),
         "log file name prefix")
        ("log_ext", po::value<std::string>(&options.log_name_ext)
            ->default_value(default_log_name_ext),
         "log file name extension")
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
        exit_on(Error::cli_option);
    } catch (boost::wrapexcept<boost::program_options::ambiguous_option>& err) {
        std::cerr << "### error: " << err.what() << std::endl;
        std::cerr << desc << std::endl;
        exit_on(Error::cli_option);
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
        exit_on(Error::cli_option);
    }

    if (options.port_nr < 1 || options.port_nr > 65535) {
        std::cerr << "### error: invalid port number" << std::endl;
        exit_on(Error::cli_option);
    }

    return options;
}

size_t send_work(zmq::socket_t& socket, const Uuid& dest,
        wp::Work_parser& parser, wm::Message_builder& msg_builder,
        Logger& logger) {
    std::string work_item = parser.next();
    size_t work_id = parser.nr_items();
    msg_builder.to(dest) .subject(wm::Subject::work)
        .id(work_id) .content(work_item);
    auto work_msg = msg_builder.build();
    BOOST_LOG_SEV(logger, info) << "work message " << work_id
                                << " to " << work_msg.to();
    auto send_result = socket.send(pack_message(work_msg), zmq::send_flags::none);
    if (!send_result) {
        BOOST_LOG_SEV(logger, error) << "server could not send messag";
    }
    return work_id;
}

void send_stop(zmq::socket_t& socket, const Uuid& dest,
        wm::Message_builder& msg_builder, Logger& logger) {
    msg_builder.to(dest) .subject(wm::Subject::stop);
    auto stop_msg = msg_builder.build();
    BOOST_LOG_SEV(logger, info) << "stop message to "
                                << stop_msg.to();
    auto send_result = socket.send(pack_message(stop_msg), zmq::send_flags::none);
    if (!send_result) {
        BOOST_LOG_SEV(logger, error) << "server could not send message";
    }
}

void send_ack(zmq::socket_t& socket, const Uuid& dest,
        wm::Message_builder& msg_builder, Logger& logger) {
    auto ack_msg = msg_builder.to(dest) .subject(wm::Subject::ack).build();
    auto send_result = socket.send(pack_message(ack_msg), zmq::send_flags::none);
    if (!send_result) {
        BOOST_LOG_SEV(logger, error) << "server could not send message";
    }
}
