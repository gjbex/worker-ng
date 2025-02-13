import argparse
import itertools
from worker.option_parser import ParseData
from worker.utils import expand_options
import shlex


def flatten(data):
    for item in data:
        if hasattr(item, '__iter__') and not isinstance(item, str):
            for subitem in item:
                yield subitem
        else:
            yield item

def _merge_resources(old_resources, new_resources):
    if old_resources is None:
        return new_resources
    if new_resources is None:
        return old_resources
    resource_dict = vars(old_resources)
    resource_dict.update(vars(new_resources))
    return argparse.Namespace(**resource_dict)


class SlurmOptionParser:
    '''Parser for Slurm command line options and job scripts'''

    def __init__(self, description='Parser for PBS torque qsub optionsa'):
        '''Constructor

        Returns
        -------
        SlurmOptionParser
            new parser instance
        '''
        self._base_parser = argparse.ArgumentParser(add_help=False)
        self._base_parser.add_argument(*self.name_option)
        self._base_parser.add_argument(*self.array_option)

    @property
    def pass_through_options(self):
        return [('-A', '--account'),  '--batch', ('-b', '--begin'), ('-D', '--chdir'),
                ('-L', '--licenses'), ('-M', '--cluster'), '--comment',
                ('-d', '--dependency'), ('-m', '--distribution'), ('-e', '--error'),
                '--mail-type', '--mail-user', '--network', '--nice', ('-k', '--no-kill'),
                '--open-mode', ('-o', '--output'), ('-p', '--partition'), '--prefer',
                '--priority', ('-q', '--qos'), '--reservation',
                ('-t', '--time'), '--time-min', '--tmp', '--wait-all-nodes', 
               ]

    @property
    def pass_through_flags(self):
        return [('-h', '--help'), ('-H', '--hold'),  '--ignore-pbs', '--no-requeue',
                ('-Q', '--quiet'), '--reboot', '--requeue', '--test-only',
                '--usage', '--use-min-nodes', ('-v', '--verbose'), ('-V', '--version'),
                ('-W', '--wait'),
               ]

    @property
    def array_option(self):
        return ('-a', '--array')

    @property
    def name_option(self):
        return ('-J', '--jobname')

    @property
    def directive_prefix_option(self):
        return []

    @property
    def default_directive_prefix(self):
        return '#SBATCH'

    @property
    def resource_options(self):
        return [('-c', '--cpus-per-task'), '--gpu-bind', ('-G', '--gpus'),
                '--gpus-per-socket', '--gpus-per-node', '--gpus-per-task', '--hint',
                '--mem', '--mem-bind', '--mem-per-cpu', '--mem-per-gpu',
                ('-N', '--nodes'), ('-n', '--ntasks'), '--ntasks-per-core',
                '--ntasks-per-gpu', '--ntasks-per-node', '--ntasks-per-socket', 
                '--propagate', '--sockets-per-node', '--threads-per-core',
               ]

    @property
    def resource_flags(self):
        return ['--contiguous', ('-O', '--overcommit'), ('-s', '--oversubscribe'),
                '--spread-job',
               ]

    def _is_resource_directive_line(self, args):
        for arg in args:
            for option in flatten(self.resource_options):
                if f'{option}=' in arg:
                    return True
            if arg in itertools.chain(*self.resource_flags):
                return True
        return False
        
    def parse_cl(self, args, context=None):
        return self._base_parser.parse_known_args(args, context)

    def _parse_script(self, file_name, directive_prefix):
        '''Concrete implementation of the parser for PBS torque scripts, private method,
        to be called by the superclass

        Parameters
        ----------
        file_name: str
            path to the job script
        directive_prefix: str
            directive prefix used in the script

        Returns
        -------
        ParseData
            * options: relevent options passed via scheduler directives (e.g., -N, -t)
            * shebang
            * directives: string of scheduler directives in the script, unfiltered
            * script
        '''
        args = list()
        shebang = None
        slurm_directives = ''
        slurm_group0_directives = ''
        script = ''
        parsing_slurm = True
        with open(file_name, 'r') as file:
            for line_nr, line in enumerate(file):
                if line.startswith('#!') and line_nr == 0:
                    shebang = line.strip()
                elif line.startswith(directive_prefix) and parsing_slurm:
                    new_args = shlex.split(line[len(directive_prefix):], comments=True)
                    args.extend(new_args)
                    if self._is_resource_directive_line(new_args):
                        slurm_group0_directives += line
                    else:
                        slurm_directives += line
                elif (line.startswith('#') or line.isspace()) and parsing_slurm:
                    continue
                else:
                    parsing_slurm = False
                    script += line
        slurm_directives += f'{slurm_group0_directives}'
        slurm_directives += f'{self.default_directive_prefix} hetjob\n{self.default_directive_prefix} --nodes=1 --ntasks=1 --cpus-per-task=1\n'
        options, _ = self._base_parser.parse_known_args(args)
        return ParseData(options, shebang, slurm_directives, script, None)

    def merge_options(self, new_args, old_args):
        arg_parser = argparse.ArgumentParser(add_help=False)
        for option in self.pass_through_options:
            if isinstance(option, str):
                arg_parser.add_argument(option)
            else:
                arg_parser.add_argument(*option)
        for flag in self.pass_through_flags:
            if isinstance(flag, str):
                arg_parser.add_argument(flag, action='store_true')
            else:
                arg_parser.add_argument(*flag, action='store_true')
        old_options, _ = arg_parser.parse_known_args(old_args)
        merged_options, _ = arg_parser.parse_known_args(new_args, namespace=old_options)
        resource_parser = argparse.ArgumentParser()
        for option in self.resource_options:
            if isinstance(option, str):
                resource_parser.add_argument(option)
            else:
                resource_parser.add_argument(*option)
        for flag in self.resource_flags:
            if isinstance(flag, str):
                resource_parser.add_argument(flag, action='store_true')
            else:
                resource_parser.add_argument(*flag, action='store_true')
        old_resources, _ = resource_parser.parse_known_args(old_args)
        new_resources, _ = resource_parser.parse_known_args(new_args)
        resources = _merge_resources(old_resources, new_resources)
        merged_options = argparse.Namespace(**vars(resources), **vars(merged_options))
        return merged_options


def preprocess_cli_options(options):
    '''ensure that options in the form [..., '-nodes', '5', ...] get reformatted to [..., '--nodes=5', ...]
    '''
    new_options = []
    ignore_next = False
    for i, item in enumerate(options):
        if ignore_next:
            ignore_next = False
            continue
        if item.startswith('-'):
            if i + 1 < len(options) and not options[i + 1].startswith('-'):
                ignore_next = True
                new_options.append(f'-{item}={options[i + 1]}')
            else:
                new_options.append(f'-{item}')
    return new_options
