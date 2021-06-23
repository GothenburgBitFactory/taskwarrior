#!/usr/bin/env bash
. bash_tap_tw.sh

# Import a task with annotation without an description
# Should fail
OUTPUT=`echo '{"description":"Buy the milk","annotations":[{"entry": 1234567890}]}' | task import - 2>&1` || :
[[ $OUTPUT =~ "missing a description" ]]

# Check that the task was NOT added
[[ `task count` == 0 ]]

# Import a task with annotation without an entry
echo '{"description":"Buy the milk","annotations":[{"description":"and Cheese"}]}' | task import -

# Check that the task was added
[[ `task count` == 1 ]]
[[ `task milk count` == 1 ]]
[[ `task _get 1.annotations.count` == 1 ]]
