#!/usr/bin/env bash
. bash_tap_tw.sh

# Setup the tasks
task add wait:1w this should INDEED show up
task add this should NOT show up
sleep 1

# Check that the wait.before filter displays the correct number of tasks
task wait.before:1w all

# Assertion: The task without wait attribute does not show up
[[ -z `task wait.before:1w all | grep NOT` ]]

# Assertion: The task with the wait attribute DOES show up
[[ ! -z `task wait.before:1w all | grep INDEED` ]]

# Assertion: There is exactly one task matching the filter
[[ `task wait.before:1w count` == 1 ]]
