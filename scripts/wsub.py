#!/usr/bin/env python

import sys
import worker.option_parser
from worker.option_parser import get_scheduler_option_parser, SubmitOptionParser
import worker.utils
from worker.utils import (get_worker_path, read_config_file, create_tempdir,
                          create_workfile, create_jobscript, submit_job)

def main():
    # read configuration file
    worker_distr_path = get_worker_path(__file__)
    config = read_config_file(worker_distr_path / 'conf' / 'worker.conf')
    config['worker']['path'] = str(worker_distr_path)
    scheduler_name = config['scheduler']['name']

    # parse command line options and job script directives
    scheduler_option_parser = get_scheduler_option_parser(scheduler_name)
    option_parser = SubmitOptionParser(scheduler_option_parser, 'submit worker job')
    parser_result = option_parser.parse(sys.argv[1:])
    original_cl_options = option_parser.filter_cl(sys.argv[1:])

    # create directory to store worker artfifacts
    tempdir_path = create_tempdir(config['worker']['tempdir_prefix'])

    # create workfile in the worker artifacts directory
    workfile_path = tempdir_path / 'workerfile.txt'
    create_workfile(workfile_path, parser_result, config)

    # create job script in the worker artifacts directory
    jobscript_path = tempdir_path / 'jobscript.sh'
    create_jobscript(jobscript_path, parser_result, config)

    # submit the job
    submit_cmd_path = tempdir_path / 'submit.sh'
    job_id = submit_job(submit_cmd_path, jobscript_path, parser_result, config, original_cl_options)

    # rename the worker artifacts directory
    if job_id:
        tempdir_path.rename(f'worker_{job_id}')

if __name__ == '__main__':
    main()
