import re


def parse(output_str):
    match = re.search(r'Submitted batch job (?P<job_id>\d+) on cluster (?P<cluster>\w+)',
                      output_str)
    if match:
        return match['job_id']
    else:
        raise ValueError(f'can not parse submission output "{output_str}"')
