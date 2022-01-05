#!/usr/bin/env bash

FILE_NUM=2
FROM_FILE=("a.txt" "folder/b.txt")
TO_FILE=("1.txt" "2.txt")
CMP_OUTPUT="../output"

rm -rf ${CMP_OUTPUT}
mkdir -p ${CMP_OUTPUT}

mkdir -p "folder"
mkdir -p "${CMP_OUTPUT}/folder"

for ((i=0;i<FILE_NUM;i++)) do
	echo "Random word" > ${FROM_FILE[i]}
	echo "Random word" > ${CMP_OUTPUT}/${FROM_FILE[i]}
done

for ((i=0;i<FILE_NUM;i++)) do
	mv ${FROM_FILE[i]} ${TO_FILE[i]}
	mv ${CMP_OUTPUT}/${FROM_FILE[i]} ${CMP_OUTPUT}/${TO_FILE[i]}
done

ans=$(ls -lh ${CMP_OUTPUT} | awk '{print $2,$5,$9}' | tail -n +2)
str=$(ls -lh | awk '{print $2,$5,$9}' | tail -n +2)
if [ "$str" != "$ans" ]; then
    echo ""
    echo "    The expect answer is ${ans}"
    echo "    But your answer is ${str}"
    exit 1
fi
exit 0
