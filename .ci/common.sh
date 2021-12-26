#!/usr/bin/env bash

function run()
{
	$1
	r=$?
	if [ $r != 0 ] ; then
    		exit $r
	fi
}
