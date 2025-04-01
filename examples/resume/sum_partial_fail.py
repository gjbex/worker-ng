#!/usr/bin/env python

import argparse
import sys


arg_parser = argparse.ArgumentParser(description='sum two numbers, fail half the time')
arg_parser.add_argument('x', type=float, help='left operand')
arg_parser.add_argument('y', type=float, help='right operand')
arg_parser.add_argument('array_id', type=int, help='array id')
args = arg_parser.parse_args()

if args.array_id % 2 == 1:
    print(f'{args.x} + {args.y} failed', file=sys.stderr)
    sys.exit(1)
else:
    print(args.x + args.y)
