#include <boost/uuid/uuid_io.hpp>
#include <sstream>

#include "message.h"

namespace worker {
    namespace message {

        std::string Message::to_string() const {
            std::stringstream str;
            str << *this;
            return str.str();
        }

        std::ostream& operator<<(std::ostream& out,
                                 const Message& message) {
            out << message.from_ << " "
                << message.to_ << " "
                << static_cast<char>(message.subject_) << " "
                << message.id_ << " "
                << message.length() << " "
                << message.content_;
            return out;
        }

        Message Message_builder::build() {
            Message msg(from_, to_, subject_, id_, content_);
            id_ = 0;
            content_ = std::string("");
            return msg;
        }

        using Uuid = boost::uuids::uuid;

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

    }
}
