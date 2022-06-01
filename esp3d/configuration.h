/*
  config.h - ESP3D configuration file

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
#ifndef _CONFIGURATION_H
#define _CONFIGURATION_H

//FEATURES - comment to disable //////////////////////////////////////////////////////////

//WiFi setup station as default, use AP mode first if not done
//Note: need both defined to enable it
//#define STATION_WIFI_SSID "*********"
//#define STATION_WIFI_PASSWORD "*********"
//you can also use a different config file for SSID/password
//this file is ignored by github
#if defined __has_include
#  if __has_include ("myconfig.h")
#    include "myconfig.h"
#  endif
#endif

//SERIAL_COMMAND_FEATURE: allow to send command by serial
#define SERIAL_COMMAND_FEATURE

//COMMUNICATION_PROTOCOL: to communicate with printer or TFT
//RAW_SERIAL standard serial
//MKS_SERIAL Mks protocol
#define COMMUNICATION_PROTOCOL RAW_SERIAL

//AUTHENTICATION_FEATURE: protect pages by login password
//#define AUTHENTICATION_FEATURE

//WIFI_FEATURE : enable WIFI function
#define WIFI_FEATURE

//ETH_FEATURE : enable Ethernet function
//#define ETH_FEATURE

//BLUETOOTH_FEATURE : enable BT Serial function
//#define BLUETOOTH_FEATURE

//HTTP_FEATURE : enable HTTP function
#define HTTP_FEATURE

//TELNET_FEATURE : enable Telnet function
#define TELNET_FEATURE

//WS_DATA_FEATURE: allow to connect serial from Websocket
//#define WS_DATA_FEATURE

//DISPLAY_DEVICE: allow screen output
//OLED_I2C_SSD1306_128X64   1
//OLED_I2C_SSDSH1106_132X64 2
//TFT_SPI_ST7789_240X240    5
//TFT_SPI_ST7789_135X240    6
//#define DISPLAY_DEVICE TFT_SPI_ST7789_240X240

//#define DISPLAY_I2C_ADDR            0x3c
//#define DISPLAY_I2C_PIN_RST         16 //comment if not applicable
//#define DISPLAY_FLIP_VERTICALY      1
//#define DISPLAY_TOUCH_DRIVER        XPT2046_SPI
//#define DISPLAY_LED_PIN             27  //-1 if none

//BUZZER_DEVICE: allow to connect passive buzzer
//#define BUZZER_DEVICE

//Printer has display and can show message using `M117 <Message>`
//#define PRINTER_HAS_DISPLAY

//INPUT_DEVICE: allow input
//ROTARY_ENCODER        1
//#define INPUT_DEVICE ROTARY_ENCODER

//SENSOR_DEVICE: send info based on defined sensor
//DHT11_DEVICE    1
//DHT22_DEVICE    2
//ANALOG_DEVICE   3
//BMP280_DEVICE   4
//BME280_DEVICE   5
//#define SENSOR_DEVICE BMP280_DEVICE

#ifdef BUZZER_DEVICE
#define ESP3D_BUZZER_PIN 15
#endif //BUZZER_DEVICE

#ifdef SENSOR_DEVICE
//pin
#define ESP3D_SENSOR_PIN 22
#define SENSOR_ADDR            0x76
//Conversion coefficient
#define SENSOR_CONVERTER(v) v*0.588
//Unit to use, if not applicaple for sensor will use default one
//it is used also for the output format
//C for Celsius / F for Fahrenheit / V for volt
#define SENSOR__UNIT "C"
#endif //SENSOR_DEVICE

//PIN_RESET_FEATURE : allow to reset settings by setting low a pin
//#define PIN_RESET_FEATURE
//#define ESP3D_RESET_PIN 0


//SD_DEVICE: to access SD Card files directly instead of access by serial using printer Board FW
//ESP_SD_NATIVE               1 //esp32 / esp8266
//ESP_SDIO                    2 //esp32 only
//ESP_SDFAT                   3 //esp8266  / esp32
//ESP_SDFAT2                  4 //esp8266  / esp32
//#define SD_DEVICE    ESP_SDFAT2

#if defined(SD_DEVICE)

//SDIO mode
#define SD_ONE_BIT_MODE true

//SD Device Connection type (default is ESP_NO_SD if not defined)
//ESP_NO_SD
//ESP_DIRECT_SD
//ESP_SHARED_SD
#define SD_DEVICE_CONNECTION  ESP_SHARED_SD


#if SD_DEVICE_CONNECTION == ESP_SHARED_SD
//Pin used by multiplexer or hardware switch to select SD device
#define ESP_FLAG_SHARED_SD_PIN 0
//value to enable SD device on ESP
#define ESP_FLAG_SHARED_SD_VALUE 0
#endif //SD_DEVICE_CONNECTION  ESP_SHARED_SD

//pin if reader has insert detection feature
//let -1 or comment if none
#define ESP_SD_DETECT_PIN       4
//value expected for ESP_SD_DETECT_PIN (0 or 1)
#define ESP_SD_DETECT_VALUE     0

#define ESP_SD_CS_PIN   5
#endif //SD_DEVICE

//FILESYSTEM_FEATURE: to host some files on flash
//ESP_SPIFFS_FILESYSTEM       0
//ESP_FAT_FILESYSTEM          1
//ESP_LITTLEFS_FILESYSTEM     2
#define FILESYSTEM_FEATURE ESP_LITTLEFS_FILESYSTEM

//Allows to mount /FS and /SD under / for FTP server
//#define GLOBAL_FILESYSTEM_FEATURE

//WEBDAV_FEATURE : enable WebDav feature
//FS_ROOT        mount all FS
//FS_FLASH       mount Flash FS
//FS_SD          mount SD FS
//FS_USBDISK     mount USB disk FS

//#define WEBDAV_FEATURE  FS_FLASH

//FTP_FEATURE : enable FTP feature
//FS_ROOT        mount all FS
//FS_FLASH       mount Flash FS
//FS_SD          mount SD FS
//FS_USBDISK     mount USB disk FS
//#define FTP_FEATURE  FS_ROOT

//DIRECT_PIN_FEATURE: allow to access pin using ESP201 command
#define DIRECT_PIN_FEATURE

//TIMESTAMP_FEATURE: set time system
//#define TIMESTAMP_FEATURE

//FILESYSTEM_TIMESTAMP_FEATURE: display last write time from Flash files
//#define FILESYSTEM_TIMESTAMP_FEATURE

//FILESYSTEM_TIMESTAMP_FEATURE:display last write time from SD files
//#define SD_TIMESTAMP_FEATURE

//MDNS_FEATURE: this feature allow  type the name defined
//in web browser by default: http:\\esp8266.local and connect
//need `bonjour` protocol on windows
#define MDNS_FEATURE

//SSDP_FEATURE: this feature is a discovery protocol, supported on Windows out of the box
#define SSDP_FEATURE

//CAPTIVE_PORTAL_FEATURE: In SoftAP redirect all unknow call to main page
#define CAPTIVE_PORTAL_FEATURE

//OTA_FEATURE: this feature is arduino update over the air
//#define OTA_FEATURE

//WEB_UPDATE_FEATURE: allow to flash fw using web UI
#define WEB_UPDATE_FEATURE

//SD_UPDATE_FEATURE: allow to flash/configure fw using SD
//#define SD_UPDATE_FEATURE

//NOTIFICATION_FEATURE : allow to push notifications
#define NOTIFICATION_FEATURE

//CAMERA_DEVICE: Enable the support of connected camera
//CAMERA_MODEL_CUSTOM           0 //Edit the pins in include/pins.h
//CAMERA_MODEL_ESP_EYE          1
//CAMERA_MODEL_M5STACK_PSRAM    2
//CAMERA_MODEL_M5STACK_WIDE     3
//CAMERA_MODEL_AI_THINKER       4 e.g. used by ESP32-CAM
//CAMERA_MODEL_WROVER_KIT       5
//#define CAMERA_DEVICE CAMERA_MODEL_AI_THINKER
//#define CAMERA_DEVICE_FLIP_VERTICALY  //comment to disable
//#define CAMERA_DEVICE_FLIP_HORIZONTALY//comment to disable
#define CUSTOM_CAMERA_NAME "ESP32-CAM"


//Allow remote access by enabling cross origin access
//check https://developer.mozilla.org/en-US/docs/Web/HTTP/CORS
//this should be enabled only in specific cases
//like show the camera in web page different than device web server
//if you do not know what is that then better left it commented
//#define ESP_ACCESS_CONTROL_ALLOW_ORIGIN

//GCODE_HOST_FEATURE : allow to send GCODE with ack
#define GCODE_HOST_FEATURE

//ESP_AUTOSTART_SCRIPT : to do some actions / send GCODE at start, need ESP_GCODE_HOST_FEATURE enabled
//can be  a line od several GCODES separated by `\n` e.g. "M21\nM117 SD mounted\n"
//can be  a file name, if exists, commands inside will be processed, e.g "/FS:/autostart.esp"
//#define ESP_AUTOSTART_SCRIPT "M117 Mounting SD;M21"
//#define ESP_AUTOSTART_SCRIPT_FILE "autoscript.gco"

//ESP_LUA_INTERPRETER_FEATURE : add lua scripting feature
//#define ESP_LUA_INTERPRETER_FEATURE

//Extra features /////////////////////////////////////////////////////////////////////////
/************************************
 *
 * DEBUG
 *
 * **********************************/
