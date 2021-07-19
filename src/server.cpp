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
 
int main(int argc, char* argv[]) {
    Uuid id = boost::uuids::random_generator()();
    std::string id_str = boost::lexical_cast<std::string>(id);
    auto options = get_options(argc, argv);
    std::string log_name = options.log_name_prefix + "-" + id_str +
        options.log_name_ext;
    init_logging(log_name);
    using namespace logging::trivial;
    src::severity_logger<severity_level> logger;
    BOOST_LOG_SEV(logger, info) << "server ID " << id;
    wm::Message_builder msg_builder(id);
    const std::string protocol {"tcp"};
    wp::Work_parser parser(std::make_shared<std::ifstream>(options.workfile_name));
    std::ofstream ofs(options.out_name + "-" + id_str);
    std::ofstream efs(options.err_name + "-" + id_str);

    auto hostname = boost::asio::ip::host_name();
    const std::string bind_str {protocol + "://*:" +
                                std::to_string(options.port_nr)};
    const std::string info_str {protocol + "://" + hostname +
                                ":" + std::to_string(options.port_nr)};

    zmq::context_t context(1);
    zmq::socket_t socket(context, ZMQ_REP);
    socket.bind(bind_str);
    BOOST_LOG_SEV(logger, info) << "server address " << info_str;
    std::cout << id << " " << info_str << std::endl;

    std::set<size_t> to_do;
    size_t msg_counter {0};
    for (int msg_nr = 0; ; ++msg_nr) {
        zmq::message_t request;
        auto recv_result = socket.recv(request, zmq::recv_flags::none);
        if (!recv_result) {
            BOOST_LOG_SEV(logger, error) << "server could not receive message";
        }
        auto msg = unpack_message(request, msg_builder);
        if (msg.subject() == wm::Subject::query) {
            BOOST_LOG_SEV(logger, info) << "query message from "
                                        << msg.from();
            if (parser.has_next()) {
                std::string work_item = parser.next();
                size_t work_id = ++msg_counter;
                msg_builder.to(msg.from()) .subject(wm::Subject::work)
                    .id(work_id) .content(work_item);
                auto work_msg = msg_builder.build();
                to_do.insert(work_id);
                BOOST_LOG_SEV(logger, info) << "work message " << work_id
                                            << " to " << work_msg.to();
                auto send_result = socket.send(pack_message(work_msg), zmq::send_flags::none);
                if (!send_result) {
                    BOOST_LOG_SEV(logger, error) << "server could not send messag";
                }
            } else {
                msg_builder.to(msg.from()) .subject(wm::Subject::stop);
                auto stop_msg = msg_builder.build();
                BOOST_LOG_SEV(logger, info) << "stop message to "
                                            << stop_msg.to();
                auto send_result = socket.send(pack_message(stop_msg), zmq::send_flags::none);
                if (!send_result) {
                    BOOST_LOG_SEV(logger, error) << "server could not send message";
                }
                if (to_do.empty()) {
                    BOOST_LOG_SEV(logger, info) << "processing done";
                    break;
                }
            }
        } else if (msg.subject() == wm::Subject::result) {
            BOOST_LOG_SEV(logger, info) << "result message for " << msg.id()
                                        << " from " << msg.from();
            std::string result_str = msg.content();
            wpr::Result result(result_str);
            ofs << result.stdout() << std::endl;
            efs << result.stderr() << std::endl;
            BOOST_LOG_SEV(logger, info) << "work item " << msg.id()
                                        << ": " << result.exit_status();
            to_do.erase(msg.id());
            auto ack_msg = msg_builder.to(msg.from())
                               .subject(wm::Subject::ack).build();
            auto send_result = socket.send(pack_message(ack_msg), zmq::send_flags::none);
            if (!send_result) {
                BOOST_LOG_SEV(logger, error) << "server could not send message";
            }
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
    const int default_port {5555};
    std::string default_out_name {"out"};
    std::string default_err_name {"err"};
    std::string default_log_name_prefix {"server"};
    std::string default_log_name_ext {".log"};

    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "produce help message")
        ("version,v", "show software version")
        ("workfile", po::value<std::string>(&options.workfile_name),
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
