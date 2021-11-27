#include <iostream>

#define BOOST_LOG_DYN_LINK 1
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/severity_feature.hpp>
#include <boost/log/sources/record_ostream.hpp>

#include "worker_ng_config.h"
#include "utils.h"

namespace wm = worker::message;

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
}

wm::Message unpack_message(const zmq::message_t& zmq_msg,
                           const wm::Message_builder& msg_builder) {
    std::string msg_str;
    msg_str.resize(zmq_msg.size());
    memcpy(&msg_str[0], zmq_msg.data(), zmq_msg.size());
    return msg_builder.build(std::string(msg_str));
}

zmq::message_t pack_message(const wm::Message& msg) {
    std::string msg_str = msg.to_string();
    size_t msg_length = msg_str.length();
    zmq::message_t zmq_msg(msg_length);
    memcpy(zmq_msg.data(), &msg_str[0], msg_length);
    return zmq_msg;
}

void init_logging(const std::string& file_name) {
    namespace logging = boost::log;
    namespace keywords = boost::log::keywords;
    using namespace logging::trivial;
    logging::register_simple_formatter_factory<severity_level,char>("Severity");
    logging::add_file_log(
            keywords::file_name = file_name,
            keywords::format = "%TimeStamp% [%Severity%]: %Message%"
    );
    logging::core::get()->set_filter(
            logging::trivial::severity >= logging::trivial::info
    );
    logging::add_common_attributes();
}