//Do not do this when connected to printer !!!
//be noted all upload may failed if enabled
//DEBUG_OUTPUT_SERIAL0 1
//DEBUG_OUTPUT_SERIAL1 2
//DEBUG_OUTPUT_SERIAL2 3
//DEBUG_OUTPUT_TELNET  4
//DEBUG_OUTPUT_WEBSOCKET  5
//#define ESP_DEBUG_FEATURE DEBUG_OUTPUT_SERIAL0

#ifdef ESP_DEBUG_FEATURE
#define DEBUG_BAUDRATE 115200
#define DEBUG_ESP3D_OUTPUT_PORT  8000
#endif //ESP_DEBUG_FEATURE

#if defined (DISPLAY_DEVICE) && (DISPLAY_UI_TYPE == UI_TYPE_ADVANCED)
//allows to use [ESP216]SNAP to do screen capture
#define DISPLAY_SNAPSHOT_FEATURE
#define AUTO_SNAPSHOT_FEATURE
#endif //DISPLAY_DEVICE

/************************************
 *
 * Serial Communications
 *
 * **********************************/
//which serial ESP use to communicate to printer (ESP32 has 3 serials available, ESP8266 only 2)
//USE_SERIAL_0 for ESP8266/32
//USE_SERIAL_1 for ESP8266/32
//USE_SERIAL_2 for ESP32 Only
#define ESP_SERIAL_OUTPUT USE_SERIAL_0

