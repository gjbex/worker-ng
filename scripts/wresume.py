#!/usr/bin/env python

import argparse
import pathlib
import sys
from worker.log_parsers import WorkitemLogParser
from worker.option_parser import get_scheduler_option_parser, ResubmitOptionParser, parse_submit_cmd
from worker.workfile_parser import WorkfileParser, filter_workfile
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
    option_parser = ResubmitOptionParser(scheduler_option_parser, 'resume worker job')
    parser_result = option_parser.parse(sys.argv[1:])
    previous_job_dir = pathlib.Path(parser_result.options.dir)
    print(parser_result)

    # create directory to store worker artfifacts
    tempdir_path = create_tempdir(config['worker']['tempdir_prefix'])

    # determine workitems to do
    log_parser = WorkitemLogParser()
    report = log_parser.parse(previous_job_dir / 'server.log')
    work_ids = report.incompletes
    if parser_result.options.redo:
        work_ids.extend(report.failures)
    filter_workfile(previous_job_dir / 'workerfile.txt',
                    tempdir_path / 'workerfile.txt',
                    '#WORKER----', work_ids)

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
