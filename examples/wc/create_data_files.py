#!/usr/bin/env python

import argparse
import random
import pathlib
import string


MAX_WORD_LENGTH = 10


def create_file(file_name: str, max_nr_words: int, nr_columns: int) -> None:
    """Create a file with random words.

    Parameters
    ----------
    file_name : str
        The name of the file to create.
    max_nr_words : int
        The maximum number of words in the file.
    nr_columns : int
        The maximum number of columns in the file.
    """
    nr_words = random.randint(1, max_nr_words)
    with open(file_name, "w") as file:
        while nr_words > 0:
            words = []
            line_length = 0
            max_word_length = min(MAX_WORD_LENGTH, nr_columns)
            while line_length < nr_columns:
                word_length = random.randint(1, max_word_length)
                word = "".join(
                    random.choices(string.ascii_lowercase, k=word_length)
                )
                words.append(word)
                line_length += len(word) + 1
            if line_length > nr_columns:
                words.pop()
            file.write(" ".join(words) + "\n")
            nr_words -= len(words)


def main() -> None:
    arg_parser = argparse.ArgumentParser(description="Create data files")
    arg_parser.add_argument(
        "--nr-files", type=int, default=10, help="Number of files to create"
    )
    arg_parser.add_argument(
        "--max-words",
        type=int,
        default=100,
        help="Maximum number of words per file",
    )
    arg_parser.add_argument(
        "--max-columns",
        type=int,
        default=80,
        help="Maximum number of columns per file",
    )
    arg_parser.add_argument(
        "--output-dir", type=str, default=".", help="Output directory"
    )
    arg_parser.add_argument(
        "--seed", type=int, default=1234, help="Random seed"
    )
    options = arg_parser.parse_args()

    # Seed the random number generator
    random.seed(options.seed)

    # Create the output directory if it does not exist
    output_dir = pathlib.Path(options.output_dir)
    if not output_dir.exists():
        output_dir.mkdir(parents=True)

    # Create the files
    for file_nr in range(1, 1 + options.nr_files):
        file_name = output_dir / f"file_{file_nr:04d}.txt"
        create_file(file_name, options.max_words, options.max_columns)


if __name__ == "__main__":
    main()
