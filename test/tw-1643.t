#!/usr/bin/env bash
source bash_tap_tw.sh

task add TW-1643 pro:YDKJS +work
task context define work +work
task context work
task pro:YDKJS mod prio:M
task all | grep TW-1643
