#!/usr/bin/env bash

source ~/.conda_init.sh
conda activate worker_ng

export PYTHONPATH=$(pwd)/scripts
pytest
