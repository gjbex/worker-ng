from collections import namedtuple


WorkerError = namedtuple('WorkerError', ['msg', 'status'])

config_error = WorkerError(
        msg='worker configuration issue, {msg}',
        status=1)

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

log_file_error = WorkerError(
        msg='log file issue, {msg}',
        status=13)

worker_dir_error = WorkerError(
        msg='worker directory issue, {msg}',
        status=14)

class WorkerException(Exception):
    pass


class JobSubmissionException(WorkerException):
    pass


class LogParseException(WorkerException):
    pass


class WorkerDirException(WorkerException):
    pass
