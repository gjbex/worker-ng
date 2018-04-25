#ifndef UTILS_HDR
#define UTILS_HDR

#include <exception>
#include <zmq.hpp>

#include "message.h"

void print_version_info();

Message unpack_message(const zmq::message_t& zmq_msg,
                       const Message_builder& msg_builder);

zmq::message_t pack_message(const Message& msg);

void init_logging(const std::string& file_name);

#endif
