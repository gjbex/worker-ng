#!/bin/bash -l

#SBATCH --account lpt2_sysadmin
#SBATCH --nodes=1
#SBATCH --ntasks=2
#SBATCH --cpus-per-task=1
#SBATCH --time=00:30:00
#SBATCH --mem-per-cpu=1gb

echo "------------------ ${WORKER_ITEM_ID} env start --------------------------"
env
echo "------------------ ${WORKER_ITEM_ID} env end   --------------------------"
