#!/usr/bin/env bash

function prologue()
{
        bash ./scripts/format-image.sh
	if [ $? != 0 ] ; then
    		exit 1
	fi
	mkdir -p mount
	bash ./scripts/mount_image.sh
	if [ $? != 0 ] ; then
    		exit 1
	fi
	cd mount
}

function main()
{
	RETURN=0
	FILES=$(ls ../.ci | grep '^test')
	for FILE in ${FILES}; do
		bash ../.ci/$FILE
		if [ $? != 0 ] ; then
    			RETURN=1
		fi
	done
	return $RETURN
}

function epilogue()
{
	cd ..
	sleep 1
	bash ./scripts/umount_image.sh
	exit $1
}

prologue
main
epilogue $?
