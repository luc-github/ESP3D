/*
  configuration.h - ESP3D configuration file

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
/* Setup station as default, use AP mode first if not done
* Note: need both defined to enable it
* Uncomment and edit them to define
*/
//#define STATION_WIFI_SSID "*********"
//#define STATION_WIFI_PASSWORD "*********"

/* You can also use a different config file for SSID/password
* Just save it in same location as this configuration.h
* This file is ignored by github
*/
#if defined __has_include
#  if __has_include ("myconfig.h")
#    include "myconfig.h"
#  endif
#endif

/************************************
*
* Serial Communications
*
* Settings and protocols
*
************************************/

/* Serial Communication protocol
* RAW_SERIAL // Basic serial protocol, without data change
* MKS_SERIAL // This is a MakerBase communication protocol, used with MKS printers and TFT, it encapsulated data in a custom protocol
*/
#define COMMUNICATION_PROTOCOL RAW_SERIAL

/* Main Serial port
* which serial ESP use to communicate to printer (ESP32 has 3 serials available, ESP8266 only 2)
* USE_SERIAL_0 //for ESP8266/32, also used by bootloader output, so consider to make it quiet
* USE_SERIAL_1 //for ESP8266/32
* USE_SERIAL_2 //for ESP32 Only
*/
//Main serial port
#define ESP_SERIAL_OUTPUT USE_SERIAL_0

/* Bridge Serial port (deprecated on esp8266 as second serial is)
* which serial ESP use to bridge to another device (ESP32 has 3 serials available, ESP8266 only 2)
* USE_SERIAL_0 //for ESP8266/32, also used by bootloader output, so consider to make it quiet
* USE_SERIAL_1 //for ESP8266/32
* USE_SERIAL_2 //for ESP32 Only\
* Comment if not used
*/
//#define ESP_SERIAL_BRIDGE_OUTPUT USE_SERIAL_1

/* Serial buffer size
*  Maximum size of the serial buffer
*/
#define SERIAL_RX_BUFFER_SIZE 512

/************************************
*
* Target firmware
*
* Targeted firmware that ESP3D will communicate with
*
************************************/
/* Target firmware (default UNKNOWN_FW can be changed later in settings)
* UNKNOWN_FW
* GRBL
* MARLIN
* SMOOTHIEWARE
* REPETIER
*/
#define DEFAULT_FW UNKNOWN_FW

/************************************
*
* Radio mode of ESP3D
*
* The radio mode ESP3D communicate with the network
*
************************************/

/* Use WiFi
* Enable wifi communications
*/
#define WIFI_FEATURE

/* Use Ethernet
* Enable ethernet communications
*/
//#define ETH_FEATURE

//Ethernet type (Check ETH.h eth_phy_type_t)
//TYPE_ETH_PHY_LAN8720
//TYPE_ETH_PHY_TLK110
//TYPE_ETH_PHY_RTL8201
//TYPE_ETH_PHY_DP83848
//TYPE_ETH_PHY_DM9051
//TYPE_ETH_PHY_KSZ8041
//TYPE_ETH_PHY_KSZ8081
#define ESP3D_ETH_PHY_TYPE TYPE_ETH_PHY_LAN8720

//Ethernet board Clock mode
// MODE_ETH_CLOCK_GPIO0_IN
// MODE_ETH_CLOCK_GPIO0_OUT
// MODE_ETH_CLOCK_GPIO16_OUT
// MODE_ETH_CLOCK_GPIO17_OUT
#define ESP3D_ETH_CLK_MODE MODE_ETH_CLOCK_GPIO17_OUT

//Pins of ethernet board
#define ESP3D_ETH_PHY_POWER_PIN 12
//#define ESP3D_ETH_PHY_MDC_PIN 23
//#define ESP3D_ETH_PHY_MDIO_PIN 18

//Address of ethernet board
//#define ESP3D_ETH_PHY_ADDR 0


/* Use Bluetooth
* Enable serial bluetooth communications
*/
//#define BLUETOOTH_FEATURE

/************************************
*
* Channels of ESP3D
*
* The way ESP3D communicate
*
************************************/

/* Use Web server
* Enable http server
*/
#define HTTP_FEATURE

