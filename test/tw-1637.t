#!/usr/bin/env bash
. bash_tap_tw.sh

# This problems only appears for "modify", not for "add"
task add a
task 1 mod proj:mod
task 1 _project | grep mod
