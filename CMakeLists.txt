cmake_minimum_required (VERSION 3.20 FATAL_ERROR)
project(worker_ng LANGUAGES CXX)

# version information
set(worker_ng_NAME "worker_ng")
set(worker_ng_VERSION_MAJOR 1)
set(worker_ng_VERSION_MINOR 0)
set(worker_ng_VERSION_PATCH 11)

# define source target
add_subdirectory(src)

# define script target
add_subdirectory(scripts)

# define configuration target
add_subdirectory(conf)

# define development documentation targets
add_subdirectory(docs_dev)

# add packer settings
include (InstallRequiredSystemLibraries)
set (CPACK_RESOURCE_FILE_LICENSE
     "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set (CPACK_PACKAGE_VERSION_MAJOR "${worker_ng_VERSION_MAJOR}")
set (CPACK_PACKAGE_VERSION_MINOR "${worker_ng_VERSION_MINOR}")
include (CPack)
