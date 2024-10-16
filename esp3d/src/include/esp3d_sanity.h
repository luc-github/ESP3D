/*
  esp3d_sanity.h - esp3d sanity check functions

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

#pragma once

/**************************
 * Arduino core version
 * ***********************/
#if ESP_ARDUINO_VERSION_MAJOR == 1
#error "ESP3D does not support Arduino Core 1.x.x"
#endif

/**************************
 * Settings
 * ***********************/
#if (ESP_SAVE_SETTINGS == SETTINGS_IN_PREFERENCES) && \
    defined(ARDUINO_ARCH_ESP8266)
#error Preferences library is not available for ESP8266
#endif

#if !defined(ESP_SAVE_SETTINGS)
#error Choose Preferences or EEPROM for settings
#endif

/**************************
 * Debug
 * ***********************/

#if defined(ESP_LOG_FEATURE)
#if ESP_LOG_FEATURE == ESP_SERIAL_OUTPUT
#if not defined(ESP_NO_SANITY_CHECK)
#warning You use same serial for output and log
#endif  // SANITY_ESP3D_H
#endif  // ESP_LOG_FEATURE == ESP_SERIAL_OUTPUT
#if (ESP_LOG_FEATURE == LOG_OUTPUT_SERIAL2) && defined(ARDUINO_ARCH_ESP8266)
#error Serial 2 is not available in ESP8266 for log
#endif  // ESP_LOG_FEATURE == LOG_OUTPUT_SERIAL2 ) && ARDUINO_ARCH_ESP8266
#endif  // ESP_LOG_FEATURE

/**************************
 * Serial
 * ***********************/

#if !defined(ESP_SERIAL_OUTPUT) && COMMUNICATION_PROTOCOL != SOCKET_SERIAL
#error ESP_SERIAL_OUTPUT must be defined
#endif  //! defined(ESP_SERIAL_OUTPUT) && COMMUNICATION_PROTOCOL!=SOCKET_SERIAL

#if COMMUNICATION_PROTOCOL != SOCKET_SERIAL && \
    defined(ESP_SERIAL_BRIDGE_OUTPUT) &&       \
    ESP_SERIAL_OUTPUT == ESP_SERIAL_BRIDGE_OUTPUT
#error ESP_SERIAL_OUTPUT cannot be same as ESP_SERIAL_BRIDGE_OUTPUT
#endif  //! defined(ESP_SERIAL_OUTPUT) && COMMUNICATION_PROTOCOL!=SOCKET_SERIAL

#if (ESP_SERIAL_OUTPUT == USE_SERIAL2) && defined(ARDUINO_ARCH_ESP8266)
#error Serial 2 is not available in ESP8266
#endif  // ESP_SERIAL_OUTPUT == USE_SERIAL_2 ) && ARDUINO_ARCH_ESP8266

#if COMMUNICATION_PROTOCOL == MKS_SERIAL
#if defined(PRINTER_HAS_DISPLAY)
#error MKS serial protocol is not compatible with `PRINTER_HAS_DISPLAY`, comment `PRINTER_HAS_DISPLAY` in configuration.h
#endif  // defined(PRINTER_HAS_DISPLAY)
#if defined(ESP_SERIAL_BRIDGE_OUTPUT)
#error MKS serial protocol is not compatible with serial bridge output
#endif  // defined(ESP_SERIAL_BRIDGE_OUTPUT)
#endif  // COMMUNICATION_PROTOCOL == MKS_SERIAL

/**************************
 * USB-Serial
 * ***********************/
#if defined(USB_SERIAL_FEATURE) && (!defined(ARDUINO_ARCH_ESP32) || (!defined(CONFIG_IDF_TARGET_ESP32S2) && !defined(CONFIG_IDF_TARGET_ESP32S3)))
#error USB-Serial is only available for ESP32 S2 and S3
#endif



#if  ESP_SERIAL_OUTPUT == USE_USB_SERIAL && !defined(USB_SERIAL_FEATURE)
#error USB_SERIAL_FEATURE is necessary for ESP_SERIAL_OUTPUT == USE_USB_SERIAL
#endif  


/**************************
 * Bluetooth
 * ***********************/
#if (defined(BLUETOOTH_FEATURE) && !defined(ARDUINO_ARCH_ESP32)) || (defined(BLUETOOTH_FEATURE) && (defined(CONFIG_IDF_TARGET_ESP32S2) || defined(CONFIG_IDF_TARGET_ESP32S3) || defined(CONFIG_IDF_TARGET_ESP32C3)))  
#error Bluetooth is only available for ESP32
#endif

