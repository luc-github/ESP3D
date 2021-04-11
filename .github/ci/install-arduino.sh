#!/bin/bash
# Exit immediately if a command exits with a non-zero status.
set -e
# Enable the globstar shell option
shopt -s globstar

wget http://downloads.arduino.cc/arduino-1.8.13-linux64.tar.xz

tar xf arduino-1.8.13-linux64.tar.xz

mv arduino-1.8.13 $HOME/arduino_ide

