cmake_minimum_required (VERSION 3.0)
project (worker_ng LANGUAGES CXX)

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

# search libraries
find_package (PkgConfig)
pkg_check_modules (PC_ZeroMQ QUIET zmq)
find_path (ZeroMQ_INCLUDE_DIR
        NAMES zmq.hpp
        PATHS ${PC_ZeroMQ_INCLUDE_DIRS}
        )

find_library (ZeroMQ_LIBRARY
        NAMES zmq
        PATHS ${PC_ZeroMQ_LIBRARY_DIRS}
        )

# define targets
add_library (work_parser "${PROJECT_SOURCE_DIR}/src/work_parser.cpp")
add_executable (parser_test "${PROJECT_SOURCE_DIR}/src/parser_test.cpp")
target_link_libraries (parser_test work_parser)

add_executable (worker_server "${PROJECT_SOURCE_DIR}/src/server.cpp")
target_include_directories (worker_server PUBLIC "${ZeroMQ_INCLUDE_DIR}")
target_link_libraries (worker_server ${ZeroMQ_LIBRARY})

add_executable (worker_client "${PROJECT_SOURCE_DIR}/src/client.cpp")
target_include_directories (worker_client PUBLIC "${ZeroMQ_INCLUDE_DIR}")
target_link_libraries (worker_client ${ZeroMQ_LIBRARY})

# define installation locations
install (TARGETS parser_test DESTINATION bin)
install (TARGETS work_parser DESTINATION lib)
install (FILES "${PROJECT_SOURCE_DIR}/src/worker_ng_config.h"
         DESTINATION include)
install (FILES "${PROJECT_SOURCE_DIR}/src/work_parser.h"
         DESTINATION include)
install (TARGETS worker_server DESTINATION bin)
install (TARGETS worker_client DESTINATION bin)

# add packer settings
include (InstallRequiredSystemLibraries)
set (CPACK_RESOURCE_FILE_LICENSE
     "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set (CPACK_PACKAGE_VERSION_MAJOR "${worker_ng_VERSION_MAJOR}")
set (CPACK_PACKAGE_VERSION_MINOR "${worker_ng_VERSION_MINOR}")
include (CPack)
