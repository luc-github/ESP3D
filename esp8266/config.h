/*
  CONFIG.H - esp8266 configuration class

  Copyright (c) 2014 Luc Lebosse. All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

//comment to disable
//MDNS_FEATURE: this feature allow in Station mode to enter the name defined
//in web browser by default : http:\\esp8266.local and connect 
//this feature does not work in AP mode
#define MDNS_FEATURE

//SSDD_FEATURE: this feature is a discovery protocol, supported on Windows out of the box
#define SSDP_FEATURE

#define PROGMEM2CHAR progmem2char

extern char * progmem2char(const char* src);
#ifndef CONFIG_h
#define CONFIG_h

#include <Arduino.h>
#include "wifi.h"
extern "C" {
#include "user_interface.h"
}
//version and sources location
#define FW_VERSION "V0.2"
#define REPOSITORY "https://github.com/luc-github/ESP8266"


//pin used to reset setting
#define RESET_CONFIG_PIN 2

//flags
#define AP_MODE			1
#define CLIENT_MODE		2
#define DHCP_MODE		1
#define STATIC_IP_MODE		2

//position in EEPROM
//AP mode = 1; Station client mode = 2
#define EP_WIFI_MODE			0    //1 byte = flag
#define EP_SSID				1    //33 bytes 32+1 = string  ; warning does not support multibyte char like chinese
#define EP_PASSWORD			34   //65 bytes 64 +1 = string ;warning does not support multibyte char like chinese
#define EP_IP_MODE			99   //1 byte = flag
#define EP_IP_VALUE			100  //4  bytes xxx.xxx.xxx.xxx
#define EP_MASK_VALUE			104  //4  bytes xxx.xxx.xxx.xxx
#define EP_GATEWAY_VALUE		108  //4  bytes xxx.xxx.xxx.xxx
#define EP_BAUD_RATE			112  //4  bytes = int
#define EP_PHY_MODE			116  //1 byte = flag
#define EP_SLEEP_MODE			117  //1 byte = flag
#define EP_CHANNEL			118 //1 byte = flag
#define EP_AUTH_TYPE			119 //1 byte = flag
#define EP_SSID_VISIBLE			120 //1 byte = flag
#define EP_WEB_PORT			121 //4  bytes = int
#define EP_DATA_PORT			125 //4  bytes = int
#define EP_POLLING_TIME			129 //1  bytes = flag



//default values
#define DEFAULT_WIFI_MODE			AP_MODE
const char DEFAULT_SSID []  PROGMEM =		"ESP8266";
const char DEFAULT_PASSWORD [] PROGMEM =	"12345678";
#define DEFAULT_IP_MODE				STATIC_IP_MODE
const byte DEFAULT_IP_VALUE[]   =	        {192, 168, 0, 1};
const byte DEFAULT_MASK_VALUE[]  =	        {255, 255, 255, 0};
#define DEFAULT_GATEWAY_VALUE   	        DEFAULT_IP_VALUE
const long DEFAULT_BAUD_RATE =			9600;
#ifdef MDNS_FEATURE
const char LOCAL_NAME[] PROGMEM =		"esp8266";
#endif
const char M117_[] PROGMEM =		"M117 ";
#define DEFAULT_PHY_MODE			PHY_MODE_11G
#define DEFAULT_SLEEP_MODE			MODEM_SLEEP_T
#define DEFAULT_CHANNEL				11
#define DEFAULT_AUTH_TYPE			AUTH_WPA_PSK
#define DEFAULT_SSID_VISIBLE			1
#define DEFAULT_MAX_CONNECTIONS			4
#define DEFAULT_BEACON_INTERVAL			100
const int DEFAULT_WEB_PORT =			80;
const int DEFAULT_DATA_PORT =			8888;
#define DEFAULT_POLLING_TIME			3

//sizes
#define EEPROM_SIZE				256 //max is 512
#define MAX_SSID_LENGH				32
#define MIN_SSID_LENGH				1
#define MAX_PASSWORD_LENGH 			64
#define MIN_PASSWORD_LENGH 			8
#define IP_LENGH 				4
#define INTEGER_LENGH 				4

class CONFIG
{
  public:
    static bool read_string(word pos, char byte_buffer[], word size_max);
    static bool read_buffer(word pos, byte byte_buffer[], word size_buffer);
    static bool read_byte(word pos, byte * value);
    static bool write_string(word pos, const char * byte_buffer, word size_buffer);
    static bool write_buffer(word pos, const byte * byte_buffer, word size_buffer);
    static bool write_byte(word pos, const byte value);
    static bool reset_config();
    static void print_config();
};

#endif
