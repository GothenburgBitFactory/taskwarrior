#!/usr/bin/env bash
. bash_tap_tw.sh

task add "foo \' bar"
task list

# Assert the task was correctly added
[[ ! -z `task list | grep "foo ' bar"` ]]
[[ `task _get 1.description` == "foo ' bar" ]]

# Bonus: Assert escaped double quotes are also handled correctly
task add 'foo \" bar'
task list

# Assert the task was correctly added
[[ ! -z `task list | grep 'foo " bar'` ]]
[[ `task _get 2.description` == 'foo " bar' ]]
