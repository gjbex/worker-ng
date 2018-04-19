#include <iostream>
#include <sstream>

#include "work_processor/processor.h"

int main() {
    std::stringstream work_item;
    std::string line;
    while (std::getline(std::cin, line))
        work_item << line << std::endl;
    std::cout << "work:" << std::endl;
    std::cout << work_item.str() << std::endl;
    Result result = process_work(work_item.str());
    std::cout << "result:" << std::endl;
    std::cout << "\texit code: " << result.exit_status() << std::endl;
    std::cout << "##### stdout #####" << std::endl;
    std::cout << result.stdout() << std::endl;
    std::cout << "##### stder #####" << std::endl;
    std::cout << result.stderr() << std::endl;
    std::cout << "#####" << std::endl;
    return 0;
}
