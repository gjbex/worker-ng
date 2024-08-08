# worker-ng

Next-generation worker implementation of worker framework.

Whereas the original worker framework was based on the
Message Passing Interface (MPI) this version uses ZeroMQ
for communication.  This has several advantages:
    * better robustness: if a client crashes, that has no
      impact on eiter the server or other clients;
    * MPI work items are now trivial to handle;
    * server and clients are decoupled, allowing for more
      flexible scenarios.

This version of worker is also easier to adapt to various
schedulers.  It has currently support for both slurm and PBS
torque, but can be extended to other schedulers if necessary.

Although worker-ng shares many features with the original
worker framework, it is not entirely a drop-in replacement.
The user-facing scripts of the original framework where
written in Perl, whereas this version is based on Python.
This has some implications on how command line options are
handled.


## What is it?

For users:

1. `examples`: a directory containing several examples of how to
   use the worker framework.
1. `docs_user`: user documentation on how to use the worker
   framework.
1. `LICENSE`: the license file for users and developers.

For developers:

1. `conf`: worker configuration files, required at runtime.
1. `docs_dev`: developer documentation.
1. `scripts`: Python scripts to submit worker jobs, monitor
   progress, analyze performance and resume jobs.  The `worker`
   subdirectory contains the Python library for the framework,
   as well as the templates for the job scripts.
1. `src`: source code for the C++ server and client and the
   supporting library.
1. `test`: tests for the Python library.
1. `tmpl`: templates to generate the Bash wrappers for the
   Python scripts.
1. `CMakeLists.txt`: top-level CMake build script.
1. `environment.yml`: conda environment file to build and run
   the worker framework.
1  `development_worker.recipe`: Singularity/Apptainer recipe to
   build an image that contains all tools and libraries required
   for development.
1. Various convenience scripts
    * `run_test.sh`: script to run the tests.
    * `build_genius.sh`: script to build worker on the KU Leuven genius
      cluster.
    * `setup_genius.sh`: script to be sourced to set up your environment
      to run worker on the genius cluster.
    * `build_wice.sh`: script to build worker on the KU Leuven wice
      cluster.
    * `setup_wice.sh`: script to be sourced to set up your environment
      to run worker on the wice cluster.


## Installation

Installion is done using CMake:

```bash
$ cmake -S . -B build/ -DSCHEDULER_NAME=slurm -DCMAKE_INSTALL_PREFIX=<where-you-want>
$ cmake --build build/
$ cmake --install build/
```

Requirements:
1. Boost 1.76.0
1. cppzmq 4.8.1
1. CMake >= 3.20
1. doxygen >= 1.9
1. Python >= 3.9
  a. numpy
  a. pandas


## Contribors

1. Geert Jan Bex ([geertjan.bex@uhasselt.be](mailto:geertjan.bex@uhasselt.be)):
   main developer.
1. Mag Selwa: testing and feedback.
