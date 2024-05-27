# Single data column

This is an example of a job script that runs a Python script with
a data file that contains a single column.


## What is it?

1. `square.py`: Python script that takes a single command line argument, computes the
   square and writes the result to standard output.
1. `jobscript.slurm`: slurm job script to use with worker.
1. `data.csv`: data file that contains 1,000 work items.
1. `create_data_file.py`: Python script to produce a data file for this job scriot.


## How to run?

Submit using, e.g.,
```bash
$ wsub  --cluster=genius  --batch=jobscript.slurm  --data=data.csv
```
