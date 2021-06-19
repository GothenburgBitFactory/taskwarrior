#!/usr/bin/env bash
. bash_tap_tw.sh

# Setup the tasks, urgent ones and unimportant
for i in `seq 1 5`; do task add unimportant task $i; done
for i in `seq 1 5`; do task add important task $i due:today+${i}d; done

# Complete all the tasks. Since the highest priority task was completed,
# nagging should not happen.
NAGGING_HAPPENS=`task rc.bulk:0 1-10 done 2>&1 | grep 'more urgent'` || :

# Nagging should not have happened
[[ -z $NAGGING_HAPPENS ]]
