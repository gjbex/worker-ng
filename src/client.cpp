#include <boost/lexical_cast.hpp>
#define BOOST_LOG_DYN_LINK 1
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/program_options.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <iostream>
#include <sstream>
#include <zmq.hpp>

#include "message.h"
#include "utils.h"
#include "work_processor/processor.h"

using Options = struct {
    std::string server_name;
    Uuid server_id;
    int time_out;
    std::string log_name_prefix;
    std::string log_name_ext;
};

Options get_options(int argc, char* argv[]);

namespace logging = boost::log;
namespace src = boost::log::sources;
namespace wm = worker::message;

int main(int argc, char* argv[]) {
    auto options = get_options(argc, argv);
    Uuid client_id = boost::uuids::random_generator()();
    std::cout << client_id << std::endl;
    std::string log_name = options.log_name_prefix + "-" +
       boost::lexical_cast<std::string>(client_id) + options.log_name_ext;
    init_logging(log_name);
    using namespace logging::trivial;
    src::severity_logger<severity_level> logger;
    BOOST_LOG_SEV(logger, info) << "client ID " << client_id;
    wm::Message_builder msg_builder(client_id);

    zmq::context_t context(1);
    zmq::socket_t socket(context, ZMQ_REQ);
    socket.setsockopt(ZMQ_RCVTIMEO, options.time_out);
    socket.setsockopt(ZMQ_SNDTIMEO, options.time_out);
    socket.connect(options.server_name);
    BOOST_LOG_SEV(logger, info) << "connected to server"
                                << options.server_name;

    for (;;) {
        auto msg = msg_builder.to(options.server_id)
                           .subject(wm::Subject::query) .build();
        zmq::message_t request = pack_message(msg);
        BOOST_LOG_SEV(logger, info) << "query message to " << msg.to();
        socket.send(request);
        zmq::message_t reply;
        socket.recv(&reply);
        msg = unpack_message(reply, msg_builder);
        if (msg.subject() == wm::Subject::stop) {
            BOOST_LOG_SEV(logger, info) << "stop message from "
                                        << msg.from();
            break;
        } else if(msg.subject() == wm::Subject::work) {
            BOOST_LOG_SEV(logger, info) << "work message for " << msg.id()
                                        << " from " << msg.from();
            auto work_str = msg.content();
            auto work_id = msg.id();
            BOOST_LOG_SEV(logger, info) << "work item " << work_id
                                        << " started";
            Result result = process_work(work_str);
            BOOST_LOG_SEV(logger, info) << "work item " << work_id
                                        << " finished: "
                                        << result.exit_status();
            auto result_msg = msg_builder.to(options.server_id)
                                  .subject(wm::Subject::result).id(work_id)
                                  .content(result.to_string()).build();
            zmq::message_t result_reply = pack_message(result_msg);
            BOOST_LOG_SEV(logger, info) << "result message for " << msg.id()
                                        << " to " << msg.to();
            socket.send(result_reply);
            zmq::message_t ack_response;
            socket.recv(&ack_response);
        } else {
            BOOST_LOG_SEV(logger, fatal) << "invalid message";
            std::exit(2);
        }
    }
    BOOST_LOG_SEV(logger, info) << "exiting normally";
    return 0;
}

Options get_options(int argc, char* argv[]) {
    Options options;
    namespace po = boost::program_options;
    std::string server_uuid_str {""};
    const int default_time_out {1000};
    std::string default_log_name_prefix {"client"};
    std::string default_log_name_ext {".log"};

    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "produce help message")
        ("version,v", "show software version")
        ("server", po::value<std::string>(&options.server_name),
         "name of the server to use")
        ("uuid", po::value<std::string>(&server_uuid_str), "server UUID")
        ("timeout,t", po::value<int>(&options.time_out)
             ->default_value(default_time_out),
         "client time out in ms")
        ("log_prefix", po::value<std::string>(&options.log_name_prefix)
             ->default_value(default_log_name_prefix),
         "log file name prefix")
        ("log_ext", po::value<std::string>(&options.log_name_ext)
             ->default_value(default_log_name_ext),
         "log file name extension")
    ;
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc << std::endl;
        std::exit(0);
    }

    if (vm.count("version")) {
        print_version_info();
        std::exit(0);
    }

    if (!vm.count("server")) {
        std::cerr << "### error: no server specified" << std::endl;
        std::exit(1);
    }

    if (!vm.count("uuid")) {
        std::cerr << "### error: no server UUID specified" << std::endl;
        std::exit(1);
    }
    options.server_id = boost::lexical_cast<Uuid>(server_uuid_str);

    return options;
}
