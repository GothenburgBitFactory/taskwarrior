#!/bin/bash

SELF=$(basename $0)
ORIGINALHOOK="$(dirname $0)/original_${SELF}"
IN="${ORIGINALHOOK}.log.in"
OUT="${ORIGINALHOOK}.log.out"

# Let it know that we were executed
echo "% Called at $(date +%s%N)" >> ${IN}

$ORIGINALHOOK < <(tee -a ${IN}) > >(tee -a ${OUT})

EXITCODE=$?
echo "! Exit code: ${EXITCODE}" >> ${OUT}

exit $EXITCODE
