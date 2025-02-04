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
            * ack_stop: x
            * stop: s
            * invalid: i, used for initialization and ensuring that
              the message's subject is properly set.
         */
        enum class Subject : char {
            ack = 'a',
            query = 'q',
            result = 'r',
            work = 'w',
            ack_stop = 'x',
            stop = 's',
            invalid = 'i'
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
                boost::uuids::uuid from() const { return from_; };
                boost::uuids::uuid to() const { return to_; };
                Subject subject() const { return subject_; };
                size_t id() const { return id_; };
                std::string content() const { return content_; };
                size_t length() const { return content_.length(); };
                std::string to_string() const;
                friend std::ostream& operator<<(std::ostream& out,
                        const Message& envelope);
            private:
                Message(const boost::uuids::uuid& from,
                        const boost::uuids::uuid& to,
                        const Subject& subject, const size_t id,
                        const std::string& str) :
                    from_ {from}, to_ {to}, subject_ {subject}, id_ {id},
                          content_ {str} {};

                boost::uuids::uuid from_;
                boost::uuids::uuid to_;
                Subject subject_;
                size_t id_;
                std::string content_;
        };

        class message_parse_exception : public Worker_exception {
            public:
                explicit message_parse_exception(const char* msg) :
                    Worker_exception(msg) {};
        };

        class Message_builder {
            public:
                explicit Message_builder(boost::uuids::uuid process_id) :
                    from_ {process_id}, subject_ {Subject::invalid}, id_ {0}, content_ {""} {};
                Message_builder& to(const boost::uuids::uuid& to) {
                    to_ = to;
                    return *this;
                };
                Message_builder& subject(const Subject& subject) {
                    subject_ = subject;
                    return *this;
                };
                Message_builder& id(const size_t& id) {
                    id_ = id;
                    return *this;
                };
                Message_builder& content(const std::string& str) {
                    content_ = str;
                    return *this;
                };
                Message build();
                Message build(const std::string& str) const;
            private:
                boost::uuids::uuid from_;
                boost::uuids::uuid to_;
                Subject subject_;
                size_t id_;
                std::string content_;
        };

    }
}

#endif
