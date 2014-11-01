#!/bin/bash

SELF=$(basename $0)
ORIGINALHOOK="$(dirname $0)/original_${SELF}"
IN="${NEWFILE}.log.in"
OUT="${NEWFILE}.log.out"

# Let it know that we were executed
echo "% Called at $(date +%s%N)" >> ${OUT}

$ORIGINALHOOK < <(tee -a ${IN}) > >(tee -a ${OUT})

EXITCODE=$?
echo "! Exit code: ${EXITCODE}" >> ${OUT}

exit $EXITCODE
