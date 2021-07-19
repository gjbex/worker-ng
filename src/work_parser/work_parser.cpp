#include <sstream>

#include "work_parser.h"

namespace worker {
    namespace work_parser {

        const std::string Work_parser::DEFAULT_SEP {"#----"};

        void Work_parser::parse_next() {
            std::stringstream item {""};
            std::string line;
            while (std::getline(ifs_, line) && line != sep_) {
                item << line << "\n";
            }
            next_item_ = item.str();
        }

        std::string Work_parser::next() {
            std::string result {next_item_};
            if (result.length() > 0)
                ++nr_items_;
            parse_next();
            return result;
        }

    }
}
