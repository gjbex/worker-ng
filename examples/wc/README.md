# Word count

Pure Bash job that will perform a word count on files in a directory.
This job can be used to experiment with the `--array` option since
the file names are derived from the array ID.


## What is it?

1. `jobscript.slurm`: job script that counts the number of words in files
   in a directory, and writes the results to individual files in an
   output directory.
1. `data`: directory that contains 100 files to do the word count on.
1. `create_data_files.py`: Python script to generate input data for
   this job.


## How to run?

This job should be submitted using the `--array` option.
```bash
$ wsub  --batch=jobscript.slurm  --array=1-100
```