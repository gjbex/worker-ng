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
        << worker_ng_VERSION_MINOR << "."
        << worker_ng_VERSION_PATCH;
    int zmq_major, zmq_minor, zmq_patch;
    std::tie(zmq_major, zmq_minor, zmq_patch) = zmq::version();
    std::cout << ", using 0MQ "
        << zmq_major << "."
        << zmq_minor << "."
        << zmq_patch << std::endl;
}

int bind_server_socket(zmq::socket_t& socket, const int init_port_nr) {
    const std::string protocol {"tcp"};
    int port_nr {init_port_nr};
    try {
        const std::string bind_str {protocol + "://*:" +
            std::to_string(options.port_nr)};
        socket.bind(bind_str);
        BOOST_LOG_TRIVIAL(info) << "socket boundi on " << bind_str;
        return port_nr;
    } catch (zmq::error_t& err) {
        BOOST_LOG_TRIVIAL(error) << "socket binding failed on port " << port_nr << ", " << err.what();
        port_nr = 1024;
    }
    while (true) {
        try {
            const std::string bind_str {protocol + "://*:" +
                std::to_string(port_nr)};
            socket.bind(bind_str);
            BOOST_LOG_TRIVIAL(info) << "socket bound on " << bind_str;
            return port_nr;
        } catch (zmq::error_t& err) {
            BOOST_LOG_TRIVIAL(error) << "socket binding failed on port " << port_nr << ", " << err.what();
            port_nr++;
            if (port_nr > 65535) {
                break;
            }
        }
    }
    BOOST_LOG_TRIVIAL(error) << "socket binding failed on all available ports";
    std::cerr << "### error: socket can not bind to any port" << std::endl;
    worker::exit(worker::Error::socket);
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
    logging::add_file_log(
            keywords::file_name = file_name,
            keywords::auto_flush = true,
            keywords::format = "%TimeStamp% [%Severity%]: %Message%"
    );
    logging::core::get()->set_filter(
            logging::trivial::severity >= logging::trivial::info
    );
    logging::add_common_attributes();
}
