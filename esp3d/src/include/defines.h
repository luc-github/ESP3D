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
#define ESP_SDFAT                   3
#define ESP_SDFAT2                  4

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
#define CAMERA_MODEL_M5STACK_WIDE     3
#define CAMERA_MODEL_AI_THINKER       4
#define CAMERA_MODEL_WROVER_KIT       5

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

#endif //_DEFINES_ESP3D_H
