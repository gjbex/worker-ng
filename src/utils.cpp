#include <iostream>

#include "worker_ng_config.h"
#include "utils.h"


void print_version_info() {
    std::cout << worker_ng_NAME << " "
        << worker_ng_VERSION_MAJOR << "."
        << worker_ng_VERSION_MINOR;
    int zmq_major, zmq_minor, zmq_patch;
    std::tie(zmq_major, zmq_minor, zmq_patch) = zmq::version();
    std::cout << ", using 0MQ "
        << zmq_major << "."
        << zmq_minor << "."
        << zmq_patch << std::endl;
};

Message get_message(const zmq::message_t& zmq_msg) {
    std::string msg_str;
    msg_str.resize(zmq_msg.size());
    memcpy(&msg_str[0], zmq_msg.data(), request.size());
    return msg_builder.build(std::string(msg_str));
}
