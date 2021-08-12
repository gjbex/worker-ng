import argparse
import collections
import importlib
import shlex


ParseData = collections.namedtuple('ParseData', ['options', 'shebang', 'directives', 'script'])

def _get_parser_class_info(scheduler_name):
    name_parts = scheduler_name.split() + ['Option', 'Parser']
    class_name = ''.join(map(str.capitalize, name_parts))
    module_name = 'worker.' + '_'.join(map(str.lower, name_parts))
    return class_name, module_name

def get_parser(scheduler_name):
    class_name, module_name = _get_parser_class_info(scheduler_name)
    parser_module = importlib.import_module(module_name)
    parser_class = getattr(parser_module, class_name)
    return parser_class()

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
        Parameters
            named tuple with the following fields
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
        return ParseData(options, shebang, directives, script)

    def filter_cl(self, args):
        _, regular_options = self._specific_parser.parse_known_args(args)
        return regular_options
