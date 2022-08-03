#include <fstream>
#include <iostream>

#include "work_parser/work_parser.h"

namespace wp = worker::work_parser;

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "### error: no work file given" << std::endl;
        return 1;
    }
    std::string file_name {argv[1]};
    std::ifstream ifs(file_name);
    if (!ifs) {
        std::cerr << "can not open file" << std::endl;
        return 1;
    }
    wp::Work_parser parser(ifs);
    while (parser.has_next()) {
        std::cout << "# item " << parser.nr_items() << std::endl;
        std::cout << parser.next() << std::endl;
    }
    return 0;
}
