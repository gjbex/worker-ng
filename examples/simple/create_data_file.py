#!/usr/bin/env python

import argparse
import random

def main():
    arg_parser = argparse.ArgumentParser(description='create data file')
    arg_parser.add_argument('--nr-of-items', type=int, default=100,
                            help='number of data items to generate')
    arg_parser.add_argument('--seed', type=int, default=1234,
                            help='seed for the random number generator')
    options = arg_parser.parse_args()

    with open('data.csv', 'w') as file:
        print('source,dest', file=file)
        for _ in range(options.nr_of_items):
            source, dest  = random.randrange(100), random.randrange(100)
            print(f'{source},{dest}', file=file)

if __name__ == '__main__':
    main()
