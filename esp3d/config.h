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
#define UNKNOWN_FW 0
#define REPETIER4DV	1
#define MARLIN		2
#define MARLINKIMBRA		3
#define SMOOTHIEWARE	4
#define REPETIER		5

#ifdef ARDUINO_ARCH_ESP32
#include "FS.h"
#include "SPIFFS.h"
#define WIFI_NONE_SLEEP WIFI_PS_NONE
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
#define FS_FILE File
#define FS_DIR File
#define ESP_SERIAL_OUT Serial
#define SD_FILE_READ FILE_READ
#define SPIFFS_FILE_READ FILE_READ
#define SD_FILE_WRITE FILE_WRITE
#define SPIFFS_FILE_WRITE FILE_WRITE

extern HardwareSerial Serial2;
#else
#define FS_DIR fs::Dir
#define FS_FILE fs::File
#define ESP_SERIAL_OUT Serial
#define SD_FILE_READ FILE_READ
#define SPIFFS_FILE_READ "r"
#define SD_FILE_WRITE FILE_WRITE
#define SPIFFS_FILE_WRITE "w"
#endif

#define MAX_FW_ID REPETIER

//number of clients allowed to use data port at once
#define MAX_SRV_CLIENTS 1

//comment to disable
//MDNS_FEATURE: this feature allow  type the name defined
//in web browser by default: http:\\esp8266.local and connect
#define MDNS_FEATURE

//SSDD_FEATURE: this feature is a discovery protocol, supported on Windows out of the box
#define SSDP_FEATURE

//NETBIOS_FEATURE: this feature is a discovery protocol, supported on Windows out of the box
#define NETBIOS_FEATURE

#ifdef ARDUINO_ARCH_ESP32
#ifdef SSDP_FEATURE
#undef SSDP_FEATURE
#endif
#ifdef NETBIOS_FEATURE
#undef NETBIOS_FEATURE
#endif
#endif

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

//Serial rx buffer size is 256 but can be extended
#define SERIAL_RX_BUFFER_SIZE 512

#ifdef ARDUINO_ARCH_ESP32
#ifdef SSDP_FEATURE
#undef SSDP_FEATURE
#endif
#ifdef NETBIOS_FEATURE
#undef NETBIOS_FEATURE
#endif
#endif


//DEBUG Flag do not do this when connected to printer !!!
//be noted all upload may failed if enabled
//#define DEBUG_ESP3D
//#define DEBUG_OUTPUT_SPIFFS
//#define DEBUG_OUTPUT_SERIAL
//#define DEBUG_OUTPUT_TCP

//store performance result in storestring variable : info_msg / status_msg
//#define DEBUG_PERFORMANCE
#define DEBUG_PERF_VARIABLE  (web_interface->info_msg)
/*
#ifndef FS_NO_GLOBALS
#define FS_NO_GLOBALS
#endif
#include <FS.h>
#define DEBUG_ESP3D(string) { FS_FILE logfile = SPIFFS.open("/log.txt", "a+");logfile.print(string);logfile.close();}
*/

#ifdef DEBUG_ESP3D
#ifdef DEBUG_OUTPUT_SPIFFS
#ifndef FS_NO_GLOBALS
#define FS_NO_GLOBALS
#endif
#include <FS.h>
#define DEBUG_PIPE NO_PIPE
#define LOG(string) { FS_FILE logfile = SPIFFS.open("/log.txt", "a+");logfile.print(string);logfile.close();}
#endif
#ifdef DEBUG_OUTPUT_SERIAL
#define LOG(string) {ESP_SERIAL_OUT.print(string);}
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

#ifndef CONFIG_h
#define CONFIG_h

#include <Arduino.h>
#ifdef ARDUINO_ARCH_ESP8266
extern "C" {
#include "user_interface.h"
}
#else
//Nothing here
#endif
#include "wificonf.h"
//version and sources location
#define FW_VERSION "1.0"
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

typedef enum {
    LEVEL_GUEST = 0,
    LEVEL_USER = 1,
    LEVEL_ADMIN = 2
} level_authenticate_type;


