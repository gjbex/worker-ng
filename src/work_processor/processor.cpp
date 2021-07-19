#include <boost/process.hpp>
#include <iostream>

#include "processor.h"

namespace worker {
    namespace work_processor {

        namespace bp = boost::process;

        Result process_work(const std::string work_item) {
            int exit_code {0};
            std::string output_str;
            std::string error_str;
            bp::ipstream ips;
            bp::ipstream eps;
            bp::opstream ops;
            bp::child process(bp::search_path("bash"),
                    bp::std_out > ips, bp::std_err > eps,
                    bp::std_in < ops);
            ops << work_item << std::endl;
            process.wait();
            std::string line;
            while (std::getline(ips, line)) {
                output_str += line + "\n";
            }
            while (std::getline(eps, line)) {
                error_str += line + "\n";
            }
            exit_code = process.exit_code();
            process.terminate();
            return Result(exit_code, output_str, error_str);
        }

    }
}
