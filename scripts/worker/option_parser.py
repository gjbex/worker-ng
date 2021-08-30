import argparse
import collections
import importlib
import shlex


ParseData = collections.namedtuple('ParseData', ['options', 'shebang', 'directives', 'script'])

def _get_parser_class_info(scheduler_name):
    '''Retrieve the appropriate class and module for the specified scheduler

    Parameters
    ----------
    scheduler_name: str
        name of the scheduler

    Returns
    -------
    tuple(str, str)
        class name and module name for the option parser
    '''
    name_parts = scheduler_name.split()
    class_parts = ['option', 'parser']
    class_name = ''.join(map(str.capitalize, name_parts + class_parts))
    module_name = 'worker.' + '_'.join(map(str.lower, name_parts)) + '.' + '_'.join(class_parts)
    return class_name, module_name

def get_scheduler_option_parser(scheduler_name):
    '''Instantiate a scheduler option parser based on the scheduler's name

    Parameters
    ----------
    scheduler_name: str
        name of the scheduler

    Returns
    -------
    object
        instance of the scheduler option parser
    '''
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

    def __init__(self, scheduler_option_parser, description):
        '''Base constructor, only to be called from subclasses
        '''
        self._scheduler_option_parser = scheduler_option_parser
        self._description = description
        self._specific_parser = argparse.ArgumentParser(add_help=False)
        self._specific_parser.add_argument('--num_cores', type=int, default=1,
                                           help='number of cores per work item')
        self._specific_parser.add_argument('--port', type=int,
                                           help='port the worker server will listen on')
        self._specific_parser.add_argument('--verbose', action='store_true',
                                           help='give verbose output for debugging')
        self._specific_parser.add_argument('--dryrun', action='store_true',
                                           help='create worker artifacts but do not submit job')

    @property
    def directive_prefix(self):
        '''Return the default directive prefix in job scripts

        Returns
        -------
        str
            Prefix for scheduler directives in job scripts
        '''
        return self._scheduler_option_parser._directive_prefix

    def _parse_script(self, file_name, directive_prefix):
        return self._scheduler_option_parser._parse_script(file_name, directive_prefix)


class SubmitOptionParser(OptionParser):
    '''Abstract base class for scheduler option parsers.  Concrete classes should be
    implemented for specific schedulers such as PBS torque or Slurm.
    '''
    def __init__(self, scheduler_option_parser, description):
        '''Base constructor, only to be called from subclasses
        '''
        super().__init__(scheduler_option_parser, description)
        self._specific_parser.add_argument('--data', action='append',
                                           help='data file containing the parameters'
                                                'for the work items')
        self._specific_parser.add_argument('--batch', dest='script', help='job script')
        self._cl_parser = argparse.ArgumentParser(parents=[self._scheduler_option_parser._base_parser,
                                                           self._specific_parser])

    def parse(self, args):
        '''Method to parse command line arguments passed to the submission command of a
        scheduler

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