#define    NO_SD 0
#define    SD_DIRECTORY 1
#define    EXT_DIRECTORY 2


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
#define EP_AP_PHY_MODE			330  //1 byte = flag
#define EP_DATA_STRING			331  //129 bytes 128+1 = string  ; warning does not support multibyte char like chinese
#define EP_REFRESH_PAGE_TIME2		460 //1  bytes = flag
#define EP_TARGET_FW		461 //1  bytes = flag
#define EP_TIMEZONE         462//1  bytes = flag
#define EP_TIME_ISDST       463//1  bytes = flag
#define EP_TIME_SERVER1 464//129 bytes 128+1 = string  ; warning does not support multibyte char like chinese  
#define EP_TIME_SERVER2  593 //129 bytes 128+1 = string  ; warning does not support multibyte char like chinese
#define EP_TIME_SERVER3  722 //129 bytes 128+1 = string  ; warning does not support multibyte char like chinese
#define EP_IS_DIRECT_SD   850//1  bytes = flag
#define EP_PRIMARY_SD   851//1  bytes = flag
#define EP_SECONDARY_SD   852//1  bytes = flag
#define EP_DIRECT_SD_CHECK   853//1  bytes = flag
#define EP_SD_CHECK_UPDATE_AT_BOOT   854//1  bytes = flag

#define LAST_EEPROM_ADDRESS 855
//next available is 855
//space left 1024 - 855 = 169

//default values
#define DEFAULT_WIFI_MODE			AP_MODE
const char DEFAULT_AP_SSID []  PROGMEM =		"ESP3D";
const char DEFAULT_AP_PASSWORD [] PROGMEM =	"12345678";
const char DEFAULT_STA_SSID []  PROGMEM =		"ESP3D";
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
const char DEFAULT_TIME_SERVER1 []  PROGMEM =	"time.nist.gov";
const char DEFAULT_TIME_SERVER2 []  PROGMEM =	"0.pool.ntp.org";
const char DEFAULT_TIME_SERVER3 []  PROGMEM =	"1.pool.ntp.org";
#define DEFAULT_TIME_ZONE			0
#define DEFAULT_TIME_DST			0
#define DEFAULT_PRIMARY_SD  1
#define DEFAULT_SECONDARY_SD 2
#define DEFAULT_DIRECT_SD_CHECK 0
#define DEFAULT_SD_CHECK_UPDATE_AT_BOOT 1


#define DEFAULT_IS_DIRECT_SD 0




const uint16_t Setting[][2] = {
    {EP_WIFI_MODE, LEVEL_ADMIN},//0
    {EP_STA_SSID, LEVEL_ADMIN},//1
    {EP_STA_PASSWORD, LEVEL_ADMIN},//2
    {EP_STA_IP_MODE, LEVEL_ADMIN},//3
    {EP_STA_IP_VALUE, LEVEL_ADMIN},//4
    {EP_STA_MASK_VALUE, LEVEL_ADMIN},//5
    {EP_STA_GATEWAY_VALUE, LEVEL_ADMIN},//6
    {EP_BAUD_RATE, LEVEL_ADMIN},//7
    {EP_STA_PHY_MODE, LEVEL_ADMIN},//8
    {EP_SLEEP_MODE, LEVEL_ADMIN},//9
    {EP_CHANNEL, LEVEL_ADMIN},//10
    {EP_AUTH_TYPE, LEVEL_ADMIN},//11
    {EP_SSID_VISIBLE, LEVEL_ADMIN},//12
    {EP_WEB_PORT, LEVEL_ADMIN},//13
    {EP_DATA_PORT, LEVEL_ADMIN},//14
    {EP_REFRESH_PAGE_TIME, LEVEL_USER},//15
    {EP_HOSTNAME, LEVEL_ADMIN},//16
    {EP_XY_FEEDRATE, LEVEL_USER},//17
    {EP_Z_FEEDRATE, LEVEL_USER},//18
    {EP_E_FEEDRATE, LEVEL_USER},//19
    {EP_ADMIN_PWD, LEVEL_ADMIN},//20
    {EP_USER_PWD, LEVEL_USER},//21
    {EP_AP_SSID, LEVEL_ADMIN},//22
    {EP_AP_PASSWORD, LEVEL_ADMIN},//23
    {EP_AP_IP_VALUE, LEVEL_ADMIN},//24
    {EP_AP_MASK_VALUE, LEVEL_ADMIN},//25
    {EP_AP_GATEWAY_VALUE, LEVEL_ADMIN},//26
    {EP_AP_IP_MODE, LEVEL_ADMIN},//27
    {EP_AP_PHY_MODE, LEVEL_ADMIN},//28
    {EP_DATA_STRING, LEVEL_USER},//29
    {EP_REFRESH_PAGE_TIME2, LEVEL_USER},//30
    {EP_TARGET_FW, LEVEL_USER},//31
    {EP_TIMEZONE, LEVEL_USER},//32
    {EP_TIME_ISDST, LEVEL_USER},//33
    {EP_TIME_SERVER1, LEVEL_USER},//34
    {EP_TIME_SERVER2, LEVEL_USER},//35
    {EP_TIME_SERVER3, LEVEL_USER},//36
    {EP_IS_DIRECT_SD, LEVEL_USER},//37
    {EP_PRIMARY_SD, LEVEL_USER},//38
    {EP_SECONDARY_SD, LEVEL_USER},//39
    {EP_DIRECT_SD_CHECK, LEVEL_USER}, //40
    {EP_SD_CHECK_UPDATE_AT_BOOT, LEVEL_USER} //41
};
#define AUTH_ENTRY_NB 42
//values
#define DEFAULT_MAX_REFRESH			120
#define DEFAULT_MIN_REFRESH			0
#define DEFAULT_MAX_XY_FEEDRATE			9999
#define DEFAULT_MIN_XY_FEEDRATE			1
#define DEFAULT_MAX_Z_FEEDRATE			9999
#define DEFAULT_MIN_Z_FEEDRATE			1
#define DEFAULT_MAX_E_FEEDRATE			9999
#define DEFAULT_MIN_E_FEEDRATE			1
#define DEFAULT_MAX_WEB_PORT			65001
#define DEFAULT_MIN_WEB_PORT			1
#define DEFAULT_MAX_DATA_PORT			65001
#define DEFAULT_MIN_DATA_PORT			1

