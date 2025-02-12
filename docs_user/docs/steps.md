# worker-ng step-by-step

As prerequisites, one should have a (sequential) job that has to be
run many times

  * for various parameter values, i.e., parameter exploration; or
  * on a large number of input files.


## Parameter exploration

By way of running example, we will use a Python script `sum.py` that simply
computes the sum of two numbers given on the command line:

```python
#!/usr/bin/env python

import argparse
import sys

def main():
    arg_parser = argparse.ArgumentParser(description='sum two values')
    arg_parser.add_argument('-a', type=float, required=True,
                            help='A value')
    arg_parser.add_argument('-b', type=float, required=True,
                            help='B value')
    options = arg_parser.parse_args()
    print(options.a + options.b)
    return 0

if __name__ == '__main__':
    sys.exit(main())
```

On the command line, we would run this as follows:
```bash
python sum.py  -a=1.3  -b=2.5
```
The program will write its results to standard output.

A slurm script (say `sum.slurm`) that would run this as a job would
then look like:
```bash
#!/usr/bin/env -S bash -l
#SBATCH --account=my_account
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=1
#SBATCH --time=00:02:00

python sum.py  -a 1.3  -b 2.5
```

When we submit this job, the calculation is performed for this particular
instance of the parameters, i.e., `a = 1.3` and `b = 2.5`.

To submit the job, the user would use:
```bash
$ sbatch sum.slurm
```

However, the user wants to run this program for many parameter instances,
e.g., he wants to run the program on 100 instances of `a` and `b`.
To this end, the PBS file can be modified as follows.

```bash
#!/usr/bin/env -S bash -l
#SBATCH --account=my_account
#SBATCH --nodes=1
#SBATCH --ntasks=10
#SBATCH --cpus-per-task=1
#SBATCH --time=00:20:00

python sum.py  -a=$a  -b=$b
```

Note that
  * the parameter values 1.3 and 2.5 are replaced by variables `$a` and `$b`
    respectively;
  * the number of tasks, i.e., cores per node has been increased to 10, i.e.,
    `--ntasks=1` is replaced by `--ntasks=10`; and
  * the walltime has been increased to 20 mintues, i.e., `--time=00:02:00`
    is replaced by `--time=00:20:00`.

The walltime is calculated as follows: one calculation takes 2 minutes,
so 100 calculations take 200 minutes on one core.  However, this job will
use 10 cores, so the 100 calculations will be done in 200/10 = 20 minutes.

Note that one core will always be required for management purposes, so if
we specify `--ntasks=10` and `--cpus-per-task=1`, the job will required
11 cores from the scheduler.  This is done automatically by the worker-ng
framework though.

The 100 parameter instances can be stored in a Comma Separated Value file (CSV)
that can be generated using a spreadsheet program such as Microsoft Excel, or
just by hand using any text editor (do not use a word processor such as
Microsoft Word though). The first few lines of the file `data.csv` would look
as follows.

```
a,b
1.3,2.5
1.05,4.3
1.05,4.3
1.15,4.3
1.25,4.3
...
```

It has to contain the names of the variables on the first line, i.e., `a` and
`b` in this example, followed by 100 parameter instances in the current
example, each on a line by itself. Values on a line are separated by commas.

The job can now be submitted as follows.

```bash
$ module load worker-ng
$ wsub --batch=sum.slurm  --data=data.txt
```

Note that the Slurm file is the value of the `--batch` option . The `sum.py`
scripts program will now be run for all 100 parameter instances--—10
concurrently--—until all computations are done. A computation for such a
parameter instance is called a work item in worker-ng parlance.


## Job arrays

Most schedulers including Slurm support job arrays. However, often they are
configured so that users can only have a fairly small number of jobs in the
queue.  This implies that the size of job arrays is subject to that same limit.

Although this makes sense from the point of view of system administrators
who don't want to have their schedulers overloaded, it is sometimes less than
convenient for users who want to have job arrays with many jobs.

worker offers a way around this by "packing" the job array into a single job,
hence allowing for large job arrays that do not impact the scheduler's
performance in any way.

As an example, we will consider the following use case.  We have a directory
`data` that contains 100 text files, named `text-001.txt` to `text-100.txt`.
We want to count the words in each of these files, and want to do the work
in parallel.

On the command line, we would to this as follows for a single file.

```bash
$ echo -n "text-001.txt: " && cat data/text-001.txt | wc -w
```

A typical slurm job script `wc.slurm` for use with job arrays would look
as follows.

```bash
#!/usr/bin/env -S bash -l
#SBATCH --account=lpt2_sysadmin
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=1
#SBATCH --time=00:02:00

# create an output directory if it doesn't exist
mkdir -p output

# define input and output file names based on the array ID
INPUT_FILE="data/file_$(printf '%04d' $SLURM_ARRAY_TASK_ID).txt"
OUTPUT_FILE="output/count_$(printf '%04d' $SLURM_ARRAY_TASK_ID).txt"

# count the words and write the result in the output file
echo -n "$INPUT_FILE: " > $OUTPUT_FILE
cat $INPUT_FILE | wc -w >> $OUTPUT_FILE
```

We can submit this as a standard slurm job array as follows.

```bash
$ sbatch --array=1-100 wc.slurm
```

However, this would result in 100 arrays jobs, which may exceed the limitations
set by the configuration of the scheduler.

Using worker, a feature akin to job arrays can be used with minimal modifications
to the job script.

```bash
#!/usr/bin/env -S bash -l
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
```

Note that
  * the number of tasks, i.e., cores per node has been increased to 10, i.e.,
    `--ntasks=1` is replaced by `--ntasks=10`; and
  * the walltime has been increased to 20 mintues, i.e., `--time=00:02:00`
    is replaced by `--time=00:20:00`.

The walltime is calculated as follows: one calculation takes 2 minutes,
so 100 calculations take 200 minutes on one core.  However, this job will
use 10 cores, so the 100 calculations will be done in 200/10 = 20 minutes.

The job is now submitted as follows.

```bash
$ module load worker-ng
$ wsub  --array=1-100  --batch=wc.slurm
```

The `wc` program  will now be run for all 100 input files--—10
concurrently--—until all computations are done. Again, a
computation for an individual input file, or, equivalently, an
array ID, is called a work item in worker-ng speak.

Note that in constrast to Slurm job arrays, a worker job array submits
a single job only.
