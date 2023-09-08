# search for doxygen
find_package (Doxygen)

# issue warming if the documentation cannot be built, but don't fail
if (NOT DOXYGEN_FOUND)
    message ("Doxygen needs to be installed to generate the documentation")
else()
    set (DOXYGEN_IN Doxyfile.in)
    set (DOXYGEN_OUT Doxyfile)
    configure_file (${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
    add_custom_target (docs_doxygen ALL
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        COMMENT "Generating API documentation using doxygen"
        )
    install (DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/docs_doxygen/"
         DESTINATION "docs/development/"
         )
endif ()
