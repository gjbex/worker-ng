#include <boost/program_options.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <iostream>
#include <sstream>
#include <tuple>
#include <zmq.hpp>

#include "envelope.h"
#include "utils.h"

namespace po = boost::program_options;

using Uuid = boost::uuids::uuid;

using Options = std::tuple<std::string, int>;

Options get_options(int argc, char* argv[]);

int main(int argc, char* argv[]) {
    Uuid id = boost::uuids::random_generator()();
    Envelope_builder env_builder(id);
    std::string server;
    int time_out;
    std::tie(server, time_out) = get_options(argc, argv);

    zmq::context_t context(1);
    zmq::socket_t socket(context, ZMQ_REQ);
    socket.setsockopt(ZMQ_RCVTIMEO, time_out);
    socket.setsockopt(ZMQ_SNDTIMEO, time_out);
    socket.connect(server);

    for (int request_nr = 0; request_nr < 5; ++request_nr) {
        zmq::message_t request(4);
        memcpy(request.data(), "ping", 4);
        std::cout << "sending ping " << request_nr << std::endl;
        socket.send(request);
        zmq::message_t reply;
        socket.recv(&reply);
        std::istringstream msg(static_cast<char*>(reply.data()));
        int msg_nr;
        msg >> msg_nr;
        std::cout << "received " << msg_nr << std::endl;
    }
    return 0;
}

Options get_options(int argc, char* argv[]) {
    std::string server {""};
    const int default_timeout {1000};
    int timeout;
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "produce help message")
        ("version,v", "show software version")
        ("server", po::value<std::string>(&server), "server to use")
        ("timeout", po::value<int>(&timeout)->default_value(default_timeout),
         "client time out in ms")
    ;
    po::positional_options_description pos_desc;
    pos_desc.add("server", -1);

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

    if (!vm.count("server")) {
        std::cerr << "### error: no server specified" << std::endl;
        std::exit(1);
    }

    return std::make_tuple(server, timeout);
}