/* Use telnet server
* Enable telnet light (raw tcp) communications
*/
#define TELNET_FEATURE

/* Use Websocket server
* Enable websocket communications
*/
//#define WS_DATA_FEATURE

// Enable notifications
// Allows to send notifications to the user
#define NOTIFICATION_FEATURE

/* Notification message when online
* The message that will be sent when the ESP is online
*/
#define NOTIFICATION_ESP_ONLINE "Hi, %ESP_NAME% is now online at %ESP_IP%"

/* Notification title message
* The title of notification
*/
#define ESP_NOTIFICATION_TITLE "ESP3D Notification"

/************************************
*
* Discovery methods of ESP3D
*
* The discovery methods of ESP3D
*
************************************/

/* Use captive portal
* Enable captive portal in AP mode
*/
#define CAPTIVE_PORTAL_FEATURE

/* Use mDNS discovery
* This method need `bonjour` protocol on windows, or `avahi` on linux
*/
#define MDNS_FEATURE

/* Use Simple Service Discovery Protocol
* It is supported on Windows out of the box
*/
#define SSDP_FEATURE

/************************************
*
* SSDP Customization settings
*
* Customize your ESP3D
*
************************************/

/* Model name
* Modele name of device
*/
#define ESP_MODEL_NAME "ESP Board"

/* Model number
* Modele number of device
*/
#define ESP_MODEL_NUMBER "ESP3D 3.0"

/* Model url
* Modele url of device
*/
#define ESP_MODEL_URL "https://www.espressif.com/en/products/devkits"

/* Manufacturer name
* Manufacturer name of device
*/
#define ESP_MANUFACTURER_NAME "Espressif Systems"

/* Manufacturer url
* Manufacturer url of device
*/
#define ESP_MANUFACTURER_URL "https://www.espressif.com"

/************************************
*
* Flash filesystem
*
* Filesystem on flash
*
************************************/

/* File system type used by ESP3D
* Type of file system used by ESP3D to store files
* ESP_SPIFFS_FILESYSTEM (Deprecated)
* ESP_FAT_FILESYSTEM (ESP32 only with large partitions)
* ESP_LITTLEFS_FILESYSTEM (Default)
*/
#define FILESYSTEM_FEATURE ESP_LITTLEFS_FILESYSTEM

/* Enable date/time on files
* Set date/time on files using SNTP or last webui connection
*/
//#define FILESYSTEM_TIMESTAMP_FEATURE

/************************************
*
* SD filesystem
*
* Filesystem on SD card
*
************************************/

/* SD card connection
* ESP_NO_SD //(default)
* ESP_DIRECT_SD //Only your ESP board is connected to SDCard
* ESP_SHARED_SD //Printer SD Card is also connected to ESP3D
* Does your system has SD card and how it is connected to your ESP3D
*/
//#define SD_DEVICE_CONNECTION ESP_DIRECT_SD

/* SD card library
* ESP_SD_NATIVE //esp32 / esp8266
* ESP_SDIO      //esp32 only
* ESP_SDFAT2    //esp8266  / esp32
*/
//#define SD_DEVICE ESP_SDFAT2

/* Sdio bit mode
* Mode used by SDIO library 1 bit / 4bits
* SD_ONE_BIT_MODE
* SD_FOUR_BIT_MODE
*/
//#define SDIO_BIT_MODE SD_ONE_BIT_MODE

/* Enable date/time on files
* Set date/time on files using SNTP or last webui connection
*/
//#define SD_TIMESTAMP_FEATURE

/************************************
*
* SD card pins
*
************************************/

/* SD card detect pin
* The pin used to detect SD card
*/
//#define ESP_SD_DETECT_PIN 4

/* SD card detect pin value
* State of SD card detect pin when card is present
*/
//#define ESP_SD_DETECT_VALUE 0

/* SD shared flag pin
* The pin used to enable SD card for ESP board
*/
//#define ESP_FLAG_SHARED_SD_PIN -1

/* SD shared flag pin value
* State of SD card shared pin for ESP board
*/
//#define ESP_FLAG_SHARED_SD_VALUE 0

/* SD card CS pin
* The pin used to select SD card in SPI mode
*/
//#define ESP_SD_CS_PIN 5

