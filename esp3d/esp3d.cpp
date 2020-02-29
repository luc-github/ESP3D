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

    This firmware is using the standard arduino IDE with module to support ESP8266/ESP32:
    https://github.com/esp8266/Arduino
    https://github.com/espressif/arduino-esp32

    Latest version of the code and documentation can be found here :
    https://github.com/luc-github/ESP3D

    Main author: luc lebosse

*/
#include "esp3d.h"
#include <EEPROM.h>
#ifdef ARDUINO_ARCH_ESP8266
#include <FS.h>
#endif
#ifndef FS_NO_GLOBALS
#define FS_NO_GLOBALS
#endif
#if defined (ASYNCWEBSERVER)
#include <ESPAsyncWebServer.h>
#endif
#include "config.h"
#if defined(TIMESTAMP_FEATURE)
#include <time.h>
#endif
#include "wificonf.h"
#include "espcom.h"
#include "webinterface.h"
#include "command.h"
#ifdef ARDUINO_ARCH_ESP8266
#include "ESP8266WiFi.h"
#if defined (ASYNCWEBSERVER)
#include <ESPAsyncTCP.h>
#else
#include <ESP8266WebServer.h>
#endif
#else //ESP32
#include <WiFi.h>
#if defined (ASYNCWEBSERVER)
#include <AsyncTCP.h>
#else
#include <WebServer.h>
#endif
#include <rom/rtc.h>
#include "esp_wifi.h"
#include "FS.h"
#include "SPIFFS.h"
#include "Update.h"
#endif
#include <WiFiClient.h>

#ifdef CAPTIVE_PORTAL_FEATURE
#include <DNSServer.h>
extern DNSServer dnsServer;
#endif

#ifndef FS_NO_GLOBALS
#define FS_NO_GLOBALS
#endif
#include <FS.h>
#ifdef ESP_OLED_FEATURE
#include "esp_oled.h"
#endif
#ifdef DHT_FEATURE
#include "DHTesp.h"
DHTesp dht;
#endif

#if defined (ASYNCWEBSERVER)
#include "asyncwebserver.h"
#else
#include "syncwebserver.h"
#endif

//Contructor
Esp3D::Esp3D()
{

}

