# create a Python virtual environment with mkdocs installed
# find Python 3.x
find_package(Python3 COMPONENTS Interpreter Development REQUIRED)

# create virtual environment
execute_process(COMMAND ${Python3_EXECUTABLE} -m venv ${CMAKE_CURRENT_BINARY_DIR}/venv)

# set virtual environment as active
set(ENV{VIRTUAL_ENV} ${CMAKE_CURRENT_BINARY_DIR}/venv)

# find Python 3.x again, this time in the virtual environment
unset(Python3_EXECUTABLE)
find_package(Python3 COMPONENTS Interpreter Development REQUIRED)

# upgrade pip, setuptools and wheel
execute_process(COMMAND ${CMAKE_CURRENT_BINARY_DIR}/venv/bin/pip install --upgrade pip)
execute_process(COMMAND ${CMAKE_CURRENT_BINARY_DIR}/venv/bin/pip install --upgrade setuptools)
execute_process(COMMAND ${CMAKE_CURRENT_BINARY_DIR}/venv/bin/pip install --upgrade wheel)

# install package from requirements.txt
execute_process(COMMAND ${CMAKE_CURRENT_BINARY_DIR}/venv/bin/pip install -r ${CMAKE_CURRENT_SOURCE_DIR}/requirements_build.txt)

# Find the mkdocs executable
find_program (MKDOCS_EXECUTABLE mkdocs)

if(NOT MKDOCS_EXECUTABLE)
    message(WARNING "mkdocs not found! Please install mkdocs to generate user documentation.")
else()
    # Set the path to your mkdocs.yml and output directory
    set(MKDOCS_CONFIG_FILE "${CMAKE_CURRENT_SOURCE_DIR}/mkdocs.yml")
    set(MKDOCS_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/docs/user")

    # Add a custom command that runs mkdocs
    add_custom_command(
        OUTPUT "${MKDOCS_OUTPUT}"
        COMMAND ${MKDOCS_EXECUTABLE} build -f ${MKDOCS_CONFIG_FILE} -d "${MKDOCS_OUTPUT}"
        COMMENT "Generating documentation with MkDocs"
        VERBATIM)

    # Add a custom target that depends on the above command
    add_custom_target(mkdocs ALL DEPENDS "${MKDOCS_OUTPUT}")
    install(DIRECTORY "${MKDOCS_OUTPUT}/" DESTINATION "${CMAKE_INSTALL_PREFIX}/docs/user")
endif()
