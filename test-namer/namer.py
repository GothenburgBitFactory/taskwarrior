""" To use this test namer, navigate to the directory that contains the tests,
    and run it using
        > python3 /path/to/taskwarrior/test-namer/namer.py
"""
import os
import re


def namer():
    """ One-time use function to rename test files from *.t.* to *.test.*,
        affecting files with cpp, py, and sh extensions. """

    for filename in os.listdir():
        # If filename doesn't match pattern *.t.*, do not change it
        if not re.match(r'.*\.t\.(cpp|py|sh)', filename):
            continue

        new_filename = re.sub(r'\.t\.', '.test.', filename)
        os.rename(filename, new_filename)


if __name__ == "__main__":
    namer()
