#include <boost/asio/ip/host_name.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/exception.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/log/trivial.hpp>
#include <boost/program_options.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <zmq.hpp>

#include "message.h"
#include "utils.h"
#include "work_processor/processor.h"
#include "worker_exception.h"

using Uuid = boost::uuids::uuid;
using EnvVarOptions = std::vector<std::string>;

using Options = struct {
    std::string server_name;
    Uuid server_id;
    int time_out;
    std::string log_name_prefix;
    std::string log_name_ext;
    std::string numactl;
    int nr_cores;
    std::string host_info;
    EnvVarOptions env_variables;
};

Options get_options(int argc, char* argv[]);

namespace logging = boost::log;
namespace wm = worker::message;
namespace wpr = worker::work_processor;

int main(int argc, char* argv[]) {
    // handle command line options
    auto options = get_options(argc, argv);

    // create UUID for this client
    Uuid client_id = boost::uuids::random_generator()();
    std::cout << client_id << std::endl;

    // parse environment variables
    wpr::Env env {boost::this_process::environment()};
    env["WORKER_CLIENT_ID"] = boost::lexical_cast<std::string>(client_id);
    env["WORKER_SERVER_NAME"] = options.server_name;
    env["WORKER_SERVER_ID"] = boost::lexical_cast<std::string>(options.server_id);
    env["WORKER_NUMACTL_OPTS"] = options.numactl;
    env["WORKER_NUM_CORES"] = std::to_string(options.nr_cores);
    env["WORKER_HOST_INFO"] = options.host_info;

    // set up logging
    std::string log_name = options.log_name_prefix + 
       boost::lexical_cast<std::string>(client_id) + options.log_name_ext;
    
    using namespace logging::trivial;
    try {
        init_logging(log_name);
        BOOST_LOG_TRIVIAL(info) << "client ID " << client_id;
    } catch (boost::wrapexcept<boost::filesystem::filesystem_error>& err) {
        std::cerr << "### error: can not create log file, " << err.what() << std::endl;
        worker::exit(worker::Error::file);
    }

    // create socket and connect to server
    zmq::context_t context(1);
    zmq::socket_t socket(context, ZMQ_REQ);
    socket.set(zmq::sockopt::rcvtimeo, options.time_out);
    socket.set(zmq::sockopt::sndtimeo, options.time_out);
    try {
        socket.connect(options.server_name);
        BOOST_LOG_TRIVIAL(info) << "connected to server"
                                    << options.server_name;
    } catch (zmq::error_t& err) {
        BOOST_LOG_TRIVIAL(error) << "socket connection failed, " << err.what();
        std::cerr << "### error: socket can not connect to " << options.server_name << std::endl;
        worker::exit(worker::Error::socket);
    }

    wm::Message_builder msg_builder(client_id);

    // message loop
    for (;;) {
        // ask server for work
        auto msg = msg_builder.to(options.server_id)
                           .subject(wm::Subject::query) .build();
        zmq::message_t request = pack_message(msg);
        BOOST_LOG_TRIVIAL(info) << "query message to " << msg.to();
        auto query_send_status = socket.send(request, zmq::send_flags::none);
        if (!query_send_status) {
            BOOST_LOG_TRIVIAL(error) << "client can not send query message";
        }
        // get and handle server's reply
        zmq::message_t reply;
        auto reply_recv_status = socket.recv(reply, zmq::recv_flags::none);
        if (!reply_recv_status) {
            BOOST_LOG_TRIVIAL(error) << "client can not receive query reply message";
        }
        msg = unpack_message(reply, msg_builder);
        if (msg.subject() == wm::Subject::stop) {
            // no more work, stop
            BOOST_LOG_TRIVIAL(info) << "stop message from "
                                        << msg.from();
            break;
        } else if(msg.subject() == wm::Subject::work) {
            // handle work
            BOOST_LOG_TRIVIAL(info) << "work message for " << msg.id()
                                        << " from " << msg.from();
            auto work_str = msg.content();
            auto work_id = msg.id();
            BOOST_LOG_TRIVIAL(info) << "work item " << work_id
                                        << " started";
            // add item-specific info to the environment
            env["WORKER_ITEM_ID"] = std::to_string(work_id);
            // execute work item
            auto result = wpr::process_work(work_str, env);
            BOOST_LOG_TRIVIAL(info) << "work item " << work_id
                                        << " finished: "
                                        << result.exit_status();
            // send result of work to server
            auto result_msg = msg_builder.to(options.server_id)
                                  .subject(wm::Subject::result).id(work_id)
                                  .content(result.to_string()).build();
            zmq::message_t result_reply = pack_message(result_msg);
            BOOST_LOG_TRIVIAL(info) << "result message for " << result_msg.id()
                                        << " to " << result_msg.to();
            auto result_send_status = socket.send(result_reply, zmq::send_flags::none);
            if (!result_send_status) {
                BOOST_LOG_TRIVIAL(error) << "client can not send result message";
            }
            // wait for acknowledgement from server
            zmq::message_t ack_response;
            auto ack_recv_status = socket.recv(ack_response, zmq::recv_flags::none);
            if (!ack_recv_status) {
                BOOST_LOG_TRIVIAL(error) << "client can not receive result ack message";
            } else {
                auto ack_msg = unpack_message(ack_response, msg_builder);
                BOOST_LOG_TRIVIAL(info) << "ack message from "
                    << ack_msg.from();
                if (ack_msg.subject() == wm::Subject::ack_stop) {
                    // no more work, stop
                    BOOST_LOG_TRIVIAL(info) << "stop message from "
                                                << ack_msg.from();
                    break;
                }
            }
        } else {
            // unknown message type
            BOOST_LOG_TRIVIAL(fatal) << "invalid message";
            worker::exit(worker::Error::unexpected);
        }
    }
    BOOST_LOG_TRIVIAL(info) << "exiting normally";
    return 0;
}

