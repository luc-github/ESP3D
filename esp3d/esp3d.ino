/*
    This file is part of ESP3D Firmware for 3D printer.

    ESP3D Firmware for 3D printer is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    ESP3D Firmware for 3D printer is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this Firmware.  If not, see <http://www.gnu.org/licenses/>.

    This firmware is using the standard arduino IDE with module to support ESP8266:
    https://github.com/esp8266/Arduino from Bootmanager

    Latest version of the code and documentation can be found here :
    https://github.com/luc-github/ESP3D

    Main author: luc lebosse

*/
//be sure correct IDE and settings are used for ESP8266 or ESP32
#if !(defined( ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32))
#error Oops!  Make sure you have 'ESP8266 or ESP32' compatible board selected from the 'Tools -> Boards' menu.
#endif
#include <EEPROM.h>
#include "config.h"
#include "wificonf.h"
#include "bridge.h"
#include "webinterface.h"
#include "command.h"
#ifdef ARDUINO_ARCH_ESP8266
#include "ESP8266WiFi.h"
#ifdef MDNS_FEATURE
#include <ESP8266mDNS.h>
#endif
#include <ESP8266WebServer.h>
#else
#include <WiFi.h>
#ifdef MDNS_FEATURE
#include <ESPmDNS.h>
#endif
#include "esp_wifi.h"
#include <WebServer.h>
#include "FS.h"
#include "SPIFFS.h"
#include "Update.h"
#endif
#include <WiFiClient.h>


#ifdef CAPTIVE_PORTAL_FEATURE
#include <DNSServer.h>
extern DNSServer dnsServer;
#endif
#ifdef SSDP_FEATURE
#ifdef ARDUINO_ARCH_ESP8266
#include <ESP8266SSDP.h>
#else
//#include <ESPSSDP.h>
#endif
#endif
#ifdef NETBIOS_FEATURE
#ifdef ARDUINO_ARCH_ESP8266
#include <ESP8266NetBIOS.h>
#else
//#include <ESPNetBIOS.h>
#endif
#endif
#ifndef FS_NO_GLOBALS
#define FS_NO_GLOBALS
#endif
#include <FS.h>

void setup()
{
    bool breset_config=false;
    bool directsd_check = false;
    web_interface = NULL;
#ifdef TCP_IP_DATA_FEATURE
    data_server = NULL;
#endif
    // init:
#ifdef DEBUG_ESP3D
    if (ESP_SERIAL_OUT.baudRate() != DEFAULT_BAUD_RATE)ESP_SERIAL_OUT.begin(DEFAULT_BAUD_RATE);
    delay(2000);
    LOG("\r\nDebug Serial set\r\n")
#endif
    //WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
    delay(8000);
    CONFIG::InitDirectSD();
    CONFIG::InitPins();
#ifdef RECOVERY_FEATURE
    delay(8000);
    //check if reset config is requested
    if (digitalRead(RESET_CONFIG_PIN)==0) {
        breset_config=true;    //if requested =>reset settings
    }
#endif
    //check if EEPROM has value
    if (  !CONFIG::InitBaudrate() || !CONFIG::InitExternalPorts()) {
        breset_config=true;    //cannot access to config settings=> reset settings
        LOG("Error no EEPROM access\r\n")
    }

    //reset is requested
    if(breset_config) {
        //update EEPROM with default settings
        if (ESP_SERIAL_OUT.baudRate() != DEFAULT_BAUD_RATE)ESP_SERIAL_OUT.begin(DEFAULT_BAUD_RATE);
#ifdef ARDUINO_ARCH_ESP8266
        ESP_SERIAL_OUT.setRxBufferSize(SERIAL_RX_BUFFER_SIZE);
#endif
        delay(2000);
        ESP_SERIAL_OUT.println(F("M117 ESP EEPROM reset"));
#ifdef DEBUG_ESP3D
        CONFIG::print_config(DEBUG_PIPE, true);
        delay(1000);
#endif
        CONFIG::reset_config();
        delay(1000);
        //put some default value to a void some exception at first start
        WiFi.mode(WIFI_AP);
#ifdef ARDUINO_ARCH_ESP8266
        WiFi.setPhyMode(WIFI_PHY_MODE_11G);
#else 
	esp_wifi_set_protocol(ESP_IF_WIFI_AP, WIFI_PHY_MODE_11G);
#endif
        CONFIG::esp_restart();
    }
#if defined(DEBUG_ESP3D) && defined(DEBUG_OUTPUT_SERIAL)
    LOG("\r\n");
    delay(500);
    ESP_SERIAL_OUT.flush();
#endif
    //get target FW
    CONFIG::InitFirmwareTarget();
    //Update is done if any so should be Ok
#ifdef ARDUINO_ARCH_ESP32
    SPIFFS.begin(true);
#else
	SPIFFS.begin();
#endif
       
    //setup wifi according settings
    if (!wifi_config.Setup()) {
        ESP_SERIAL_OUT.println(F("M117 Safe mode 1"));
        //try again in AP mode
       if (!wifi_config.Setup(true)) {
            ESP_SERIAL_OUT.println(F("M117 Safe mode 2"));
            wifi_config.Safe_Setup();
        }
    }
    delay(1000);
    //setup servers
    if (!wifi_config.Enable_servers()) {
        ESP_SERIAL_OUT.println(F("M117 Error enabling servers"));
    }
    LOG("Setup Done\r\n");
}



//main loop
void loop()
{
    //be sure wifi is on to proceed wifi function
     if (WiFi.getMode()!=WIFI_OFF ) {
#ifdef CAPTIVE_PORTAL_FEATURE
        if (WiFi.getMode()!=WIFI_STA ) {
            dnsServer.processNextRequest();
        }
#endif
//web requests
        web_interface->web_server.handleClient();
#ifdef TCP_IP_DATA_FEATURE
        BRIDGE::processFromTCP2Serial();
#endif
    }
        BRIDGE::processFromSerial2TCP();
    //in case of restart requested
    if (web_interface->restartmodule) {
        CONFIG::esp_restart();
    }
}
