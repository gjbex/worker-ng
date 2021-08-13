import csv
import itertools
import re

class DataSource:

    def __init__(self, filenames, sniff_length, array_spec, array_var):
        data_sources = list()
        for filename in filenames:
            try:
                data_sources.append(CsvDataSource(filename, sniff_length))
            except csv.Error as csv_error:
                try:
                    data_sources.append(SingleColumnDataSource(filename))
                except ValueError:
                    raise csv_error
        if array_spec is not None:
            data_sources.append(RangeDataSource(array_var, array_spec))
        self._iter = zip(*data_sources)

    def __iter__(self):
        return self

    def __next__(self):
        values = dict()
        for d in next(self._iter):
            for k, v in d.items():
                values[k] = v
        return values


class CsvDataSource:

    def __init__(self, filename, sniff_length=1024):
        self._filename = filename
        self._sniff_length = sniff_length
        self._file = open(self._filename, newline='')
        dialect = csv.Sniffer().sniff(self._file.read(self._sniff_length))
        self._file.seek(0)
        self._reader = csv.reader(self._file, dialect)
        self._fieldnames = next(self._reader)

    def __iter__(self):
        return self

    def __next__(self):
        values = next(self._reader)
        return {k:v for k, v in zip(self._fieldnames, values)}


class SingleColumnDataSource:

    def __init__(self, filename):
        self._filename = filename
        self._file = open(self._filename)
        self._fieldname = self._file.readline().strip()
        if not re.match(r'^[A-Za-z][A-Za-z0-9_]*$', self._fieldname):
            raise ValueError(f"'{self._fieldname}' is not a valid column name")

    def __iter__(self):
        return self

    def __next__(self):
        line = self._file.readline()
        if line:
            return {self._fieldname: line.rstrip()}
        else:
            raise StopIteration


class RangeDataSource:

    def __init__(self, fieldname, range_str):
        if not RangeDataSource._is_valid(range_str):
            raise ValueError(f"'{range_str}' is not a valid array expressin")
        self._fieldname = fieldname
        ranges = range_str.split(',')
        all_ranges = list()
        for range_str in ranges:
            match = re.match(r'^(\d+)-(\d+)$', range_str)
            if match is not None:
                all_ranges.append(range(int(match.group(1)), int(match.group(2)) + 1))
            else:
                all_ranges.append(range(int(range_str), int(range_str) + 1))
        self._iter = itertools.chain(*all_ranges)

    def __iter__(self):
        return self

    def __next__(self):
        return {self._fieldname: str(next(self._iter))}

    @staticmethod
    def _is_valid(range_str):
        match = re.match(r'^\d+(?:\-\d+)?(?:,\d+(?:\-\d+)?)*$', range_str)
        return match is not None
