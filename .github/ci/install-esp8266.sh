#!/bin/bash
# Exit immediately if a command exits with a non-zero status.
set -e
# Enable the globstar shell option
shopt -s globstar

cd $HOME/arduino_ide/hardware
mkdir esp8266com
cd esp8266com
git clone -b 3.0.2 https://github.com/esp8266/Arduino.git esp8266
cd esp8266
git submodule update --init
cd tools
python get.py
