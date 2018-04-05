#include <boost/asio/ip/host_name.hpp>
#include <boost/program_options.hpp>
#include <iostream>
#include <zmq.hpp>

#include "utils.h"

namespace po = boost::program_options;

using Options = std::tuple<std::string, int>;

Options get_options(int argc, char* argv[]);
int main(int argc, char* argv[]) {
    const std::string protocol {"tcp"};
    std::string workfile_name;
    int port_nr;
    std::tie(workfile_name, port_nr) = get_options(argc, argv);
    auto hostname = boost::asio::ip::host_name();
    const std::string bind_str {protocol + "://*:" +
                                std::to_string(port_nr)};
    const std::string info_str {protocol + "://" + hostname +
                                ":" + std::to_string(port_nr)};


    zmq::context_t context(1);
    zmq::socket_t socket(context, ZMQ_REP);
    socket.bind(bind_str);
    std::cerr << "Server listening on " << info_str << std::endl;

    for (int msg_nr = 0; ; ++msg_nr) {
        zmq::message_t request;

        socket.recv(&request);
        std::cout << "received ping" << std::endl;
        std::string msg {std::to_string(msg_nr)};
        zmq::message_t reply(msg.length());
        memcpy(reply.data(), msg.c_str(), msg.length());
        socket.send(reply);
    }
    return 0;
}

Options get_options(int argc, char* argv[]) {
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
