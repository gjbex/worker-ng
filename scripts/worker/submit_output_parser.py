import importlib


def get_submit_output_parser(scheduler_name):
    '''get the submit output parser based on the scheduler's name

    Parameters
    ----------
    scheduler_name: str
        name of the scheduler

    Returns
    -------
    object
        instance of the submit output parser
    '''
    name_parts = scheduler_name.split()
    module_name = 'worker.' + '_'.join(map(str.lower, name_parts)) + '.submit_output_parser'
    parser_module = importlib.import_module(module_name)
    return getattr(parser_module, 'parse')
