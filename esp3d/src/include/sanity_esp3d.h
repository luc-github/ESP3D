/*
  sanity_esp3d.h - esp3d sanity check functions

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

#ifndef _SANITY_ESP3D_H
#define _SANITY_ESP3D_H

#include "esp3d_config.h"

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

#if !defined(ESP_SERIAL_OUTPUT)
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
#if defined(FILESYSTEM_TIMESTAMP_FEATURE) && defined( ARDUINO_ARCH_ESP8266)
#error SPIFFS time is not available in ESP8266
#endif

/**************************
 * Filesystem
 * ***********************/
#if FILESYSTEM_FEATURE == ESP_FAT_FILESYSTEM && defined( ARDUINO_ARCH_ESP8266)
#error Fat FS is not available in ESP8266
#endif

#endif //SANITY_ESP3D_H 
