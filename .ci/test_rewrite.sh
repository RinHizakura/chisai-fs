#!/usr/bin/env bash

PARENT_SRC_FILE="../src/main.c"
RESULT_FILE="a.txt"

echo "hello" > ${RESULT_FILE}
cat ${PARENT_SRC_FILE} > ${RESULT_FILE}
diff ${PARENT_SRC_FILE} ${RESULT_FILE} > /dev/null
if [ $? != 0 ]; then
    exit 1
fi
exit 0
