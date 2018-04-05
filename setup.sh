#!/usr/bin/env bash

module purge
module load libzmq/4.2.5

export PATH="$(pwd)/distr/bin:${PATH}"
