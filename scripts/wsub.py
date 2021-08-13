#!/usr/bin/env python

import sys
import worker.option_parser
import worker.templates
from worker.data_sources import DataSource
from worker.utils import get_worker_path, read_config_file, create_workitem_file


def main():
    worker_path = get_worker_path(__file__)
    config = read_config_file(worker_path / 'conf' / 'worker.conf')
    option_parser = worker.option_parser.get_parser(config['scheduler']['name'])
    parser_result = option_parser.parse(sys.argv[1:])
    print(parser_result.script)
    template = worker.templates.get_jobscript_template(config['scheduler']['name'])
    data_sources = DataSource(parser_result.options.data, 1024,
                              parser_result.options.array_request, 'PBS_ARRAYID')
    create_workitem_file('bla.txt', parser_result.script, data_sources, '#WORKER ------')

if __name__ == '__main__':
    main()
