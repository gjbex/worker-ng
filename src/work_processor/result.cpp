#include <sstream>

#include "result.h"

Result::Result(const std::string& str) {
    std::stringstream sstr(str);
    if (!(sstr >> _exit_status))
        throw result_parse_exception("can't read exit status");
    size_t stdout_size;
    if (!(sstr >> stdout_size))
        throw result_parse_exception("can't read stdout size");
    size_t stderr_size;
    if (!(sstr >> stderr_size))
        throw result_parse_exception("can't read stderr size");
    _stdout.resize(stdout_size);
    _stderr.resize(stderr_size);
    char buffer;
    sstr.read(&buffer, 1);
    sstr.read(&_stdout[0], stdout_size);
    sstr.read(&buffer, 1);
    sstr.read(&_stderr[0], stderr_size);
}
