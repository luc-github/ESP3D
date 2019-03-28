/*
  config.h - ESP3D configuration file

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
#ifndef _CONFIGURATION_H
#define _CONFIGURATION_H
//FEATURES - comment to disable //////////////////////////////////////////////////////////

//SERIAL_COMMAND_FEATURE: allow to send command by serial
#define SERIAL_COMMAND_FEATURE

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
#define WS_DATA_FEATURE

//DISPLAY_DEVICE: allow screen output
//OLED_I2C_SSD1306    		1
//OLED_I2C_SSDSH1106  		2
//TFT_SPI_ILI9341_320X240 	3
//#define DISPLAY_DEVICE OLED_I2C_SSD1306

#if defined (DISPLAY_DEVICE)
//for ILI9143 edit User_Setup.h of TFT_eSPI library
#define DISPLAY_I2C_PIN_SDA         4
#define DISPLAY_I2C_PIN_SCL         15
#define DISPLAY_I2C_PIN_RST         16 //comment if not applicable
#define DISPLAY_I2C_ADDR	        0x3c
#define DISPLAY_FLIP_VERTICALY      1 //comment to disable
#endif //DISPLAY_DEVICE
//DHT_DEVICE: send update of temperature / humidity based on DHT 11/22
//#define DHT_DEVICE
#ifdef DHT_DEVICE
#define ESP3D_DHT_PIN 22
//USE_CELSIUS
//USE_FAHRENHEIT
#define DHT_UNIT USE_CELSIUS
#endif //DHT_DEVICE

//PIN_RESET_FEATURE : allow to reset settings by setting low a pin
//#define PIN_RESET_FEATURE
#if defined (PIN_RESET_FEATURE)
#define ESP3D_RESET_PIN 2
#endif //PIN_RESET_FEATURE
//SDCARD_FEATURE: to access SD Card files directly instead of access by serial using printer Board FW
//#define SDCARD_FEATURE

//FILESYSTEM_FEATURE: to host some files on flash
//ESP_SPIFFS_FILESYSTEM       0
//ESP_FAT_FILESYSTEM          1
//ESP_LITTLEFS_FILESYSTEM     2 //Not Yet implemented
#define FILESYSTEM_FEATURE ESP_SPIFFS_FILESYSTEM

//DIRECT_PIN_FEATURE: allow to access pin using ESP201 command
#define DIRECT_PIN_FEATURE

//TIMESTAMP_FEATURE: set time system
#define TIMESTAMP_FEATURE

//FILESYSTEM_TIMESTAMP_FEATURE: allow to get last write time from FILESYSTEM files
//#define FILESYSTEM_TIMESTAMP_FEATURE

//MDNS_FEATURE: this feature allow  type the name defined
//in web browser by default: http:\\esp8266.local and connect
//need `bonjour` protocol on windows
#define MDNS_FEATURE

//SSDP_FEATURE: this feature is a discovery protocol, supported on Windows out of the box
#define SSDP_FEATURE

//CAPTIVE_PORTAL_FEATURE: In SoftAP redirect all unknow call to main page
#define CAPTIVE_PORTAL_FEATURE

//OTA_FEATURE: this feature is arduino update over the air
#define OTA_FEATURE

//WEB_UPDATE_FEATURE: allow to flash fw using web UI
#define WEB_UPDATE_FEATURE

//NOTIFICATION_FEATURE : allow to push notifications
#define NOTIFICATION_FEATURE

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
#define DEBUG_ESP3D_OUTPUT_PORT  8000
#endif //ESP_DEBUG_FEATURE

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

//Serial Parameters
#define ESP_SERIAL_PARAM SERIAL_8N1

//Serial Pins
//-1 means use default pins of your board what ever the serial you choose
//   * UART 0 possible options are (1, 3), (2, 3) or (15, 13)
//   * UART 1 allows only TX on 2 if UART 0 is not (2, 3)
#define ESP_RX_PIN -1
#define ESP_TX_PIN -1

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

#endif //_CONFIGURATION_H
