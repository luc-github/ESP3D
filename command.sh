#!/bin/bash

function build_sketch()
{
	local sketch=$1
    local target=$2
    local ide=$3
    local bt=$4
    local auth=$5
    local fs=$6
    
    if [[ "$ide" == "arduino" ]];
    then
    echo "Setup for Arduino"
    rm -f $HOME/.arduino15/preferences.txt
    #be sure everything is enabled by default as reference
    echo "Authentication is enabled"
    sed -i "s/\/\/#define AUTHENTICATION_FEATURE/#define AUTHENTICATION_FEATURE/g" $TRAVIS_BUILD_DIR/esp3d/configuration.h
    echo "Bluetooth is enabled"
    sed -i "s/\/\/#define BLUETOOTH_FEATURE/#define BLUETOOTH_FEATURE/g" $TRAVIS_BUILD_DIR/esp3d/configuration.h
    if [[ "$bt" == "no" ]];
    then
        echo "Disable Bluetooth"
        sed -i "s/#define BLUETOOTH_FEATURE/\/\/#define BLUETOOTH_FEATURE/g" $TRAVIS_BUILD_DIR/esp3d/configuration.h
    fi
    if [[ "$auth" == "no" ]];
    then
        echo "Disable Authentication"
        sed -i "s/#define AUTHENTICATION_FEATURE/\/\/#define AUTHENTICATION_FEATURE/g" $TRAVIS_BUILD_DIR/esp3d/configuration.h
    fi
    if [[ "$target" == "esp32" ]];
    then
        echo "setup for esp32"
        arduino --board esp32:esp32:esp32:PartitionScheme=min_spiffs,FlashFreq=80,PSRAM=disabled,CPUFreq=240,FlashMode=qio,FlashSize=4M,DebugLevel=none --pref compiler.warning_level=all --save-prefs
    else
        echo "setup for esp8266"
        sed -i "s/#define DISPLAY_DEVICE/\/\/#define DISPLAY_DEVICE/g" $TRAVIS_BUILD_DIR/esp3d/configuration.h
        sed -i "s/#define ETH_FEATURE/\/\/#define ETH_FEATURE/g" $TRAVIS_BUILD_DIR/esp3d/configuration.h
        arduino --board esp8266com:esp8266:generic:eesz=4M3M,xtal=160,FlashMode=dio,FlashFreq=40,sdk=nonosdk221,ip=lm2f,dbg=Disabled,vt=flash,exception=disabled,ssl=basic --save-prefs
    fi
    if [[ "$fs" == "SPIFFS" ]];
    then
        echo "Set Filesystem to SPIFFS"
        sed -i "s/#define FILESYSTEM_FEATURE ESP_FAT_FILESYSTEM/#define FILESYSTEM_FEATURE ESP_SPIFFS_FILESYSTEM/g" $TRAVIS_BUILD_DIR/esp3d/configuration.h
        sed -i "s/#define FILESYSTEM_FEATURE ESP_LITTLEFS_FILESYSTEM/#define FILESYSTEM_FEATURE ESP_SPIFFS_FILESYSTEM/g" $TRAVIS_BUILD_DIR/esp3d/configuration.h
    fi
    if [[ "$fs" == "FAT" ]];
    then
        echo "Set Filesystem to FAT"
        sed -i "s/#define FILESYSTEM_FEATURE ESP_SPIFFS_FILESYSTEM/#define FILESYSTEM_FEATURE ESP_FAT_FILESYSTEM/g" $TRAVIS_BUILD_DIR/esp3d/configuration.h
        sed -i "s/#define FILESYSTEM_FEATURE ESP_LITTLEFS_FILESYSTEM/#define FILESYSTEM_FEATURE ESP_FAT_FILESYSTEM/g" $TRAVIS_BUILD_DIR/esp3d/configuration.h
    fi
    echo "Display configuration"
    cat $TRAVIS_BUILD_DIR/esp3d/configuration.h
	# build sketch with arduino ide
	echo -e "\n Build $sketch \n"
	arduino --verbose --verify $sketch

	# get build result from arduino
	local re=$?

	# check result
	if [ $re -ne 0 ]; then
		echo "Failed to build $sketch"
		return $re
	fi
    else
        echo "setup for platformIO"
        sed -i "s/#define BLUETOOTH_FEATURE/\/\/#define BLUETOOTH_FEATURE/g" $TRAVIS_BUILD_DIR/esp3d/configuration.h
        sed -i "s/#define DISPLAY_DEVICE/\/\/#define DISPLAY_DEVICE/g" $TRAVIS_BUILD_DIR/esp3d/configuration.h
        sed -i "s/#define ETH_FEATURE/\/\/#define ETH_FEATURE/g" $TRAVIS_BUILD_DIR/esp3d/configuration.h
        sed -i "s/#define FILESYSTEM_FEATURE ESP_FAT_FILESYSTEM/#define FILESYSTEM_FEATURE ESP_SPIFFS_FILESYSTEM/g" $TRAVIS_BUILD_DIR/esp3d/configuration.h
        sed -i "s/#define FILESYSTEM_FEATURE ESP_LITTLEFS_FILESYSTEM/#define FILESYSTEM_FEATURE ESP_SPIFFS_FILESYSTEM/g" $TRAVIS_BUILD_DIR/esp3d/configuration.h
        rm -fr $HOME/arduino_ide
        rm -fr $HOME/.arduino15
        platformio run
    fi
    
}

