#!/usr/bin/env bash
. bash_tap_tw.sh

# Setup the context
task config context.work.read '+work'
task config context.work.write '+work'

# Create a task outside of the context
task add outside

# Activate the context
task context work

# Add multiple tasks within the context, some of which contain numbers or uuids
task add inside
task add inside 2
task add inside 3
task add inside aabbccdd
task add inside 4-5

# Assertion: Task defined outside of the context should not show up
[[ -z `task all | grep outside` ]]

# Five tasks were defined within the context
task count
[[ `task count` == "5" ]]

# Unset the context
task context none

# Exactly five tasks have the tag work
task +work count
[[ `task +work count` == "5" ]]