#define MAX_TRY 2000

//sizes
#define EEPROM_SIZE				1024 //max is 1024
#define MAX_SSID_LENGTH				32
#define MIN_SSID_LENGTH				1
#define MAX_PASSWORD_LENGTH 			64
//min size of password is 0 or upper than 8 char
//so let set min is 0
#define MIN_PASSWORD_LENGTH 			0
#define MAX_LOCAL_PASSWORD_LENGTH 			16
#define MIN_LOCAL_PASSWORD_LENGTH 			1
#define MAX_DATA_LENGTH				128
#define MIN_DATA_LENGTH				0
#define IP_LENGTH 				4
#define INTEGER_LENGTH 				4
#define MAX_HOSTNAME_LENGTH		32
#define MIN_HOSTNAME_LENGTH		1
#define WL_MAC_ADDR_LENGTH 6

class CONFIG
{
public:
    static bool is_direct_sd;
    static bool read_string(int pos, char byte_buffer[], int size_max);
    static bool read_string(int pos, String & sbuffer, int size_max);
    static bool read_buffer(int pos, byte byte_buffer[], int size_buffer);
    static bool read_byte(int pos, byte * value);
    static bool write_string(int pos, const char * byte_buffer);
    static bool write_string(int pos, const __FlashStringHelper *str);
    static bool write_buffer(int pos, const byte * byte_buffer, int size_buffer);
    static bool write_byte(int pos, const byte value);
    static bool reset_config();
    static void print_config(tpipe output, bool plaintext);
    static bool SetFirmwareTarget(uint8_t fw);
    static void InitFirmwareTarget();
    static void InitDirectSD();
    static void InitPins();
    static bool InitBaudrate();
    static bool InitExternalPorts();
    static bool check_update_presence();
    static uint8_t GetFirmwareTarget();
    static const char* GetFirmwareTargetName();
    static const char* GetFirmwareTargetShortName();
    static bool isHostnameValid(const char * hostname);
    static bool isSSIDValid(const char * ssid);
    static bool isPasswordValid(const char * password);
    static bool isLocalPasswordValid(const char * password);
    static bool isIPValid(const char * IP);
    static char * intTostr(int value);
    static String formatBytes(uint32_t bytes);
    static char * mac2str(uint8_t mac [WL_MAC_ADDR_LENGTH]);
    static byte split_ip (const char * ptr,byte * part);
    static void esp_restart();
private:
    static uint8_t FirmwareTarget;
};

#endif
