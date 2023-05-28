#!/usr/bin/env bash
. bash_tap_tw.sh

task add modtest modified:yesterday
old_modified=`task _get 1.modified`
echo $old_modified

task 1 start
new_modified=`task _get 1.modified`
echo $new_modified

# `task start` should have updated modified
[[ $old_modified != $new_modified ]]
