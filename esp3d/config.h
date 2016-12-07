/*
  config.h - ESP3D configuration class

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

//definition
#define REPETIER		0
#define REPETIER4DV	    1
#define MARLIN		2
#define MARLINKIMBRA		3
#define SMOOTHIEWARE	    4

//FIRMWARE_TARGET: the targeted FW, can be REPETIER (Original Repetier)/ REPETIER4DV (Repetier for Davinci) / MARLIN (Marlin)/ SMOOTHIEWARE (Smoothieware)
#define FIRMWARE_TARGET REPETIER4DV


//number of clients allowed to use data port at once
#define MAX_SRV_CLIENTS 1

//comment to disable
//MDNS_FEATURE: this feature allow  type the name defined
//in web browser by default: http:\\esp8266.local and connect
#define MDNS_FEATURE

//SSDD_FEATURE: this feature is a discovery protocol, supported on Windows out of the box
#define SSDP_FEATURE

//NETBIOS_FEATURE: this feature is a discovery protocol, supported on Windows out of the box
//#define NETBIOS_FEATURE

//CAPTIVE_PORTAL_FEATURE: In SoftAP redirect all unknow call to main page
#define CAPTIVE_PORTAL_FEATURE

//AUTHENTICATION_FEATURE: protect pages by login password
//#define AUTHENTICATION_FEATURE

//WEB_UPDATE_FEATURE: allow to flash fw using web UI
#define WEB_UPDATE_FEATURE

//SERIAL_COMMAND_FEATURE: allow to send command by serial
#define SERIAL_COMMAND_FEATURE

//TCP_IP_DATA_FEATURE: allow to connect serial from TCP/IP
#define TCP_IP_DATA_FEATURE

//RECOVERY_FEATURE: allow to use GPIO2 pin as hardware reset for EEPROM, add 8s to boot time to let user to jump GPIO2 to GND
//#define RECOVERY_FEATURE

#ifdef RECOVERY_FEATURE
//pin used to reset setting
#define RESET_CONFIG_PIN 2
#endif

//DIRECT_PIN_FEATURE: allow to access pin using ESP201 command
#define DIRECT_PIN_FEATURE

//INFO_MSG_FEATURE: catch the Info msg and filter it to specific table
#define INFO_MSG_FEATURE

//ERROR_MSG_FEATURE: catch the error msg and filter it to specific table
#define ERROR_MSG_FEATURE

//STATUS_MSG_FEATURE: catch the status msg and filter it to specific table
#define STATUS_MSG_FEATURE

//TEMP_MONITORING_FEATURE : catch the specific answer and store it to variable
#define TEMP_MONITORING_FEATURE
//SPEED_MONITORING_FEATURE : catch the specific answer and store it to variable
#define SPEED_MONITORING_FEATURE
//POS_MONITORING_FEATURE : catch the specific answer and store it to variable
#define POS_MONITORING_FEATURE
//FLOW_MONITORING_FEATURE : catch the specific answer and store it to variable
#define FLOW_MONITORING_FEATURE


//Serial rx buffer size is 256 but can be extended
#define SERIAL_RX_BUFFER_SIZE 512

//DEBUG Flag do not do this when connected to printer unless you know what you are doing!!!
//#define DEBUG_ESP3D
//#define DEBUG_OUTPUT_SPIFFS
//#define DEBUG_OUTPUT_SD
//#define DEBUG_OUTPUT_SERIAL
//#define DEBUG_OUTPUT_TCP

//store performance result in storestring variable : info_msg / status_msg
//#define DEBUG_PERFORMANCE
#define DEBUG_PERF_VARIABLE  (web_interface->info_msg)

#ifdef DEBUG_ESP3D
#ifdef DEBUG_OUTPUT_SPIFFS
    /*#ifdef SDCARD_FEATURE
    #ifndef FS_NO_GLOBALS
    #define FS_NO_GLOBALS
    #endif
    #endif
    #include <FS.h>*/
    #define DEBUG_PIPE NO_PIPE
    #define LOG(string) {FSFILE logfile = SPIFFS.open("/log.txt", "a+");logfile.print(string);logfile.close();}
