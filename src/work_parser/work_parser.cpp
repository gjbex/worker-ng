#include <sstream>

#include "work_parser.h"

const std::string Work_parser::DEFAULT_SEP {"#----"};

void Work_parser::parse_next() {
    std::stringstream item {""};
    std::string line;
    while (std::getline(*_ifs, line)) {
        if (line == _sep)
            break;
        item << line << std::endl;
    }
    _next_item = item.str();
    if (_next_item.length() > 0)
        ++_nr_items;
}

std::string Work_parser::next() {
    std::string result {_next_item};
    parse_next();
    return result;
}
