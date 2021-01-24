#!/usr/bin/env bash
. bash_tap_tw.sh

# This sets foo to "PT13H" despite it being a string UDA
task add test project:c.vs.2021-01

# Show the problem in TAP output
task rc.debug.parser:3 project:c.vs.2021-01 _ids

[[ `task project:c.vs.2021-01 _ids` == "1" ]]
