#!/usr/bin/env python3
import os
import argparse
import datetime
import subprocess
from typing import List

def ymd():
    return datetime.datetime.now().strftime("%Y-%m-%d")

def git_current_branch() -> str :
    out = subprocess.check_output(["git", "branch", "--show-current"])
    return out.strip().decode("utf-8")

def get_dir() -> str:
    here = os.path.dirname(os.path.abspath(__file__))
    return os.path.join(
        here,
        "../.changelogs")

def get_changefiles() -> List[str]:
    changedir = get_dir()
    changefiles = []
    for f in os.listdir(changedir):
        if f.endswith(".md") and not f.startswith("."):
            changefiles.append(os.path.join(changedir, f))

    return changefiles

def cmd_add(args):
    text = args.text.strip()
    if not text.startswith("- "):
        text = "- %s" % text

    timestamp = ymd()
    branchname = git_current_branch()
    fname = os.path.join(get_dir(), "%s-%s.md" % (timestamp, branchname))
    with open(fname, "a") as f:
        f.write(text)
        f.write("\n")

def cmd_build(args):
    print("## x.y.z - %s" % (ymd()))
    for e in get_changefiles():
        print(open(e).read().strip())

def main() -> None:
    parser = argparse.ArgumentParser()
    subparsers = parser.add_subparsers(title='Sub commands', dest='command')
    subparsers.required = True

    parser_add = subparsers.add_parser('add')
    parser_add.add_argument("text")
    parser_add.set_defaults(func=cmd_add)

    parser_build = subparsers.add_parser('build')
    parser_build.set_defaults(func=cmd_build)

    args = parser.parse_args()
    args.func(args)


if __name__ == "__main__":
    main()
