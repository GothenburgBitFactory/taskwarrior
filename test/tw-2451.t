#!/usr/bin/env bash
. bash_tap_tw.sh

# Setup the tasks
task add Participate in the marine search
task add Find the pearls

# Search for tasks containing "mar"
task mar
task mar count
[[ `task mar count` == 1 ]]
