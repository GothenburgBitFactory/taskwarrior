#!/usr/bin/env bash

. bash_tap_tw.sh

task add first project:someday
task add second project:bar

task project:someday list | grep first
