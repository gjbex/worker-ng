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

def create_workitem_file(filename, script, data_sources, sep):
    with open(filename, 'w') as file:
        data = next(data_sources)
        for var_name, value in data.items():
            print(f"export {var_name}='{value}'", file=file)
        print(file=file)
        print(script, file=file)
        for data in data_sources:
            print(sep, file=file)
            for var_name, value in data.items():
                print(f"export {var_name}='{value}'", file=file)
            print(file=file)
            print(script, file=file)
