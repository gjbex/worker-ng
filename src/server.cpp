#include <boost/asio/ip/host_name.hpp>
#include <iostream>
#include <zmq.hpp>

int main() {
    const std::string protocol {"tcp"};
    const std::string port_nr {"5555"};
    auto hostname = boost::asio::ip::host_name();
    const std::string bind_str {protocol + "://*:" + port_nr};
    const std::string info_str {protocol + "://" + hostname +
                                ":" + port_nr};


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
