import configparser
import pathlib


def get_worker_path(obj):
    script_dir = pathlib.Path(obj).parent.absolute()
    return script_dir.parent.absolute()

def read_config_file(file_name):
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
