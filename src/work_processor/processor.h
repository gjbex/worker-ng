#ifndef PROCEESSOR_HDR
#define PROCEESSOR_HDR

#include <boost/process.hpp>
#include <tuple>

namespace bp = boost::process;

using Process_result = std::tuple<int,std::string,std::string>;

Process_result process_work(const std::string work_item);

#endif
