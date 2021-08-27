import abc
from collections import defaultdict
import dataclasses
from dataclasses import dataclass
import datetime
from functools import singledispatchmethod
import pandas as pd
import re
import typing


@dataclass
class WorkItem:
    item_id: int = None
    client_id: str = None
    start_time: datetime.datetime = None
    duration: datetime.timedelta = None
    status: int = None


def convert_to_dict(workitems):
    workitem_dict = defaultdict(list)
    for workitem in workitems:
        for field in dataclasses.fields(workitem):
            workitem_dict[field.name].append(getattr(workitem, field.name))
    return workitem_dict

class LogParser(abc.ABC):

    def __init__(self):
        self._exprs = {
            'date': r'\d{4}-\d{2}-\d{2}',
            'time': r'\d{2}:\d{2}:\d{2}\.\d+',
            'log_level': r'\[(?P<log_level>\w+)\]',
            'workitem_id': r'(?P<workitem_id>\d+)',
            'client_id': r'(?P<client_id>[\w\-]+)',
            'exit_status': r'(?P<exit_status>-?\d+)'
        }
        self._exprs['datetime'] = r'(?P<datetime>{date}\s+{time})'.format(**self._exprs)
        self._exprs['prefix'] = r'{datetime}\s+{log_level}\s*:'.format(**self._exprs)

    @staticmethod
    def parse_time(datetime_str):
        return datetime.datetime.strptime(datetime_str, '%Y-%m-%d %H:%M:%S.%f')

    def parse(self, obj):
        if type(obj) == str:
            with open(obj) as file:
                return self._parse(file)
        else:
            return self._parse(obj)

    @abc.abstractmethod
    def _parse(self, file):
        ...


class WorkitemLogParser(LogParser):

    def __init__(self):
        super().__init__()
        self._exprs['msg'] = r'workitem\s+{workitem_id}'.format(**self._exprs)
        self._exprs['started_msg'] = r'{msg}\s+started\s*:\s*{client_id}'.format(**self._exprs)
        self._exprs['done_msg'] = r'{msg}\s+done\s*:\s*{exit_status}'.format(**self._exprs)
        self._started_expr = re.compile(r'{prefix}\s+{started_msg}'.format(**self._exprs))
        self._done_expr = re.compile(r'{prefix}\s+{done_msg}'.format(**self._exprs))

    def _parse(self, file):
        workitems = defaultdict(WorkItem)
        for line in file:
            if (match := self._started_expr.match(line)) is not None:
                item_id = int(match.group('workitem_id'))
                workitems[item_id].item_id = item_id
                workitems[item_id].client_id = match.group('client_id')
                workitems[item_id].start_time = WorkitemLogParser.parse_time(match.group('datetime'))
            elif (match := self._done_expr.match(line)) is not None:
                item_id = int(match.group('workitem_id'))
                workitems[item_id].status = int(match.group('exit_status'))
                end_time = WorkitemLogParser.parse_time(match.group('datetime'))
                workitems[item_id].duration = end_time - workitems[item_id].start_time
        return WorkitemReport(pd.DataFrame(convert_to_dict(workitems.values())).set_index('item_id'))


class WorkitemReport:

    def __init__(self, df):
        self._df = df

    @property
    def raw(self):
        return self._df.copy()

    @property
    def successes(self):
        return list(sorted(self._df.query('status == 0').index))

    @property
    def failures(self):
        return list(sorted(self._df.dropna().query('status != 0').index))

    @property
    def incompletes(self):
        return list(sorted(self._df[self._df['status'].isnull()].index))

    @property
    def walltime_stats(self):
        return self._df.dropna().duration.describe()

    @property
    def client_ids(self):
        return list(self._df['client_id'].unique())

    @property
    def client_stats(self):
        df = self._df.dropna().groupby('client_id').duration.describe()
        df['total'] = self._df.dropna().groupby('client_id').duration.sum()
        incomplete = self._df[self._df.status.isnull()].groupby('client_id').start_time.count()
        print(incomplete)
        df.insert(1, 'incomplete', 0)
        for idx in incomplete.index:
            df.loc[idx, 'incomplete'] = incomplete.loc[idx]
        failure = self._df[self._df.status != 0].dropna().groupby('client_id').status.count()
        df.insert(1, 'failure', 0)
        for idx in failure.index:
            df.loc[idx, 'failure'] = failure.loc[idx]
        success = self._df[self._df.status == 0].groupby('client_id').status.count()
        df.insert(1, 'success', 0)
        for idx in success.index:
            df.loc[idx, 'success'] = success.loc[idx]
        return df
