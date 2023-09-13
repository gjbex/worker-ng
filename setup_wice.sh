#!/usr/bin/env bash

module purge
module unuse /apps/leuven/skylake/2018a/modules/all
module use /apps/leuven/icelake/2021a/modules/all
module load cppzmq/4.8.1-GCCcore-10.3.0
module load Boost/1.76.0-GCC-10.3.0

export PATH="$(pwd)/distr_wice/bin:${PATH}"
export LD_LIBRARY_PATH="$(pwd)/distr_wice/lib:${LD_LIBRARY_PATH}"

source "${HOME}/.conda_init.sh"
conda activate worker_ng
export PYTHONPATH="$(pwd)/scripts:${PYTHONPATH}"
