# Resume

Systematic example of a partially failing worker-ng job that can be resumed by
"fixing" the Python script that is used to perform the computations.


## What is it?

1. `sum_partial_fail.py`: Python script that computes the sum of two floating
   point numbers.  It also takes the job array ID as an argument, and will
   on purpose fail for all odd array IDs.
1. `sum_succeed.py`:Python script that will also compute the sume of two
   floating point numbers.  It will ignore the third command line argument,
   i.e., the job array ID and always succeed.
1. `data.txt`: CSV file that contains the values of the two operands for
   the sum.
1. `jobscript.slurm`: Slurm job script to perform the computation.
1. `submit.sh`: Bash script that submits the job, using `sum_partial_fail.py`
   for the computation.
1. `resume.sh`: Bash script that resumes the job, using `sum_succeed.py`
   for the computation.
