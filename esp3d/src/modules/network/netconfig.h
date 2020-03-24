/*
  netconfig.h -  network functions class

  Copyright (c) 2014 Luc Lebosse. All rights reserved.

  This code is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This code is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with This code; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

//boundaries


#define MAX_HTTP_PORT               65001
#define MIN_HTTP_PORT               1
#define MAX_FTP_PORT                65001
#define MIN_FTP_PORT                1
#define MAX_TELNET_PORT             65001
#define MIN_TELNET_PORT             1
#define MAX_WEBSOCKET_PORT          65001
#define MIN_WEBSOCKET_PORT          1
#define MAX_HOSTNAME_LENGTH         32
#define MIN_HOSTNAME_LENGTH         1

//IP mode
#define DHCP_MODE       1
#define STATIC_IP_MODE  2

//Network Mode
#define ESP_RADIO_OFF 0
#define ESP_WIFI_STA 1
#define ESP_WIFI_AP  2
#define ESP_BT       3
#define ESP_ETH_STA  4
//#define ESP_ETH_SRV  5

#ifdef ARDUINO_ARCH_ESP32
#include <WiFi.h>
#endif //ARDUINO_ARCH_ESP32
#ifdef ARDUINO_ARCH_ESP8266
#include <ESP8266WiFi.h>
#endif //ARDUINO_ARCH_ESP8266

#ifndef _NET_CONFIG_H
#define _NET_CONFIG_H

class NetConfig
{
public:
    NetConfig();
    ~NetConfig();
    static bool isValidIP(const char * string);
    static bool isHostnameValid (const char * hostname);
    static uint32_t IP_int_from_string(const char * s);
    static String IP_string_from_int(uint32_t ip_int);
    static bool isIPModeDHCP(uint8_t mode);
    static bool isDHCPServer (uint8_t mode);
    static const char* hostname(bool fromsettings = false);
    static char * mac2str (uint8_t mac [8]);
    static bool begin();
    static void end();
    static void handle();
    static uint8_t getMode();
    static bool started()
    {
        return _started;
    }
    static String localIP();
private :
    static String _hostname;
    static void onWiFiEvent(WiFiEvent_t event);
    static bool _needReconnect2AP;
    static bool _events_registered;
    static bool _started;
    static uint8_t _mode;
};

#endif //_NET_CONFIG_H