//Begin which setup everything
void Esp3D::begin(uint16_t startdelayms, uint16_t recoverydelayms)
{
    // init:
    //WiFi.disconnect();
    WiFi.mode (WIFI_OFF);
    wifi_config.WiFi_on = false;
    //check EEPROM Version
#if defined( DEBUG_ESP3D) && defined(DEBUG_OUTPUT_SERIAL)
    CONFIG::InitBaudrate(DEFAULT_BAUD_RATE);
    delay (2000);
    LOG ("\r\nDebug Serial set\r\n")
#endif
    CONFIG::adjust_EEPROM_settings();
    CONFIG::InitOutput();
#ifdef ESP_OLED_FEATURE
    OLED_DISPLAY::begin();
    OLED_DISPLAY::splash();
#endif
    bool breset_config = false;
    web_interface = NULL;
#ifdef TCP_IP_DATA_FEATURE
    data_server = NULL;
#endif

#ifdef MKS_TFT_FEATURE
    startdelayms = 1000;
#endif
#ifdef ESP_OLED_FEATURE
    uint32_t start_display_time = millis();
    uint32_t now = millis();
    while ( now - start_display_time < startdelayms) {
        int v = (100 * (millis() - start_display_time)) / startdelayms;
        OLED_DISPLAY::display_mini_progress(v);
        OLED_DISPLAY::update_lcd();
        delay(100);
        now = millis();
    }
#else
    delay (startdelayms);
#endif
    CONFIG::InitDirectSD();
    CONFIG::InitPins();
#ifdef RECOVERY_FEATURE
    delay (recoverydelayms);
    //check if reset config is requested
    if (digitalRead (RESET_CONFIG_PIN) == 0) {
        breset_config = true;  //if requested =>reset settings
    }
#endif
    //check if EEPROM has value
    if (  !CONFIG::InitBaudrate() || !CONFIG::InitExternalPorts() ) {
        breset_config = true;  //cannot access to config settings=> reset settings
        LOG ("Error no EEPROM access\r\n")
    }
    //reset is requested
    if (breset_config) {
        //update EEPROM with default settings
        CONFIG::InitBaudrate(DEFAULT_BAUD_RATE);
#ifdef ARDUINO_ARCH_ESP8266
        Serial.setRxBufferSize (SERIAL_RX_BUFFER_SIZE);
#endif
        delay (2000);
        ESPCOM::println (F ("ESP EEPROM reset"), PRINTER_PIPE);
#ifdef DEBUG_ESP3D
        CONFIG::print_config (DEBUG_PIPE, true);
        delay (1000);
#endif
        CONFIG::reset_config();
        delay (1000);
        //put some default value to a void some exception at first start
        WiFi.mode (WIFI_AP);
        wifi_config.WiFi_on = true;
#ifdef ARDUINO_ARCH_ESP8266
        WiFi.setPhyMode (WIFI_PHY_MODE_11G);
#else
        esp_wifi_set_protocol (ESP_IF_WIFI_AP, WIFI_PHY_MODE_11G);
#endif
        CONFIG::esp_restart();
    }
#if defined(DEBUG_ESP3D) && defined(DEBUG_OUTPUT_SERIAL)
    LOG ("\r\n");
    delay (500);
    ESPCOM::flush (DEFAULT_PRINTER_PIPE);
#endif
    //get target FW
    CONFIG::InitFirmwareTarget();
    delay(100);
    //Update is done if any so should be Ok
#ifdef ARDUINO_ARCH_ESP32
    SPIFFS.begin (true);
#else
    SPIFFS.begin();
#endif
//basic autostart
    if(SPIFFS.exists("/autostart.g")) {
        FS_FILE file = SPIFFS.open("/autostart.g", SPIFFS_FILE_READ);
        if (file) {
            String autoscript = file.readString();
            if (autoscript.length() > 0) {
                //clean line
                autoscript.replace("\n","");
                autoscript.replace("\r","");
                ESPCOM::println (autoscript.c_str(), DEFAULT_PRINTER_PIPE);
            }
            file.close();
        }
    }
    //setup wifi according settings
    if (!wifi_config.Setup() ) {
        ESPCOM::println (F ("Safe mode 1"), PRINTER_PIPE);
        //try again in AP mode
        if (!wifi_config.Setup (true) ) {
            ESPCOM::println (F ("Safe mode 2"), PRINTER_PIPE);
            wifi_config.Safe_Setup();
        }
    }
    delay (100);
    //setup servers
    if (!wifi_config.Enable_servers() ) {
        ESPCOM::println (F ("Error enabling servers"), PRINTER_PIPE);
    }
    /*#ifdef ARDUINO_ARCH_ESP8266
        if	(rtc_info->reason	==	REASON_WDT_RST	||

                rtc_info->reason	==	REASON_EXCEPTION_RST	||

                rtc_info->reason	==	REASON_SOFT_WDT_RST)	{
    			String s = "reset ";
    			s+= String(rtc_info->reason);

            if	(rtc_info->reason	==	REASON_EXCEPTION_RST)	{
    			s+=" except ";
    			s+=String(rtc_info->exccause);

            }
            ESPCOM::println (s, PRINTER_PIPE);
        }
    #else
        if((( reason_0< 17) || ( reason_1< 17)) && !(((reason_0 == 1) && (reason_1 == 14)) || ((reason_0 == 16) && (reason_1 == 14))))
        {
    		String s = "reset ";
    		ESPCOM::println (s, PRINTER_PIPE);
    		s+=String(reason_0);
    		s+="/";
    		s+=String(reason_1);

    	}
    #endif*/

#ifdef ASYNCWEBSERVER
    if (WiFi.getMode() != WIFI_AP) {
        WiFi.scanNetworks (true);
    }
#endif
    LOG ("Setup Done\r\n");
}

