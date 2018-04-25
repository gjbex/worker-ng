#ifndef UTILS_HDR
#define UTILS_HDR

#include <exception>
#include <zmq.hpp>

#include "message.h"

void print_version_info();

worker::message::Message unpack_message(
        const zmq::message_t& zmq_msg,
        const worker::message::Message_builder& msg_builder
);

zmq::message_t pack_message(const worker::message::Message& msg);

void init_logging(const std::string& file_name);

#endif
