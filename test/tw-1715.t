#!/usr/bin/env bash
. bash_tap_tw.sh

# This adds a task with a wrong due date. If rc.dateformat is set, it works as expected.
task add basetask due:2015-09-03T08:00:00Z

# For seeing the problem in test output; not needed to make the test fail.
task export

task export | grep -E '"due":\s*"20150903T080000Z"'
