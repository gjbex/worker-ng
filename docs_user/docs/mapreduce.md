# Map-reduce scenarios

Often, an embarrassingly parallel computation can be abstracted to three simple
steps:

  * a preparation phase in which the data is split up into smaller, more
    manageable chuncks;
  * on these chuncks, the same algorithm is applied independently (these are
    the work items); and
  * the results of the computations on those chuncks are aggregated into, e.g.,
    a statistical description of some sort.

The Worker-ng framework does not directly supports this scenario since it is
easily implemented using job dependencies which are support by most schedulers,
including Slurm.
