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

#ifndef CONFIG_h
#define CONFIG_h

#include <Arduino.h>
//version and sources location
#define FW_VERSION "V0.1"
#define REPOSITORY "https://github.com/luc-github/ESP8266"

#define MDNS_FEATURE  1

//pin used to reset setting
#define RESET_CONFIG_PIN 2

//flags
#define AP_MODE            1
#define CLIENT_MODE        2
#define DHCP_MODE          1 
#define STATIC_IP_MODE     2

//position in EEPROM
//AP mode = 1; Station client mode = 2
#define EP_WIFI_MODE       0    //1 byte = flag
#define EP_SSID            1    //33 bytes 32+1 = string  ; warning does not support multibyte char like chinese
#define EP_PASSWORD        34   //65 bytes 64 +1 = string ;warning does not support multibyte char like chinese
#define EP_IP_MODE         99   //1 byte = flag
#define EP_IP_VALUE        100  //16 bytes xxx.xxx.xxx\0 = string
#define EP_MASK_VALUE      116  //16 bytes xxx.xxx.xxx\0 = string
#define EP_GATEWAY_VALUE   132  //16 bytes xxx.xxx.xxx\0 = string
#define EP_BAUD_RATE       151  //7 bytes = string (if integer value => save 4 bytes but need to create new integer function for eeprom that will take more than 4 bytes)

//default values
#define DEFAULT_WIFI_MODE       AP_MODE
const char DEFAULT_SSID []  PROGMEM =  "ESP8266";
const char DEFAULT_PASSWORD [] PROGMEM = "12345678";
#define DEFAULT_IP_MODE         STATIC_IP_MODE
const char DEFAULT_IP_VALUE[]  PROGMEM = "192.168.0.1";
const char DEFAULT_MASK_VALUE[] PROGMEM = "255.255.255.0";
#define DEFAULT_GATEWAY_VALUE   DEFAULT_IP_VALUE
const char DEFAULT_BAUD_RATE[] PROGMEM = "9600";
#if MDNS_FEATURE
const char LOCAL_NAME[] PROGMEM = "esp8266";
#endif

#define EEPROM_SIZE 256 //max is 512
#define MAX_SSID_LENGH 32
#define MAX_PASSWORD_LENGH 64
#define MAX_IP_LENGH 17
#define MAX_BAUD_LENGH 6

class CONFIG
{
  public:
  static bool read_string(word pos, char byte_buffer[], word size_max);
  static bool read_byte(word pos, byte * value);
  static bool write_string(word pos, const char * byte_buffer, word size_buffer);
  static bool write_byte(word pos, const byte value);
  static bool reset_config();
  static void print_config();
};

#endif
