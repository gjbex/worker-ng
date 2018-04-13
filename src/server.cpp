#include <boost/asio/ip/host_name.hpp>
#include <boost/program_options.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <fstream>
#include <iostream>
#include <zmq.hpp>

#include "message.h"
#include "utils.h"
#include "work_parser/work_parser.h"

using Options = std::tuple<std::string, int>;

Options get_options(int argc, char* argv[]);

int main(int argc, char* argv[]) {
    Uuid id = boost::uuids::random_generator()();
    Message_builder msg_builder(id);
    const std::string protocol {"tcp"};
    std::string workfile_name;
    int port_nr;
    std::tie(workfile_name, port_nr) = get_options(argc, argv);
    Work_parser parser(std::make_shared<std::ifstream>(workfile_name));

    auto hostname = boost::asio::ip::host_name();
    const std::string bind_str {protocol + "://*:" +
                                std::to_string(port_nr)};
    const std::string info_str {protocol + "://" + hostname +
                                ":" + std::to_string(port_nr)};


    zmq::context_t context(1);
    zmq::socket_t socket(context, ZMQ_REP);
    socket.bind(bind_str);
    std::cerr << "Server id: " << id << std::endl;
    std::cerr << "Server listening on " << info_str << std::endl;

    for (int msg_nr = 0; ; ++msg_nr) {
        zmq::message_t request;
        socket.recv(&request);
        auto msg = unpack_message(request, msg_builder);
        if (msg.subject() == Subject::query) {
            std::cerr << "query message from " << msg.from() << std::endl;
            if (parser.has_next()) {
                std::string work_item = parser.next();
                size_t work_id = parser.nr_items();
                msg_builder.to(msg.from()) .subject(Subject::work)
                    .id(work_id) .content(work_item);
            } else {
                msg_builder.to(msg.from()) .subject(Subject::stop);
            }
            auto work_msg = msg_builder.build();
            socket.send(pack_message(work_msg));
        } else if (msg.subject() == Subject::result) {
            std::cerr << "reply message from " << msg.from() << std::endl;
            std::cout << "result: " << msg.content() << std::endl;
            auto ack_msg = msg_builder.to(msg.from()).subject(Subject::ack)
                               .build();
            socket.send(pack_message(ack_msg));
        } else {
            std::cerr << "### error: receive invalid message" << std::endl;
            std::exit(2);
        }
    }
    return 0;
}

Options get_options(int argc, char* argv[]) {
    namespace po = boost::program_options;
    std::string workfile_name;
    const int default_port {5555};
    int port_nr;
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "produce help message")
        ("version,v", "show software version")
        ("workfile", po::value<std::string>(&workfile_name),
         "work file to use")
        ("port", po::value<int>(&port_nr)->default_value(default_port),
         "port to listen on")
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

    return std::make_tuple(workfile_name, port_nr);
}
