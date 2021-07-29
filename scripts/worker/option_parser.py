import argparse
import shlex


def expand_options(options):
    new_options = list()
    for option in options:
        new_options.extend(option.split(','))
    return new_options

def get_nodes_resource(resource_list):
    nodes = list(option for option in expand_options(resource_list) if option.startswith('nodes'))
    return nodes[-1] if nodes else None


class OptionParserException(Exception):
    
    def __init__(self, *args):
        super().__init__(*args)


class OptionParser:

    def __init__(self):
        self._description='Parser for resource manager options'
        self._specific_parser = argparse.ArgumentParser(description=self._description, add_help=False)
        self._specific_parser.add_argument('--num_cores', type=int, default=1,
                                           help='number of cores per work item')

    def parse(self, args):
        cl_options, unknown_args = self._cl_parser.parse_known_args(args)
        script_options, shebang, pbs_directives, script = self._parse_script(cl_options.script,
                                                                             cl_options.directive_prefix)
        options, _ = self._cl_parser.parse_known_args(args, script_options)
        return options, shebang, pbs_directives, script

    def filter_cl(self, args):
        _, regular_options = self._specific_parser.parse_known_args(args)
        return regular_options
