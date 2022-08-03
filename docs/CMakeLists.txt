if (DOXYGEN_FOUND)
    set (DOXYGEN_IN Doxyfile.in)
    set (DOXYGEN_OUT Doxyfile)
    configure_file (${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
    add_custom_target (docs_doxygen ALL
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        COMMENT "Generating API documentation using doxygen"
        )
    install (DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/docs_doxygen"
         DESTINATION docs
         )
else (DOXYGEN_FOUND)
    message ("Doxygen needs to be installed to generate the documentation")
endif (DOXYGEN_FOUND)
