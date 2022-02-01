cmake_minimum_required (VERSION 3.11 FATAL_ERROR)
project (worker_ng LANGUAGES CXX)

# version information
set (worker_ng_NAME "worker_ng")
set (worker_ng_VERSION_MAJOR 0)
set (worker_ng_VERSION_MINOR 1)

# set C++ specification
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS NO)

# set extra compiler options
add_compile_options(-Wall -Wextra -Wpedantic)

# search and configure doxygen
find_package (Doxygen)
    
# use shared libraries for build
set(CMAKE_FIND_LIBRARY_SUFFIXES .so)

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
find_package (Boost 1.66.0 COMPONENTS
         program_options filesystem system log_setup log
         )
if (NOT Boost_FOUND)
    message (FATAL_ERROR "Boost is required, but not found")
endif (NOT Boost_FOUND)

# define source target
add_subdirectory (src)

# define documentation target
add_subdirectory (docs)

# add packer settings
include (InstallRequiredSystemLibraries)
set (CPACK_RESOURCE_FILE_LICENSE
     "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set (CPACK_PACKAGE_VERSION_MAJOR "${worker_ng_VERSION_MAJOR}")
set (CPACK_PACKAGE_VERSION_MINOR "${worker_ng_VERSION_MINOR}")
include (CPack)
