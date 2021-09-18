#!/usr/bin/env bash
# This tests the migration path from 2.5.3 or earlier to 2.6.0 with respect to
# the upgrade of the status field from waiting to pending
. bash_tap_tw.sh

# Setup
task add Actionable task wait:yesterday
task add Non-actionable task wait:tomorrow+1h

# Simulate this was created in 2.5.3 or earlier (status is equal to waiting,
# not pending). Using more cumbersome sed syntax for Mac OS-X compatibility.
sed -i".bak" 's/pending/waiting/g' $TASKDATA/pending.data
rm -f $TASKDATA/pending.data.bak

# Trigger upgrade
task all

# Report file content
echo pending.data
cat $TASKDATA/pending.data
echo completed.data
cat $TASKDATA/completed.data

# Assertion: Exactly one task is considered waiting
[[ `task +WAITING count` == "1" ]]
[[ `task status:waiting count` == "1" ]]

# Assertion: Exactly one task is considered pending
[[ `task +PENDING count` == "1" ]]
[[ `task status:pending count` == "1" ]]

# Assertion: Task 1 is pending
[[ `task _get 1.status` == "pending" ]]

# Assertion: Task 2 is waiting
[[ `task _get 2.status` == "waiting" ]]

# Assertion: No lines in data files with "waiting" status
[[ -z `cat $TASKDATA/pending.data | grep waiting` ]]
[[ -z `cat $TASKDATA/completed.data | grep waiting` ]]

# Assertion: No tasks were moved into completed.data
cat $TASKDATA/pending.data | wc -l | tr -d ' '
[[ `cat $TASKDATA/pending.data | wc -l | tr -d ' '` == "2" ]]
[[ `cat $TASKDATA/completed.data | wc -l | tr -d ' '` == "0" ]]
