#!/usr/bin/env bash
. bash_tap_tw.sh

# Setup the tasks
task add Something I did yesterday
task 1 mod start:yesterday+18h
task 1 done end:yesterday+20h

# Check that 2 hour interval is reported by task info
task info | grep -F "Start deleted"
[[ ! -z `task info | grep -F "Start deleted (duration: 2:00:00)."` ]]
