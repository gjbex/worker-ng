# Find the mkdocs executable
find_program (MKDOCS_EXECUTABLE mkdocs)

# Add a custom target to generate and install the documentation
add_custom_target (docs_user
    COMMAND ${MKDOCS_EXECUTABLE} build
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_BINARY_DIR}/site ${CMAKE_INSTALL_PREFIX}/docs
    COMMENT "Generating and installing documentation"
    )

# Add the docs target as a dependency of the install target
install(TARGETS docs_user
   DESTINATION ${CMAKE_INSTALL_PREFIX}/docs/user/
)
