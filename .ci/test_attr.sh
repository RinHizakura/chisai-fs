#!/usr/bin/env bash

rm -rf ./*
echo "hello world!" > "hi.txt"
echo "RinHizakura" > "whoami.txt"
ans=$'1 13 hi.txt\n1 12 whoami.txt'

str=$(ls -lh | awk '{print $2,$5,$9}' | tail -n +2)
if [ "$str" != "$ans" ]; then
    exit 1
fi
exit 0
