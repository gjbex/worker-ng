#!/usr/bin/env bash

module purge
module load libzmq/4.2.5
module load boost/1.66.0

export PATH="$(pwd)/distr/bin:${PATH}"
