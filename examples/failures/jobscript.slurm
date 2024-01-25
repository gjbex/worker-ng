#!/usr/bin/env bash

#SBATCH --account=lpt2_sysadmin
#SBATCH --nodes=1
#SBATCH --ntasks=10
#SBATCH --cpus-per-task=1
#SBATCH --time=00:20:00

if [ $fails == 1 ]
then
    exit 1
else
    python sum.py  -a=$a  -b=$b
fi
