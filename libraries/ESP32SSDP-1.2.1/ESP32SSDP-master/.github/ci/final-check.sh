#!/bin/bash
# Exit immediately if a command exits with a non-zero status.
set -e
# Enable the globstar shell option
shopt -s globstar
# Make sure we are inside the github workspace
cd $GITHUB_WORKSPACE
echo $STEPS_CONTEXT
step=$1
status=$2

if [[ "$status" == "success" ]];
then
    echo "Success build"
    exit 0
 else
    echo "Build failed"
    exit 1
fi


