# Python script

This is an example of a job script that runs a Python script.


## What is it?

1. `sum.py`: Python script that takes two command line arguments, computes the
   sum and writes the result to standard output.
1. `sum.slurm`: slurm job script to use with worker.
1. `data.csv`: data file that contains 1,000 work items.
1. `create_data_file.py`: Python script to produce a data file for this job scriot.


## How to run?

Submit using, e.g.,
```bash
$ wsub  --cluster=genius  --batch=sum.slurm  -data=data.csv
```
