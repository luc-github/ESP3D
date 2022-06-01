#!/bin/bash
# Exit immediately if a command exits with a non-zero status.
set -e
# Enable the globstar shell option
shopt -s globstar
ls $HOME
# Make sure we are inside the github workspace
cd $GITHUB_WORKSPACE
cp -r ./libraries/ESP32SSDP-1.2.0 $HOME/arduino_ide/libraries/
cp -r ./libraries/arduinoWebSockets-2.3.6 $HOME/arduino_ide/libraries/
cp -r ./libraries/DHT_sensor_library_for_ESPx-1.0.6 $HOME/arduino_ide/libraries/
cp -r ./libraries/esp8266-oled-ssd1306-4.3.0 $HOME/arduino_ide/libraries/
cp -r ./libraries/TFT_eSPI-2.4.61 $HOME/arduino_ide/libraries/
cp -r ./libraries/lvgl-8.2.0 $HOME/arduino_ide/libraries/
cp -r ./libraries/ESP8266-Arduino-Lua-0.0.30 $HOME/arduino_ide/libraries/
cp -r ./libraries/BMx280MI-1.2.0 $HOME/arduino_ide/libraries/
#TODO add SDFat libraries according version and target 

