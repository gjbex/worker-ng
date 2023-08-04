#!/usr/bin/env python

import argparse
import random


def main():
    arg_parser = argparse.ArgumentParser(description='create a data file')
    arg_parser.add_argument('--nr-work-items', type=int, default=100,
                            help='number of work items to generate')
    arg_parser.add_argument('--failure-rate', type=float, default=0.2,
                            help='failure rate, 0.0 for no failures, 1.0 for all failures')
    options = arg_parser.parse_args()

    with open('data.csv', 'w') as file:
        print('a,b,fails', file=file)
        for _ in range(options.nr_work_items):
            a, b = random.random(), random.random()
            fails = random.choices([0, 1], weights=[1.0 - options.failure_rate, options.failure_rate])[0]
            print(f'{a},{b},{fails}', file=file)

if __name__ == '__main__':
    main()
