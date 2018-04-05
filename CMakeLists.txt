cmake_minimum_required (VERSION 3.0)
project (worker_ng)

# version information
set (worker_ng_VERSION_MAJOR 0)
set (worker_ng_VERSION_MINOR 1)

# header file to pass CMake settings to source code
configure_file (
    "${PROJECT_SOURCE_DIR}/src/worker_ng_config.h.in"
    "${PROJECT_SOURCE_DIR}/src/worker_ng_config.h"
    )

# set compiler optons
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -g -Wall -Wextra")
set (CMAKE_C_FLAGS "-O4")

# define targets
add_library (WorkParser "${PROJECT_SOURCE_DIR}/src/work_parser.cpp")
add_executable (ParserTest "${PROJECT_SOURCE_DIR}/src/parser_test.cpp")
target_link_libraries (ParserTest WorkParser)

# define installation locations
install (TARGETS ParserTest DESTINATION bin)
install (FILES "${PROJECT_SOURCE_DIR}/src/worker_ng_config.h"
         DESTINATION include)
install (FILES "${PROJECT_SOURCE_DIR}/src/work_parser.h"
         DESTINATION include)

# add packer settings
include (InstallRequiredSystemLibraries)
set (CPACK_RESOURCE_FILE_LICENSE
     "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set (CPACK_PACKAGE_VERSION_MAJOR "${worker_ng_VERSION_MAJOR}")
set (CPACK_PACKAGE_VERSION_MINOR "${worker_ng_VERSION_MINOR}")
include (CPack)
