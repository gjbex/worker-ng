# Troubleshooting

The most common problem with the worker-ng framework is that it doesn't
seem to work at all, showing messages in the error file about module
failing to work.  The cause is trivial, and easy to remedy.

Like any Slurm script, a worker Slurm file has to be in UNIX format!

If you edited a Slurm script on your desktop, or something went wrong during
sftp/scp, the PBS file may end up in DOS/Windows format, i.e., it has the wrong
line endings.  The PBS/torque queue system can not deal with that, so you will
have to convert the file, e.g., for file `run.slurm`:

```bash
$ dos2unix run.slurm
```
