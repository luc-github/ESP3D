
/*
  settings_esp3d.h -  settings esp3d functions class

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



#ifndef _SETTINGS_ESP3D_H
#define _SETTINGS_ESP3D_H

//Supported FW /////////////////////////////////////////////////////////////
#define UNKNOWN_FW      0
#define GRBL            10
#define MARLIN          20
#define MARLIN_EMBEDDED 30
#define SMOOTHIEWARE    40
#define REPETIER        50
#define FLUIDNC         60
#define REPRAP          70

//Default flags
#define DEFAULT_SERIAL_OUTPUT_FLAG 1
#define DEFAULT_REMOTE_SCREEN_FLAG 1
#define DEFAULT_WEBSOCKET_FLAG 1
#define DEFAULT_TELNET_FLAG 1
#define DEFAULT_BT_FLAG 1
#define DEFAULT_SCREEN_FLAG 1

//position in EEPROM / preferences will use `P_` + <position> to make a string : P_0 for 0
#define ESP_RADIO_MODE          0       //1 byte = flag
#define ESP_STA_SSID            1       //33 bytes 32+1 = string  ; warning does not support multibyte char like chinese
#define ESP_STA_PASSWORD        34      //65 bytes 64 +1 = string ;warning does not support multibyte char like chinese
#define ESP_STA_IP_MODE         99      //1 byte = flag
#define ESP_STA_IP_VALUE        100     //4  bytes xxx.xxx.xxx.xxx
#define ESP_STA_MASK_VALUE      104     //4  bytes xxx.xxx.xxx.xxx
#define ESP_STA_GATEWAY_VALUE   108     //4  bytes xxx.xxx.xxx.xxx
#define ESP_BAUD_RATE           112     //4  bytes = int
#define ESP_NOTIFICATION_TYPE   116     //1 byte = flag
#define ESP_CALIBRATION         117     //1 byte = flag
#define ESP_AP_CHANNEL          118     //1 byte = flag
#define ESP_BUZZER              119     //1 byte = flag
#define ESP_INTERNET_TIME       120     //1  byte = flag
#define ESP_HTTP_PORT           121     //4  bytes = int
#define ESP_TELNET_PORT         125     //4  bytes = int
#define ESP_SERIAL_FLAG         129     //1  bytes = flag
#define ESP_HOSTNAME            130     //33 bytes 32+1 = string  ; warning does not support multibyte char like chinese
#define ESP_SENSOR_INTERVAL     164     //4  bytes = int
#define ESP_SETTINGS_VERSION    168     //8  bytes = 7+1 = string ESP3D + 2 digits
#define ESP_ADMIN_PWD           176     //21  bytes 20+1 = string  ; warning does not support multibyte char like chinese
#define ESP_USER_PWD            197     //21  bytes 20+1 = string  ; warning does not support multibyte char like chinese
#define ESP_AP_SSID             218     //33 bytes 32+1 = string  ; warning does not support multibyte char like chinese
#define ESP_AP_PASSWORD         251     //65 bytes 64 +1 = string ;warning does not support multibyte char like chinese
#define ESP_AP_IP_VALUE         316     //4  bytes xxx.xxx.xxx.xxx
#define ESP_BOOT_DELAY          320     //4  bytes = int
#define ESP_WEBSOCKET_PORT      324     //4  bytes= int
#define ESP_HTTP_ON             328     //1 byte = flag
#define ESP_TELNET_ON           329     //1 byte = flag
#define ESP_WEBSOCKET_ON        330     //1 byte = flag
#define ESP_SD_SPEED_DIV        331     //1 byte = flag
#define ESP_NOTIFICATION_TOKEN1 332     //64 bytes 63+1 = string  ; warning does not support multibyte char like chinese
#define ESP_NOTIFICATION_TOKEN2 396     //64 bytes 63+1 = string  ; warning does not support multibyte char like chinese
#define ESP_SENSOR_TYPE         460     //1  bytes = flag
#define ESP_TARGET_FW           461     //1  bytes = flag
#define ESP_TIMEZONE            462     //1  bytes = flag
#define ESP_TIME_IS_DST         463     //1  bytes = flag
#define ESP_TIME_SERVER1        464     //129 bytes 128+1 = string  ; warning does not support multibyte char like chinese
#define ESP_TIME_SERVER2        593     //129 bytes 128+1 = string  ; warning does not support multibyte char like chinese
#define ESP_TIME_SERVER3        722     //129 bytes 128+1 = string  ; warning does not support multibyte char like chinese
#define ESP_REMOTE_SCREEN_FLAG    851     //1  bytes = flag
#define ESP_SD_MOUNT            852     //1  bytes = flag
#define ESP_SESSION_TIMEOUT     853     //1  bytes = flag
#define ESP_WEBSOCKET_FLAG      854     //1  bytes = flag
#define ESP_SD_CHECK_UPDATE_AT_BOOT   855//1  bytes = flag
#define ESP_NOTIFICATION_SETTINGS 856   //129 bytes 128+1 = string  ; warning does not support multibyte char like chinese
#define ESP_CALIBRATION_1       985     //4  bytes = int
#define ESP_CALIBRATION_2       989     //4  bytes = int
#define ESP_CALIBRATION_3       993     //4  bytes = int 
#define ESP_CALIBRATION_4       997     //4  bytes = int
#define ESP_CALIBRATION_5       1001     //4  bytes = int
#define ESP_SETUP               1005    //1 byte = flag
#define ESP_TELNET_FLAG         1006    //1 byte = flag
#define ESP_BT_FLAG             1007    //1 byte = flag
#define ESP_SCREEN_FLAG            1008    //1 byte = flag
#define ESP_FTP_CTRL_PORT       1009    //4  bytes = int
#define ESP_FTP_DATA_ACTIVE_PORT       1013    //4  bytes = int
#define ESP_FTP_DATA_PASSIVE_PORT      1017    //4  bytes = int
#define ESP_FTP_ON             1021     //1 byte = flag
#define ESP_AUTO_NOTIFICATION   1022    //1 byte = flag
#define ESP_VERBOSE_BOOT        1023    //1 byte = flag
#define ESP_WEBDAV_ON           1024    //1 byte = flag
#define ESP_WEBDAV_PORT         1025    //4  bytes= int
#define ESP_STA_DNS_VALUE       1029    //4  bytes= int
#define ESP_SECURE_SERIAL       1033    //1 byte = flag
#define ESP_BOOT_RADIO_STATE    1034    //1 byte = flag
#define ESP_STA_FALLBACK_MODE   1035    //1 byte = flag


//Hidden password
#define HIDDEN_PASSWORD "********"


#include <Arduino.h>

class Settings_ESP3D
{
public:
    static bool begin();
    static uint8_t get_default_byte_value(int pos);
    static uint32_t get_default_int32_value(int pos);
    static uint32_t get_default_IP_value(int pos);
    static const String & get_default_string_value(int pos);
    static uint8_t get_max_string_size(int pos);
    static uint8_t get_min_string_size(int pos);
    static uint32_t get_max_int32_value(int pos);
    static uint32_t get_min_int32_value(int pos);
    static uint8_t get_max_byte(int pos);
    static int8_t get_min_byte(int pos);
    static uint8_t read_byte (int pos, bool * haserror = NULL);
    static uint32_t read_uint32(int pos, bool * haserror = NULL);
    static uint32_t read_IP(int pos, bool * haserror = NULL);
    static String read_IP_String(int pos, bool * haserror = NULL);
    static const char * read_string (int pos, bool *haserror = NULL);
    static bool write_byte (int pos, const uint8_t value);
    static bool write_string (int pos, const char * byte_buffer);
    static bool write_uint32 (int pos, const uint32_t value);
    static bool write_IP (int pos, const uint32_t value);
    static bool write_IP_String (int pos, const char * value);
    static bool reset(bool networkonly = false);
    static int8_t GetSettingsVersion();
    static uint8_t GetFirmwareTarget(bool fromsettings = false);
    static bool isVerboseBoot(bool fromsettings = false);
    static uint8_t GetSDDevice();
    static const char* GetFirmwareTargetShortName();
    static String IPtoString(uint32_t ip_int);
    static uint32_t StringtoIP(const char *s);
    static const char * TargetBoard();
    static bool isLocalPasswordValid (const char * password);
private:
    static bool is_string(const char * s, uint len);
    static uint8_t _FirmwareTarget;
    static bool _isverboseboot;
};


#endif //_SETTINGS_ESP3D_H

