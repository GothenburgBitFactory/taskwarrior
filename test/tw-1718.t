#!/usr/bin/env bash
. bash_tap_tw.sh

echo "uda.foo.label=foo" >> taskrc
echo "uda.foo.type=string" >> taskrc

# This sets foo to "PT13H" despite it being a string UDA
task add bar foo:"3h+10h"

# Show the problem in TAP output
task _get 1.foo

task _get 1.foo | grep '3h+10h'
