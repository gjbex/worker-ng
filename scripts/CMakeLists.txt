# there are two types of Python scripts:
#   * scripts that will interact with the scheduler;
#   * scripts that don't interact with the scheduler
# for te former, it is very important to avoid poluting the
# runtime envirnment, while this is of little concern
# for the second category

# find Python 3.x that will be used for scheduler interaction scripts
find_package(Python3  COMPONENTS Interpreter Development REQUIRED)

# create executables for Python scripts using PyInstaller
add_custom_command(
    OUTPUT dist/wsummarize dist/wsub dist/wresume
    COMMAND python -m venv ${CMAKE_CURRENT_BINARY_DIR}/build_venv
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/build_venv/bin/pip install
                -r ${CMAKE_CURRENT_SOURCE_DIR}/requirements.txt
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/build_venv/bin/pyinstaller --onefile
                ${CMAKE_CURRENT_SOURCE_DIR}/wsummarize.py
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/build_venv/bin/pyinstaller --onefile
                --hidden-import worker.slurm.option_parser
                --hidden-import worker.slurm.submit_output_parser
                --collect-submodules worker.slurm.option_parser
                --collect-submodules worker.slurm.submit_output_parser
                --hidden-import worker.pbs_torque.option_parser
                --hidden-import worker.pbs_torque.submit_output_parser
                --collect-submodules worker.pbs_torque.option_parser
                --collect-submodules worker.pbs_torque.submit_output_parser
                ${CMAKE_CURRENT_SOURCE_DIR}/wsub.py
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/build_venv/bin/pyinstaller --onefile
                --hidden-import worker.slurm.option_parser
                --hidden-import worker.slurm.submit_output_parser
                --collect-submodules worker.slurm.option_parser
                --collect-submodules worker.slurm.submit_output_parser
                --hidden-import worker.pbs_torque.option_parser
                --hidden-import worker.pbs_torque.submit_output_parser
                --collect-submodules worker.pbs_torque.option_parser
                --collect-submodules worker.pbs_torque.submit_output_parser
                ${CMAKE_CURRENT_SOURCE_DIR}/wresume.py
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/wsummarize.py
            ${CMAKE_CURRENT_SOURCE_DIR}/wsub.py
            ${CMAKE_CURRENT_SOURCE_DIR}/wresume.py
    VERBATIM)

# create a custom target to build the executables
add_custom_target(create_executables ALL
    DEPENDS dist/wsummarize dist/wsub dist/wresume)

# install the executables
install(
    PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/dist/wsummarize ${CMAKE_CURRENT_BINARY_DIR}/dist/wsub ${CMAKE_CURRENT_BINARY_DIR}/dist/wresume
    DESTINATION bin)

# install Bash function library file
install(FILES worker_functions.sh DESTINATION scripts)
