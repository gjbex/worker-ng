import argparse
import collections
import importlib
import pathlib
import shlex
import worker.errors


ParseData = collections.namedtuple('ParseData', ['options', 'shebang', 'directives', 'script',
                                                 'scheduler_options'])

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

def _options_dict2list(opt_dict, opt_sep=','):
    opt_list = []
    for key, value in opt_dict.items():
        if value is not False and value is not None:
            opt_list.append(f'-{key}')
            if value is not True:
                if type(value) is list:
                    opt_list.append(opt_sep.join(value))
                else:
                    opt_list.append(value)
    return opt_list

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
        self._worker_parser.add_argument('--debug', action='store_true',
                                         help='enable debug mode')
        self._relevant_parser = argparse.ArgumentParser(add_help=False)
        if isinstance(scheduler_option_parser.name_option, str):
            self._relevant_parser.add_argument(scheduler_option_parser.name_option, dest='name',
                                               help='job name')
        else:
            self._relevant_parser.add_argument(*scheduler_option_parser.name_option, dest='name',
                                               help='job name')
        self._relevant_parser.add_argument(scheduler_option_parser.directive_prefix_option,
                                           dest='directive_prefix',
                                           default=scheduler_option_parser.default_directive_prefix,
                                           help='directive prefix')
        self._filter_parser = argparse.ArgumentParser(add_help=False)
        self._filter_parser.add_argument(scheduler_option_parser.array_option,
                                         dest='array_request',
                                         help='array request')

    def _verify_scheduler_args(self, args):
        arg_parser = argparse.ArgumentParser(add_help=False, exit_on_error=False)
        for option in self._scheduler_option_parser.pass_through_options:
            arg_parser.add_argument(option)
        for flag in self._scheduler_option_parser.pass_through_flags:
            arg_parser.add_argument(flag, action='store_true')
        _ = arg_parser.parse_known_args(args)
        
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
        '''Return all options relevant to worker, i.e.,
          * the options for worker itself,
          * the options for the scheduler that worker intercepts (e.g., job array request),
          * relevant scheduler options (e.g., scheduler directive, job name)

        Parameters
        ----------
        args: list
            list of all arguments passed via the command line

        Returns
        -------
        namespace
            command line options as a namespace
        '''
        worker_options, _ = self._cl_parser.parse_known_args(args)
        return worker_options

    def filter_command_cl(self, args):
        '''Return only options that should be passed to the scheduler's submission command

        Parameters
        ----------
        args: list
            list of all arguments passed via the command line

        Returns
        -------
        list
            arguments that should be passed to the scheduler's submit command
        '''
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
        scheduler_options = self.filter_command_cl(args)
        self._verify_scheduler_args(scheduler_options)
        return ParseData(options, parser_result.shebang, parser_result.directives,
                         parser_result.script, scheduler_options)


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
        if not previous_job_dir.exists():
            raise worker.errors.WorkerDirException(f'directory {previous_job_dir} does not exist')
        if not previous_job_dir.is_dir():
            raise worker.errors.WorkerDirException(f'{previous_job_dir} is not a directory')

        # get command line options for the original submit command
        submit_cmd_name = previous_job_dir / 'submit.sh'
        original_submit_options = parse_submit_cmd(submit_cmd_name)

        # get the resubmit command line options
        resubmit_options = self.filter_command_cl(args)

        # merge submit and resubmit options
        submit_options = self._scheduler_option_parser.merge_options(resubmit_options,
                                                                     original_submit_options)

        # parse worker jobsccript to get options and directives
        script_name = previous_job_dir / 'jobscript.sh'
        script_options, shebang, directives, _, _ = self._parse_script(script_name,
                                                                       cl_options.directive_prefix)

        # merge script options for job name
        merged_options = vars(script_options)
        merged_options.update(vars(submit_options))
        return ParseData(cl_options, shebang, directives, None,
                         _options_dict2list(merged_options))
