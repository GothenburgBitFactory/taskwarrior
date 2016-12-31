#!/bin/bash

REFERENCE_LANGUAGE_FILE="eng-USA.h"
TESTED_LANGUAGE_FILES=`ls *.h | grep -v $REFERENCE_LANGUAGE_FILE`

# At the beginning, we haven't detected any invalid translation files
MISMATCHED=0

# Generate list of keys, not including defines that are commented out. Strips out the leading whitespace.
cat $REFERENCE_LANGUAGE_FILE | grep "^[[:space:]]*#define" | sed -e 's/^ *//' - | cut -f2 -d' ' | sort > identifiers

# Generate report
for LANGUAGE_FILE in $TESTED_LANGUAGE_FILES
do
    echo "Comparing $REFERENCE_LANGUAGE_FILE (left) to $LANGUAGE_FILE (right)"
    cat $LANGUAGE_FILE | grep "^[[:space:]]*#define" | sed -e 's/^ *//' - | cut -f2 -d' ' | sort | diff identifiers -
    MISMATCHED=$(($MISMATCHED+$?))
    echo ""
done

# Cleanup
rm -f identifiers

# Exit with number of not synced translations files
exit $MISMATCHED
