#!/bin/bash
# Exit immediately if a command exits with a non-zero status.
set -e
# Enable the globstar shell option
shopt -s globstar

#arduino or PlatformIO
target=$1
ide=$2
#bluetooth
#local bt=$3
#authentication
#local auth=$4
#filesystem
#local fs=$5


# Make sure we are inside the github workspace
cd $GITHUB_WORKSPACE
#export paths
export PATH="$HOME/arduino_ide:$PATH"
export ARDUINO_IDE_PATH="$HOME/arduino_ide"
if [[ "$ide" == "arduino" ]];
    then
    if [[ "$target" == "esp32" ]];
    then
        echo "setup for esp32"
        fqbn=esp32:esp32:esp32:PartitionScheme=min_spiffs,FlashFreq=80,PSRAM=disabled,CPUFreq=240,FlashMode=qio,FlashSize=4M,DebugLevel=none
    else
        echo "setup for esp8266"
        sed -i "s/#define DISPLAY_DEVICE/\/\/#define DISPLAY_DEVICE/g" $GITHUB_WORKSPACE/esp3d/configuration.h
        sed -i "s/#define ETH_FEATURE/\/\/#define ETH_FEATURE/g" $GITHUB_WORKSPACE/esp3d/configuration.h
        fqbn="esp8266com:esp8266:generic:eesz=4M3M,xtal=160,FlashMode=dio,FlashFreq=40,sdk=nonosdk221,ip=lm2f,dbg=Disabled,vt=flash,exception=disabled,ssl=basic,mmu=3232 ./esp3d/esp3d.ino"
        
    fi

    arduino-builder -hardware "$ARDUINO_IDE_PATH/hardware" -tools "$ARDUINO_IDE_PATH/tools-builder" -tools "$ARDUINO_IDE_PATH/tools" -libraries "$ARDUINO_IDE_PATH/libraries" -fqbn=$fqbn -compile -logger=human -core-api-version=10810 ./esp3d/esp3d.ino 
else
    platformio run -e esp32dev
    platformio run -e esp32-s2
    platformio run -e esp32-s3
    platformio run -e esp32-c3
    platformio run -e esp8266dev
fi
