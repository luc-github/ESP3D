/*
  esp3d_defines.h - ESP3D defines file

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

// Settings
#define SETTINGS_IN_EEPROM 1
#define SETTINGS_IN_PREFERENCES 2

// Supported FW /////////////////////////////////////////////////////////////
#define UNKNOWN_FW 0
#define GRBL 10
#define MARLIN 20
#define MARLIN_EMBEDDED 30
#define SMOOTHIEWARE 40
#define REPETIER 50
#define REPRAP 70
#define GRBLHAL 80
#define HP_GL 90

typedef uint ESP3DSettingIndex;

// position in EEPROM / preferences will use `P_` + <position> to make a string
// : P_0 for 0
#define ESP_RADIO_MODE 0                 // 1 byte = flag
#define ESP_STA_SSID 1                   // 33 bytes 32+1 = string  ; warning does not support multibyte char like chinese
#define ESP_STA_PASSWORD 34              // 65 bytes 64 +1 = string ;warning does not support multibyte char like chinese
#define ESP_STA_IP_MODE 99               // 1 byte = flag
#define ESP_STA_IP_VALUE 100             // 4  bytes xxx.xxx.xxx.xxx
#define ESP_STA_MASK_VALUE 104           // 4  bytes xxx.xxx.xxx.xxx
#define ESP_STA_GATEWAY_VALUE 108        // 4  bytes xxx.xxx.xxx.xxx
#define ESP_BAUD_RATE 112                // 4  bytes = int
#define ESP_NOTIFICATION_TYPE 116        // 1 byte = flag
#define ESP_CALIBRATION 117              // 1 byte = flag
#define ESP_AP_CHANNEL 118               // 1 byte = flag
#define ESP_BUZZER 119                   // 1 byte = flag
#define ESP_INTERNET_TIME 120            // 1  byte = flag
#define ESP_HTTP_PORT 121                // 4  bytes = int
#define ESP_TELNET_PORT 125              // 4  bytes = int
// #define FREE 129                      // 1  bytes = flag
#define ESP_HOSTNAME 130                 // 33 bytes 32+1 = string  ; warning does not support multibyte char like chinese
#define ESP_SENSOR_INTERVAL 164          // 4  bytes = int
#define ESP_SETTINGS_VERSION 168         // 8  bytes = 7+1 = string ESP3D + 2 digits
#define ESP_ADMIN_PWD 176                // 21  bytes 20+1 = string  ; warning does not support multibyte char like chinese
#define ESP_USER_PWD 197                 // 21  bytes 20+1 = string  ; warning does not support multibyte char like chinese
#define ESP_AP_SSID 218                  // 33 bytes 32+1 = string  ; warning does not support multibyte char like chinese
#define ESP_AP_PASSWORD 251              // 65 bytes 64 +1 = string ;warning does not support multibyte char like chinese
#define ESP_AP_IP_VALUE 316              // 4  bytes xxx.xxx.xxx.xxx
#define ESP_BOOT_DELAY 320               // 4  bytes = int
#define ESP_WEBSOCKET_PORT 324           // 4  bytes= int
#define ESP_HTTP_ON 328                  // 1 byte = flag
#define ESP_TELNET_ON 329                // 1 byte = flag
#define ESP_WEBSOCKET_ON 330             // 1 byte = flag
#define ESP_SD_SPEED_DIV 331             // 1 byte = flag
#define ESP_NOTIFICATION_TOKEN1 332      // 251 bytes 250+1 = string  ; warning does not support multibyte char like chinese
#define ESP_NOTIFICATION_TOKEN2 583      // 64 bytes 63+1 = string  ; warning does not support multibyte char like chinese
#define ESP_SENSOR_TYPE 647              // 1  bytes = flag
#define ESP_TARGET_FW 648                // 1  bytes = flag
#define ESP_FREE 649                     // 1  bytes = flag
// #define FREE 650                      // 1  bytes = flag
#define ESP_TIME_SERVER1 651             // 129 bytes 128+1 = string  ; warning does not support multibyte char like chinese
#define ESP_TIME_SERVER2 780             // 129 bytes 128+1 = string  ; warning does not support multibyte char like chinese
#define ESP_TIME_SERVER3 909             // 129 bytes 128+1 = string  ; warning does not support multibyte char like chinese
// #define FREE 1038                     // 1  bytes = flag
#define ESP_SD_MOUNT 1039                // 1  bytes = flag
#define ESP_SESSION_TIMEOUT 1040         // 1  bytes = flag
// #define FREE 1041                     // 1  bytes = flag
#define ESP_SD_CHECK_UPDATE_AT_BOOT 1042 // 1  bytes = flag
#define ESP_NOTIFICATION_SETTINGS 1043   // 129 bytes 128+1 = string  ; warning does not support multibyte char like chinese
#define ESP_CALIBRATION_1 1172           // 4  bytes = int
#define ESP_CALIBRATION_2 1176           // 4  bytes = int
#define ESP_CALIBRATION_3 1180           // 4  bytes = int
#define ESP_CALIBRATION_4 1184           // 4  bytes = int
#define ESP_CALIBRATION_5 1188           // 4  bytes = int
#define ESP_SETUP 1192                   // 1 byte = flag
// #define FREE 1193                     // 1 byte = flag
// #define FREE 1194                     // 1 byte = flag
// #define FREE 1195                     // 1 byte = flag
#define ESP_FTP_CTRL_PORT 1196           // 4  bytes = int
#define ESP_FTP_DATA_ACTIVE_PORT 1200    // 4  bytes = int
#define ESP_FTP_DATA_PASSIVE_PORT 1204   // 4  bytes = int
#define ESP_FTP_ON 1208                  // 1 byte = flag
#define ESP_AUTO_NOTIFICATION 1209       // 1 byte = flag
#define ESP_VERBOSE_BOOT 1210            // 1 byte = flag
#define ESP_WEBDAV_ON 1211               // 1 byte = flag
#define ESP_WEBDAV_PORT 1212             // 4  bytes= int
#define ESP_STA_DNS_VALUE 1216           // 4  bytes= int
#define ESP_SECURE_SERIAL 1220           // 1 byte = flag
#define ESP_BOOT_RADIO_STATE 1221        // 1 byte = flag
#define ESP_STA_FALLBACK_MODE 1222       // 1 byte = flag
#define ESP_SERIAL_BRIDGE_ON 1223        // 1 byte = flag
// #define FREE 1224                     // 1 byte = flag
#define ESP_SERIAL_BRIDGE_BAUD 1225      // 4  bytes= int
#define ESP_TIME_ZONE 1229               // 7 bytes 6+1 = string

// Hidden password
#define HIDDEN_PASSWORD "********"

// Debug
#define LOG_OUTPUT_SERIAL0 1
#define LOG_OUTPUT_SERIAL1 2
#define LOG_OUTPUT_SERIAL2 3
#define LOG_OUTPUT_TELNET 4
#define LOG_OUTPUT_WEBSOCKET 5

#define LOG_LEVEL_NONE 0
#define LOG_LEVEL_ERROR 1
#define LOG_LEVEL_DEBUG 2
#define LOG_LEVEL_VERBOSE 3

// Serial
#define USE_SERIAL_0 1
#define USE_SERIAL_1 2
#define USE_SERIAL_2 3

// Serial service ID
#define MAIN_SERIAL 1
#define BRIDGE_SERIAL 2

// Communication protocols
#define RAW_SERIAL 0
#define MKS_SERIAL 1
#define SOCKET_SERIAL 2

// Display
#define OLED_I2C_SSD1306_128X64 1
#define OLED_I2C_SSDSH1106_132X64 2
#define TFT_SPI_ILI9341_320X240 3
#define TFT_SPI_ILI9488_480X320 4
#define TFT_SPI_ST7789_240X240 5
#define TFT_SPI_ST7789_135X240 6

// UI type for display
#define UI_TYPE_BASIC 1
#define UI_TYPE_ADVANCED 2
#define UI_COLORED 1
#define UI_MONOCHROME 2

// SD connection
#define ESP_NO_SD 0
#define ESP_DIRECT_SD 1
#define ESP_SHARED_SD 2

// SD Device type
#define ESP_NORMAL_SDCARD 0
#define ESP_FYSETC_WIFI_PRO_SDCARD 1

// Upload type
#define ESP_UPLOAD_DIRECT_SD 1
#define ESP_UPLOAD_SHARED_SD 2
#define ESP_UPLOAD_SERIAL_SD 3
#define ESP_UPLOAD_FAST_SERIAL_SD 4
#define ESP_UPLOAD_FAST_SERIAL_USB 5
#define ESP_UPLOAD_DIRECT_USB 6

// IP mode
#define DHCP_MODE 1
#define STATIC_IP_MODE 0

// Network Mode
#define ESP_NO_NETWORK 0
#define ESP_WIFI_STA 1
#define ESP_WIFI_AP 2
#define ESP_BT 3
#define ESP_ETH_STA 4
#define ESP_AP_SETUP 5
// #define ESP_ETH_SRV  5

// SD mount point
#define ESP_SD_ROOT 1
#define ESP_SD_SUB_SD 2
#define ESP_SD_SUB_EXT 3

// Touch
#define XPT2046_SPI 1

// Input
#define ROTARY_ENCODER 1

// File systems
#define ESP_SPIFFS_FILESYSTEM 1
#define ESP_FAT_FILESYSTEM 2
#define ESP_LITTLEFS_FILESYSTEM 3

// SD READER FS type supported
#define ESP_SD_NATIVE 1
#define ESP_SDIO 2
#define ESP_SDFAT2 3

// SDIO Mode
#define SD_ONE_BIT_MODE 1
#define SD_FOUR_BIT_MODE 0

// SD state
#define ESP_SDCARD_IDLE 0
#define ESP_SDCARD_NOT_PRESENT 1
#define ESP_SDCARD_BUSY 2

// Notifications
#define ESP_NO_NOTIFICATION 0
#define ESP_PUSHOVER_NOTIFICATION 1
#define ESP_EMAIL_NOTIFICATION 2
#define ESP_LINE_NOTIFICATION 3
#define ESP_TELEGRAM_NOTIFICATION 4
#define ESP_IFTTT_NOTIFICATION 5
#define ESP_HOMEASSISTANT_NOTIFICATION 6

// SENSOR
#define NO_SENSOR_DEVICE 0
#define DHT11_DEVICE 1
#define DHT22_DEVICE 2
#define ANALOG_DEVICE 3
#define BMP280_DEVICE 4
#define BME280_DEVICE 5

#define USE_CELSIUS 1
#define USE_FAHRENHEIT 2

// Camera
#define CAMERA_MODEL_CUSTOM 0
#define CAMERA_MODEL_ESP_EYE 1
#define CAMERA_MODEL_M5STACK_PSRAM 2
#define CAMERA_MODEL_M5STACK_V2_PSRAM 3
#define CAMERA_MODEL_M5STACK_WIDE 4
#define CAMERA_MODEL_AI_THINKER 7
#define CAMERA_MODEL_WROVER_KIT 8
#define CAMERA_MODEL_ESP32_CAM_BOARD 10
#define CAMERA_MODEL_ESP32S2_CAM_BOARD 11
#define CAMERA_MODEL_ESP32S3_CAM_LCD 12
#define CAMERA_MODEL_ESP32S3_EYE 13

// Errors code
#define ESP_ERROR_AUTHENTICATION 1
#define ESP_ERROR_FILE_CREATION 2
#define ESP_ERROR_FILE_WRITE 3
#define ESP_ERROR_UPLOAD 4
#define ESP_ERROR_NOT_ENOUGH_SPACE 5
#define ESP_ERROR_UPLOAD_CANCELLED 6
#define ESP_ERROR_FILE_CLOSE 7
#define ESP_ERROR_NO_SD 8
#define ESP_ERROR_MOUNT_SD 9
#define ESP_ERROR_RESET_NUMBERING 10
#define ESP_ERROR_BUFFER_OVERFLOW 11
#define ESP_ERROR_START_UPLOAD 12
#define ESP_ERROR_SIZE 13
#define ESP_ERROR_UPDATE 14

// File system
#define ESP_FILE_READ 0
#define ESP_FILE_WRITE 1
#define ESP_FILE_APPEND 2

#define ESP_SEEK_SET 0
#define ESP_SEEK_CUR 1
#define ESP_SEEK_END 2

#define FS_ROOT 0
#define FS_FLASH 1
#define FS_SD 2
#define FS_USBDISK 3
#define FS_UNKNOWN 254
#define MAX_FS 3

// ethernet clock modes (check ETH.h for eth_clock_mode_t)
#define MODE_ETH_CLOCK_GPIO0_IN 0
#define MODE_ETH_CLOCK_GPIO0_OUT 1
#define MODE_ETH_CLOCK_GPIO16_OUT 2
#define MODE_ETH_CLOCK_GPIO17_OUT 3

// Ethernet type (Check ETH.h eth_phy_type_t)
#define TYPE_ETH_PHY_LAN8720 0
#define TYPE_ETH_PHY_TLK110 1
#define TYPE_ETH_PHY_RTL8201 2
#define TYPE_ETH_PHY_DP83848 3
#define TYPE_ETH_PHY_DM9051 4
#define TYPE_ETH_PHY_KSZ8041 5
#define TYPE_ETH_PHY_KSZ8081 6

// Host path
#define ESP3D_HOST_PATH "/"

#endif  //_DEFINES_ESP3D_H
