#!/usr/bin/env bash

FILE_NUM=2
PARENT_SRC_FILE=("../src/chisai-format.c" "../src/chisai-mount.c")
RESULT_FILE=("1.txt" "2.txt")

for ((i=0;i<FILE_NUM;i++)) do
    cat ${PARENT_SRC_FILE[i]} > ${RESULT_FILE[i]}
    diff ${PARENT_SRC_FILE[i]} ${RESULT_FILE[i]} > /dev/null
    if [ $? != 0 ]; then
	echo "fail for the source file ${PARENT_SRC_FILE[i]}"
        exit 1
    fi
done
exit 0
