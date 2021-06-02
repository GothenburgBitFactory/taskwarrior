#!/usr/bin/env bash
# Test for TW-1883 (#1896 on Github)
# https://github.com/GothenburgBitFactory/taskwarrior/issues/1896

. bash_tap_tw.sh

task add sample
task +PENDING '(' ')'
task '(' ')'
task '(' '(' ')' ')'
task '(' '(' ')' '(' ')' ')'

# Detect that the task is actually displayed
task +PENDING '(' ')' | grep sample
task '(' ')' | grep sample
task '(' '(' ')' ')' | grep sample
task '(' '(' ')' '(' ')' ')' | grep sample
