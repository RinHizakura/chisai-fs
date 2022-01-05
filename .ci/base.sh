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
	r=$'\r'
	RED='\033[0;31m'
	GREEN='\033[0;32m'
	NC='\033[0m' # No Color

	FILES=$(ls ../.ci | grep '^test')
	for FILE in ${FILES}; do
	        rm -rf ./*
	    	echo -n "run test: ${FILE}..."
		bash ../.ci/$FILE
		if [ $? != 0 ] ; then
			echo -e "${r}run test: ${FILE}... ${RED}fail${NC}"
    			RETURN=1
			break
		fi
		echo -e "${r}run test: ${FILE}... ${GREEN}pass${NC}"
	done
	return $RETURN
}

function epilogue()
{
	cd ..
	sleep 1
	bash ./scripts/umount_image.sh
	echo "exit: status $1"
	exit $1
}

prologue
main
epilogue $?
