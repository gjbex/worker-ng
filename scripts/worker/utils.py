import configparser
import pathlib
import shlex
import subprocess
import uuid
from worker.data_sources import DataSource
from worker.templates import get_jobscript_template


def get_worker_dir_path(config):
    '''Return the path to the woker directory based on the job ID

    Parameters
    ----------
    config: configparser.configparser
        worker configuration object

    Returns
    -------
    libpath.Path
        path to the worker directory
    '''
    return pathlib.Path.cwd() / f"worker_${{{config['scheduler']['jobid_var_name']}}}" 

def create_tempdir(prefix=''):
    '''create the worker directory with a temporary name, this directory is to
    be renamed as soon as the job ID is known

    Returns
    -------
    pathlib.Path
        path to the worker directory under its temporay name
    '''
    dir_path = pathlib.Path.cwd() / f'{prefix}{str(uuid.uuid4())}'
    dir_path.mkdir()
    return dir_path

def get_worker_path(obj):
    '''return the  path to the worker installation, top-level directory

    Returns
    -------
    pathlib.Path
        worker installation directory path
    '''
    script_dir = pathlib.Path(obj).parent.absolute()
    return script_dir.parent.absolute()

def read_config_file(file_name):
    '''parse the worker configuatin file

    Parameters
    ----------
    file_name: str
        path to the configuration file to read

    Returns
    -------
    configparser.configparser
        worker configuration
    '''
    config_parser = configparser.ConfigParser()
    config_parser.read(file_name)
    return config_parser

def expand_options(options):
    '''some options can have multiple values separated by comma, this function expands
    those options

    Parameters
    ----------
    options: list
        list of options, some of which may have to be expanded

    Returns
    -------
    list
        expanded options list, order is preserved
    '''
    new_options = list()
    for option in options:
        new_options.extend(option.split(','))
    return new_options

def create_workfile(file_path, parser_result, config):
    '''create the workfile for the computation

    Parameters
    ----------
    file_path: pathlib.Path
        file path to save the workfile to
    parser_result: ParserResults
        configuration options from the job script directives and the command
        line arguments
    config: configparser.configparser
        configuration of the worker framework
    '''
    data_sources = DataSource(parser_result.options.data, 1024,
                              parser_result.options.array_request,
                              config['scheduler']['arrayid_var_name'])
    with open(file_path, 'w') as file:
        data = next(data_sources)
        for var_name, value in data.items():
            print(f"export {var_name}='{value}'", file=file)
        print(file=file)
        print(parser_result.script, file=file)
        for data in data_sources:
            print(config['worker']['workitem_separator'], file=file)
            for var_name, value in data.items():
                print(f"export {var_name}='{value}'", file=file)
            print(file=file)
            print(parser_result.script, file=file)

def create_jobscript(file_path, parser_result, config):
    '''create the job script for the computation

    Parameters
    ----------
    file_path: pathlib.Path
        file path to save the job script to
    parser_result: ParserResults
        configuration options from the job script directives and the command
        line arguments
    config: configparser.configparser
        configuration of the worker framework
    '''
    worker_dir_path = get_worker_dir_path(config)
    template = get_jobscript_template(config['scheduler']['name'])
    templ_params = {
        'shebang': parser_result.shebang,
        'scheduler_directives': parser_result.directives,
        'worker_path': config['worker']['path'],
        'server_info': str(worker_dir_path / 'server_info.txt'),
        'log_prefix_opt': f'--log_prefix "{str(worker_dir_path / "log_")}"',
        'port_opt': f"--port {parser_result.options.port or config['worker']['worker_port']}",
        'server_start_delay': config['worker']['server_start_delay'],
        'workfile': str(worker_dir_path / 'workerfile.txt'),
        'o_opt': f'-out "{parser_result.options.output}"' if parser_result.options.output else '',
        'e_opt': f'-err "{parser_result.options.error}"' if parser_result.options.error else '',
        'num_cores': parser_result.options.num_cores,
        'exit_on_client_fail': 'false',
    }
    with open(file_path, 'w') as jobscript_file:
        print(template.format(**templ_params), file=jobscript_file)

def submit_job(submit_cmd_path, jobscript_path, parser_result, config, original_cl_options):
    command = [config['scheduler']['submit_command']] + original_cl_options + [str(jobscript_path)]
    command_str = shlex.join(command)
    with open(submit_cmd_path, 'w') as file:
        print(command_str, file=file)
    if parser_result.options.dryrun:
        print(command_str)
        return ''
    else:
        completed = subprocess.run(command, capture_output=True, text=True)
        if completed.returncode:
            raise(OSError(completed.returncode, completed.stderr))
        else:
            return completed.stdout.strip()
