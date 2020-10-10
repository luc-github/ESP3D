#!/bin/bash

function build_sketch()
{
	local sketch=$1

	# buld sketch with arudino ide
	echo -e "\n Build $sketch \n"
	arduino --verbose --verify $sketch

	# get build result from arduino
	local re=$?

	# check result
	if [ $re -ne 0 ]; then
		echo "Failed to build $sketch"
		return $re
	fi
}
