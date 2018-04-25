#ifndef PROCESSOR_RESULT_HDR
#define PROCESSOR_RESULT_HDR

#include <iostream>

#include "../worker_exception.h"

class Result {
    public:
        Result(const int exit_status, const std::string& stdout,
               const std::string& stderr) :
            _exit_status {exit_status}, _stdout {stdout},
            _stderr {stderr} {};
        Result(const std::string& str);
        int exit_status() const { return _exit_status; };
        std::string stdout() const { return _stdout; };
        std::string stderr() const { return _stderr; };
        std::string to_string() const;
        friend std::ostream& operator<<(std::ostream& out,
                                        const Result& result);
    private:
        int _exit_status;
        std::string _stdout;
        std::string _stderr;
};

class result_parse_exception : public Worker_exception {
    public:
        result_parse_exception(const char* msg) : Worker_exception(msg) {};
};

#endif
