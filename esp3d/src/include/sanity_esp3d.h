/*
  sanity_esp3d.h - esp3d sanity check functions

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

#ifndef _SANITY_ESP3D_H
#define _SANITY_ESP3D_H

#include "esp3d_config.h"
#if not defined(ESP_NO_SANITY_CHECK)
/**************************
 * Settings
 * ***********************/
#if (ESP_SAVE_SETTINGS == SETTINGS_IN_PREFERENCES) && defined( ARDUINO_ARCH_ESP8266)
#error Preferences library is not available for ESP8266
#endif

#if !defined (ESP_SAVE_SETTINGS)
#error Choose Preferences or EEPROM for settings
#endif

/**************************
 * Debug
 * ***********************/

#if defined(ESP_DEBUG_FEATURE)
#if ESP_DEBUG_FEATURE == ESP_SERIAL_OUTPUT
#warning You use same serial for output and debug
#endif //ESP_DEBUG_FEATURE == ESP_SERIAL_OUTPUT
#if (ESP_DEBUG_FEATURE == DEBUG_OUTPUT_SERIAL2) && defined( ARDUINO_ARCH_ESP8266)
#error Serial 2 is not available in ESP8266 for debug
#endif //ESP_DEBUG_FEATURE == DEBUG_OUTPUT_SERIAL2 ) && ARDUINO_ARCH_ESP8266
#endif //ESP_DEBUG_FEATURE

/**************************
 * Serial
 * ***********************/

#if !defined(ESP_SERIAL_OUTPUT) && COMMUNICATION_PROTOCOL!=SOCKET_SERIAL
#error ESP_SERIAL_OUTPUT must be defined
#endif

#if (ESP_SERIAL_OUTPUT == USE_SERIAL2) && defined( ARDUINO_ARCH_ESP8266)
#error Serial 2 is not available in ESP8266
#endif //ESP_SERIAL_OUTPUT == USE_SERIAL_2 ) && ARDUINO_ARCH_ESP8266


/**************************
 * Bluetooth
 * ***********************/
#if defined (BLUETOOTH_FEATURE) && defined( ARDUINO_ARCH_ESP8266)
#error Bluetooth is not available in ESP8266
#endif


/**************************
 * Ethernet
 * ***********************/
#if defined (ETH_FEATURE) && defined( ARDUINO_ARCH_ESP8266)
#error Ethernet is not available in ESP8266
#endif

/**************************
 * Time
 * ***********************/


/**************************
 * Filesystem
 * ***********************/
#if FILESYSTEM_FEATURE == ESP_FAT_FILESYSTEM && defined( ARDUINO_ARCH_ESP8266)
#error Fat FS is not available in ESP8266
#endif
#if FILESYSTEM_FEATURE == ESP_SPIFFS_FILESYSTEM && defined( ARDUINO_ARCH_ESP8266)
#error ESP_SPIFFS_FILESYSTEM is no more available in ESP8266, use ESP_LITTLEFS_FILESYSTEM instead
#endif

/**************************
 * Camera
 * ***********************/
#if defined(CAMERA_DEVICE) && defined( ARDUINO_ARCH_ESP8266)
#error Camera is not available in ESP8266
#endif

/**************************
 * SD
 * ***********************/
#if defined(SD_DEVICE) && defined( ARDUINO_ARCH_ESP8266)
#if SD_DEVICE == ESP_SDIO
#error SDIO is not available in ESP8266
#endif
#endif

#if defined (SD_DEVICE_CONNECTION) && defined(PIN_RESET_FEATURE)
#if SD_DEVICE_CONNECTION == ESP_SHARED_SD && ESP_FLAG_SHARED_SD_PIN == ESP3D_RESET_PIN
#error ESP_FLAG_SHARED_SD_PIN and ESP3D_RESET_PIN are same, it is not allowed.
#endif
#endif

/**************************
 * FTP
 * ***********************/
#if defined(FTP_FEATURE) && !defined(GLOBAL_FILESYSTEM_FEATURE)
#if FTP_FEATURE == FS_ROOT
#error FTP_FEATURE == FS_ROOT is not available because GLOBAL_FILESYSTEM_FEATURE is not enabled
#endif
#endif

#if defined(FTP_FEATURE) && !defined(FILESYSTEM_FEATURE)
#if FTP_FEATURE == FS_FLASH
#error FTP_FEATURE == FS_FLASH is not available because FILESYSTEM_FEATURE is not enabled
#endif
#endif

#if defined(FTP_FEATURE) && !defined(SD_DEVICE)
#if FTP_FEATURE == FS_SD
#error FTP_FEATURE == FS_SD is not available because SD_DEVICE is not enabled
#endif
#endif

/**************************
 * WebDav
 * ***********************/
#if defined(WEBDAV_FEATURE) && !defined(GLOBAL_FILESYSTEM_FEATURE)
#if WEBDAV_FEATURE == FS_ROOT
#error WEBDAV_FEATURE == FS_ROOT is not available because GLOBAL_FILESYSTEM_FEATURE is not enabled
#endif
#endif

#if defined(WEBDAV_FEATURE) && !defined(FILESYSTEM_FEATURE)
#if WEBDAV_FEATURE == FS_FLASH
#error WEBDAV_FEATURE == FS_FLASH is not available because FILESYSTEM_FEATURE is not enabled
#endif
#endif

#if defined(WEBDAV_FEATURE) && !defined(SD_DEVICE)
#if WEBDAV_FEATURE == FS_SD
#error WEBDAV_FEATURE == FS_SD is not available because SD_DEVICE is not enabled
#endif
#endif

/**************************
 * Update
 * ***********************/
#if defined(SD_UPDATE_FEATURE) && !defined(SD_DEVICE)
#error SD_UPDATE_FEATURE is not available because SD_DEVICE is not enabled
#endif

#endif //ESP_NO_SANITY_CHECK

#endif //SANITY_ESP3D_H 
