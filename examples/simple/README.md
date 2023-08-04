# Simple example

Trivial example of job scripts for slurm and PBS torque that
echo values of variables to standard output.


## What is it?

1. `jobscript.slurm`: slurm job script to use with worker.
1. `jobscript.pbs`: PBS torque job script to use with worker.
1. `data.csv`: data file for the job.
1. `create_data_file.py`: Python script to create a data file.


## How to run?

To submit the job on a system using slurm, simply use the following.
```bash
$ wsub  --batch=jobscript.slurm  --data=data.csv  --cluster=genius
```

*Note:* don't forget to adapt `--cluster` and `--account`.

To submit the job on a system using PBS torque, simply use the following.
```bash
$ wsub  --batch=jobscript.pbs  --data=data.csv
```

*Note:* don't forget to adapt `-A`.
