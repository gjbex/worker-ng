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
install (FILES "${PROJECT_SOURCE_DIR}/src/worker_ng_config.h"
         DESTINATION include)

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
set (Boost_USE_STATIC_LIBS ON)
set (Boost_USE_MULTIHREADED OFF)
find_package (Boost 1.66.0 COMPONENTS program_options system)

# define work_parser target
add_library (work_parser "${PROJECT_SOURCE_DIR}/src/work_parser.cpp")
add_executable (parser_test "${PROJECT_SOURCE_DIR}/src/parser_test.cpp")
target_link_libraries (parser_test work_parser)
install (TARGETS parser_test DESTINATION bin)
install (TARGETS work_parser DESTINATION lib)
install (FILES "${PROJECT_SOURCE_DIR}/src/work_parser.h"
         DESTINATION include)

if (Boost_FOUND)
# define worker_server target and installation
    add_executable (worker_server "${PROJECT_SOURCE_DIR}/src/server.cpp")
    target_include_directories (worker_server PRIVATE
            "${ZeroMQ_INCLUDE_DIR}"
            "${Boost_INCLUDE_DIR}"
            )
    target_link_libraries (worker_server LINK_PRIVATE
            ${ZeroMQ_LIBRARY}
            ${Boost_LIBRARIES}
            )
    install (TARGETS worker_server DESTINATION bin)

# define worker_client target and installation
    add_executable (worker_client "${PROJECT_SOURCE_DIR}/src/client.cpp")
    target_include_directories (worker_client PRIVATE
            "${ZeroMQ_INCLUDE_DIR}"
            "${Boost_INCLUDE_DIR}"
            )
    target_link_libraries (worker_client LINK_PRIVATE
            ${ZeroMQ_LIBRARY}
            ${Boost_LIBRARIES}
            )
    install (TARGETS worker_client DESTINATION bin)
endif()

# add packer settings
include (InstallRequiredSystemLibraries)
set (CPACK_RESOURCE_FILE_LICENSE
     "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set (CPACK_PACKAGE_VERSION_MAJOR "${worker_ng_VERSION_MAJOR}")
set (CPACK_PACKAGE_VERSION_MINOR "${worker_ng_VERSION_MINOR}")
include (CPack)
