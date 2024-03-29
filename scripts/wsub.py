#!/usr/bin/env python

import argparse
import sys
import worker.errors
import worker.option_parser
from worker.submit_output_parser import get_submit_output_parser
from worker.option_parser import get_scheduler_option_parser, SubmitOptionParser
import worker.utils
from worker.utils import (get_worker_path, read_config_file, create_tempdir,
                          create_workfile, create_jobscript, submit_job,
                          exit_on_error, create_cleanup_function)

def main():
    # read configuration file
    worker_distr_path = get_worker_path(__file__)
    config_path = worker_distr_path / 'conf' / 'worker.conf'
    if not config_path.exists():
        exit_on_error(worker.errors.config_error,
                      msg=f'"{config_path}" not found')
    config = read_config_file(config_path)
    config['worker']['path'] = str(worker_distr_path)
    scheduler_name = config['scheduler']['name']

    # parse command line options and job script directives
    scheduler_option_parser = get_scheduler_option_parser(scheduler_name)
    option_parser = SubmitOptionParser(scheduler_option_parser, 'submit worker job')
    try:
        parser_result = option_parser.parse(sys.argv[1:])
    except FileNotFoundError as error:
        exit_on_error(worker.errors.batch_file_error, msg=error)
    except argparse.ArgumentError as error:
        exit_on_error(worker.errors.scheduler_option_error, msg=error)

    # create directory to store worker artfifacts
    tempdir_path = create_tempdir(config['worker']['tempdir_prefix'])

    # create cleanup function to remove the tempdir if something goes wrong
    cleanup_function = create_cleanup_function([tempdir_path],
                                               is_debug=parser_result.options.debug,
                                               is_verbose=parser_result.options.verbose)

    # create workfile in the worker artifacts directory
    workfile_path = tempdir_path / 'workerfile.txt'
    try:
        nr_workitems = create_workfile(workfile_path, parser_result, config)
    except ValueError as error:
        exit_on_error(worker.errors.data_error, cleanup_function, msg=error)
    except FileNotFoundError as error:
        exit_on_error(worker.errors.data_error, cleanup_function, msg=error)

    # create job script in the worker artifacts directory
    jobscript_path = tempdir_path / 'jobscript.sh'
    create_jobscript(jobscript_path, parser_result, config)

    # get submit output parse function
    parse_submit_output = get_submit_output_parser(scheduler_name)

    # submit the job
    submit_cmd_path = tempdir_path / 'submit.sh'
    try:
        submit_output = submit_job(submit_cmd_path, jobscript_path, parser_result, config,
                                   parser_result.scheduler_options)
        if not parser_result.options.dryrun:
            job_id = parse_submit_output(submit_output)
        else:
            job_id = None
    except worker.errors.JobSubmissionException as error:
        exit_on_error(worker.errors.submission_error, cleanup_function, msg=error.args[0])

    # rename the worker artifacts directory
    if job_id:
        tempdir_path.rename(f'worker_{job_id}')

    # write the job ID to standard output
    if job_id:
        print(f'total number of work items: {nr_workitems}')
        print(job_id)

    # everything was fine
    return 0


if __name__ == '__main__':
    sys.exit(main())
