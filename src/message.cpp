#include <boost/uuid/uuid_io.hpp>
#include <sstream>

#include "message.h"

std::ostream& operator<<(std::ostream& out, const Message& message) {
    out << message._from << " "
        << message._to << " "
        << static_cast<char>(message._subject) << " "
        << message._id << " "
        << message.length() << " "
        << message._content;
    return out;
}

Message Message_builder::build() {
    Message msg(_from, _to, _subject, _id, _content);
    _id = 0;
    _content = std::string("");
    return msg;
}

std::string Message_builder::build_str() {
    std::stringstream str;
    str << build();
    return str.str();
}

Message Message_builder::build(const std::string& str) const {
    std::stringstream input(str);
    Uuid from;
    if (!(input >> from))
        throw message_parse_exception("can't read origin UUID");
    Uuid to;
    if (!(input >> to))
        throw message_parse_exception("can't read destination UUID");
    char buffer;
    if (!(input >> buffer))
        throw message_parse_exception("can't read subject");
    Subject subject = static_cast<Subject>(buffer);
    size_t id;
    if (!(input >> id))
        throw message_parse_exception("can't read id");
    size_t length;
    if (!(input >> length))
        throw message_parse_exception("can't read length");
    std::string content {""};
    if (length > 0) {
        input.read(&buffer, 1);
        content.resize(length);
        input.read(&content[0], length);
    }
    return Message(from, to, subject, id, content);
}
