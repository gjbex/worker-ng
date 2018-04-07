#include <boost/uuid/uuid_io.hpp>
#include <sstream>

#include "envelope.h"

std::ostream& operator<<(std::ostream& out, const Envelope& envelope) {
    out << envelope._from << " "
        << envelope._to << " "
        << static_cast<char>(envelope._subject) << " "
        << envelope._id << " "
        << envelope.length() << " "
        << envelope._msg;
    return out;
}

Envelope Envelope_builder::build(const std::string& str) const {
    std::stringstream input(str);
    Uuid from;
    if (!(input >> from))
        throw "can't read from UUID";
    Uuid to;
    if (!(input >> to))
        throw "can't read to UUID";
    char buffer;
    if (!(input >> buffer))
        throw "can't read subject";
    Subject subject = static_cast<Subject>(buffer);
    size_t id;
    if (!(input >> id))
        throw "can't read id";
    size_t length;
    if (!(input >> length))
        throw "can't read length";
    input.read(&buffer, 1);
    std::string msg;
    msg.resize(length);
    input.read(&msg[0], length);
    return Envelope(from, to, subject, id, msg);
}