//Process which handle all input
void Esp3D::process()
{
#ifdef ARDUINO_ARCH_ESP8266
#ifdef MDNS_FEATURE
    wifi_config.mdns.update();
#endif
#endif
#if !defined(ASYNCWEBSERVER)
//web requests for sync
    web_interface->web_server.handleClient();
    socket_server->loop();
#endif
//be sure wifi is on to proceed wifi function
    if ((WiFi.getMode() != WIFI_OFF)  || wifi_config.WiFi_on) {
#ifdef CAPTIVE_PORTAL_FEATURE
        if (WiFi.getMode() != WIFI_STA ) {
            dnsServer.processNextRequest();
        }
#endif
        //TODO use config
        CONFIG::wait(0);
    }
//read / bridge all input
    ESPCOM::bridge();
//in case of restart requested
    if (web_interface->restartmodule) {
        CONFIG::esp_restart();
    }

#ifdef ESP_OLED_FEATURE
    static uint32_t last_oled_update= 0;
    if ( !CONFIG::is_locked(FLAG_BLOCK_OLED)) {
        uint32_t now_oled = millis();
        if (now_oled - last_oled_update > 1000) {
            last_oled_update = now_oled;
            //refresh signal
            if ((WiFi.getMode() == WIFI_OFF) || !wifi_config.WiFi_on) {
                OLED_DISPLAY::display_signal(-1);
            } else {
                OLED_DISPLAY::display_signal(wifi_config.getSignal (WiFi.RSSI ()));
            }
            //if line 0 is > 85 refresh
            if(OLED_DISPLAY::L0_size >85) {
                OLED_DISPLAY::display_text(OLED_DISPLAY::L0.c_str(), 0, 0, 85);
            }
            //if line 1 is > 128 refresh
            if(OLED_DISPLAY::L1_size >128) {
                OLED_DISPLAY::display_text(OLED_DISPLAY::L1.c_str(), 0, 16, 128);
            }
            //if line 2 is > 128 refresh
            if(OLED_DISPLAY::L2_size >128) {
                OLED_DISPLAY::display_text(OLED_DISPLAY::L2.c_str(), 0, 32, 128);
            }
            //if line 3 is > 128 refresh
            if(OLED_DISPLAY::L3_size >128) {
                OLED_DISPLAY::display_text(OLED_DISPLAY::L3.c_str(), 0, 48, 128);
            }
            OLED_DISPLAY::update_lcd();
        }
    }
#endif

#ifdef DHT_FEATURE
    if (CONFIG::DHT_type  != 255) {
        static uint32_t last_dht_update= 0;
        uint32_t now_dht = millis();
        if (now_dht - last_dht_update > (CONFIG::DHT_interval * 1000)) {
            last_dht_update = now_dht;
            float humidity = dht.getHumidity();
            float temperature = dht.getTemperature();
            if (strcmp(dht.getStatusString(),"OK") == 0) {
                String s = String(temperature,2);
                String s2 = s + " " +String(humidity,2);
#if defined (ASYNCWEBSERVER)
                web_interface->web_events.send( s2.c_str(),"DHT", millis());
#else
                s = "DHT:" + s2;
                socket_server->sendTXT(ESPCOM::current_socket_id, s);
#endif
#ifdef ESP_OLED_FEATURE
                if ( !CONFIG::is_locked(FLAG_BLOCK_OLED)) {
                    s = String(temperature,2);
                    s +="°C";
                    OLED_DISPLAY::display_text(s.c_str(), 84, 16);
                }
#endif
            }
        }
    }
#endif
//todo use config
    CONFIG::wait(0);
}
