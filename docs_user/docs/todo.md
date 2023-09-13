# To do

This is the initial release, so some work is still to be done.

  * Field testing;
  * Add documentation and examples for multithreaded and MPI work items;
  * Port `areduce` (part of atools) to worker and add documentation.

In the long run a number of features are possible.

  * Client-only jobs: clients can join a running server;
  * Remote server setup: a worker server can run on any system;
  * Dynamic workload: a server uses a database rather than a
    file to get its workloads, allowing for dynamic workloads.
