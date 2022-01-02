#!/usr/bin/env bash

echo "hello world!" > "hi.txt"
str=$(cat hi.txt)
if [ "$str" != "hello world!" ]; then
    exit 1
fi
exit 0
