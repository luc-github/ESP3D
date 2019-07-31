#!/bin/bash

function build_sketch()
{
	local sketch=$1
    local target=$2
    local ide=$3
    if [[ "$3" == "arduino" ]];
    then
    rm -f $HOME/.arduino15/preferences.txt
    if [[ "$2" == "esp32" ]];
    then
        echo "setup for esp32"
        arduino --board esp32:esp32:esp32:PartitionScheme=min_spiffs,FlashFreq=80,PSRAM=disabled,CPUFreq=240,FlashMode=qio,FlashSize=4M,DebugLevel=none --pref compiler.warning_level=all --save-prefs
    else
        echo "setup for esp8266"
        arduino --board esp8266com:esp8266:generic:eesz=4M3M,xtal=160,FlashMode=dio,FlashFreq=40,sdk=nonosdk221,ip=lm2f,dbg=Disabled,vt=flash,exception=disabled,ssl=basic --save-prefs
    fi
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
        rm -fr $HOME/arduino_ide
        rm -fr $HOME/.arduino15
        platformio run
    fi
    
}