/************************************
*
* Remote access
*
* Remote filesystem access
*
************************************/

/* Enable global filesystem
* Allows to access to all filesystems from same location
*/
//#define GLOBAL_FILESYSTEM_FEATURE

/* WebDav access
* Use WebDav to access to your filesystem
* FS_ROOT        //mount all FS, need GLOBAL_FILESYSTEM_FEATURE
* FS_FLASH       //mount Flash FS
* FS_SD          mount SD FS
*/
//#define WEBDAV_FEATURE FS_ROOT

/* FTP access
* Use FTP to access to your filesystem (1 connection only)
* FS_ROOT        //mount all FS, need GLOBAL_FILESYSTEM_FEATURE
* FS_FLASH       //mount Flash FS
* FS_SD          //mount SD FS
*/
//#define FTP_FEATURE FS_ROOT

/************************************
*
* Reset ESP3D
*
* Reset ESP3D settings
*
************************************/

/* Enable pin reset feature
* Use a pin to reset ESP3D settings
*/
#define PIN_RESET_FEATURE

/* Reset pin
* The pin used to reset ESP3D setting if set to low for more than 1 second at start
*/
#define ESP3D_RESET_PIN 0

/************************************
*
* Update ESP3D
*
* Update ESP3D firmware
*
************************************/

/* Enable OTA
* Over The Air Update (OTA)
*/
//#define OTA_FEATURE

/* Enable Web Update
* Update firmware using WebUI, need 4MB of flash
*/
#define WEB_UPDATE_FEATURE

/* Enable SD card Update
* Update firmware and settings using file on SDCard
*/
//#define SD_UPDATE_FEATURE

/************************************
*
* Display settings
*
* Rendering screens
*
************************************/

/* Printer screen
*  If your printer has a display
*/
#define PRINTER_HAS_DISPLAY

/* ESP3D screen
* Screen connected to ESP board
* OLED I2C SSD1306 128X64
* OLED_I2C_SSDSH1106_132X64
* TFT_SPI_ST7789_240X240
* TFT_SPI_ST7789_135X240
*/
//#define DISPLAY_DEVICE OLED_I2C_SSD1306_128X64

/* Flip screen
* Flip/rotate screen
*/
//#define DISPLAY_FLIP_VERTICALY

/* Display i2C address
* Wire address of display
*/
//#define DISPLAY_I2C_ADDR 0x3c

/* Display reset pin
* The pin used to reset the screen (optional)
*/
//#define DISPLAY_I2C_PIN_RST 22

/* TFT led pin
* The pin used for the backlight
*/
//#define DISPLAY_LED_PIN -1

/************************************
*
* Audio settings
*
* Buzzer feature
*
************************************/

/* Enable buzzer
* Your esp board has a passive buzzer
*/
//#define BUZZER_DEVICE

/* Buzzer pin
* The pin used for the passive buzzer
*/
//#define ESP3D_BUZZER_PIN 33

/************************************
*
* Sensor settings
*
* Sensor feature
*
************************************/
/* Sensor pin
* The pin used for the sensor
*/
//#define ESP3D_SENSOR_PIN 34

/* Sensor Unit
*  Unit of the sensor result
*/
//#define SENSOR__UNIT "C"

/************************************
*
* Camera settings
*
* Connected camera
*
************************************/

/* Camera type
* CAMERA_MODEL_CUSTOM          //Edit the pins in include/pins.h
* CAMERA_MODEL_ESP_EYE
* CAMERA_MODEL_M5STACK_PSRAM
* CAMERA_MODEL_M5STACK_V2_PSRAM
* CAMERA_MODEL_M5STACK_WIDE
* CAMERA_MODEL_AI_THINKER     //ESP32-CAM
* CAMERA_MODEL_WROVER_KIT
* CAMERA_MODEL_ESP32_CAM_BOARD
* CAMERA_MODEL_ESP32S2_CAM_BOARD
* CAMERA_MODEL_ESP32S3_CAM_LCD
* CAMERA_MODEL_ESP32S3_EYE
* Camera connected to ESP board, only ones with PSRAM are supported
*/
//#define CAMERA_DEVICE CAMERA_MODEL_AI_THINKER

