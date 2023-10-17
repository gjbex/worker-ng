# install correct configuration file depending on scheduler
set(config_file "worker.conf")
if(SCHEDULER_NAME STREQUAL "pbs")
    install(FILES pbs_config.conf DESTINATION conf RENAME ${config_file})
elseif(SCHEDULER_NAME STREQUAL "slurm")
    install(FILES slurm_config.conf DESTINATION conf RENAME ${config_file})
else()
    message(FATAL_ERROR "Invalid SCHEDULER_NAME specified")
endif()

# create worker environment file
get_filename_component(boost_lib_dir ${Boost_SYSTEM_LIBRARY_RELEASE} DIRECTORY)
get_filename_component(cppzmq_lib_dir ${ZeroMQ_LIBRARY} DIRECTORY)
set(env_file_text "# adjust LD_LIBRARY_PATH for runtime libraries\nexport LD_LIBRARY_PATH=\"${boost_lib_dir}:${cppzmq_lib_dir}:$LD_LIBRARY_PATH\"")
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/worker_env.sh "${env_file_text}")

# install the environment file
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/worker_env.sh DESTINATION conf)

# find Python 3.x
find_package(Python3 3.9.5 EXACT COMPONENTS Interpreter REQUIRED)

# create worker Python environment file
get_filename_component(python_bin_dir ${Python3_EXECUTABLE} DIRECTORY)
set(python_env_file_text "# Python executable\nPYTHON_EXE=\"${Python3_EXECUTABLE}\"\n# Python shared object directory\nLD_LIBRARY_PATH=\"${python_bin_dir}/../lib/\"")
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/worker_python_env.sh "${python_env_file_text}")

# install the Python environment file
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/worker_python_env.sh DESTINATION conf)
