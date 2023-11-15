# set C++ specification
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS NO)

# set extra compiler options
add_compile_options(-Wall -Wextra -Wpedantic)

# use shared libraries for build
set(CMAKE_FIND_LIBRARY_SUFFIXES .so)

# search 0MQ library
find_package(PkgConfig)
pkg_check_modules(PC_ZeroMQ QUIET zmq)
find_path(ZeroMQ_INCLUDE_DIR
    NAMES zmq.hpp
    PATHS ${PC_ZeroMQ_INCLUDE_DIRS}
)
find_library(ZeroMQ_LIBRARY
    NAMES zmq
    PATHS ${PC_ZeroMQ_LIBRARY_DIRS}
)

# find Boost
set(Boost_USE_MULTITHREADED ON)
find_package(Boost 1.74.0 COMPONENTS
    program_options filesystem system log_setup log
)
if(NOT Boost_FOUND)
    message (FATAL_ERROR "Boost is required, but not found")
endif()

# header file to pass CMake settings to source code
configure_file (
    worker_ng_config.h.in
    worker_ng_config.h
    )
install (FILES ${CMAKE_CURRENT_BINARY_DIR}/worker_ng_config.h
         DESTINATION include)
include_directories (${CMAKE_CURRENT_BINARY_DIR})

# define work_parser target
add_subdirectory ("work_parser")

# define work_processor target
add_subdirectory ("work_processor")

add_executable (parser_test parser_test.cpp)
target_link_libraries (parser_test work_parser)
install (TARGETS parser_test DESTINATION bin)

set (worker_ng_COMMON_SRCS utils.cpp message.cpp worker_exception.cpp)
# define message_test target and installation
add_executable (message_test
        message_test.cpp
        message.cpp
        )
install (TARGETS message_test DESTINATION bin)
# define processor_test target and installation
add_executable (processor_test
        processor_test.cpp
        )
target_link_libraries (processor_test LINK_PRIVATE
        work_processor
        pthread
        )
install (TARGETS processor_test DESTINATION bin)
# define worker_server target and installation
add_executable (worker_server
        server.cpp
        "${worker_ng_COMMON_SRCS}"
        )
target_include_directories (worker_server PRIVATE
        "${ZeroMQ_INCLUDE_DIR}"
        "${Boost_INCLUDE_DIR}"
        )
target_link_libraries (worker_server LINK_PRIVATE
        pthread
        "${ZeroMQ_LIBRARY}"
        "${Boost_LIBRARIES}"
        work_parser
        work_processor
        )
install (TARGETS worker_server DESTINATION bin)
# define worker_client target and installation
add_executable (worker_client
        client.cpp
        "${worker_ng_COMMON_SRCS}"
        )
target_include_directories (worker_client PRIVATE
        "${ZeroMQ_INCLUDE_DIR}"
        "${Boost_INCLUDE_DIR}"
        )
target_link_libraries (worker_client LINK_PRIVATE
        "${ZeroMQ_LIBRARY}"
        "${Boost_LIBRARIES}"
        work_processor
        pthread
        )
install (TARGETS worker_client DESTINATION bin)
