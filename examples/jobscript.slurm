#!/usr/bin/env bash
#SBATCH --accout lp_worker_test
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=1
#SBATCH --time=3:00:00
#SBATCH --mem=3gb

echo "hello to $dest from $source"
