import argparse
import shlex


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


class OptionParserException(Exception):
    '''Base class for exceptions related to parsing options for schedulers
    '''
    
    def __init__(self, *args):
        super().__init__(*args)


class OptionParser:
    '''Abstract base class for scheduler option parsers.  Concrete classes should be
    implemented for specific schedulers such as PBS torque or Slurm.
    '''

    def __init__(self):
        '''Base constructor, only to be called from subclasses
        '''
        self._description='Parser for resource manager options'
        self._specific_parser = argparse.ArgumentParser(description=self._description, add_help=False)
        self._specific_parser.add_argument('--num_cores', type=int, default=1,
                                           help='number of cores per work item')

    def parse(self, args):
        '''Method to parse command line arguments passed to the submission command of a scheduler

        Parameters
        ----------
        args: list
            command line arguments as passed to the submission command

        Returns
        -------
        options: namespace
            relevant options for worker
        shebang: str
            shebang line used in the job script
        directives: str
            directives for the scheduler in the job script
        script: str
            script to be executed, payload of the job
        '''

        cl_options, unknown_args = self._cl_parser.parse_known_args(args)
        script_options, shebang, directives, script = self._parse_script(cl_options.script,
                                                                         cl_options.directive_prefix)
        options, _ = self._cl_parser.parse_known_args(args, script_options)
        return options, shebang, directives, script

    def filter_cl(self, args):
        _, regular_options = self._specific_parser.parse_known_args(args)
        return regular_options
