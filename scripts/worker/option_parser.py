import argparse
import collections
import importlib
import pathlib
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

def parse_submit_cmd(submit_cmd_name):
    '''Parse the submit command stored in the given file name and return the
    command line arguments

    Parameters
    ----------
    submit_cmd_name: str
        file name that contains the submit command

    Returns
    -------
    shlex.shlex
        list of command line arguments
    '''
    with open(submit_cmd_name) as file:
        cmd_str = file.readline()
    _, arg_str = cmd_str.strip().split(maxsplit=1)
    return shlex.split(arg_str)


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
        self._worker_parser = argparse.ArgumentParser(add_help=False)
        self._worker_parser.add_argument('--num_cores', type=int, default=1,
                                           help='number of cores per work item')
        self._worker_parser.add_argument('--port', type=int,
                                           help='port the worker server will listen on')
        self._worker_parser.add_argument('--verbose', action='store_true',
                                           help='give verbose output for debugging')
        self._worker_parser.add_argument('--dryrun', action='store_true',
                                           help='create worker artifacts but do not submit job')
        self._relevant_parser = argparse.ArgumentParser(add_help=False)
        self._relevant_parser.add_argument(scheduler_option_parser.name_option, dest='name',
                                           help='job name')
        self._relevant_parser.add_argument(scheduler_option_parser.directive_prefix_option,
                                           dest='directive_prefix',
                                           default=scheduler_option_parser.default_directive_prefix,
                                           help='directive prefix')
        self._filter_parser = argparse.ArgumentParser(add_help=False)
        self._filter_parser.add_argument(scheduler_option_parser.array_option,
                                         dest='array_request',
                                         help='array request')

    def _merge_parsers(self):
        self._cl_parser = argparse.ArgumentParser(parents=[self._relevant_parser,
                                                           self._filter_parser,
                                                           self._worker_parser])
        self._passthrough_parser = argparse.ArgumentParser(parents=[self._filter_parser,
                                                                    self._worker_parser])

    @property
    def directive_prefix(self):
        '''Return the default directive prefix in job scripts

        Returns
        -------
        str
            Prefix for scheduler directives in job scripts
        '''
        return self._scheduler_option_parser._directive_prefix

    def filter_worker_cl(self, args):
        worker_options, _ = self._cl_parser.parse_known_args(args)
        return worker_options

    def filter_command_cl(self, args):
        _, command_args = self._passthrough_parser.parse_known_args(args)
        return command_args

    def _parse_script(self, file_name, directive_prefix):
        return self._scheduler_option_parser._parse_script(file_name, directive_prefix)



class SubmitOptionParser(OptionParser):
    '''Class for scheduler option parser for submit command line arguments
    '''
    def __init__(self, scheduler_option_parser, description):
        '''Base constructor, only to be called from subclasses
        '''
        super().__init__(scheduler_option_parser, description)
        self._worker_parser.add_argument('--data', action='append',
                                         help='data file containing the parameters'
                                              'for the work items')
        self._worker_parser.add_argument('--batch', dest='script', required=True,
                                         help='job script')
        self._merge_parsers()

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
        # get all command line option for worker + relevant + filter
        cl_options = self.filter_worker_cl(args)
        # parse_result.options contains the  relevant + filter options in the job script
        parser_result = self._parse_script(cl_options.script, cl_options.directive_prefix)
        # merge script and command line options
        options = argparse.Namespace(**vars(parser_result.options), **vars(cl_options))
        return ParseData(options, parser_result.shebang, parser_result.directives,
                         parser_result.script)


class ResubmitOptionParser(OptionParser):
    '''Class for scheduler option parser for resubmit command line options
    '''
    def __init__(self, scheduler_option_parser, description):
        '''Base constructor, only to be called from subclasses
        '''
        super().__init__(scheduler_option_parser, description)
        self._worker_parser.add_argument('--dir', required=True,
                                         help='directory containing job information to resume')
        self._worker_parser.add_argument('--redo', action='store_true',
                                         help='redo failed workitems')
        self._merge_parsers()

    def parse(self, args):
        # get current worker resubmit command line options
        cl_options = self.filter_worker_cl(args)
        previous_job_dir = pathlib.Path(cl_options.dir)

        # get command line options for the original submit command
        submit_cmd_name = previous_job_dir / 'submit.sh'
        original_submit_options = parse_submit_cmd(submit_cmd_name)

        # get the resubmit command line options
        resubmit_options = self.filter_command_cl(args)

        # merge submit and resubmit optoins
        options = self._scheduler_option_parser.merge_options(resubmit_options, original_submit_options)

        # parse worker jobsccript to get options and directives
        script_name = previous_job_dir / 'jobscript.sh'
        script_options, shebang, directives, _ = self._parse_script(script_name,
                                                                    cl_options.directive_prefix)

        # merge script options for job name
        merged_options = vars(script_options)
        merged_options.update(vars(options))
        options = argparse.Namespace(**merged_options, **vars(cl_options))
        return ParseData(options, shebang, directives, None)
