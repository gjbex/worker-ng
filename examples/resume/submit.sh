#!/usr/bin/env -S bash -l

module load worker-ng

# use "buggy" version of sum.py
cp sum_partial_fail.py sum.py

# submit the worker-ng job
wsub --ntasks 4 --data data.txt --batch jobscript.slurm
