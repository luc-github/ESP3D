/*
  wificonfig.h -  wifi functions class

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
#define MIN_RSSI                    -78
#define MAX_SSID_LENGTH             32
#define MIN_SSID_LENGTH             1
#define MIN_CHANNEL                 1
#define MAX_CHANNEL                 14
#define MAX_PASSWORD_LENGTH         64
//min size of password is 0 or upper than 8 char
//0 is special case so let's put 8
#define MIN_PASSWORD_LENGTH         8
#ifdef ARDUINO_ARCH_ESP32
#include <WiFi.h>
#define WIFI_NONE_SLEEP WIFI_PS_NONE
#define WIFI_LIGHT_SLEEP WIFI_PS_MIN_MODEM
#define WIFI_MODEM_SLEEP WIFI_PS_MAX_MODEM
#define WIFI_PHY_MODE_11B WIFI_PROTOCOL_11B
#define WIFI_PHY_MODE_11G WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G
#define WIFI_PHY_MODE_11N WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N
#define AUTH_OPEN WIFI_AUTH_OPEN
#define AUTH_WEP WIFI_AUTH_WEP
#define AUTH_WPA_PSK WIFI_AUTH_WPA_PSK
#define AUTH_WPA2_PSK WIFI_AUTH_WPA2_PSK
#define AUTH_WPA_WPA2_PSK WIFI_AUTH_WPA_WPA2_PSK
#define ENC_TYPE_NONE AUTH_OPEN
#define WiFiMode_t wifi_mode_t
#endif //ARDUINO_ARCH_ESP32
#ifdef ARDUINO_ARCH_ESP8266
#include <ESP8266WiFi.h>
#endif //ARDUINO_ARCH_ESP8266

#ifndef _WIFI_CONFIG_H
#define _WIFI_CONFIG_H

class WiFiConfig
{
public:
    static bool isPasswordValid (const char * password);
    static bool isSSIDValid (const char * ssid);
    static bool StartAP(bool setupMode = false);
    static bool StartSTA();
    static void StopWiFi();
    static int32_t getSignal (int32_t RSSI, bool filter=true);
    static const char* getSleepModeString ();
    static const char* getPHYModeString (uint8_t wifimode);
    static bool is_AP_visible();
    static const char * AP_SSID();
    static const char * hostname();
    static const char * AP_Auth_String();
    static const char * AP_Gateway_String();
    static const char * AP_Mask_String();
    static const char*  getConnectedSTA(uint8_t * totalcount = NULL, bool reset = false);
    static bool started();
    static bool begin(int8_t & espMode);
    static void end();
    static void handle();
private :
    static bool ConnectSTA2AP();
};

#endif //_WIFI_CONFIG_H
