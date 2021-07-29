import argparse
import option_parser
import shlex


class PbsTorqueOptionParser(option_parser.OptionParser):

    def __init__(self):
        super().__init__()
        self._base_parser = argparse.ArgumentParser(description=self._description, add_help=False)
        self._base_parser.add_argument('-l', dest='resources', action='append',
                                       help='resource list')
        self._base_parser.add_argument('-j', dest='join', choices=['oe', 'eo', 'n'],
                                       help='join output/erroro')
        self._base_parser.add_argument('-C', dest='directive_prefix', default='#PBS ',
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
        self._cl_parser = argparse.ArgumentParser(parents=[self._base_parser, self._specific_parser])
        self._cl_parser.add_argument('script', help='script file')


    def _parse_script(self, file_name, directive_prefix):
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
