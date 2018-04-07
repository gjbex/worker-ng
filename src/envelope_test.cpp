#include <boost/uuid/uuid_generators.hpp>
#include <iostream>
#include <sstream>

#include "envelope.h"

int main() {
    auto uuid_generator = boost::uuids::random_generator();
    Uuid from = uuid_generator();
    Uuid to = uuid_generator();
    size_t id {17};
    Subject subject = Subject::work;
    std::string msg {"echo 'hello world!'"};
    Envelope_builder builder(from);
    builder.to(to).subject(subject).id(id).message(msg);
    auto envelope = builder.build();
    std::cout << envelope << std::endl;
    std::stringstream sstr;
    sstr << envelope;
    try {
        envelope = builder.build(sstr.str());
    } catch (char const* e) {
        std::cout << e << std::endl;
    }
    std::cout << envelope << std::endl;
    return 0;
}
