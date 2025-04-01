#!/usr/bin/env -S bash -l

module load worker-ng

# "fix" Python script
cp sum_succeed.py sum.py

# determine last worker directory
prev_dir=$(ls worker_[0-9]*/ | sort | tail -1)

# resume the worker-ng job
wresume --ntasks 2 --dir "$prev_dir"
