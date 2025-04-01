The worker-ng framework has been developed to meet specific use cases: many
small computations determined by parameter variations; the scheduler's task is
easier when it does not have to deal with too many jobs.

Such use cases often have a common root: the user wants to run a program with a
large number of parameter settings, and the program does not allow for
aggregation, i.e., it has to be run once for each instance of the parameter
values.

This how-to shows you how to use the worker-ng framework.
