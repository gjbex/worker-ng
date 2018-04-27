#include <boost/uuid/uuid_generators.hpp>
#include <iostream>
#include <sstream>

#include "message.h"

namespace wm = worker::message;

using Uuid = boost::uuids::uuid;

int main() {
    auto uuid_generator = boost::uuids::random_generator();
    Uuid from = uuid_generator();
    Uuid to = uuid_generator();
    size_t id {17};
    auto subject = wm::Subject::work;
    std::string content {"echo 'hello world!'"};
    wm::Message_builder builder(from);
    builder.to(to).subject(subject).id(id).content(content);
    auto message = builder.build();
    std::cout << message << std::endl;
    std::stringstream sstr;
    sstr << message;
    try {
        message = builder.build(sstr.str());
    } catch (wm::message_parse_exception e) {
        std::cout << e.what() << std::endl;
    }
    std::cout << message << std::endl;
    return 0;
}
