#!/usr/bin/env bash
# For more information, see https://github.com/wbsch/bash_tap
# Subject to the MIT License. See LICENSE file or http://opensource.org/licenses/MIT
# Copyright (c) 2015-2016 Wilhelm Schürmann

function bashtap_on_error {
    # A command in the parent script failed, interpret this as a test failure.
    # $bashtap_line contains the last executed line, or an error.
    echo -n "$bashtap_output"
    echo "not ok 1 - ${bashtap_line}"
    bashtap_clean_tmpdir
}

function bashtap_run_testcase {
    # Run each line in the parent script up to the first "exit".
    bashtap_output=""
    while IFS= read -r bashtap_line && [ "${bashtap_line:0:4}" != "exit" ]; do
        # Skip shebang.
        if [ "${bashtap_line:0:2}" == "#!" ]; then
            continue
        fi

        # Avoid recursively sourcing this script, and any helper scripts.
        if [[ "$bashtap_line" =~ ^(\.|source).*(/|[[:blank:]])bash_tap[^/]*$ ]]; then
            continue
        fi

        # Include comments as-is.
        if [ "${bashtap_line:0:1}" == "#" ]; then
            bashtap_output+="$bashtap_line"
            bashtap_output+=$'\n'
            continue
        fi

        # Run file line by line.
        if [ ! -z "$bashtap_line" ] && [ "${bashtap_line:0:2}" != "#!" ]; then
            bashtap_output+="# $ $bashtap_line"
            bashtap_output+=$'\n'
            local cmd_output
            local cmd_ret

            eval "$bashtap_line" &> bashtap_out_tmp
            cmd_ret=$?
            cmd_output="$(sed 's/^/# >>> /' < bashtap_out_tmp)"

            if [ ! -z "$cmd_output" ]; then
                bashtap_output+="$cmd_output"
                bashtap_output+=$'\n'
            fi
            if [ "$cmd_ret" -ne 0 ]; then
                exit $cmd_ret
            fi
        fi
    done <"$bashtap_org_script"
}

function bashtap_clean_tmpdir {
    if [ ! -z "$bashtap_tmpdir" ] && [ -d "$bashtap_tmpdir" ]; then
        cd "$bashtap_org_pwd"
        rm -rf "$bashtap_tmpdir"
    fi
    if [ -f bashtap_out_tmp ]; then
        rm bashtap_out_tmp
    fi
}

function bashtap_get_absolute_path {
    # NOTE: No actual thought put into this. Might break. Horribly.
    # Using this instead of readlink/realpath for OSX compatibility.
    echo $(cd "$(dirname "$1")" && pwd)/$(basename "$1")
}


bashtap_org_pwd=$(pwd)
bashtap_org_script=$(bashtap_get_absolute_path "$0")

if [ "${0:(-2)}" == ".t" ] || [ "$1" == "-t" ]; then
    # Make sure any failing commands are caught.
    set -e
    set -o pipefail

    # TAP header. Hardcoded number of tests, 1.
    echo "1..1"

    # Output TAP failure on early exit.
    trap bashtap_on_error EXIT

    # The different calls to mktemp are necessary for OSX compatibility.
    bashtap_tmpdir=$(mktemp -d 2>/dev/null || mktemp -d -t 'bash_tap')
    if [ ! -z "$bashtap_tmpdir" ]; then
        cd "$bashtap_tmpdir"
    else
        bashtap_line="Unable to create temporary directory."
        exit 1
    fi

    # Scripts sourced before bash_tap.sh may declare this function.
    if declare -f bashtap_setup >/dev/null; then
        bashtap_setup
    fi

    # Run test file interpreting failing commands as a test failure.
    bashtap_run_testcase && echo "ok 1"

    # Since we're in a sourced file and just ran the parent script,
    # exit without running it a second time.
    trap - EXIT
    bashtap_clean_tmpdir
    exit
else
    if declare -f bashtap_setup >/dev/null; then
        bashtap_setup
    fi
fi
