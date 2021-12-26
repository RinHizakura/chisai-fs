#!/usr/bin/env bash

source scripts/common.sh

function test_basic()
{
	cd mount
	echo "hello world!" > "hi.txt"
	str=$(cat hi.txt)
	if [ "$str" != "hello world!" ]; then
    		exit 1
	fi
}

run ./scripts/umount_image.sh
run ./scripts/format-image.sh
mkdir mount
run ./scripts/mount_image.sh
test_basic
exit 0
