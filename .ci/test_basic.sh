#!/usr/bin/env bash

TEST_STR="hello world!"
RESULT_FILE="a.txt"

echo ${TEST_STR} > ${RESULT_FILE}
str=$(cat ${RESULT_FILE})
if [ "$str" != "${TEST_STR}" ]; then
    exit 1
fi
exit 0
