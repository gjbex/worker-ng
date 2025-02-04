import configparser
import pathlib
import shlex
import shutil
import subprocess
import sys
import uuid
from worker.data_sources import DataSource
from worker.errors import JobSubmissionException


def exit_on_error(error, cleanup=None, **extra):
    msg = error.msg.format(**extra)
    print(f'error: {msg}', file=sys.stderr)
    if cleanup:
        cleanup()
    sys.exit(error.status)

def create_cleanup_function(artifacts, is_debug=False, is_verbose=False):
    if is_debug:
        def cleanup():
            if is_verbose:
                print(f'not cleaning up, debug mode is on', file=sys.stderr)
    else:
        def cleanup():
            for artifact in artifacts:
                if is_verbose:
                    print(f'removing {artifact}', file=sys.stderr)
                if artifact.is_dir():
                    shutil.rmtree(artifact)
                elif artifact.is_file():
                    artifact.unlink()
    return cleanup
        
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
    nr_workitems = 0
    with open(file_path, 'w') as file:
        nr_workitems += 1
        data = next(data_sources)
        for var_name, value in data.items():
            print(f"export {var_name}='{value}'", file=file)
        print(file=file)
        print(parser_result.script, file=file)
        for data in data_sources:
            nr_workitems += 1
            print(config['worker']['workitem_separator'], file=file)
            for var_name, value in data.items():
                print(f"export {var_name}='{value}'", file=file)
            print(file=file)
            print(parser_result.script, file=file)
    return nr_workitems

def create_jobscript(file_path, parser_result, template_path, config):
    '''create the job script for the computation

    Parameters
    ----------
    file_path: pathlib.Path
        file path to save the job script to
    parser_result: ParserResults
        configuration options from the job script directives and the command
        line arguments
    template_path: pathlib.Path
        path to the job script template
    config: configparser.configparser
        configuration of the worker framework
    '''
    worker_dir_path = get_worker_dir_path(config)
    template = template_path.read_text()
    templ_params = {
        'shebang': parser_result.shebang,
        'scheduler_directives': parser_result.directives,
        'worker_path': config['worker']['path'],
        'server_info': str(worker_dir_path / 'server_info.txt'),
        'server_log_opt': f'--log "{str(worker_dir_path / "server.log")}"',
        'port_opt': f"--port {parser_result.options.port or config['worker']['worker_port']}",
        'server_start_delay': config['worker']['server_start_delay'],
        'workfile': str(worker_dir_path / 'workerfile.txt'),
        'client_log_prefix_opt': f'--log_prefix "{str(worker_dir_path / "client_")}"',
        'env_var_exprs': f"{config['worker']['env_var_exprs']} {config['scheduler']['env_var_exprs']}",
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
        if completed.returncode != 0:
            raise(JobSubmissionException(completed.stderr))
        else:
            return completed.stdout.strip()
