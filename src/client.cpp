#include <iostream>
#include <sstream>
#include <zmq.hpp>

int main() {
    const int time_out {1000}; // milliseconds
    zmq::context_t context(1);
    zmq::socket_t socket(context, ZMQ_REQ);
    socket.setsockopt(ZMQ_RCVTIMEO, time_out);
    socket.setsockopt(ZMQ_SNDTIMEO, time_out);
    socket.connect("tcp://localhost:5555");

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
