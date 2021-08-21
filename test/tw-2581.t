#!/usr/bin/env bash
# Test setting configuration variable with a trailing comment works
. bash_tap_tw.sh

# Add configuration variable with a trailing comment into taskrc
echo 'weekstart=Monday  # Europe standard' >> taskrc
cat taskrc

# Use config the change the value to "Sunday"
task config weekstart Sunday

# Ensure the comment was preserved and value changed
cat taskrc | grep weekstart=Sunday
[[ `cat taskrc | grep weekstart=Sunday` == 'weekstart=Sunday  # Europe standard' ]]
