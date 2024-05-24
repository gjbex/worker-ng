#!/usr/bin/env python

import argparse
import sys

def main():
    arg_parser = argparse.ArgumentParser(description='sum two values')
    arg_parser.add_argument('a', type=float, required=True,
                            help='A value')
    options = arg_parser.parse_args()
    print(options.a**2)
    return 0

if __name__ == '__main__':
    sys.exit(main())
