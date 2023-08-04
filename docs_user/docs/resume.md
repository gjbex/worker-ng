Unfortunately, it is not always easy to estimate the walltime for a job, and consequently, sometimes the latter is underestimated. When using the worker framework, this implies that not all work items will have been processed. worker makes it very easy to resume such a job without having to figure out which work items did complete successfully, and which remain to be computed. Suppose the job that did not complete all its work items had ID '1234'.

```bash
$ wresume  --dir=worker_1234
```

This will submit a new job that will start to work on the work items that were not done yet. Note that it is possible to change almost all job parameters when resuming, specifically the requested resources such as the number of cores and the walltime.

```bash
$ wresume  --time=1:30:00  --dir=worker_1234
```

Work items may fail to complete successfully for a variety of reasons, e.g., a data file that is missing, a (minor) programming error, etc. Upon resuming a job, the work items that failed are considered to be done, so resuming a job will only execute work items that did not terminate when the job ended, either because they were being executed at that point, or had notstarted yet.

It is very easy to get a list of the work items that fail using `wsummarize`.

```bash
$ wsummarize  --dir=worker_1234  --show_failed
```

Using the work item IDs you can inspect the data to try to find the cause of the failure.

Once you have identified and solved the problem, It is you can redo work items that failed easily.

```bash
$ wresume  --dir=worker_1234  --redo
```
