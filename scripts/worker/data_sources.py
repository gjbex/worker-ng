import csv
import itertools
import re

class DataSource:
    '''Iterator that aggregates over data sources.  Data sources can be CSV files,
    text files representing a single column of data and a job array specificatoin.

    The first line of a data file has the column haeaders.  As they will be used
    as variable names in a Bash script they should adhere to the implied
    syntax restrictions.

    Each following line is a parameter instance and will be passed as a work item.

    It will iterate until the smallest data source is exhausted.
    '''

    def __init__(self, filenames, sniff_length, array_spec, array_var):
        '''Constructor for an aggregated data source.

        Parameters
        ----------
        filenames: list
            names of text data files, CSV or single column files
        sniff_length: int
            number of bytes to use for the CSV sniffer
        array_spec: str
            job array specification
        array_var: str
            name of the variable that holds the array ID
        '''
        data_sources = list()
        if filenames is not None:
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
        if not data_sources:
            raise ValueError('no data sources specified')
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
        dialect = csv.Sniffer().sniff(self._file.read(self._sniff_length), delimiters='; ,\t')
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
