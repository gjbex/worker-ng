# find Python 3.x
find_package(Python3 3.9.5 EXACT COMPONENTS Interpreter Development REQUIRED)

# there are two types of Python scripts:
#   * scripts that will interact with the scheduler;
#   * scripts that don't interact with the scheduler
# for te former, it is very important to avoid poluting the
# runtime envirnment, while this is of little concern
# for the second category

set(PANDAS_MODULE "system" CACHE STRING "module that provides Python with pandas installed")

# check whether a module has been specified to load so that a Python version
# with pandas is available
if("${PANDAS_MODULE}" STREQUAL "system")
    message(WARN "no module for a Python version that has pandas installed specified, relying on current Python version")
endif()

# list of Python scripts that interact with the scheduler
set(python_submit_scripts "${CMAKE_CURRENT_SOURCE_DIR}/wsub.py" "${CMAKE_CURRENT_SOURCE_DIR}/wresume.py")

# list of Python scripts that don't interact with the scheduler
set(python_query_scripts "${CMAKE_CURRENT_SOURCE_DIR}/wsummarize.py")

# list of all Python scripts
file(GLOB python_scripts "${CMAKE_CURRENT_SOURCE_DIR}/*.py")

# set the Bash wrapper template file
set(scheduler_cmd_tmpl "${CMAKE_CURRENT_SOURCE_DIR}/../tmpl/run_scheduler_cmd.tmpl")
set(query_cmd_tmpl "${CMAKE_CURRENT_SOURCE_DIR}/../tmpl/run_query_cmd.tmpl")

# create a wrapper file for each Python script and add install targts
foreach(python_script ${python_scripts})
    if("${python_script}" IN_LIST python_submit_scripts)
        set(cmd_tmpl "${scheduler_cmd_tmpl}")
    elseif("${python_script}" IN_LIST python_query_scripts)
        set(cmd_tmpl "${query_cmd_tmpl}")
    else()
        message(WARNING "skipping ${python_script}")
        continue()
    endif()
    get_filename_component(bash_script_name ${python_script} NAME_WE)
    get_filename_component(python_script_name ${python_script} NAME)
    install(FILES ${python_script_name} TYPE LIB)
    set(wrapper_file_name "${CMAKE_CURRENT_BINARY_DIR}/${bash_script_name}")
    configure_file(${cmd_tmpl} ${wrapper_file_name} @ONLY)
    set(target "wrapper_script_${bash_script_name}")
    add_custom_target(${target} ALL DEPENDS ${wrapper_file_name})
    install(PROGRAMS ${wrapper_file_name} DESTINATION bin)
endforeach()

# install worker Python library
install(DIRECTORY worker TYPE LIB
    PATTERN "*~" EXCLUDE
    PATTERN "*.bak" EXCLUDE
    PATTERN "__pycache__" EXCLUDE)

# install Bash function library file
install(FILES worker_functions.sh DESTINATION scripts)
