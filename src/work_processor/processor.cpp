#include "processor.h"

Process_result process_work(const std::string work_item) {
    int exit_code {0};
    std::string output_str;
    std::string error_str;
    bp::ipstream ips;
    bp::ipstream eps;
    bp::opstream ops;
    bp::child process(bp::search_path("bash"), "",
                      bp::std_out > ips, bp::std_err > eps, bp::std_in < ops);
    ops << work_item;
    std::string line;
    while (process.running() && std::getline(ips, line))
        output_str += line + "\n";
    while (process.running() && std::getline(eps, line))
        error_str += line + "\n";
    process.wait();
    exit_code = process.exit_code();
    return std::make_tuple(exit_code, output_str, error_str);
}
