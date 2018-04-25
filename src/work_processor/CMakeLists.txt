if (Boost_FOUND)
    add_library (work_processor processor.cpp result.cpp)
    target_compile_options (work_processor PRIVATE
            "-Wno-unused-result" "-Wno-unused-parameter"
    )
    target_include_directories (work_processor PRIVATE
            "${Boost_INCLUDE_DIR}"
    )
    target_link_libraries (work_processor LINK_PRIVATE
            "${Boost_LIBRARIES}"
    )
    install (TARGETS work_processor DESTINATION lib)
    install (FILES processor.h DESTINATION include/processor)
endif()
