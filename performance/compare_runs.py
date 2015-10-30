#!/usr/bin/env python
#
# Compare performance of two Taskwarrior versions.
#
# Run without arguments for usage information.
#
# "Perf task" lines in debug output differing between the two versions is
# poorly handled, if at all.  Best used for comparing performance between two
# similar Taskwarrior versions only differing in performance related code.
#
# "total" will probably not be the sum of the shown individual parts, but
# slightly larger since not all minimum values will have been present in the
# same run.
#

import collections
import re
import sys

# Adjust if more performance tests are added
COMMANDS = "next list all add export import".split()

TaskPerf = collections.namedtuple("TaskPerf", "version commit at timing")


def parse_perf(input):
    tests = {}
    for command in COMMANDS:
        tests[command] = []

        # Parse concatenated run_perf output
        for i in re.findall("^  - task %s\.\.\.\n"
                            "Perf task ([^ ]+) ([^ ]+) ([^ ]+) (.+)$"
                            % command, input, re.MULTILINE):
            info = i[0:3] + ({k:v for k, v in (i.split(":") for i in i[-1].split())},)
            pt = TaskPerf(*info)
            tests[command].append(pt)
    return tests


def get_best(tests):
    best = {}
    for command in tests:
        best[command] = {}
        for k in tests[command][0].timing:
            best[command][k] = str(min(int(t.timing[k]) for t in tests[command]))
    return best


if len(sys.argv) != 3:
    print("Usage:")
    print(" $ %s file1 file2" % sys.argv[0])
    print("Where file1, file2 are generated as such:")
    print(" $ for i in `seq 20`; do ./run_perf >> filename 2>&1; done")
    sys.exit(1)

with open(sys.argv[1], "r") as fh:
    tests_prev = parse_perf(fh.read())
    best_prev = get_best(tests_prev)
with open(sys.argv[2], "r") as fh:
    tests_cur = parse_perf(fh.read())
    best_cur = get_best(tests_cur)

print("Previous: %s (%s)" % (tests_prev[COMMANDS[0]][0].version, tests_prev[COMMANDS[0]][0].commit))
print("Current:  %s (%s)" % (tests_cur[COMMANDS[0]][0].version, tests_cur[COMMANDS[0]][0].commit))

for test in COMMANDS:
    print("# %s:" % test)

    out = ["" for i in range(5)]
    for k in sorted(best_prev[test].keys()):
        diff = str(int(best_cur[test][k]) - int(best_prev[test][k]))

        if float(best_prev[test][k]) > 0:
            percentage = str(int((float(diff) / float(best_prev[test][k]) * 100))) + "%"
        else:
            percentage = "0%"

        pad = max(map(len, (k, best_prev[test][k], best_cur[test][k], diff, percentage)))
        out[0] += " %s" % k.rjust(pad)
        out[1] += " %s" % best_prev[test][k].rjust(pad)
        out[2] += " %s" % best_cur[test][k].rjust(pad)
        out[3] += " %s" % diff.rjust(pad)
        out[4] += " %s" % percentage.rjust(pad)
    for line in out:
        print(line)
