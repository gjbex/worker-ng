#ifndef UTILS_HDR
#define UTILS_HDR

#include <exception>

void print_version_info();

class Worker_exception : public std::exception {
    public:
        Worker_exception(const char* msg) : _msg {msg} {};
        virtual char const* what() const throw() { return _msg; };
    private:
        const char* _msg;
};

#endif
