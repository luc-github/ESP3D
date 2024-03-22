#!/bin/bash
# Exit immediately if a command exits with a non-zero status.
set -e
# Enable the globstar shell option
shopt -s globstar
ls $HOME
# Make sure we are inside the github workspace
cd $GITHUB_WORKSPACE
mkdir -p $HOME/arduino_ide/libraries/SSDP
cp -R ./src $HOME/arduino_ide/libraries/SSDP
cp ./library.properties $HOME/arduino_ide/libraries/SSDP

