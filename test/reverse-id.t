#! /usr/bin/env bash
. bash_tap_tw.sh

task add First task
first_uuid="$(task _get 1.uuid)"

task add Second task
second_uuid="$(task _get 2.uuid)"

# Check that the order isn't messed up on normal settings
[[ "$(task _get "${first_uuid}".id )" == 1  ]]
[[ "$(task _get "${second_uuid}".id )" == 2  ]]


# Check that the order is reversed
[[ "$(task rc.reverse_id:true _get "${first_uuid}".id )" == 2  ]]
[[ "$(task rc.reverse_id:true _get "${second_uuid}".id )" == 1  ]]

# Check that modifying a task doesn't change it
task "${first_uuid}" modify "modified title"

[[ "$(task _get "${first_uuid}".id )" == 1  ]]
[[ "$(task _get "${second_uuid}".id )" == 2  ]]
[[ "$(task rc.reverse_id:true _get "${first_uuid}".id )" == 2  ]]
[[ "$(task rc.reverse_id:true _get "${second_uuid}".id )" == 1  ]]

task add Third task
[[ "$(task _get "${first_uuid}".id )" == 1  ]]
[[ "$(task _get "${second_uuid}".id )" == 2  ]]
[[ "$(task rc.reverse_id:true _get "${first_uuid}".id )" == 3  ]]
[[ "$(task rc.reverse_id:true _get "${second_uuid}".id )" == 2  ]]
