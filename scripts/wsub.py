#!/usr/bin/env python

import pathlib
import sys
import uuid
import worker.option_parser
import worker.templates
from worker.data_sources import DataSource
from worker.utils import get_worker_path, read_config_file, create_workitem_file


def create_directory():
    dir_path = pathlib.Path.cwd() / str(uuid.uuid4())
    dir_path.mkdir()
    return dir_path

def create_workfile(file_path, parser_result, config):
    data_sources = DataSource(parser_result.options.data, 1024,
                              parser_result.options.array_request,
                              config['scheduler']['arrayid_var_name'])
    create_workitem_file(file_path, parser_result.script,
                         data_sources, config['worker']['workitem_separator'])
    return pathlib.Path.cwd() / f"worker_${{{config['scheduler']['jobid_var_name']}}}" / 'workerfile.txt'

def main():
    worker_path = get_worker_path(__file__)
    config = read_config_file(worker_path / 'conf' / 'worker.conf')
    scheduler_name = config['scheduler']['name']
    option_parser = worker.option_parser.get_parser(scheduler_name)
    parser_result = option_parser.parse(sys.argv[1:])
    template = worker.templates.get_jobscript_template(scheduler_name)
    dir_path = create_directory()
    workfile_path = dir_path / 'workerfile.txt'
    workfile_name = f'"{create_workfile(workfile_path, parser_result, config)}"'
    templ_params = {
        'shebang': parser_result.shebang,
        'scheduler_directives': parser_result.directives,
        'log_prefix_opt': '',
        'port_opt': f"--port {parser_result.options.port or config['worker']['worker_port']}",
        'worker_path': str(worker_path),
        'server_start_delay': config['worker']['server_start_delay'],
        'workfile': workfile_name,
        'o_opt': f'-out "{parser_result.options.output}"' if parser_result.options.output else '',
        'e_opt': f'-err "{parser_result.options.error}"' if parser_result.options.error else '',
        'num_cores': parser_result.options.num_cores,
        'exit_on_client_fail': 'false',
    }
    jobscript_path = dir_path / 'jobscript.sh'
    with open(jobscript_path, 'w') as jobscript_file:
        print(template.format(**templ_params), file=jobscript_file)

if __name__ == '__main__':
    main()
