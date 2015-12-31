#!/usr/bin/env bash
# This file only contains helper functions for making Taskwarrior testing
# easier. The magic happens in bash_tap.sh sourced at the end of this file.
#
# "task" is a bash function calling "/path/to/compiled/task rc:taskrc"
# Only local paths are searched, see bash_tap_tw.sh:find_task_binary().
#
# "taskrc" is a file set up in bash_tap_tw.sh:setup_taskrc(), and can be
# appended to or changed as needed.
#
# Subject to the MIT License. See LICENSE file or http://opensource.org/licenses/MIT
# Copyright (c) 2015-2016 Wilhelm Schürmann

function setup_taskrc {
    # Configuration
    for i in pending.data completed.data undo.data backlog.data taskrc; do
       if [ -f "$i" ]; then
           rm "$i" 2>&1 >/dev/null
       fi
    done

    export TASKDATA=.

    echo 'confirmation=off'               > taskrc
    echo 'color.debug=rgb025'             >> taskrc
    echo 'color.header=rgb025'            >> taskrc
    echo 'color.footer=rgb025'            >> taskrc
    echo 'color.error=bold white on red'  >> taskrc
}

function find_task_binary {
    # $bashtap_org_pwd is set in bash_tap.sh. It is the directory the parent script is
    # run from. Check for the task binary relative to that directory.
    # Do not use the system "task" if no local one is found, error out instead.
    for t in "${bashtap_org_pwd}/task" "${bashtap_org_pwd}/src/task" "${bashtap_org_pwd}/../task" "${bashtap_org_pwd}/../src/task" "${bashtap_org_pwd}/../build/src/task"; do
        if [ -f "$t" ] && [ -x "$t" ]; then
            t_abs=$(bashtap_get_absolute_path "$t")
            eval "function task { ${t_abs} rc:taskrc \"\$@\"; }"
            return 0
        fi
    done

    echo "# ERROR: Could not find task binary!"

    # Needed for normal, i.e. "non-test" mode.
    eval "function task { exit; }"

    # Set $line so we can have useful TAP output.
    line="bash_tap.sh:find_task_binary()"

    return 1
}

function bashtap_setup {
    # This function is called by bash_tap.sh before running tests, or before
    # running the parent script normally.
    find_task_binary
    setup_taskrc
}


# Include the base script that does the actual work.
source bash_tap.sh
