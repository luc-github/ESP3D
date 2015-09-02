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
#include <SPI.h>
#include <EEPROM.h>
#include "config.h"
#include "wifi.h"
#include "webinterface.h"
#include "command.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#ifdef MDNS_FEATURE
#include <ESP8266mDNS.h>
#endif
#ifdef SSDP_FEATURE
#include <ESP8266SSDP.h>
#endif
extern "C" {
#include "user_interface.h"
}
#define MAX_SRV_CLIENTS 1
WiFiServer * data_server;
WiFiClient serverClients[MAX_SRV_CLIENTS];

void setup() {
  // init :
 // ESP.wdtDisable();
  system_update_cpu_freq(SYS_CPU_160MHZ);
  delay(8000);
  EEPROM.begin(EEPROM_SIZE);
  bool breset_config=false;
  //check if reset config is requested
  pinMode(RESET_CONFIG_PIN, INPUT);
  if (digitalRead(RESET_CONFIG_PIN)==0)breset_config=true;//if requested =>reset settings
  //default baud rate
  long baud_rate=0;
  
  //check if EEPROM has value
  if ( CONFIG::read_buffer(EP_BAUD_RATE,  (byte *)&baud_rate , INTEGER_LENGH)&&CONFIG::read_buffer(EP_WEB_PORT,  (byte *)&(wifi_config.iweb_port) , INTEGER_LENGH)&&CONFIG::read_buffer(EP_DATA_PORT,  (byte *)&(wifi_config.idata_port) , INTEGER_LENGH))
    {
      //check if baud value is one of allowed ones
      if ( ! (baud_rate==9600 || baud_rate==19200 ||baud_rate==38400 ||baud_rate==57600 ||baud_rate==115200 ||baud_rate==230400 ||baud_rate==250000) )breset_config=true;//baud rate is incorrect =>reset settings
	  if (wifi_config.iweb_port<1 ||wifi_config.iweb_port>65001 || wifi_config.idata_port <1 || wifi_config.idata_port >65001)breset_config=true; //out of range =>reset settings
    
    }
  else breset_config=true;//cannot access to config settings=> reset settings
 
  
  //reset is requested
  if(breset_config)
    {
    //update EEPROM with default settings
    CONFIG::reset_config();
    //use default baud rate and ports
    baud_rate=DEFAULT_BAUD_RATE;
    wifi_config.iweb_port=DEFAULT_WEB_PORT;
    wifi_config.idata_port=DEFAULT_DATA_PORT;
    }
  //setup serial
  Serial.begin(baud_rate);
  //Serial1.setDebugOutput(true);
  //setup wifi according settings
  wifi_config.Setup();
  delay(1000);
  //start interfaces
  web_interface = new WEBINTERFACE_CLASS(wifi_config.iweb_port);
  data_server = new WiFiServer (wifi_config.idata_port);
  web_interface->WebServer.begin();
  data_server->begin();
  data_server->setNoDelay(true);

#ifdef MDNS_FEATURE
	// Check for any mDNS queries and send responses
	if (wifi_get_opmode()==WIFI_STA )
		{
			wifi_config.mdns.addService("http", "tcp", wifi_config.iweb_port);
		}
#endif

#ifdef SSDP_FEATURE
	SSDP.setSchemaURL("description.xml");
    SSDP.setHTTPPort( wifi_config.iweb_port);
    SSDP.setName("ESP8266 Module");
    SSDP.setSerialNumber(String(system_get_chip_id()).c_str());
    SSDP.setURL("/");
    SSDP.setModelName("ESP8266 01");
    SSDP.setModelNumber("01");
    SSDP.setModelURL("http://espressif.com/en/products/esp8266/");
    SSDP.setManufacturer("Espressif Systems");
    SSDP.setManufacturerURL("http://espressif.com");
    SSDP.begin();
#endif
}


//main loop
void loop() {
//web requests
web_interface->WebServer.handleClient();
//TODO use a method to handle serial also in class and call it instead of this one
uint8_t i,data;
  //check if there are any new clients
  if (data_server->hasClient()){
    for(i = 0; i < MAX_SRV_CLIENTS; i++){
      //find free/disconnected spot
      if (!serverClients[i] || !serverClients[i].connected()){
        if(serverClients[i]) serverClients[i].stop();
        serverClients[i] = data_server->available();
        continue;
      }
     yield();
    }
    //no free/disconnected spot so reject
    WiFiClient serverClient = data_server->available();
    serverClient.stop();
  }
  //check clients for data
  for(i = 0; i < MAX_SRV_CLIENTS; i++){
    if (serverClients[i] && serverClients[i].connected()){
      if(serverClients[i].available()){
        //get data from the tcp client and push it to the UART
        while(serverClients[i].available()){
         data = serverClients[i].read();
         Serial.write(data);
         COMMAND::read_buffer_tcp(data);
         }
         yield();
      }
    }
  }
  //check UART for data
  if(Serial.available()){
    size_t len = Serial.available();
    uint8_t sbuf[len];
    Serial.readBytes(sbuf, len);
    //push UART data to all connected tcp clients
    for(i = 0; i < MAX_SRV_CLIENTS; i++){
      if (serverClients[i] && serverClients[i].connected()){
        serverClients[i].write(sbuf, len);
        delay(1);
      }
	  COMMAND::read_buffer_serial(sbuf, len);
    }
  }
  if (web_interface->restartmodule)
	{
			ESP.restart();
			while (1){delay(1);};
	}
}
