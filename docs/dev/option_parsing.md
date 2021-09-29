# Scheduler options

In general, there are two ways to pass options to a scehduler when submitting
a job, as directives in the job script, and/or as command line arguments when
submitting a job.

Options passed via the command line will override corresponding directives in
the job script.

The scheduler's options can be divided into three categories:
1. options that are irrelevant to worker and should simply be passed to the
   scheduler, e.g., mail notification, I/O redirection, ...
1. options that are relevant to worker, and should be passed to the
   scheduler, e.g., scheduler directive prefix, job name, ...
1. options that are relevant to worker but should *not* be passed to the
   scheduler, e.g., array job specifications.

worker has two commands for which scheduler options are relevant:
1. initial job submission (wsub) and
1. resuming a job (wresume).

On initial job submission, all scheduler options that should be passed to
the scheduler can be passed unaltered.

On resuming a job, the situation is a bit more complicated.  The options
to be passed to the scheduler can now be specified in
1. the original job script,
1. the command line of the submission command,
1. the command line of the resume command.

The options specified as directives can be passed without issue.  The options
specified at resume command line should override those specified on submission.

The latter is no problem, except for command line options that can be specified
multiple itmes to accumulate settings, e.g., PBS torque, use a single option to
specify diverse resources.  The `-l` flag is used to specify
* the number of nodes and processes per node
* the walltime
* memory requirements
* the parition
* the QOS

## Relevant options

These options should not be handled by the option parser for the scehduler,
but by that of worker.  The scheduler parser should expose the flags for
these options.

### Job name

The job name is used to determine the name for the worker directory that is used
to store the actual job script, the log, etc.

### Scheduler directive prefix

The scheduler directive prefix is required to parse the job script properly.

### Array request

The array IDs will be used by worker as a data source, and not be passed to
the scheduler.  The array request should *not* be specified in the job script.
Although directives could be filtered by worker, it would increase the
complexity, quite likely needlessly.

## Option parsers

### Scheduler

The option parser for the scheduler should deal with all irrelevant scheduler
options and provide help.

The parser exposes the flags for the relevant options, as well as the
default scheduler directive prefix.

### worker

Worker requires several option parsers.

1. a parser for common worker options,
1. a parser for worker's own options for submission,
1. a parser for worker's own option for resume,
1. a parser for relevant scheduler options,
1. a parser for filtered scheduler options.
