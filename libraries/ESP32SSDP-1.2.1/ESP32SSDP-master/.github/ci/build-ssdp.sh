#!/bin/bash
# Exit immediately if a command exits with a non-zero status.
set -e
# Enable the globstar shell option
shopt -s globstar

#arduino or PlatformIO
ide=$1

# Make sure we are inside the github workspace
cd $GITHUB_WORKSPACE
#export paths
export PATH="$HOME/arduino_ide:$PATH"
export ARDUINO_IDE_PATH="$HOME/arduino_ide"
if [[ "$ide" == "arduino" ]];
then
    echo "Arduino"
    fqbn=esp32:esp32:esp32:PartitionScheme=min_spiffs,FlashFreq=80,PSRAM=disabled,CPUFreq=240,FlashMode=qio,FlashSize=4M,DebugLevel=none
    arduino-builder -hardware "$ARDUINO_IDE_PATH/hardware" -tools "$ARDUINO_IDE_PATH/tools-builder" -tools "$ARDUINO_IDE_PATH/tools" -libraries "$ARDUINO_IDE_PATH/libraries" -fqbn=$fqbn -compile -logger=human -core-api-version=10810 ./examples/SSDP/SSDP.ino 
else
    echo "PlatformIO"
    cp -r ./src/ESP32SSDP.cpp ./examples/SSDP/
    cp -r ./src/ESP32SSDP.h ./examples/SSDP/
    cp ./test/platformio.ini ./examples/
    cd examples
    platformio run -e esp32dev
fi
