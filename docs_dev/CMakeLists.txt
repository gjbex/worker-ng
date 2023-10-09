# search for doxygen
find_package (Doxygen)

# issue warming if the documentation cannot be built, but don't fail
if(NOT DOXYGEN_FOUND)
    message ("Doxygen needs to be installed to generate the documentation")
else()
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
    set(DOXYGEN_FILE ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
    configure_file(${DOXYGEN_IN} ${DOXYGEN_FILE} @ONLY)
    add_custom_target (docs_doxygen ALL
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_FILE}
        COMMENT "Generating API documentation using doxygen"
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/)
    install (DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/docs_doxygen/
         DESTINATION "docs/development/")
endif()
