#!/usr/bin/env bash

TEST_STR="hello world!"
FROM_FILE="a.txt"
TO_FILE="b.txt"
ans=$'1 13 b.txt'

echo ${TEST_STR} > ${FROM_FILE}
mv ${FROM_FILE} ${TO_FILE}

str=$(ls -lh | awk '{print $2,$5,$9}' | tail -n +2)
if [ "$str" != "$ans" ]; then
    echo $str
    echo $ans
    exit 1
fi
exit 0
