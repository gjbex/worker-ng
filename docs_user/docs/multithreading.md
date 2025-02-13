# Multithreaded work items

If a work item uses threading, the number of threads for each work item
can simply be specified by using the `--cpus-per-task` option, similar
to a regular Slurm job.

If you need to know the number of threads in your Slurm job script,
you can use the `SLURM_CPUS_PER_TASK_HET_GROUP_0` environment variable.
This variable has to be used, rather than `SLURM_CPUS_PER_TASK`, because
under the hood, worker-ng uses a heterogeneous job.
