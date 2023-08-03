# Examples

Examples of workloads to run using worker.


## What is it?

1. `python_sum`: the job runs a Python script for each work item in a CSV file.
   This illustrates the use of `--data`.
1. `wc`: pure Bash job that uses job arrays to operate on text files, for each
   file the word count is computed using the `wc` command, and the output is
    written to files in an output directory, one per input file.  Illustrates
    the use of `--array`.
