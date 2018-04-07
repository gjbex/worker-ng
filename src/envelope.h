#ifndef ENVELOPE_HDR
#define ENVELOPE_HDR

#include <boost/uuid/uuid.hpp>
#include <iostream>

using Uuid = boost::uuids::uuid;

class Envelope_builder;

enum class Subject : char {
    query = 'q',
    result = 'r',
    work = 'w',
    stop = 's'
};

class Envelope {
    friend class Envelope_builder;
    public:
        Uuid from() const { return _from; };
        Uuid to() const { return _to; };
        Subject subject() const { return _subject; };
        size_t id() const { return _id; };
        std::string msg() const { return _msg; };
        size_t length() const { return _msg.length(); };
        friend std::ostream& operator<<(std::ostream& out,
                                        const Envelope& envelope);
    private:
        Envelope(const Uuid& from, const Uuid& to,
                 const Subject& subject, const size_t id,
                 const std::string& msg) :
            _from {from}, _to {to}, _subject {subject}, _id {id},
            _msg {msg} {};
        Uuid _from;
        Uuid _to;
        Subject _subject;
        size_t _id;
        std::string _msg;
};

class Envelope_builder {
    public:
        Envelope_builder(Uuid process_id) : _from {process_id} {};
        Envelope_builder& to(const Uuid& to) {
            _to = to;
            return *this;
        };
        Envelope_builder& subject(const Subject& subject) {
            _subject = subject;
            return *this;
        };
        Envelope_builder& id(const size_t& id) {
            _id = id;
            return *this;
        };
        Envelope_builder& message(const std::string& msg) {
            _msg = msg;
            return *this;
        };
        Envelope build() const {
            return Envelope(_from, _to, _subject, _id, _msg);
        };
        Envelope build(const std::string& str) const;
    private:
        Uuid _from;
        Uuid _to;
        Subject _subject;
        size_t _id;
        std::string _msg;
};

#endif
