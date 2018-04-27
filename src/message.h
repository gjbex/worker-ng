#ifndef ENVELOPE_HDR
#define ENVELOPE_HDR

#include <boost/uuid/uuid.hpp>
#include <iostream>

#include "worker_exception.h"

namespace worker {
    namespace message {

        class Message_builder;

        /*!
          \brief enumeration of the message types.

          Each message type is encoded by a single character, i.e.,
            * ack: a
            * query: q
            * result: r
            * work: w
            * stop: s
         */
        enum class Subject : char {
            ack = 'a',
            query = 'q',
            result = 'r',
            work = 'w',
            stop = 's'
        };

        /*!
          \brief Class to represent messages exchanged between servers
                 and clients.

          Messages have an origin, a destination, a subject, and content.
          To simplify decoding the messages, a message also includes the
          number of characters of the content.
         */
        class Message {
            friend class Message_builder;
            public:
                boost::uuids::uuid from() const { return _from; };
                boost::uuids::uuid to() const { return _to; };
                Subject subject() const { return _subject; };
                size_t id() const { return _id; };
                std::string content() const { return _content; };
                size_t length() const { return _content.length(); };
                std::string to_string() const;
                friend std::ostream& operator<<(std::ostream& out,
                        const Message& envelope);
            private:
                Message(const boost::uuids::uuid& from,
                        const boost::uuids::uuid& to,
                        const Subject& subject, const size_t id,
                        const std::string& str) :
                    _from {from}, _to {to}, _subject {subject}, _id {id},
                          _content {str} {};
                boost::uuids::uuid _from;
                boost::uuids::uuid _to;
                Subject _subject;
                size_t _id;
                std::string _content;
        };

        class message_parse_exception : public Worker_exception {
            public:
                message_parse_exception(const char* msg) :
                    Worker_exception(msg) {};
        };

        class Message_builder {
            public:
                Message_builder(boost::uuids::uuid process_id) :
                    _from {process_id}, _id {0}, _content {""} {};
                Message_builder& to(const boost::uuids::uuid& to) {
                    _to = to;
                    return *this;
                };
                Message_builder& subject(const Subject& subject) {
                    _subject = subject;
                    return *this;
                };
                Message_builder& id(const size_t& id) {
                    _id = id;
                    return *this;
                };
                Message_builder& content(const std::string& str) {
                    _content = str;
                    return *this;
                };
                Message build();
                Message build(const std::string& str) const;
            private:
                boost::uuids::uuid _from;
                boost::uuids::uuid _to;
                Subject _subject;
                size_t _id;
                std::string _content;
        };

    }
}

#endif
