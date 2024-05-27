# Environment variables

Simple job script that will write the values of all environment
variables to standard output.


## What is it?

1. `jobscript.slurm`: slurm job script that will writes the values
   of all environment variables.


## How to run?

To submit the job on a system using slurm, simply use the following.
```bash
$ wsub  --batch=jobscript.slurm  --array=1-4  --cluster=genius
```

*Note:* don't forget to adapt `--cluster` and `--account`.
