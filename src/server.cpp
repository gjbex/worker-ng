#include <iostream>
#include <zmq.hpp>

int main() {
    zmq::context_t context(1);
    zmq::socket_t socket(context, ZMQ_REP);
    socket.bind("tcp://*:5555");

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
