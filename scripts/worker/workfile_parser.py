class WorkfileParser:

    def __init__(self, sep):
        self._sep = sep

    def parse(self, file_name):
        with open(file_name) as file:
            workitem = ''
            for line in file:
                if line.strip() == self._sep:
                    yield workitem
                    workitem = ''
                else:
                    workitem += line
            return workitem


def filter_workfile(input_file_name, output_file_name, sep, item_ids):
    parser  = WorkfileParser(sep)
    item_count = 0
    with open(output_file_name, 'w') as output_file:
        for i, workitem in enumerate(parser.parse(input_file_name)):
            if i + 1 in item_ids:
                print(f'keep {i + 1}')
                if item_count > 0:
                    print(sep, file=output_file)
                print(workitem, file=output_file)
                item_count += 1
    return item_count
