import argparse
from worker.option_parser import ParseData
from worker.utils import expand_options
import shlex

def _normalize_resources(resources):
    resource_dict = {}
    for resource in resources:
        parts = resource.split(',')
        for part in parts:
            key, value = part.split('=', maxsplit=1)
            resource_dict[key] = value
    return resource_dict

def _merge_resources(old_resources, new_resources):
    if old_resources is None:
        return new_resources
    if new_resources is None:
        return old_resources
    resources = _normalize_resources(old_resources)
    resources.update(_normalize_resources(new_resources))
    return [f'{key}={value}' for key, value in resources.items()]




class PbsTorqueOptionParser:
    '''Parser for PBS torque command line options and job scripts'''

    def __init__(self, description='Parser for PBS torque qsub optionsa'):
        '''Constructor

        Returns
        -------
        PbsTorqueOptionParser
            new parser instance
        '''
        self._base_parser = argparse.ArgumentParser(add_help=False)
        self._base_parser.add_argument(self.name_option)
        self._base_parser.add_argument(self.directive_prefix_option)
        self._base_parser.add_argument(self.array_option)

    @property
    def pass_through_options(self):
        return ['-a', '-A', '-b', '-c', '-d', '-D', '-e', '-j', '-k', '-K', '-L',
                '-m', '-M', '-n', '-N', '-o', '-p', '-P', '-q', '-r', '-S',
                '-T', '-u', '-v', '-w', '-W', ]

    @property
    def pass_through_flags(self):
        return ['-V', '-h', '-f', '-F', ]

    @property
    def array_option(self):
        return '-t'

    @property
    def name_option(self):
        return '-N'

    @property
    def directive_prefix_option(self):
        return '-C'

    @property
    def default_directive_prefix(self):
        return '#PBS'

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
        '''
        args = list()
        shebang = None
        pbs_directives = ''
        script = ''
        parsing_pbs = True
        with open(file_name, 'r') as file:
            for line_nr, line in enumerate(file):
                if line.startswith('#!') and line_nr == 0:
                    shebang = line.strip()
                elif line.startswith(directive_prefix) and parsing_pbs:
                    args.extend(shlex.split(line[len(directive_prefix):], comments=True))
                    pbs_directives += line
                elif (line.startswith('#') or line.isspace()) and parsing_pbs:
                    continue
                else:
                    parsing_pbs = False
                    script += line
        options, _ = self._base_parser.parse_known_args(args)
        return ParseData(options, shebang, pbs_directives, script)

    def merge_options(self, new_args, old_args):
        arg_parser = argparse.ArgumentParser(add_help=False)
        for option in self.pass_through_options:
            arg_parser.add_argument(option)
        for flag in self.pass_through_flags:
            arg_parser.add_argument(flag, action='store_true')
        old_options, _ = arg_parser.parse_known_args(old_args)
        merged_options, _ = arg_parser.parse_known_args(new_args, namespace=old_options)
        resource_parser = argparse.ArgumentParser()
        resource_parser.add_argument('-l', action='append')
        old_resources, _ = resource_parser.parse_known_args(old_args)
        new_resources, _ = resource_parser.parse_known_args(new_args)
        resources = _merge_resources(old_resources.l, new_resources.l)
        merged_options = argparse.Namespace(l=resources, **vars(merged_options))
        return merged_options
