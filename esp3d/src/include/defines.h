/*
  defines.h - ESP3D defines file

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

#ifndef _DEFINES_ESP3D_H
#define _DEFINES_ESP3D_H

//Settings
#define SETTINGS_IN_EEPROM 1
#define SETTINGS_IN_PREFERENCES 2

//Supported FW /////////////////////////////////////////////////////////////
#define UNKNOWN_FW      0
#define GRBL            10
#define MARLIN          20
#define MARLIN_EMBEDDED 30
#define SMOOTHIEWARE    40
#define REPETIER        50
#define REPRAP          70
#define GRBLHAL         80

//Default flags
#define DEFAULT_SERIAL_OUTPUT_FLAG 1
#define DEFAULT_REMOTE_SCREEN_FLAG 1
#define DEFAULT_WEBSOCKET_FLAG 1
#define DEFAULT_TELNET_FLAG 1
#define DEFAULT_BT_FLAG 1
#define DEFAULT_SCREEN_FLAG 1
#define DEFAULT_SERIAL_BRIDGE_FLAG 1

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
#define ESP_SERIAL_BRIDGE_ON    1036    //1 byte = flag
#define ESP_SERIAL_BRIDGE_FLAG  1037    //1 byte = flag
#define ESP_SERIAL_BRIDGE_BAUD  1038    //4  bytes= int

//Hidden password
#define HIDDEN_PASSWORD "********"

//Debug
#define DEBUG_OUTPUT_SERIAL0    1
#define DEBUG_OUTPUT_SERIAL1    2
#define DEBUG_OUTPUT_SERIAL2    3
#define DEBUG_OUTPUT_TELNET     4
#define DEBUG_OUTPUT_WEBSOCKET  5


//Serial
#define USE_SERIAL_0 1
#define USE_SERIAL_1 2
#define USE_SERIAL_2 3

//Serial service ID
#define MAIN_SERIAL   1
#define BRIDGE_SERIAL 2


//Communication protocols
#define RAW_SERIAL    0
#define MKS_SERIAL    1
#define SOCKET_SERIAL 2

//Display
#define OLED_I2C_SSD1306_128X64        1
#define OLED_I2C_SSDSH1106_132X64      2
#define TFT_SPI_ILI9341_320X240        3
#define TFT_SPI_ILI9488_480X320        4
#define TFT_SPI_ST7789_240X240         5
#define TFT_SPI_ST7789_135X240         6

//UI type for display
#define UI_TYPE_BASIC      1
#define UI_TYPE_ADVANCED   2
#define UI_COLORED         1
#define UI_MONOCHROME      2

//SD connection
#define ESP_NO_SD           0
#define ESP_DIRECT_SD       1
#define ESP_SHARED_SD       2

//Upload type
#define ESP_UPLOAD_DIRECT_SD       1
#define ESP_UPLOAD_SHARED_SD       2
#define ESP_UPLOAD_SERIAL_SD       3
#define ESP_UPLOAD_FAST_SERIAL_SD  4
#define ESP_UPLOAD_FAST_SERIAL_USB 5
#define ESP_UPLOAD_DIRECT_USB      6

//IP mode
#define DHCP_MODE       1
#define STATIC_IP_MODE  0

//Network Mode
#define ESP_NO_NETWORK 0
#define ESP_WIFI_STA   1
#define ESP_WIFI_AP    2
#define ESP_BT         3
#define ESP_ETH_STA    4
#define ESP_AP_SETUP   5
//#define ESP_ETH_SRV  5

//SD mount point
#define ESP_SD_ROOT     1
#define ESP_SD_SUB_SD   2
#define ESP_SD_SUB_EXT   3

//Touch
#define XPT2046_SPI     1

//Input
#define ROTARY_ENCODER  1

//File systems
#define ESP_SPIFFS_FILESYSTEM       1
#define ESP_FAT_FILESYSTEM          2
#define ESP_LITTLEFS_FILESYSTEM     3

//SD READER FS type supported
#define ESP_SD_NATIVE               1
#define ESP_SDIO                    2
#define ESP_SDFAT2                  3

//SDIO Mode
#define SD_ONE_BIT_MODE             1
#define SD_FOUR_BIT_MODE            0

//SD state
#define ESP_SDCARD_IDLE             0
#define ESP_SDCARD_NOT_PRESENT      1
#define ESP_SDCARD_BUSY             2

//Notifications
#define ESP_NO_NOTIFICATION         0
#define ESP_PUSHOVER_NOTIFICATION   1
#define ESP_EMAIL_NOTIFICATION      2
#define ESP_LINE_NOTIFICATION       3
#define ESP_TELEGRAM_NOTIFICATION   4
#define ESP_IFTTT_NOTIFICATION      5

//SENSOR
#define NO_SENSOR_DEVICE   0
#define DHT11_DEVICE    1
#define DHT22_DEVICE    2
#define ANALOG_DEVICE   3
#define BMP280_DEVICE   4
#define BME280_DEVICE   5

#define USE_CELSIUS     1
#define USE_FAHRENHEIT  2

//Camera
#define CAMERA_MODEL_CUSTOM           0
#define CAMERA_MODEL_ESP_EYE          1
#define CAMERA_MODEL_M5STACK_PSRAM    2
#define CAMERA_MODEL_M5STACK_V2_PSRAM 3
#define CAMERA_MODEL_M5STACK_WIDE     4
#define CAMERA_MODEL_AI_THINKER       7
#define CAMERA_MODEL_WROVER_KIT       8
#define CAMERA_MODEL_ESP32_CAM_BOARD 10
#define CAMERA_MODEL_ESP32S2_CAM_BOARD 11
#define CAMERA_MODEL_ESP32S3_CAM_LCD   12
#define CAMERA_MODEL_ESP32S3_EYE       13

//Errors code
#define ESP_ERROR_AUTHENTICATION    1
#define ESP_ERROR_FILE_CREATION     2
#define ESP_ERROR_FILE_WRITE        3
#define ESP_ERROR_UPLOAD            4
#define ESP_ERROR_NOT_ENOUGH_SPACE  5
#define ESP_ERROR_UPLOAD_CANCELLED  6
#define ESP_ERROR_FILE_CLOSE        7
#define ESP_ERROR_NO_SD             8
#define ESP_ERROR_MOUNT_SD          9
#define ESP_ERROR_RESET_NUMBERING   10
#define ESP_ERROR_BUFFER_OVERFLOW   11
#define ESP_ERROR_START_UPLOAD      12
#define ESP_ERROR_SIZE              13
#define ESP_ERROR_UPDATE            14

//File system
#define ESP_FILE_READ       0
#define ESP_FILE_WRITE      1
#define ESP_FILE_APPEND     2

#define ESP_SEEK_SET  0
#define ESP_SEEK_CUR  1
#define ESP_SEEK_END  2

#define FS_ROOT        0
#define FS_FLASH       1
#define FS_SD          2
#define FS_USBDISK     3
#define FS_UNKNOWN     254
#define MAX_FS 3

//Host path
#define ESP3D_HOST_PATH "/"

#endif //_DEFINES_ESP3D_H
