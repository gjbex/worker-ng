#ifndef UTILS_HDR
#define UTILS_HDR

#include <exception>
#include <zmq.hpp>

#include "message.h"

void print_version_info();

class Worker_exception : public std::exception {
    public:
        Worker_exception(const char* msg) : _msg {msg} {};
        virtual char const* what() const throw() { return _msg; };
    private:
        const char* _msg;
};

Message unpack_message(const zmq::message_t& zmq_msg);

zmq::message_t pack_message(const Message& msg);

#endif