//Serial rx buffer size is 256 but can be extended
#define SERIAL_RX_BUFFER_SIZE 512

/************************************
 *
 * Benchmark report
 *
 * **********************************/
//#define ESP_BENCHMARK_FEATURE

//Serial need speed up on esp32
#define SERIAL_INDEPENDANT_TASK

/************************************
 *
 * Settings
 *
 * **********************************/
//SETTINGS_IN_EEPROM 0
//SETTINGS_IN_PREFERENCES 1
#define ESP_SAVE_SETTINGS SETTINGS_IN_EEPROM

/************************************
 *
 * SSL Client
 *
 * **********************************/
//Using BearSSL need to decrease size of packet to not be OOM on ESP8266
#define BEARSSL_MFLN_SIZE   512
#define BEARSSL_MFLN_SIZE_FALLBACK  4096

/************************************
 *
 * Disable sanity check
 *
 * **********************************/
//#define ESP_NO_SANITY_CHECK

/************************************
 *
 * Customize ESP3D
 *
 * **********************************/
#if defined( ARDUINO_ARCH_ESP8266)
#define ESP_MODEL_NAME "ESP8266"
#define ESP_MODEL_URL "http://espressif.com/en/products/esp8266/"
#endif //ARDUINO_ARCH_ESP8266
#if defined( ARDUINO_ARCH_ESP32)
#define ESP_MODEL_NAME "ESP32"
#define ESP_MODEL_URL "https://www.espressif.com/en/products/hardware/esp-wroom-32/overview"
#endif //ARDUINO_ARCH_ESP32
#define ESP_MODEL_NUMBER "ESP3D 3.0"
#define ESP_MANUFACTURER_NAME "Espressif Systems"
#define ESP_MANUFACTURER_URL "http://espressif.com"

#define NOTIFICATION_ESP_ONLINE "Hi, %ESP_NAME% is now online at %ESP_IP%"
#define ESP_NOTIFICATION_TITLE "ESP3D Notification"

#if !defined(WIFI_FEATURE) && !defined(ETH_FEATURE)
#undef HTTP_FEATURE
#undef TELNET_FEATURE
#undef WEBDAV_FEATURE
#undef FTP_FEATURE
#undef WEB_UPDATE_FEATURE
#undef CAPTIVE_PORTAL_FEATURE
#undef SSDP_FEATURE
#undef MDNS_FEATURE
#undef NOTIFICATION_FEATURE
#endif

/************************************
 *
 * Printer display (M117 support)
 *
 * **********************************/
#if defined(PRINTER_HAS_DISPLAY)
#define HAS_SERIAL_DISPLAY ""
#endif //PRINTER_HAS_DISPLAY

#endif //_CONFIGURATION_H
