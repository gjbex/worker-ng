#include <fstream>
#include <iostream>

#include "work_parser/work_parser.h"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "### error: no work file given" << std::endl;
        return 1;
    }
    Work_parser parser(std::make_shared<std::ifstream>(std::string(argv[1])));
    while (parser.has_next()) {
        std::cout << "# item " << parser.nr_items() << std::endl;
        std::cout << parser.next() << std::endl;
    }
    return 0;
}
