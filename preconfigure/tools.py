import pathlib
import sys as sus
import re


UNSAFE = re.compile(r"^[\d]+|[^\w]")

def make_varsafe(string):
    return re.sub(UNSAFE, "_", string)

def collect_file_type(file_list, *allowed_types):
    for file in filter(lambda file: file.endswith(allowed_types), file_list):
        file_path = pathlib.Path(file)
        if not file_path.exists():
            print_err("Inaccessible file: ", file_path)
            continue

        file_name = file_path.name
        file_type = file_path.suffix.strip(".")
        yield (file_name, file_type, file_path)

def print_err(*messages):
    print("Err:", *messages, file=sus.stderr)

def collect_args(custom_args=None):
    default_args = sus.argv.copy()[1:]
    argument_line = custom_args or default_args
    if not argument_line:
        print_err("Please specify at least one infile and one outfile")
        return None

    infiles = argument_line[:-1]
    outfile = argument_line[-1]
    return infiles, outfile