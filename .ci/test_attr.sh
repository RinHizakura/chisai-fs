#!/usr/bin/env bash

FILE_NUM=2
TEST_STR=("hello world!" "RinHizakura")
RESULT_FILE=("a.txt" "b.txt")
CMP_OUTPUT="../output"

rm -rf ${CMP_OUTPUT}
mkdir -p ${CMP_OUTPUT}
for ((i=0;i<FILE_NUM;i++)) do
	echo ${TEST_STR[i]} > ${RESULT_FILE[i]}
	echo ${TEST_STR[i]} > ${CMP_OUTPUT}/${RESULT_FILE[i]}
done

ans=$(ls -lh ${CMP_OUTPUT} | awk '{print $2,$5,$9}' | tail -n +2)
str=$(ls -lh | awk '{print $2,$5,$9}' | tail -n +2)
if [ "$str" != "$ans" ]; then

    exit 1
fi
exit 0
