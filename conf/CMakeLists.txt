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

# set CPPZMQ_MODULE and BOOST_MODULE as cache variables
set(CPPZMQ_MODULE "system" CACHE STRING "name of the cppzmq module to use")
set(BOOST_MODULE "system" CACHE STRING "name of the Boost module to use")

# create worker environment file
if(${CPPZMQ_MODULE} STREQUAL "system" AND ${BOOST_MODULE} STREQUAL "system")
    set(env_file_text "# relying on system versions of cppzmq and Boost libraries")
else()
    set(env_file_text "# load the cppzmq and Boost modules\nmodule load ${CPPZMQ_MODULE} ${BOOST_MODULE}")
endif()
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/worker_env.sh "${env_file_text}")

# install the environment file
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/worker_env.sh DESTINATION conf)
