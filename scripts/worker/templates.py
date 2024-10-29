import pathlib


def get_jobscript_template(scheduler_name):
    path = pathlib.Path(__file__).parent / 'worker' / '_'.join(map(str.lower, scheduler_name.split())) / 'jobscript.tmpl'   
    return path.read_text()
