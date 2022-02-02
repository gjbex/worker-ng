from collections import namedtuple


WorkerError = namedtuple('WorkerError', ['msg', 'status'])

batch_file_error = WorkerError(
        msg='batch file issue, {msg}',
        status=2)

data_error = WorkerError(
        msg='error: data source issue, {msg}',
        status=10)

scheduler_option_error = WorkerError(
        msg='error: invalid scheduler argument, {msg}',
        status=11)
