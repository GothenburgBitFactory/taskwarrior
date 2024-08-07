#!/usr/bin/env bash
# A test case for TW-2124.
# https://github.com/GothenburgBitFactory/taskwarrior/issues/2124

. bash_tap_tw.sh

# Filtering for description with a dash works
task add foo-bar
task foo-bar list | grep foo-bar
