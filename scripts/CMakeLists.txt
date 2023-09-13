# get a list of all Python scripts
file(GLOB python_scripts "${CMAKE_CURRENT_SOURCE_DIR}/*.py")

# set the Bash wrapper template file
set(template_bash_script "${CMAKE_CURRENT_SOURCE_DIR}/../tmpl/run.tmpl")

# create a wrapper file for each Python script and add install targts
foreach(python_script ${python_scripts})
    get_filename_component(bash_script_name ${python_script} NAME_WE)
    get_filename_component(python_script_name ${python_script} NAME)
    install(FILES ${python_script_name} TYPE LIB)
    set(wrapper_file_name "${CMAKE_CURRENT_BINARY_DIR}/${bash_script_name}")
    configure_file(${template_bash_script} ${wrapper_file_name} @ONLY)
    set(target "wrapper_script_${bash_script_name}")
    add_custom_target(${target} ALL DEPENDS ${wrapper_file_name})
    install(PROGRAMS ${wrapper_file_name} DESTINATION bin)
endforeach()

# install Python library
install(DIRECTORY worker TYPE LIB
    PATTERN "*~" EXCLUDE
    PATTERN "*.bak" EXCLUDE
    PATTERN "__pycache__" EXCLUDE)

# create and install the runtime Python virtual environment
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

# install package from the requirements file
execute_process(COMMAND ${CMAKE_CURRENT_BINARY_DIR}/venv/bin/pip install -r ${CMAKE_CURRENT_SOURCE_DIR}/requirements_runtime.txt)

# add installation target that copies the virtual environment
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/venv DESTINATION lib)


# install Bash function library file
install(FILES worker_functions.sh DESTINATION scripts)
