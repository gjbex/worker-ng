from collections import namedtuple


WorkerError = namedtuple('WorkerError', ['msg', 'status'])

batch_file_error = WorkerError(
        msg='batch file issue, {msg}',
        status=2)

data_error = WorkerError(
        msg='data source issue, {msg}',
        status=10)

scheduler_option_error = WorkerError(
        msg='invalid scheduler argument, {msg}',
        status=11)

submission_error = WorkerError(
        msg='job submission issue, {msg}',
        status=12)


class WorkerException(Exception):
    pass


class JobSubmissionException(WorkerException):
    pass
