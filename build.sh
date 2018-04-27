#!/usr/bin/env bash

module purge
module load cmake/3.11.0
module load boost/1.66.0

ZMQ_LIB="/usr/local/software/libzmq/4.2.5/lib/libzmq.so"
CPPZMQ_INCLUDE="/usr/local/software/4.2.3/include"

BUILD_DIR="$(pwd)/build"
DISTR_DIR="$(pwd)/distr"
COMPILER="g++"
BUILD_TYPE="RelWithDebInfo"

if [[ $1 == 'clean' ]]
then
    rm -rf ${BUILD_DIR}
    rm -rf ${DISTR_DIR}
fi

mkdir -p ${BUILD_DIR}
cd ${BUILD_DIR}

cmake \
    -DCMAKE_CXX_COMPILER=${COMPILER} \
    -DCMAKE_INSTALL_PREFIX=${DISTR_DIR} \
    -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
    -DZeroMQ_INCLUDE_DIR=${CPPZMQ_INCLUDE} \
    -DZeroMQ_LIBRARY=${ZMQ_LIB}  \
    ..

if [ $? -eq 0 ]
then
    make
fi

if [ $? -eq 0 ]
then
    make install
fi
