import pytest
from worker.data_sources import RangeDataSource

def collect_values(results):
    var_names = {var_name for var_name in results[0].keys()}
    var_values = {var_name: list() for var_name in var_names}
    for result in results:
        assert {var_name for var_name in result.keys()} == var_names
        for var_name, value in result.items():
            var_values[var_name].append(value)
    return var_values


def test_single():
    var_name = 'x'
    range_str = '9'
    expected = ['9']
    results = [value for value in RangeDataSource(var_name, range_str)]
    assert len(results) == 1
    var_values = collect_values(results)
    assert var_values[var_name] == expected

def test_list():
    var_name = 'x'
    range_str = '9,10,14'
    expected = ['9', '10', '14']
    results = [value for value in RangeDataSource(var_name, range_str)]
    assert len(results) == len(expected)
    var_values = collect_values(results)
    assert var_values[var_name] == expected

def test_single_range():
    var_name = 'x'
    range_str = '9-14'
    expected = [str(i) for i in range(9, 15)]
    results = [value for value in RangeDataSource(var_name, range_str)]
    assert len(results) == len(expected)
    var_values = collect_values(results)
    assert var_values[var_name] == expected

def test_values_single_range():
    var_name = 'x'
    range_str = '5,6,9-14'
    expected = ['5', '6'] + [str(i) for i in range(9, 15)]
    results = [value for value in RangeDataSource(var_name, range_str)]
    assert len(results) == len(expected)
    var_values = collect_values(results)
    assert var_values[var_name] == expected

def test_single_range_values():
    var_name = 'x'
    range_str = '9-14,5,6'
    expected =  [str(i) for i in range(9, 15)] + ['5', '6']
    results = [value for value in RangeDataSource(var_name, range_str)]
    assert len(results) == len(expected)
    var_values = collect_values(results)
    assert var_values[var_name] == expected

def test_two_ranges():
    var_name = 'x'
    range_str = '9-14,18-23'
    expected =  [str(i) for i in range(9, 15)] + [str(i) for i in range(18, 24)]
    results = [value for value in RangeDataSource(var_name, range_str)]
    assert len(results) == len(expected)
    var_values = collect_values(results)
    assert var_values[var_name] == expected

def test_two_ranges_values():
    var_name = 'x'
    range_str = '5,9-14,16,18-23,30'
    expected =  ['5'] + [str(i) for i in range(9, 15)] + ['16'] + [str(i) for i in range(18, 24)] + ['30']
    results = [value for value in RangeDataSource(var_name, range_str)]
    assert len(results) == len(expected)
    var_values = collect_values(results)
    assert var_values[var_name] == expected

def test_empty_range():
    var_name = 'x'
    range_str = '9-5'
    expected =  []
    results = [value for value in RangeDataSource(var_name, range_str)]
    assert len(results) == len(expected)

def test_invalid_range():
    var_name = 'x'
    range_str = '5+9'
    with pytest.raises(ValueError):
        _ = RangeDataSource(var_name, range_str)
