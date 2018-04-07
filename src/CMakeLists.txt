# header file to pass CMake settings to source code
configure_file (
    worker_ng_config.h.in
    worker_ng_config.h
    )
install (FILES worker_ng_config.h
         DESTINATION include)

# define work_parser target
add_subdirectory ("work_parser")

add_executable (parser_test parser_test.cpp)
target_link_libraries (parser_test work_parser)
install (TARGETS parser_test DESTINATION bin)

if (Boost_FOUND)
    set (worker_ng_COMMON_SRCS utils.cpp envelope.cpp)
# define envelope_test target and installation
    add_executable (envelope_test
            envelope_test.cpp
            envelope.cpp
            )
    target_include_directories (envelope_test PRIVATE
            "${Boost_INCLUDE_DIR}"
            )
    install (TARGETS envelope_test DESTINATION bin)
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
            "${ZeroMQ_LIBRARY}"
            "${Boost_LIBRARIES}"
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
            )
    install (TARGETS worker_client DESTINATION bin)
endif()
