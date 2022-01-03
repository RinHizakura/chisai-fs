#!/usr/bin/env bash

PARENT_SRC_FILE="../src/chisai-format.c"
RESULT_FILE="a.txt"

cat ${PARENT_SRC_FILE} > ${RESULT_FILE}
diff ${PARENT_SRC_FILE} ${RESULT_FILE} > /dev/null
if [ $? != 0 ]; then
    exit 1
fi
exit 0
