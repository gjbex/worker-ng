cmake_minimum_required (VERSION 3.11 FATAL_ERROR)
project (worker_ng LANGUAGES CXX)

# version information
set (worker_ng_NAME "worker_ng")
set (worker_ng_VERSION_MAJOR 0)
set (worker_ng_VERSION_MINOR 1)

# set compiler optons
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -Wextra")

# search 0MQ library
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

# find Boost
set (Boost_USE_MULTIHREADED ON)
find_package (Boost 1.66.0 COMPONENTS
         program_options filesystem system log
         )

# define work_parser target
add_subdirectory (src)

# add packer settings
include (InstallRequiredSystemLibraries)
set (CPACK_RESOURCE_FILE_LICENSE
     "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set (CPACK_PACKAGE_VERSION_MAJOR "${worker_ng_VERSION_MAJOR}")
set (CPACK_PACKAGE_VERSION_MINOR "${worker_ng_VERSION_MINOR}")
include (CPack)
