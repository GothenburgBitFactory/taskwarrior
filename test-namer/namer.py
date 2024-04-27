import os


def namer():
    """ One-time use function to rename test files to include .sh or .py """

    for filename in os.listdir():
        # If filename doesn't end with .t, do not change it
        if filename[-2:] != '.t':
            continue

        with open(filename, "r") as file:
            first_line = str(file.readline())
            new_filename = filename

            # Identify test file types based on their shebangs
            if first_line == "#!/usr/bin/env python3\n":
                new_filename += '.py'

            if first_line == "#!/usr/bin/env bash\n":
                new_filename += '.sh'

            os.rename(filename, new_filename)


if __name__ == "__main__":
    namer()
