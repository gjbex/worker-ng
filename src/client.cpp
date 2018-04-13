#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <iostream>
#include <sstream>
#include <tuple>
#include <zmq.hpp>

#include "message.h"
#include "utils.h"

using Options = std::tuple<std::string, Uuid, int>;

Options get_options(int argc, char* argv[]);

int main(int argc, char* argv[]) {
    Uuid client_id = boost::uuids::random_generator()();
    Message_builder msg_builder(client_id);
    std::string server;
    Uuid server_id;
    int time_out;
    std::tie(server, server_id, time_out) = get_options(argc, argv);

    zmq::context_t context(1);
    zmq::socket_t socket(context, ZMQ_REQ);
    socket.setsockopt(ZMQ_RCVTIMEO, time_out);
    socket.setsockopt(ZMQ_SNDTIMEO, time_out);
    socket.connect(server);
    std::cerr << "Client id: " << client_id << std::endl;

    for (;;) {
        auto msg = msg_builder.to(server_id).subject(Subject::query)
                           .build();
        zmq::message_t request = pack_message(msg);
        std::cerr << "sending request to " << msg.to() << std::endl;
        socket.send(request);
        zmq::message_t reply;
        socket.recv(&reply);
        msg = unpack_message(reply, msg_builder);
        if (msg.subject() == Subject::stop) {
            std::cerr << "received stop from " << msg.from()
                      << ", exiting..." << std::endl;
            break;
        } else if(msg.subject() == Subject::work) {
            std::cerr << "received work from " << msg.from()
                      << ", processing..." << std::endl;
            auto work_str = msg.content();
            auto work_id = msg.id();
            std::cout << work_str << std::endl;
            std::string result {"done"};
            auto result_msg = msg_builder.to(server_id)
                                  .subject(Subject::result).id(work_id)
                                  .content(result).build();
            zmq::message_t result_reply = pack_message(result_msg);
            std::cerr << "sending result to " << msg.to() << std::endl;
            socket.send(result_reply);
            zmq::message_t ack_response;
            socket.recv(&ack_response);
        } else {
            std::cerr << "### error: receive invalid message" << std::endl;
            std::exit(2);
        }
    }
    return 0;
}

Options get_options(int argc, char* argv[]) {
    namespace po = boost::program_options;
    std::string server {""};
    std::string server_uuid_str {""};
    const int default_timeout {1000};
    int timeout;
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "produce help message")
        ("version,v", "show software version")
        ("server", po::value<std::string>(&server), "server to use")
        ("uuid", po::value<std::string>(&server_uuid_str), "server UUID")
        ("timeout", po::value<int>(&timeout)->default_value(default_timeout),
         "client time out in ms")
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
    Uuid uuid = boost::lexical_cast<Uuid>(server_uuid_str);

    return std::make_tuple(server, uuid, timeout);
}
