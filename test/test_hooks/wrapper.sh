#!/usr/bin/env bash

SELF=$(basename $0)
ORIGINALHOOK="$(dirname $0)/original_${SELF}"
IN="${ORIGINALHOOK}.log.in"
OUT="${ORIGINALHOOK}.log.out"

# Let it know that we were executed
echo "% Called at $(python -c 'import time; print(time.time())') with '$@'" >> ${IN}

# Log what arrives via stdin to ${IN} and what comes via stdout to ${OUT}
$ORIGINALHOOK "$@" < <(tee -a ${IN}) > >(tee -a ${OUT})
# More on the < <() syntax at: http://tldp.org/LDP/abs/html/process-sub.html

EXITCODE=$?
echo "! Exit code: ${EXITCODE}" >> ${OUT}

exit $EXITCODE
