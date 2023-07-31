#!/usr/bin/env bash

module purge
module unuse /apps/leuven/skylake/2018a/modules/all
module use /apps/leuven/skylake/2021a/modules/all
module load CMake/3.20.1-GCCcore-10.3.0
module load Boost/1.76.0-GCC-10.3.0
module load Doxygen/1.9.1-GCCcore-10.3.0
module load cppzmq/4.8.1-GCCcore-10.3.0

BUILD_DIR="$(pwd)/build_genius"
DISTR_DIR="$(pwd)/distr_genius"
BUILD_TYPE="RelWithDebInfo"
SCHEDULER="wice"

if [[ $1 == 'clean' ]]
then
    rm -rf ${BUILD_DIR}
    rm -rf ${DISTR_DIR}
fi

mkdir -p ${BUILD_DIR}

cmake -B ${BUILD_DIR} -S . \
    -DCMAKE_INSTALL_PREFIX=${DISTR_DIR} \
    -DBUILD_SHARED_LIBS=ON \
    -DBoost_USE_STATIC_LIBS=OFF \
    -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
    -DSCHEDULER_NAME=${SCHEDULER}


if [ $? -eq 0 ]
then
    cmake --build ${BUILD_DIR}
fi

if [ $? -eq 0 ]
then
    cmake --install ${BUILD_DIR}
fi
