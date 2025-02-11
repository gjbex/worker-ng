# there are two types of Python scripts:
#   * scripts that will interact with the scheduler;
#   * scripts that don't interact with the scheduler
# for te former, it is very important to avoid poluting the
# runtime envirnment, while this is of little concern
# for the second category

# find Python 3.x that will be used for scheduler interaction scripts
find_package(Python3  COMPONENTS Interpreter Development REQUIRED)

# define a virtual environment and install dependencies
add_custom_target(setup_venv
    COMMAND python -m venv ${CMAKE_CURRENT_BINARY_DIR}/build_venv
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/build_venv/bin/pip install -r ${CMAKE_CURRENT_SOURCE_DIR}/requirements.txt
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Setting up virtual environment and installing dependencies"
    VERBATIM
)

# Define custom commands for each PyInstaller executable
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/dist/wsummarize
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/build_venv/bin/pyinstaller --onefile
                --distpath ${CMAKE_CURRENT_BINARY_DIR}/dist
                ${CMAKE_CURRENT_SOURCE_DIR}/wsummarize.py
    DEPENDS setup_venv ${CMAKE_CURRENT_SOURCE_DIR}/wsummarize.py
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Building wsummarize executable"
    VERBATIM
)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/dist/wsub
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/build_venv/bin/pyinstaller --onefile
                --distpath ${CMAKE_CURRENT_BINARY_DIR}/dist
                --hidden-import worker.slurm.option_parser
                --hidden-import worker.slurm.submit_output_parser
                --collect-submodules worker.slurm.option_parser
                --collect-submodules worker.slurm.submit_output_parser
                --hidden-import worker.pbs_torque.option_parser
                --hidden-import worker.pbs_torque.submit_output_parser
                --collect-submodules worker.pbs_torque.option_parser
                --collect-submodules worker.pbs_torque.submit_output_parser
                ${CMAKE_CURRENT_SOURCE_DIR}/wsub.py
    DEPENDS setup_venv ${CMAKE_CURRENT_SOURCE_DIR}/wsub.py
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Building wsub executable"
    VERBATIM
)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/dist/wresume
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/build_venv/bin/pyinstaller --onefile
                --distpath ${CMAKE_CURRENT_BINARY_DIR}/dist
                --hidden-import worker.slurm.option_parser
                --hidden-import worker.slurm.submit_output_parser
                --collect-submodules worker.slurm.option_parser
                --collect-submodules worker.slurm.submit_output_parser
                --hidden-import worker.pbs_torque.option_parser
                --hidden-import worker.pbs_torque.submit_output_parser
                --collect-submodules worker.pbs_torque.option_parser
                --collect-submodules worker.pbs_torque.submit_output_parser
                ${CMAKE_CURRENT_SOURCE_DIR}/wresume.py
    DEPENDS setup_venv ${CMAKE_CURRENT_SOURCE_DIR}/wresume.py
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Building wresume executable"
    VERBATIM
)

# Create explicit custom targets for each executable
add_custom_target(wsummarize ALL
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/dist/wsummarize
)

add_custom_target(wsub ALL
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/dist/wsub
)

add_custom_target(wresume ALL
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/dist/wresume
)

# Create a meta target to build all executables
add_custom_target(create_executables ALL
    DEPENDS wsummarize wsub wresume
)

# install the executables
install(
    PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/dist/wsummarize ${CMAKE_CURRENT_BINARY_DIR}/dist/wsub ${CMAKE_CURRENT_BINARY_DIR}/dist/wresume
    DESTINATION bin)

# install Bash function library file
install(FILES worker_functions.sh DESTINATION scripts)
