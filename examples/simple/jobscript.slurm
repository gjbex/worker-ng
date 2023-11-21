#!/usr/bin/env bash

#SBATCH --account lpt2_sysadmin
#SBATCH --nodes=1
#SBATCH --ntasks=10
#SBATCH --cpus-per-task=1
#SBATCH --time=00:30:00
#SBATCH --mem=3gb

echo "hello to $dest from $source"
