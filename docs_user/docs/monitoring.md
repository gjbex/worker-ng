# Monitoring worker jobs

You will have noticed that when you have submitted a job using `wsub`, a directory is created with a name that starts with `worker_`, and ends in the job ID.  Among other things, this directory contains files that allow you to monitor the progress of a running worker job, or analyze its performance once it is done.

Since a worker job will typically run for several hours, it may be reassuring to monitor its progress. worker server keeps a log of its activity in the directory mentioned abovewhere the job was submitted.  You can use the `wsummarize` command to get information, e.g., for job ID was `1234`.

```bash
$ wsummarize  --dir=worker_1234/
```

This will give you an overview of the status of your work items, i.e., the number of
  * succesful items: the number of computations that finished with exit status 0;
  * failed items: the computations that finished with a non-zero exit status;
  * incomplete items: the number of items that are currnetly being executed.

To monitor progress "in real time", you can use the `watch` Linux command.
```bash
$ watch  -n 60  wsummarize  --dir=worker_1234/
```
This will summarize the status of the work items every 60 seconds.  *Note:* use a reasonable value for the update period, this will cause load on the login node where you run this ocmmand.

The `wsummarize` command has various command line options to get a more detailed analysis of perfornmance issues.  For instance, to get statistics on the walltime of your work items, you can use the `--show_walltime_stats` flag.  This will give you descriptive statistics on the walltime of your work items such as the minimum and maximum, the average and median, as well as informaiton on the spread.

In order to detect problems with load balancing between the worker clients, you can use the `--show_client_stats` flag.  This will provide you with the same descriptive statistics on the walltime, but grouped by client.  In addition, you will get the total walltime for each client, a good measure for load balance.

Finally, the `--show_all` options will given the output of `--show_walltime_stats` and `--show_client_stats` in a single `wsammarize` invocation.
