# Time limits

Sometimes, the execution of a work item takes long than expected, or worse,
some work items get stuck in an infinite loop. This situation is unfortunate,
since it implies that work items that could successfully are not even started.
Again, a simple and yet versatile solution is offered by the Worker framework.
If we want to limit the execution of each work item to at most 20 minutes, this
can be accomplished by modifying the script of the running example.

```
#!/usr/bin/env -S bash -l
#SBATCH --account=my_account
#SBATCH --ntasks=8
#SBATCH time=04:00:00

module load timedrun

timedrun -t 00:20:00 cfd_test -t $temperature  -p $pressure  -v $volume
```

Note that it is trivial to set individual time constraints for work items by
introducing a parameter, and including the values of the latter in the CSV
file, along with those for the temperature, pressure and volume.

Also note that 'timedrun' is in fact offered in a module of its own, so it can
be used outside the worker-ng framework as well.
