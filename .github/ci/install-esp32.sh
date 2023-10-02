#!/bin/bash
# Exit immediately if a command exits with a non-zero status.
set -e
# Enable the globstar shell option
shopt -s globstar

#install pyserial
echo "Installing Python Serial ..."
pip install pyserial

echo "Clone esp32 core"
cd $HOME/arduino_ide/hardware
mkdir esp32
cd esp32
git clone https://github.com/espressif/arduino-esp32.git esp32
cd esp32
git submodule update --init
cd tools
python get.py
