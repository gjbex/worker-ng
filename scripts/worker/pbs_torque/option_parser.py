import argparse
from worker.option_parser import OptionParser
from worker.utils import expand_options
import shlex


def get_nodes_resource(resource_list):
    '''Select the nodes rersource out of a list of resource specifications

    Parameters
    ----------
    resource_list: list
        list of resource specifications

    Returns
    -------
    str | None
        relevant node specification
    '''
    nodes = list(option for option in expand_options(resource_list) if option.startswith('nodes'))
    return nodes[-1] if nodes else None


class PbsTorqueOptionParser:
    '''Parser for PBS torque command line options and job scripts'''

    def __init__(self, description='Parser for PBS torque qsub optionsa'):
        '''Constructor

        Returns
        -------
        PbsTorqueOptionParser
            new parser instance
        '''
        self._directive_prefix = '#PBS'
        self._base_parser = argparse.ArgumentParser(add_help=False)
        self._base_parser.add_argument('-l', dest='resources', action='append',
                                       help='resource list')
        self._base_parser.add_argument('-j', dest='join', choices=['oe', 'eo', 'n'],
                                       help='join output/erroro')
        self._base_parser.add_argument('-C', dest='directive_prefix', default=self._directive_prefix,
                                       help='directive prefix')
        self._base_parser.add_argument('-e', dest='error',
                                       help='error file path')
        self._base_parser.add_argument('-o', dest='output',
                                       help='output file path')
        self._base_parser.add_argument('-N', dest='name',
                                       help='job name')
        self._base_parser.add_argument('-t', dest='array_request',
                                       help='array request')
        self._base_parser.add_argument('-w', dest='working_dir',
                                       help='working directory')

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
        return options, shebang, pbs_directives, script