/**************************
 * Ethernet
 * ***********************/
#if defined(ETH_FEATURE) && defined(ARDUINO_ARCH_ESP8266)
#error Ethernet is not available in ESP8266
#endif

/**************************
 * Hooks
 * ***********************/
#if defined(ESP_AUTOSTART_SCRIPT) || defined(ESP_AUTOSTART_SCRIPT_FILE) || \
    defined(ESP_GOT_IP_HOOK) || defined(ESP_GOT_DATE_TIME_HOOK)
#ifndef GCODE_HOST_FEATURE
#error GCODE_HOST_FEATURE is necessary for ESP_AUTOSTART_SCRIPT/ESP_AUTOSTART_SCRIPT_FILE/ESP_GOT_IP_HOOK/ESP_GOT_DATE_TIME_HOOK
#endif  // ifndef GCODE_HOST_FEATURE
#endif  // #if defined(ESP_AUTOSTART_SCRIPT) ||
        // defined(ESP_AUTOSTART_SCRIPT_FILE) || defined(ESP_GOT_IP_HOOK) ||
        // defined(ESP_GOT_DATE_TIME_HOOK)

#if defined(ESP_GOT_IP_HOOK) || defined(ESP_GOT_DATE_TIME_HOOK)
#if !defined(WIFI_FEATURE) && !defined(ETH_FEATURE)
#error Hooks need at least one network defined (WIFI_FEATURE or ETH_FEATURE)
#endif  //! defined(WIFI_FEATURE) && !defined(ETH_FEATURE)
#endif  // #if defined(ESP_GOT_IP_HOOK) || defined(ESP_GOT_DATE_TIME_HOOK)

/**************************
 * Filesystem
 * ***********************/
#if FILESYSTEM_FEATURE == ESP_FAT_FILESYSTEM && defined(ARDUINO_ARCH_ESP8266)
#error Fat FS is not available in ESP8266
#endif
#if FILESYSTEM_FEATURE == ESP_SPIFFS_FILESYSTEM && defined(ARDUINO_ARCH_ESP8266)
#error ESP_SPIFFS_FILESYSTEM is no more available in ESP8266, use ESP_LITTLEFS_FILESYSTEM instead
#endif

/**************************
 * Camera
 * ***********************/
#if defined(CAMERA_DEVICE) && defined(ARDUINO_ARCH_ESP8266)
#error Camera is not available in ESP8266
#endif

/**************************
 * SD
 * ***********************/
#if defined(SD_DEVICE) && defined(ARDUINO_ARCH_ESP8266)
#if SD_DEVICE == ESP_SDIO
#error SDIO is not available in ESP8266
#endif
#endif

#if defined(SD_DEVICE_CONNECTION) && defined(PIN_RESET_FEATURE) && \
    ESP3D_RESET_PIN != -1
#if SD_DEVICE_CONNECTION == ESP_SHARED_SD && \
    ESP_FLAG_SHARED_SD_PIN == ESP3D_RESET_PIN
#error ESP_FLAG_SHARED_SD_PIN and ESP3D_RESET_PIN are same, it is not allowed.
#endif
#endif

#if SD_CARD_TYPE == ESP_FYSETC_WIFI_PRO_SDCARD && \
    (SD_DEVICE != ESP_SD_NATIVE || SD_DEVICE_CONNECTION != ESP_SHARED_SD)
#error ESP_FYSETC_WIFI_PRO_SDCARD only works with ESP_SD_NATIVE and ESP_SHARED_SD
#define ESP_NO_SANITY_CHECK 1
#endif  // SD_CARD_TYPE == ESP_FYSETC_WIFI_PRO_SDCARD && SD_DEVICE !=
        // ESP_SD_NATIVE && SD_DEVICE_CONNECTION!=ESP_SHARED_SD

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

/**************************
 * Lua
 * ***********************/
#if defined(ESP_LUA_INTERPRETER_FEATURE) && (defined(ARDUINO_ARCH_ESP8266) ||  defined(ARDUINO_ARCH_ESP8285))
#error ESP_LUA_INTERPRETER_FEATURE is not available for  ESP8266 and ESP8285   
#endif  // SANITY_CHECK