#endif
#ifdef DEBUG_OUTPUT_SERIAL
        #define LOG(string) {Serial.print(string);}
        #define DEBUG_PIPE SERIAL_PIPE
#endif
#ifdef DEBUG_OUTPUT_TCP
        #include "bridge.h"
        #define LOG(string) {BRIDGE::send2TCP(string);}
        #define DEBUG_PIPE TCP_PIPE
#endif
#else
#define LOG(string) {}
#define DEBUG_PIPE NO_PIPE
#endif

#ifdef SDCARD_FEATURE
#define FSFILE fs::File
#define FSDIR fs::Dir
#define FSINFO fs::FSInfo
#else
#define FSFILE File
#define FSDIR fs::Dir
#define FSINFO FSInfo
#endif

#ifndef TCP_IP_DATA_FEATURE
#undef MAX_SRV_CLIENTS
#define MAX_SRV_CLIENTS 0
#endif

#ifndef CONFIG_h
#define CONFIG_h

#include <Arduino.h>
extern "C" {
#include "user_interface.h"
}
#include "wifi.h"
//version and sources location
#define FW_VERSION "0.9.76"
#define REPOSITORY "https://github.com/luc-github/ESP3D"

typedef enum {
    NO_PIPE = 0, 
    SERIAL_PIPE = 2, 
    SERIAL1_PIPE = 3,
#ifdef TCP_IP_DATA_FEATURE
    TCP_PIPE = 4, 
#endif
    WEB_PIPE = 5
    } tpipe;  

//flags
#define AP_MODE			1
#define CLIENT_MODE		2
#define DHCP_MODE		1
#define STATIC_IP_MODE		2

//position in EEPROM
//AP mode = 1; Station client mode = 2
#define EP_WIFI_MODE			0    //1 byte = flag
#define EP_STA_SSID				1    //33 bytes 32+1 = string  ; warning does not support multibyte char like chinese
#define EP_STA_PASSWORD			34   //65 bytes 64 +1 = string ;warning does not support multibyte char like chinese
#define EP_STA_IP_MODE			99   //1 byte = flag
#define EP_STA_IP_VALUE			100  //4  bytes xxx.xxx.xxx.xxx
#define EP_STA_MASK_VALUE			104  //4  bytes xxx.xxx.xxx.xxx
#define EP_STA_GATEWAY_VALUE			108  //4  bytes xxx.xxx.xxx.xxx
#define EP_BAUD_RATE			112  //4  bytes = int
#define EP_STA_PHY_MODE			116  //1 byte = flag
#define EP_SLEEP_MODE			117  //1 byte = flag
#define EP_CHANNEL			118 //1 byte = flag
#define EP_AUTH_TYPE			119 //1 byte = flag
#define EP_SSID_VISIBLE			120 //1 byte = flag
#define EP_WEB_PORT			121 //4  bytes = int
#define EP_DATA_PORT			125 //4  bytes = int
#define EP_REFRESH_PAGE_TIME			129 //1  bytes = flag
#define EP_HOSTNAME				130//33 bytes 32+1 = string  ; warning does not support multibyte char like chinese
#define EP_XY_FEEDRATE		    164//4  bytes = int
#define EP_Z_FEEDRATE		    168//4  bytes = int
#define EP_E_FEEDRATE		    172//4  bytes = int
#define EP_ADMIN_PWD		    176//21  bytes 20+1 = string  ; warning does not support multibyte char like chinese
#define EP_USER_PWD		    197//21  bytes 20+1 = string  ; warning does not support multibyte char like chinese
#define EP_AP_SSID				218    //33 bytes 32+1 = string  ; warning does not support multibyte char like chinese
#define EP_AP_PASSWORD			251   //65 bytes 64 +1 = string ;warning does not support multibyte char like chinese
#define EP_AP_IP_VALUE			316  //4  bytes xxx.xxx.xxx.xxx
#define EP_AP_MASK_VALUE			320  //4  bytes xxx.xxx.xxx.xxx
#define EP_AP_GATEWAY_VALUE			324  //4  bytes xxx.xxx.xxx.xxx
#define EP_AP_IP_MODE			329   //1 byte = flag
#define EP_AP_PHY_MODE			182  //1 byte = flag
//next available is 330
//space left 512 - 330 = 18

