#!/usr/bin/env bash
. bash_tap_tw.sh

task add emptyval
task 1 done
task 1 mod end: status:pending
task_end=`task 1 info | grep ^End | sed -e 's/^End //' || true`
echo "task_end: $task_end"

# `task mod end:` should have deleted the end.
[[ "$task_end" == "" ]]
