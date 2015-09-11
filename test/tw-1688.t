#!/usr/bin/env bash
. bash_tap_tw.sh

# TW-1688 task fails to import
#   The problem is when a completed task, with a dependency is exported, then
#   imported after the data is removed. On import, the circular dependency
#   check didn't notice that a UUID failed to exist, and generated a JSON error.
#
#   Although an unusual circumstance, people do delete data from their
#   completed.data file.

task add one
task log two depends:1

task /two/ export > JSON

rm pending.data completed.data
task import JSON
