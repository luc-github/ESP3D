/*
    This file is part of ESP8266 Firmware for 3D printer.

    ESP8266 Firmware for 3D printer is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    ESP8266 Firmware for 3D printer is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Repetier-Firmware.  If not, see <http://www.gnu.org/licenses/>.

    This firmware is using the standard arduino IDE with module to support ESP8266:
    https://github.com/sandeepmistry/esp8266-Arduino based on :
    https://github.com/esp8266/Arduino

    Latest version of the code and documentation can be found here :
    https://github.com/luc-github/ESP8266
    
    Main author: luc lebosse

*/
//be sure correct IDE and settings are used for ESP8266
#ifndef ARDUINO_ESP8266_ESP01
#error Oops!  Make sure you have 'ESP8266' selected from the 'Tools -> Boards' menu.
#endif
//includes: why EEPROM.h need to be there ???
#include <EEPROM.h>
#include "config.h"
#include "wifi.h"
#include "webinterface.h"
#include "datainterface.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>


void setup() {
  // init :
  delay(8000);
  EEPROM.begin(EEPROM_SIZE);
  bool breset_config=false;
  //check if reset config is requested
  pinMode(RESET_CONFIG_PIN, INPUT);
  if (digitalRead(RESET_CONFIG_PIN)==0)breset_config=true;//if requested =>reset settings
  //default baud rate
  word baud_rate;
  char sbuf[7];
  //check if EEPROM has value
  if ( CONFIG::read_string(EP_BAUD_RATE, sbuf , MAX_BAUD_LENGH))
    {
      word baud_tmp = atoi(sbuf);
      //check if baud value is one of allowed ones
      if (baud_tmp==9600 || baud_tmp==19200 ||baud_tmp==38400 ||baud_tmp==57600 ||baud_tmp==115200 ||baud_tmp==230400) baud_rate=baud_tmp;
      else breset_config=true;//baud rate is incorrect =>reset settings
    }
    else breset_config=true;//cannot access to config settings=> reset settings
  //reset is requested
  if(breset_config)
    {
    //update EEPROM with default settings
    CONFIG::reset_config();
    //use default baud rate
    baud_rate=atol(DEFAULT_BAUD_RATE);
    }
  //setup serial
  Serial.begin(baud_rate);
  //setup wifi according settings
  wifi_config.Setup();
  delay(1000);
  //start interfaces
  web_interface.WebServer.begin();
  data_interface.WebServer.begin();
}


//main loop
void loop() {
//web requests
web_interface.WebServer.handleClient();
//TODO use a method to handle serial also in class and call it instead of this one
data_interface.WebServer.handleClient();

}