//default values
#define DEFAULT_WIFI_MODE			AP_MODE
const char DEFAULT_AP_SSID []  PROGMEM =		"ESP8266";
const char DEFAULT_AP_PASSWORD [] PROGMEM =	"12345678";
const char DEFAULT_STA_SSID []  PROGMEM =		"ESP8266";
const char DEFAULT_STA_PASSWORD [] PROGMEM =	"12345678";
const byte DEFAULT_STA_IP_MODE  = 				DHCP_MODE;
const byte DEFAULT_AP_IP_MODE = 				STATIC_IP_MODE;
const byte DEFAULT_IP_VALUE[]   =	        {192, 168, 0, 1};
const byte DEFAULT_MASK_VALUE[]  =	        {255, 255, 255, 0};
#define DEFAULT_GATEWAY_VALUE   	        DEFAULT_IP_VALUE
const long DEFAULT_BAUD_RATE =			115200;
const char M117_[] PROGMEM =		"M117 ";
#define DEFAULT_PHY_MODE			WIFI_PHY_MODE_11G
#define DEFAULT_SLEEP_MODE			WIFI_MODEM_SLEEP
#define DEFAULT_CHANNEL				11
#define DEFAULT_AUTH_TYPE			AUTH_WPA_PSK
#define DEFAULT_SSID_VISIBLE			1
#define DEFAULT_MAX_CONNECTIONS			4
#define DEFAULT_BEACON_INTERVAL			100
const int DEFAULT_WEB_PORT =			80;
const int DEFAULT_DATA_PORT =			8888;
#define DEFAULT_REFRESH_PAGE_TIME			3
const int  DEFAULT_XY_FEEDRATE=1000;
const int  DEFAULT_Z_FEEDRATE	=100;
const int  DEFAULT_E_FEEDRATE=400;
const char DEFAULT_ADMIN_PWD []  PROGMEM =	"admin";
const char DEFAULT_USER_PWD []  PROGMEM =	"user";
const char DEFAULT_ADMIN_LOGIN []  PROGMEM =	"admin";
const char DEFAULT_USER_LOGIN []  PROGMEM =	"user";


//sizes
#define EEPROM_SIZE				512 //max is 512
#define MAX_SSID_LENGTH				32
#define MIN_SSID_LENGTH				1
#define MAX_PASSWORD_LENGTH 			64
#define MIN_PASSWORD_LENGTH 			8
#define MAX_LOCAL_PASSWORD_LENGTH 			16
#define MIN_LOCAL_PASSWORD_LENGTH 			1
#define IP_LENGTH 				4
#define INTEGER_LENGTH 				4
#define MAX_HOSTNAME_LENGTH		32
#define WL_MAC_ADDR_LENGTH 6

class CONFIG
{
public:
    static bool read_string(int pos, char byte_buffer[], int size_max);
    static bool read_string(int pos, String & sbuffer, int size_max);
    static bool read_buffer(int pos, byte byte_buffer[], int size_buffer);
    static bool read_byte(int pos, byte * value);
    static bool write_string(int pos, const char * byte_buffer);
    static bool write_string(int pos, const __FlashStringHelper *str);
    static bool write_buffer(int pos, const byte * byte_buffer, int size_buffer);
    static bool write_byte(int pos, const byte value);
    static bool reset_config();
    static void print_config(tpipe output);
    static bool isHostnameValid(const char * hostname);
    static bool isSSIDValid(const char * ssid);
    static bool isPasswordValid(const char * password);
    static bool isLocalPasswordValid(const char * password);
    static bool isIPValid(const char * IP);
    static char * intTostr(int value);
    static String formatBytes(size_t bytes);
    static char * mac2str(uint8_t mac [WL_MAC_ADDR_LENGTH]);
    static byte split_ip (const char * ptr,byte * part);
    static void esp_restart();
    static void flashfromSD(const char * Filename, int flashtype);
};

#endif
