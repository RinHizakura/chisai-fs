#!/usr/bin/env bash

echo $PWD
. .ci/common.sh

function test_basic()
{
	echo "hello world!" > "hi.txt"
	str=$(cat hi.txt)
	if [ "$str" != "hello world!" ]; then
    		exit 1
	fi
}

run ./scripts/format-image.sh
mkdir -p mount
run ./scripts/mount_image.sh
cd mount
test_basic
cd ..
run ./scripts/umount_image.sh
exit 0
