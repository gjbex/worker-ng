#include <boost/asio/ip/host_name.hpp>
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

using Options = struct {
    std::string workfile_name;
    int port_nr;
    std::string log_name;
};

void print_to_do(const std::set<size_t> to_do) {
    std::cerr << "To do: ";
    for (const auto& id: to_do)
        std::cerr << " " << id;
    std::cerr << std::endl;
}

Options get_options(int argc, char* argv[]);

namespace logging = boost::log;
namespace src = boost::log::sources;
 
int main(int argc, char* argv[]) {
    auto options = get_options(argc, argv);
    init_logging(options.log_name);
    using namespace logging::trivial;
    src::severity_logger<severity_level> logger;
    Uuid id = boost::uuids::random_generator()();
    Message_builder msg_builder(id);
    const std::string protocol {"tcp"};
    Work_parser parser(std::make_shared<std::ifstream>(options.workfile_name));

    auto hostname = boost::asio::ip::host_name();
    const std::string bind_str {protocol + "://*:" +
                                std::to_string(options.port_nr)};
    const std::string info_str {protocol + "://" + hostname +
                                ":" + std::to_string(options.port_nr)};

    zmq::context_t context(1);
    zmq::socket_t socket(context, ZMQ_REP);
    socket.bind(bind_str);
    BOOST_LOG_SEV(logger, info) << "server ID " << id;
    BOOST_LOG_SEV(logger, info) << "server address " << info_str;
    std::cerr << "Server id: " << id << std::endl;
    std::cerr << "Server listening on " << info_str << std::endl;

    std::set<size_t> to_do;
    for (int msg_nr = 0; ; ++msg_nr) {
        zmq::message_t request;
        socket.recv(&request);
        auto msg = unpack_message(request, msg_builder);
        if (msg.subject() == Subject::query) {
            BOOST_LOG_SEV(logger, info) << "query message from " << msg.from();
            if (parser.has_next()) {
                std::string work_item = parser.next();
                size_t work_id = parser.nr_items();
                msg_builder.to(msg.from()) .subject(Subject::work)
                    .id(work_id) .content(work_item);
                auto work_msg = msg_builder.build();
                to_do.insert(work_id);
                BOOST_LOG_SEV(logger, info) << "work message to "
                                            << work_msg.to();
                socket.send(pack_message(work_msg));
            } else {
                msg_builder.to(msg.from()) .subject(Subject::stop);
                auto stop_msg = msg_builder.build();
                BOOST_LOG_SEV(logger, info) << "stop message to "
                                            << stop_msg.to();
                socket.send(pack_message(stop_msg));
                if (to_do.empty()) {
                    BOOST_LOG_SEV(logger, info) << "processing done";
                    break;
                }
            }
        } else if (msg.subject() == Subject::result) {
            BOOST_LOG_SEV(logger, info) << "reply message from " << msg.from();
            std::string result_str = msg.content();
            Result result(result_str);
            std::cout << "result: " << result.stdout() << std::endl;
            std::cerr << "result: " << result.stderr() << std::endl;
            int work_id = msg.id();
            to_do.erase(work_id);
            auto ack_msg = msg_builder.to(msg.from()).subject(Subject::ack)
                               .build();
            socket.send(pack_message(ack_msg));
        } else {
            BOOST_LOG_SEV(logger, fatal) << "invalid message";
            std::exit(2);
        }
    }
    BOOST_LOG_SEV(logger, info) << "exiting normally";
    return 0;
}

Options get_options(int argc, char* argv[]) {
    namespace po = boost::program_options;
    Options options;
    const std::string default_log_name {"server.log"};
    const int default_port {5555};
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "produce help message")
        ("version,v", "show software version")
        ("workfile", po::value<std::string>(&options.workfile_name),
         "work file to use")
        ("port", po::value<int>(&options.port_nr)->default_value(default_port),
         "port to listen on")
        ("log,l", po::value<std::string>(&options.log_name)->default_value(default_log_name),
         "name of the log file to use")
    ;
    po::positional_options_description pos_desc;
    pos_desc.add("workfile", -1);

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv)
              .options(desc).positional(pos_desc).run(), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc << std::endl;
        std::exit(0);
    }

    if (vm.count("version")) {
        print_version_info();
        std::exit(0);
    }

    if (!vm.count("workfile")) {
        std::cerr << "### error: no work file specified" << std::endl;
        std::exit(1);
    }

    return options;
}
