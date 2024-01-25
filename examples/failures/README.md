# Failures

This job ensures that some work items fail on purpose to allow
you to experiment with the `wresume` command.


## What is it?

1. `sum.py`: Python script that takes two command line arguments, computes the
   sum and writes the result to standard output.
1. `jobscript.slurm`: slurm job script to use with worker.
1. `jobscript.pbs`: PBS job script to use with worker.
1. `data.csv`: data file that contains 1,000 work items.
1. `create_data_file.py`: Python script to produce a data file for this job scriot.


## How to run?

Submit using, e.g.,
```bash
$ wsub  --cluster=genius  --batch=jobscript.slurm  --data=data.csv
```
