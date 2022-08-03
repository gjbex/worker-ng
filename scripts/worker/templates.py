import importlib.resources


def get_jobscript_template(scheduler_name):
    package_name = 'worker.' + '_'.join(map(str.lower, scheduler_name.split()))
    return importlib.resources.read_text(package_name, 'jobscript.tmpl')
