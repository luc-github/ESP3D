#!/bin/bash
# Exit immediately if a command exits with a non-zero status.
set -e
# Enable the globstar shell option
shopt -s globstar
ls $HOME
# Make sure we are inside the github workspace
cd $GITHUB_WORKSPACE
cp -r ./libraries/ESP32SSDP $HOME/arduino_ide/libraries/
cp -r ./libraries/arduinoWebSockets $HOME/arduino_ide/libraries/
cp -r ./libraries/DHT_sensor_library_for_ESPx $HOME/arduino_ide/libraries/
cp -r ./libraries/oled-ssd1306 $HOME/arduino_ide/libraries/
cp -r ./libraries/ESP32NETBIOS $HOME/arduino_ide/libraries/


