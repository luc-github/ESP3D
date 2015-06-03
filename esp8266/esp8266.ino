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
    https://github.com/esp8266/Arduino from Bootmanager

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
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#ifdef MDNS_FEATURE
#include <ESP8266mDNS.h>
#endif
extern "C" {
#include "user_interface.h"
}
#define MAX_SRV_CLIENTS 1
WiFiServer server(8888);
WiFiClient serverClients[MAX_SRV_CLIENTS];

void setup() {
  // init :
 // ESP.wdtDisable();
  delay(8000);
  EEPROM.begin(EEPROM_SIZE);
  bool breset_config=false;
  //check if reset config is requested
  pinMode(RESET_CONFIG_PIN, INPUT);
  if (digitalRead(RESET_CONFIG_PIN)==0)breset_config=true;//if requested =>reset settings
  //default baud rate
  int baud_rate=0;
  //check if EEPROM has value
  if ( CONFIG::read_buffer(EP_BAUD_RATE,  (byte *)&baud_rate , BAUD_LENGH))
    {
      //check if baud value is one of allowed ones
      if ( ! (baud_rate==9600 || baud_rate==19200 ||baud_rate==38400 ||baud_rate==57600 ||baud_rate==115200 ||baud_rate==230400) )breset_config=true;//baud rate is incorrect =>reset settings
    }
  else breset_config=true;//cannot access to config settings=> reset settings
  //reset is requested
  if(breset_config)
    {
    //update EEPROM with default settings
    CONFIG::reset_config();
    //use default baud rate
    baud_rate=DEFAULT_BAUD_RATE;
    }
  //setup serial
  Serial.begin(baud_rate);
  //setup wifi according settings
  wifi_config.Setup();
  delay(1000);
  //start interfaces
  web_interface.WebServer.begin();
  server.begin();
  server.setNoDelay(true);
}


//main loop
void loop() {
#ifdef MDNS_FEATURE
	// Check for any mDNS queries and send responses
	wifi_config.mdns.update();
#endif
//web requests
web_interface.WebServer.handleClient();
//TODO use a method to handle serial also in class and call it instead of this one
uint8_t i;
  //check if there are any new clients
  if (server.hasClient()){
    for(i = 0; i < MAX_SRV_CLIENTS; i++){
      //find free/disconnected spot
      if (!serverClients[i] || !serverClients[i].connected()){
        if(serverClients[i]) serverClients[i].stop();
        serverClients[i] = server.available();
        continue;
      }
    }
    //no free/disconnected spot so reject
    WiFiClient serverClient = server.available();
    serverClient.stop();
  }
  //check clients for data
  for(i = 0; i < MAX_SRV_CLIENTS; i++){
    if (serverClients[i] && serverClients[i].connected()){
      if(serverClients[i].available()){
        //get data from the telnet client and push it to the UART
        while(serverClients[i].available()) Serial.write(serverClients[i].read());
      }
    }
  }
  //check UART for data
  if(Serial.available()){
    size_t len = Serial.available();
    uint8_t sbuf[len];
    Serial.readBytes(sbuf, len);
    //push UART data to all connected telnet clients
    for(i = 0; i < MAX_SRV_CLIENTS; i++){
      if (serverClients[i] && serverClients[i].connected()){
        serverClients[i].write(sbuf, len);
        delay(1);
      }
    }
  }
}