/* Flip vertically
* Flip camera vertically
*/
//#define CAMERA_DEVICE_FLIP_VERTICALY


/************************************
*
* Levels of security
*
* How commands are allowed to be sent to ESP3D
*
************************************/

/* Enable serial commands
* Allow commands to be sent to ESP3D via serial port
*/
#define SERIAL_COMMAND_FEATURE

/* Allow remote access by enabling cross origin access
* check https://developer.mozilla.org/en-US/docs/Web/HTTP/CORS
* this should be enabled only in specific cases
* like show the camera in web page different than device web server
* /if you do not know what is that then better left it commented
* Allow to show the camera in web page different than device web server
*/
//#define ESP_ACCESS_CONTROL_ALLOW_ORIGIN

/* Enable authentication
* Force usage of authentication for commands
*/
//#define AUTHENTICATION_FEATURE

/************************************
*
* Additional features
*
* Extra settings
*
************************************/

/* Enable direct control pin
* Controls pins using [ESP201]
*/
#define DIRECT_PIN_FEATURE

/************************************
*
* Scripting settings
*
* Scripting on ESP3D
*
************************************/
/* Enable Autostart
* Commands to run on startup
* Separate commands with ';' or use file
*/
//#define ESP_AUTOSTART_SCRIPT "M117 Mounting SD;M21"
//#define ESP_AUTOSTART_SCRIPT_FILE "autoscript.gco"

/* Enable lua interpreter
* Allow to use lua interpreter on ESP3D
*/
//#define ESP_LUA_INTERPRETER_FEATURE

/* Gcode Host Feature
* This feature allows to process Gcode files like macros.
*/
#define GCODE_HOST_FEATURE

/* Settings location
* SETTINGS_IN_EEPROM //ESP8266/ESP32
* SETTINGS_IN_PREFERENCES //ESP32 only
*  Location where ESP3D will save settings
*/
#define ESP_SAVE_SETTINGS SETTINGS_IN_EEPROM

/* Add serial task
* ESP32 need to add a task to handle serial communication
*/
#define SERIAL_INDEPENDANT_TASK



/************************************
*
* Development setting
* Do not modify them for production
************************************/

//Enable debug mode
//Do not do this when connected to printer !!!
//be noted all upload may failed if enabled
//DEBUG_OUTPUT_SERIAL0
//DEBUG_OUTPUT_SERIAL1
//DEBUG_OUTPUT_SERIAL2
//DEBUG_OUTPUT_TELNET
//DEBUG_OUTPUT_WEBSOCKET
//#define ESP_DEBUG_FEATURE DEBUG_OUTPUT_SERIAL0

#ifdef ESP_DEBUG_FEATURE
#define DEBUG_BAUDRATE 115200
#define DEBUG_ESP3D_OUTPUT_PORT  8000
#endif //ESP_DEBUG_FEATURE

//Enable benchmark report in dev console
//#define ESP_BENCHMARK_FEATURE

//Disable sanity check at compilation
//#define ESP_NO_SANITY_CHECK


/************************************
*
* Sanity checks
* Do not modify
************************************/

#if defined (SD_TIMESTAMP_FEATURE) || defined (FILESYSTEM_TIMESTAMP_FEATURE)
#define TIMESTAMP_FEATURE
#endif //SD_TIMESTAMP_FEATURE || FILESYSTEM_TIMESTAMP_FEATURE 

#if defined(PRINTER_HAS_DISPLAY)
#define HAS_SERIAL_DISPLAY ""
#endif // PRINTER_HAS_DISPLAY

#if defined(CAMERA_DEVICE)
#if CAMERA_DEVICE==CAMERA_MODEL_ESP32_CAM_BOARD || CAMERA_DEVICE==CAMERA_MODEL_ESP32S2_CAM_BOARD
#define USE_BOARD_HEARDER 1
#endif // CAMERA_DEVICE==CAMERA_MODEL_ESP32_CAM_BOARD || CAMERA_DEVICE==CAMERA_MODEL_ESP32S2_CAM_BOARD
#endif // CAMERA_DEVICE

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

#endif //_CONFIGURATION_H
