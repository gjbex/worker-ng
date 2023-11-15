# set supported schedulers
set(schedulers "'slurm' or 'torque'")

# set SCHEDULER_NAME as cache variable
set(SCHEDULER_NAME "slurm" CACHE STRING "name of the scheduler to use, can be either ${schedulers}")

# install correct configuration file depending on scheduler
set(config_file "worker.conf")
if(SCHEDULER_NAME STREQUAL "pbs")
    install(FILES pbs_config.conf DESTINATION conf RENAME ${config_file})
elseif(SCHEDULER_NAME STREQUAL "slurm")
    install(FILES slurm_config.conf DESTINATION conf RENAME ${config_file})
else()
    message(FATAL_ERROR "Invalid SCHEDULER_NAME specified, should be ${schedulers}")
endif()

# create worker environment file
get_filename_component(boost_lib_dir ${Boost_SYSTEM_LIBRARY_RELEASE} DIRECTORY)
get_filename_component(cppzmq_lib_dir ${ZeroMQ_LIBRARY} DIRECTORY)
set(env_file_text "# adjust LD_LIBRARY_PATH for runtime libraries\nexport LD_LIBRARY_PATH=\"${boost_lib_dir}:${cppzmq_lib_dir}:$LD_LIBRARY_PATH\"")
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/worker_env.sh "${env_file_text}")

# install the environment file
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/worker_env.sh DESTINATION conf)