Options get_options(int argc, char* argv[]) {
    Options options;
    namespace po = boost::program_options;
    std::string server_uuid_str {""};
    const int default_time_out {1000};
    std::string default_log_name_prefix {"client"};
    std::string default_log_name_ext {".log"};
    std::string default_numactl {""};
    int default_nr_cores {1};
    std::string default_host_info {boost::asio::ip::host_name() + ":1"};

    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "produce help message")
        ("version,v", "show software version")
        ("server", po::value<std::string>(&options.server_name)->required(),
         "name of the server to use")
        ("uuid", po::value<std::string>(&server_uuid_str)->required(),
         "server UUID")
        ("timeout,t", po::value<int>(&options.time_out)
         ->default_value(default_time_out),
         "client time out in ms")
        ("log_prefix", po::value<std::string>(&options.log_name_prefix)
         ->default_value(default_log_name_prefix),
         "log file name prefix")
        ("log_ext", po::value<std::string>(&options.log_name_ext)
         ->default_value(default_log_name_ext),
         "log file name extension")
        ("numactl", po::value<std::string>(&options.numactl)
         ->default_value(default_numactl),
         "numactl optoins")
        ("num_cores", po::value<int>(&options.nr_cores)
         ->default_value(default_nr_cores),
         "number of cores the work items can use")
        ("host_info", po::value<std::string>(&options.host_info)
         ->default_value(default_host_info),
         "host information to construct an MPI hostfile")
        ("env", po::value<EnvVarOptions>(&options.env_variables)
         ->composing(),
         "environment varialbes to pass to process")
    ;
    po::variables_map vm;
    try {
        po::store(po::parse_command_line(argc, argv, desc), vm);
    } catch (boost::wrapexcept<boost::program_options::invalid_option_value>& err) {
        std::cerr << "### error: " << err.what() << std::endl;
        std::cerr << desc << std::endl;
        worker::exit(worker::Error::cli_option);
    } catch (boost::wrapexcept<boost::program_options::ambiguous_option>& err) {
        std::cerr << "### error: " << err.what() << std::endl;
        std::cerr << desc << std::endl;
        worker::exit(worker::Error::cli_option);
    }

    // first deal with options that don't require the required arguments
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

    try {
        options.server_id = boost::lexical_cast<Uuid>(server_uuid_str);
    } catch (boost::wrapexcept<boost::bad_lexical_cast>&) {
        std::cerr << "### error: invalid UUID" << std::endl;
        worker::exit(worker::Error::cli_option);
    }

    return options;
}
