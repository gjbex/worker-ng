#!/usr/bin/env python

import argparse
import pandas as pd
import worker.errors
from worker.log_parsers import WorkitemLogParser
from worker.utils import exit_on_error
import sys


SEP = 72*'-'

if __name__ == '__main__':
    arg_parser = argparse.ArgumentParser(description='parse log file')
    arg_parser.add_argument('log', help='log file to parse')
    arg_parser.add_argument('--show_raw', action='store_true',
                            help='show raw data')
    arg_parser.add_argument('--show_failed', action='store_true',
                            help='show workitem IDs of failed items')
    arg_parser.add_argument('--show_incomplete', action='store_true',
                            help='show workitem IDs of incomplete items')
    arg_parser.add_argument('--show_walltime_stats', action='store_true',
                            help='show statitics on workitem walltime')
    arg_parser.add_argument('--show_client_stats', action='store_true',
                            help='show client statistics')
    arg_parser.add_argument('--show_all', action='store_true',
                            help='show all information')
    options = arg_parser.parse_args()
    parser = WorkitemLogParser()
    try:
        report = parser.parse(options.log)
    except FileNotFoundError as error:
        exit_on_error(worker.errors.log_file_error, msg=error)
    except worker.errors.LogParseException as error:
        exit_on_error(worker.errors.log_file_error, msg=error)
    if report is None:
        print('no work done')
        sys.exit(0)
    if options.show_raw:
        print(report.raw)
        if not options.show_all:
            sys.exit(0)
        else:
            print(SEP)
    print(f'success: {len(report.successes)}')
    print(f'failure: {len(report.failures)}')
    if options.show_failed or options.show_all:
        print('\t' + ' '.join(str(work_id) for work_id in report.failures))
    print(f'incomplete: {len(report.incompletes)}')
    if options.show_incomplete or options.show_all:
        print('\t' + ' '.join(str(work_id) for work_id in report.incompletes))
    if options.show_walltime_stats or options.show_all:
        print(SEP)
        print('workitem walltime statistics:')
        for quantity, value in report.walltime_stats.iteritems():
            print(f'\t{quantity:10s} {value}')
    if options.show_client_stats or options.show_all:
        print(SEP)
        with pd.option_context('display.max_columns', None, "max_rows", None):
            print(report.client_stats)
