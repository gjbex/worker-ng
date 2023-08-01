#!/usr/bin/env bash
#SBATCH --account=lpt2_sysadmin
#SBATCH --nodes=1
#SBATCH --ntasks=10
#SBATCH --cpus-per-task=1
#SBATCH --time=00:20:00

# create an output directory if it doesn't exist
mkdir -p output

# define input and output file names based on the array ID
INPUT_FILE="data/file_$(printf '%04d' $SLURM_ARRAY_TASK_ID).txt"
OUTPUT_FILE="output/count_$(printf '%04d' $SLURM_ARRAY_TASK_ID).txt"

# count the words and write the result in the output file
echo -n "$INPUT_FILE: " > $OUTPUT_FILE
cat $INPUT_FILE | wc -w >> $OUTPUT_FILE
