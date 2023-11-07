/*
  settings_esp3d.cpp -  settings esp3d functions class

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

#include "../include/esp3d_config.h"
#define STRINGIFY(x) #x
#define STRING(x) STRINGIFY(x)
#if defined(ESP_SAVE_SETTINGS)
#include "esp3doutput.h"
#include "settings_esp3d.h"

#if ESP_SAVE_SETTINGS == SETTINGS_IN_EEPROM
#include <EEPROM.h>
// EEPROM SIZE (Up to 4096)
#define EEPROM_SIZE 2048  // max is 2048
#endif                    // SETTINGS_IN_EEPROM

#if defined(WIFI_FEATURE) || defined(ETH_FEATURE)
#include "../modules/network/netconfig.h"
#if defined(WIFI_FEATURE)
#include "../modules/wifi/wificonfig.h"
#endif  // WIFI_FEATURE
#endif  // WIFI_FEATURE || ETH_FEATURE
#ifdef SENSOR_DEVICE
#include "../modules/sensor/sensor.h"
#endif  // SENSOR_DEVICE

#if ESP_SAVE_SETTINGS == SETTINGS_IN_PREFERENCES
#include <Preferences.h>
#define NAMESPACE "ESP3D"
#endif  // SETTINGS_IN_PREFERENCES

#if defined(TIMESTAMP_FEATURE)
#include "../modules/time/time_service.h"
#endif  // TIMESTAMP_FEATURE

#include "../modules/serial/serial_service.h"

// Current Settings Version
#define CURRENT_SETTINGS_VERSION "ESP3D04"

// boundaries
#define MAX_SENSOR_INTERVAL 60000
#define MIN_SENSOR_INTERVAL 0
#define MAX_LOCAL_PASSWORD_LENGTH 20
#define MIN_LOCAL_PASSWORD_LENGTH 1
#define MAX_VERSION_LENGTH 7  // ESP3DXX
#define MAX_BOOT_DELAY 40000
#define MIN_BOOT_DELAY 0
#define MIN_NOTIFICATION_TOKEN_LENGTH 0
#define MIN_NOTIFICATION_SETTINGS_LENGTH 0
#define MAX_NOTIFICATION_TOKEN_LENGTH 63
#define MAX_NOTIFICATION_SETTINGS_LENGTH 128
#define MAX_SERVER_ADDRESS_LENGTH 128
#define MAX_TIME_ZONE_LENGTH 6
#define MIN_SERVER_ADDRESS_LENGTH 0

// default byte values
#ifdef WIFI_FEATURE
#define DEFAULT_STA_FALLBACK_MODE STRING(ESP_AP_SETUP)
#if defined(STATION_WIFI_SSID) && defined(STATION_WIFI_PASSWORD)
#define DEFAULT_ESP_RADIO_MODE STRING(ESP_WIFI_STA)
#else
#define DEFAULT_ESP_RADIO_MODE STRING(ESP_AP_SETUP)
#endif  // STATION_WIFI_SSID && STATION_WIFI_PASSWORD
#else   // WIFI_FEATURE
#define DEFAULT_STA_FALLBACK_MODE STRING(ESP_NO_NETWORK)
#ifdef BLUETOOTH_FEATURE
#define DEFAULT_ESP_RADIO_MODE STRING(ESP_BT)
#else  // BLUETOOTH_FEATURE
#ifdef ETH_FEATURE
#define DEFAULT_ESP_RADIO_MODE STRING(ESP_ETH_STA)
#else  // BLUETOOTH_FEATURE
#define DEFAULT_ESP_RADIO_MODE STRING(ESP_NO_NETWORK)
#endif  // ETH_FEATURE
#endif  // BLUETOOTH_FEATURE
#endif  // WIFI_FEATURE

#define DEFAULT_BUZZER_STATE "1"
#define DEFAULT_INTERNET_TIME "0"
#define DEFAULT_SETUP "0"
#define DEFAULT_VERBOSE_BOOT "0"
#define DEFAULT_STA_IP_MODE "1"
#define DEFAULT_AP_CHANNEL "11"
#define DEFAULT_SSID_VISIBLE "1"
#define DEFAULT_OUTPUT_FLAG "255"
#define DEFAULT_SD_SPI_DIV "4"
#ifndef DEFAULT_FW
#define DEFAULT_FW UNKNOWN_FW
#endif  // DEFAULT_FW
#define DEFAULT_TIME_ZONE "+00:00"
#define DEFAULT_TIME_DST "0"
#define DEFAULT_SD_MOUNT "1"
#define DEFAULT_SD_CHECK_UPDATE_AT_BOOT "1"
#define DEFAULT_SENSOR_TYPE "0"
#define DEFAULT_HTTP_ON "1"
#define DEFAULT_FTP_ON "1"
#define DEFAULT_SERIAL_BRIDGE_ON "1"
#define DEFAULT_WEBDAV_ON "1"
#define DEFAULT_TELNET_ON "1"
#define DEFAULT_WEBSOCKET_ON "1"
#define DEFAULT_NOTIFICATION_TYPE "0"
#define DEFAULT_NOTIFICATION_TOKEN1 ""
#define DEFAULT_NOTIFICATION_TOKEN2 ""
#define DEFAULT_NOTIFICATION_SETTINGS ""
#define DEFAULT_AUTO_NOTIFICATION_STATE "1"
#define DEFAULT_SECURE_SERIAL "1"
#define DEFAULT_BOOT_RADIO_STATE "1"

// default int values
#define DEFAULT_ESP_INT "0"
#define DEFAULT_BAUD_RATE "115200"
#define DEFAULT_SERIAL_BRIDGE_BAUD_RATE "115200"
#define DEFAULT_HTTP_PORT "80"
#define DEFAULT_FTP_CTRL_PORT "21"
#define DEFAULT_FTP_ACTIVE_PORT "20"
#define DEFAULT_FTP_PASSIVE_PORT "55600"
#define DEFAULT_WEBSOCKET_PORT "8282"
#define DEFAULT_WEBDAV_PORT "8181"
#define DEFAULT_TELNET_PORT "23"
#define DEFAULT_SENSOR_INTERVAL "30000"
#define DEFAULT_BOOT_DELAY "10000"
#define DEFAULT_CALIBRATION_VALUE "0"
#define DEFAULT_CALIBRATION_DONE "0"
#define DEFAULT_SESSION_TIMEOUT "3"

// Default flags
#define DEFAULT_SERIAL_OUTPUT_FLAG "1"
#define DEFAULT_REMOTE_SCREEN_FLAG "1"
#define DEFAULT_WEBSOCKET_FLAG "1"
#define DEFAULT_TELNET_FLAG "1"
#define DEFAULT_BT_FLAG "1"
#define DEFAULT_SCREEN_FLAG "1"
#define DEFAULT_SERIAL_BRIDGE_FLAG "1"

#ifdef WIFI_FEATURE
// default string values
#define DEFAULT_AP_SSID "ESP3D"
#define DEFAULT_AP_PASSWORD "12345678"
#if defined(STATION_WIFI_SSID) && defined(STATION_WIFI_PASSWORD)
#define DEFAULT_STA_SSID STATION_WIFI_SSID
#define DEFAULT_STA_PASSWORD STATION_WIFI_PASSWORD
#else
#define DEFAULT_STA_SSID "NETWORK_SSID"
#define DEFAULT_STA_PASSWORD "12345678"
#endif  // STATION_WIFI_SSID && STATION_WIFI_PASSWORD
#endif  // WIFI_FEATURE
#if defined(BLUETOOTH_FEATURE) || defined(WIFI_FEATURE) || defined(ETH_FEATURE)
#define DEFAULT_HOSTNAME "esp3d"
#endif  // BLUETOOTH_FEATURE ||WIFI_FEATURE || ETH_FEATURE

#define DEFAULT_ADMIN_PWD "admin"
#define DEFAULT_USER_PWD "user"

#define DEFAULT_TIME_SERVER1 "time.windows.com"
#define DEFAULT_TIME_SERVER2 "time.google.com"
#define DEFAULT_TIME_SERVER3 "0.pool.ntp.org"

#define DEFAULT_SETTINGS_VERSION "ESP3D"

// default IP values
#define DEFAULT_STA_IP_VALUE "192.168.0.254"
#define DEFAULT_STA_GATEWAY_VALUE DEFAULT_STA_IP_VALUE
#define DEFAULT_STA_MASK_VALUE "255.255.255.0"
#define DEFAULT_STA_DNS_VALUE DEFAULT_STA_IP_VALUE

#define DEFAULT_AP_IP_VALUE "192.1680.0.1"
#define DEFAULT_AP_GATEWAY_VALUE DEFAULT_AP_IP_VALUE
#define DEFAULT_AP_MASK_VALUE "255.255.255.0"
#define DEFAULT_AP_DNS_VALUE DEFAULT_AP_IP_VALUE

uint8_t Settings_ESP3D::_FirmwareTarget = 0;
bool Settings_ESP3D::_isverboseboot = false;

uint16_t ESP3DSettingsData[] = {ESP_RADIO_MODE,
                                ESP_STA_PASSWORD,
                                ESP_STA_SSID,
                                ESP_BOOT_RADIO_STATE,
                                ESP_STA_FALLBACK_MODE,
                                ESP_AP_SSID,
                                ESP_AP_PASSWORD,
                                ESP_STA_IP_VALUE,
                                ESP_STA_GATEWAY_VALUE,
                                ESP_STA_MASK_VALUE,
                                ESP_STA_DNS_VALUE,
                                ESP_AP_IP_VALUE,
                                ESP_SETTINGS_VERSION,
                                ESP_NOTIFICATION_TYPE,
                                ESP_CALIBRATION,
                                ESP_AP_CHANNEL,
                                ESP_BUZZER,
                                ESP_INTERNET_TIME,
                                ESP_SERIAL_FLAG,
                                ESP_HTTP_ON,
                                ESP_TELNET_ON,
                                ESP_WEBSOCKET_ON,
                                ESP_SD_SPEED_DIV,
                                ESP_SENSOR_TYPE,
                                ESP_TARGET_FW,
                                ESP_TIME_IS_DST,
                                ESP_REMOTE_SCREEN_FLAG,
                                ESP_SD_MOUNT,
                                ESP_SESSION_TIMEOUT,
                                ESP_WEBSOCKET_FLAG,
                                ESP_SD_CHECK_UPDATE_AT_BOOT,
                                ESP_SETUP,
                                ESP_TELNET_FLAG,
                                ESP_BT_FLAG,
                                ESP_SCREEN_FLAG,
                                ESP_FTP_ON,
                                ESP_AUTO_NOTIFICATION,
                                ESP_VERBOSE_BOOT,
                                ESP_WEBDAV_ON,
                                ESP_SECURE_SERIAL,
                                ESP_SERIAL_BRIDGE_FLAG,
                                ESP_SERIAL_BRIDGE_ON,
                                ESP_HOSTNAME,
                                ESP_ADMIN_PWD,
                                ESP_USER_PWD,
                                ESP_NOTIFICATION_TOKEN1,
                                ESP_NOTIFICATION_TOKEN2,
                                ESP_NOTIFICATION_SETTINGS,
                                ESP_TIME_SERVER1,
                                ESP_TIME_SERVER2,
                                ESP_TIME_SERVER3,
                                ESP_TIME_ZONE,
                                ESP_BAUD_RATE,
                                ESP_HTTP_PORT,
                                ESP_TELNET_PORT,
                                ESP_SENSOR_INTERVAL,
                                ESP_BOOT_DELAY,
                                ESP_WEBSOCKET_PORT,
                                ESP_CALIBRATION_1,
                                ESP_CALIBRATION_2,
                                ESP_CALIBRATION_3,
                                ESP_CALIBRATION_4,
                                ESP_CALIBRATION_5,
                                ESP_FTP_CTRL_PORT,
                                ESP_FTP_DATA_ACTIVE_PORT,
                                ESP_FTP_DATA_PASSIVE_PORT,
                                ESP_WEBDAV_PORT,
                                ESP_SERIAL_BRIDGE_BAUD

};
#if defined(SD_DEVICE)
const uint8_t SupportedSPIDivider[] = {1, 2, 4, 6, 8, 16, 32};
const uint8_t SupportedSPIDividerSize =
    sizeof(SupportedSPIDivider) / sizeof(uint8_t);
#endif  // #ifdef SD_DEVICE
#if defined(WIFI_FEATURE)
const uint8_t SupportedApChannels[] = {1, 2, 3,  4,  5,  6,  7,
                                       8, 9, 10, 11, 12, 13, 14};
const uint8_t SupportedApChannelsSize =
    sizeof(SupportedApChannels) / sizeof(uint8_t);
#endif  // WIFI_FEATURE

bool Settings_ESP3D::begin() {
  if (GetSettingsVersion() == -1) {
    return false;
  }
  // get target FW
  Settings_ESP3D::GetFirmwareTarget(true);
  Settings_ESP3D::isVerboseBoot(true);
  return true;
}

bool Settings_ESP3D::isVerboseBoot(bool fromsettings) {
#if COMMUNICATION_PROTOCOL != MKS_SERIAL
  if (fromsettings) {
    _isverboseboot = read_byte(ESP_VERBOSE_BOOT);
  }
#else
  _isverboseboot = false;
#endif  // #if COMMUNICATION_PROTOCOL == MKS_SERIAL
  return _isverboseboot;
}

uint8_t Settings_ESP3D::GetFirmwareTarget(bool fromsettings) {
#if defined(FIXED_FW_TARGET)
  (void)fromsettings;
  _FirmwareTarget = FIXED_FW_TARGET;
#else
  if (fromsettings) {
    _FirmwareTarget = read_byte(ESP_TARGET_FW);
  }
#endif  // #if defined( FIXED_FW_TARGET )
  return _FirmwareTarget;
}

uint8_t Settings_ESP3D::GetSDDevice() {
#ifdef SD_DEVICE
  return SD_DEVICE_CONNECTION;
#else   // !SD_DEVICE
  return ESP_NO_SD;
#endif  // SD_DEVICE
}

const char *Settings_ESP3D::GetFirmwareTargetShortName() {
  static String response;

  if (_FirmwareTarget == REPETIER) {
    response = F("repetier");
  } else if (_FirmwareTarget == MARLIN) {
    response = F("marlin");
  } else if (_FirmwareTarget == MARLIN_EMBEDDED) {
    response = F("marlin");
  } else if (_FirmwareTarget == SMOOTHIEWARE) {
    response = F("smoothieware");
  } else if (_FirmwareTarget == GRBL) {
    response = F("grbl");
  } else {
    response = F("unknown");
  }
  return response.c_str();
}

uint8_t Settings_ESP3D::read_byte(int pos, bool *haserror) {
  if (haserror) {
    *haserror = true;
  }
  uint8_t value = getDefaultByteSetting(pos);
#if ESP_SAVE_SETTINGS == SETTINGS_IN_EEPROM
  // check if parameters are acceptable
  if ((pos + 1 > EEPROM_SIZE)) {
    log_esp3d_e("Error read byte %d", pos);
    return value;
  }
  // read byte
  EEPROM.begin(EEPROM_SIZE);
  value = EEPROM.read(pos);
  EEPROM.end();
#endif  // SETTINGS_IN_EEPROM
#if ESP_SAVE_SETTINGS == SETTINGS_IN_PREFERENCES
  Preferences prefs;
  if (!prefs.begin(NAMESPACE, true)) {
    log_esp3d_e("Error opening %s", NAMESPACE);
    return value;
  }
  String p = "P_" + String(pos);
  if (prefs.isKey(p.c_str())) {
    value = prefs.getChar(p.c_str(), getDefaultByteSetting(pos));
  } else {
    value = getDefaultByteSetting(pos);
  }
  prefs.end();
#endif  // SETTINGS_IN_PREFERENCES
  if (haserror) {
    *haserror = false;
  }
  return value;
}

// write a flag / byte
bool Settings_ESP3D::write_byte(int pos, const uint8_t value) {
#if ESP_SAVE_SETTINGS == SETTINGS_IN_EEPROM
  // check if parameters are acceptable
  if (pos + 1 > EEPROM_SIZE) {
    log_esp3d_e("Error read byte %d", pos);
    return false;
  }
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.write(pos, value);
  if (!EEPROM.commit()) {
    log_esp3d_e("Error commit %d", pos);
    return false;
  }
  EEPROM.end();
#endif  // SETTINGS_IN_EEPROM
#if ESP_SAVE_SETTINGS == SETTINGS_IN_PREFERENCES
  Preferences prefs;
  if (!prefs.begin(NAMESPACE, false)) {
    log_esp3d_e("Error opening %s", NAMESPACE);
    return false;
  }
  String p = "P_" + String(pos);
  uint8_t r = prefs.putChar(p.c_str(), value);
  prefs.end();
  if (r == 0) {
    log_esp3d_e("Error commit %s", p.c_str());
    return false;
  }
#endif  // SETTINGS_IN_PREFERENCES
  return true;
}

bool Settings_ESP3D::is_string(const char *s, uint len) {
  for (uint p = 0; p < len; p++) {
    if (!isPrintable(char(s[p]))) {
      return false;
    }
  }
  return true;
}

// read a string
// a string is multibyte + \0, this is won't work if 1 char is multibyte like
// chinese char
const char *Settings_ESP3D::read_string(int pos, bool *haserror) {
  const ESP3DSettingDescription *query = getSettingPtr(pos);
  if (haserror) {
    *haserror = true;
  }
  if (!query) {
    log_esp3d_e("Error unknow entry %d", pos);
    return "";
  }
  uint8_t size_max = query->size;

#if ESP_SAVE_SETTINGS == SETTINGS_IN_EEPROM
  static char *byte_buffer = NULL;
  size_max++;  // do not forget the 0x0 for the end
  if (byte_buffer) {
    free(byte_buffer);
    byte_buffer = NULL;
  }
  // check if parameters are acceptable
  if (pos + size_max + 1 > EEPROM_SIZE) {
    log_esp3d_e("Error read string %d", pos);
    return "";
  }
  byte_buffer = (char *)malloc(size_max + 1);
  if (!byte_buffer) {
    log_esp3d_e("Error mem read string %d", pos);
    return "";
  }
  EEPROM.begin(EEPROM_SIZE);
  byte b = 1;  // non zero for the while loop below
  int i = 0;

  // read until max size is reached or \0 is found
  while (i < size_max && b != 0) {
    b = EEPROM.read(pos + i);
    byte_buffer[i] = isPrintable(char(b)) ? b : 0;
    i++;
  }

  // Be sure there is a 0 at the end.
  if (b != 0) {
    byte_buffer[i - 1] = 0x00;
  }
  EEPROM.end();

  if (haserror) {
    *haserror = false;
  }
  return byte_buffer;

#endif  // SETTINGS_IN_EEPROM
#if ESP_SAVE_SETTINGS == SETTINGS_IN_PREFERENCES
  Preferences prefs;
  static String res;

  if (!prefs.begin(NAMESPACE, true)) {
    log_esp3d_e("Error opening %s", NAMESPACE);
    return "";
  }
  String p = "P_" + String(pos);
  if (prefs.isKey(p.c_str())) {
    res = prefs.getString(p.c_str(), getDefaultStringSetting(pos));
  } else {
    res = getDefaultStringSetting(pos);
  }
  prefs.end();

  if (res.length() > size_max) {
    log_esp3d_e("String too long %d vs %d", res.length(), size_max);
    res = res.substring(0, size_max - 1);
  }

  if (haserror) {
    *haserror = false;
  }
  return res.c_str();

#endif  // SETTINGS_IN_PREFERENCES
}

// write a string (array of byte with a 0x00  at the end)
bool Settings_ESP3D::write_string(int pos, const char *byte_buffer) {
  int size_buffer = strlen(byte_buffer);
  const ESP3DSettingDescription *query = getSettingPtr(pos);
  if (!query) {
    log_esp3d_e("Error unknow entry %d", pos);
    return false;
  }
  uint8_t size_max = query->size;

  // check if parameters are acceptable
  if (size_max == 0) {
    log_esp3d_e("Error unknow entry %d", pos);
    return false;
  }
  if (size_max < size_buffer) {
    log_esp3d_e("Error string too long %d, %d", pos, size_buffer);
    return false;
  }
#if ESP_SAVE_SETTINGS == SETTINGS_IN_EEPROM
  if (pos + size_buffer + 1 > EEPROM_SIZE || byte_buffer == NULL) {
    log_esp3d_e("Error write string %d", pos);
    return false;
  }
  // copy the value(s)
  EEPROM.begin(EEPROM_SIZE);
  for (int i = 0; i < size_buffer; i++) {
    EEPROM.write(pos + i, byte_buffer[i]);
  }
  // 0 terminal
  EEPROM.write(pos + size_buffer, 0x00);
  if (!EEPROM.commit()) {
    log_esp3d_e("Error commit %d", pos);
    return false;
  }
  EEPROM.end();
#endif  // SETTINGS_IN_EEPROM
#if ESP_SAVE_SETTINGS == SETTINGS_IN_PREFERENCES
  Preferences prefs;
  if (!prefs.begin(NAMESPACE, false)) {
    log_esp3d_e("Error opening %s", NAMESPACE);
    return false;
  }
  String p = "P_" + String(pos);
  uint8_t r = prefs.putString(p.c_str(), byte_buffer);
  prefs.end();
  if (r != size_buffer) {
    log_esp3d_e("Error commit %s", p.c_str());
    return false;
  }
#endif  // SETTINGS_IN_PREFERENCES
  return true;
}

// read a uint32
uint32_t Settings_ESP3D::read_uint32(int pos, bool *haserror) {
  if (haserror) {
    *haserror = true;
  }
  uint32_t res = getDefaultIntegerSetting(pos);
#if ESP_SAVE_SETTINGS == SETTINGS_IN_EEPROM
  // check if parameters are acceptable
  uint8_t size_buffer = sizeof(uint32_t);
  if (pos + size_buffer > EEPROM_SIZE) {
    log_esp3d_e("Error read int %d", pos);
    return res;
  }
  uint8_t i = 0;
  EEPROM.begin(EEPROM_SIZE);
  // read until max size is reached
  while (i < size_buffer) {
    ((uint8_t *)(&res))[i] = EEPROM.read(pos + i);
    i++;
  }
  EEPROM.end();
#endif  // SETTINGS_IN_EEPROM
#if ESP_SAVE_SETTINGS == SETTINGS_IN_PREFERENCES
  Preferences prefs;
  if (!prefs.begin(NAMESPACE, true)) {
    log_esp3d_e("Error opening %s", NAMESPACE);
    return res;
  }
  String p = "P_" + String(pos);
  if (prefs.isKey(p.c_str())) {
    res = prefs.getUInt(p.c_str(), res);
  } else {
    res = getDefaultIntegerSetting(pos);
  }
  prefs.end();
#endif  // SETTINGS_IN_PREFERENCES
  if (haserror) {
    *haserror = false;
  }
  return res;
}

// read an IP
uint32_t Settings_ESP3D::read_IP(int pos, bool *haserror) {
  return read_uint32(pos, haserror);
}

// read an IP
String Settings_ESP3D::read_IP_String(int pos, bool *haserror) {
  return _IpToString(read_uint32(pos, haserror));
}

// write a uint32
bool Settings_ESP3D::write_uint32(int pos, const uint32_t value) {
#if ESP_SAVE_SETTINGS == SETTINGS_IN_EEPROM
  uint8_t size_buffer = sizeof(uint32_t);
  // check if parameters are acceptable
  if (pos + size_buffer > EEPROM_SIZE) {
    log_esp3d_e("Error invalid entry %d", pos);
    return false;
  }
  EEPROM.begin(EEPROM_SIZE);
  // copy the value(s)
  for (int i = 0; i < size_buffer; i++) {
    EEPROM.write(pos + i, ((uint8_t *)(&value))[i]);
  }
  if (!EEPROM.commit()) {
    log_esp3d_e("Error commit %d", pos);
    return false;
  }
  EEPROM.end();
#endif  // SETTINGS_IN_EEPROM
#if ESP_SAVE_SETTINGS == SETTINGS_IN_PREFERENCES
  Preferences prefs;
  if (!prefs.begin(NAMESPACE, false)) {
    log_esp3d_e("Error opening %s", NAMESPACE);
    return false;
  }
  String p = "P_" + String(pos);
  uint8_t r = prefs.putUInt(p.c_str(), value);
  prefs.end();
  if (r == 0) {
    log_esp3d_e("Error commit %s", p.c_str());
    return false;
  }
#endif  // SETTINGS_IN_PREFERENCES
  return true;
}

// write a IP
bool Settings_ESP3D::write_IP(int pos, const uint32_t value) {
  return write_uint32(pos, value);
}

// clear all entries
bool Settings_ESP3D::reset(bool networkonly) {
  uint nb_settings = sizeof(ESP3DSettingsData) / sizeof(uint16_t);
  for (uint i = 0; i < nb_settings; i++) {
    if (networkonly && i == ESP_SETTINGS_VERSION) {
      return true;
    }

    const ESP3DSettingDescription *query = getSettingPtr(i);
    if (query) {
      switch (query->type) {
        case ESP3DSettingType::string_t:
          if (!Settings_ESP3D::write_string(i, query->default_val)) {
            log_esp3d_e("Error reset string %d to %s", i, query->default_val);
            return false;
          }
          break;
        case ESP3DSettingType::byte_t:
          if (!Settings_ESP3D::write_byte(
                  i, (uint8_t)strtoul(query->default_val, NULL, 0))) {
            log_esp3d_e("Error reset byte %d to %s", i, query->default_val);
            return false;
          }
          break;
        case ESP3DSettingType::integer_t:
        case ESP3DSettingType::ip_t:
          if (!Settings_ESP3D::write_uint32(i, getDefaultIntegerSetting(i))) {
            log_esp3d_e("Error reset uint32 %d to %s", i, query->default_val);
            return false;
          }
          break;
        default:
          log_esp3d_e("Error unknow entry %d", i);
          break;
      }
    } else {
      log_esp3d_e("Error unknow entry %d", settingElement);
    }
  }
  return true;
}

// Get Settings Version
//  * -1 means no version detected
//  * 00 / 01 Not used
//  * 03 and up is version
int8_t Settings_ESP3D::GetSettingsVersion() {
  int8_t v = -1;
  String version = Settings_ESP3D::read_string(ESP_SETTINGS_VERSION);
  if ((version == "ESP3D") || (version.length() != 7) ||
      (version.indexOf("ESP3D") != 0)) {
    log_esp3d_e("Invalid Settings Version %s", version.c_str());
    return v;
  }
  v = version.substring(5).toInt();
  log_esp3d("Settings Version %d", v);
  return v;
}

// write a IP from string
bool Settings_ESP3D::write_IP_String(int pos, const char *value) {
  return write_uint32(pos, _stringToIP(value));
}

// Helper to convert  IP string to int
uint32_t Settings_ESP3D::_stringToIP(const char *s) {
  uint32_t ip_int = 0;
  IPAddress ipaddr;
  if (ipaddr.fromString(s)) {
    ip_int = ipaddr;
  }
  return ip_int;
}

// Helper to convert int to IP string
String Settings_ESP3D::_IpToString(uint32_t ip_int) {
  static IPAddress ipaddr;
  ipaddr = ip_int;
  return ipaddr.toString();
}

const char *Settings_ESP3D::TargetBoard() {
#ifdef ARDUINO_ARCH_ESP32
#if CONFIG_IDF_TARGET_ESP32
#define TYPE_BOARD "ESP32"
#elif CONFIG_IDF_TARGET_ESP32S2
#define TYPE_BOARD "ESP32-S2"
#elif CONFIG_IDF_TARGET_ESP32S3
#define TYPE_BOARD "ESP32-S3"
#elif CONFIG_IDF_TARGET_ESP32C3
#define TYPE_BOARD "ESP32-C3"
#endif
#ifdef BOARD_HAS_PSRAM
#define IS_PSRAM " (PSRAM)"
#else
#define IS_PSRAM ""
#endif  // BOARD_HAS_PSRAM
  return TYPE_BOARD IS_PSRAM;
#endif  // ARDUINO_ARCH_ESP32
#ifdef ARDUINO_ARCH_ESP8266
  return "ESP82XX";
#endif  // ARDUINO_ARCH_ESP8266
}

#endif  // ESP_SAVE_SETTINGS

bool Settings_ESP3D::isValidIPStringSetting(const char *value,
                                            ESP3DSettingIndex settingElement) {
  const ESP3DSettingDescription *settingPtr = getSettingPtr(settingElement);
  if (!settingPtr) {
    return false;
  }
  if (settingPtr->type != ESP3DSettingType::ip_t) {
    return false;
  }
  String ippart[4];
  uint index = 0;
  for (uint8_t i = 0; i < strlen(value); i++) {
    // only digit and . are allowed
    if (!isDigit(value[i]) && value[i] != '.') {
      return false;
    }
    if (value[i] == '.') {  // new ip part
      index++;
      if (index > 3) {  // only 4 parts allowed (IPv4 only for the moment)
        return false;
      }
    } else {  // fill the part
      ippart[index] += value[i];
    }
  }
  if (index != 3) {  // only exactly 4 parts allowed, no less
    return false;
  }
  for (uint8_t i = 0; i < 4; i++) {  // check each part
    // max value is 255, no empty part, no part longer than 3 digits
    if (ippart[i].toInt() > 255 || ippart[i].length() > 3 ||
        ippart[i].length() == 0) {
      return false;
    }
  }
  return true;
}
bool Settings_ESP3D::isValidStringSetting(const char *value,
                                          ESP3DSettingIndex settingElement) {
  const ESP3DSettingDescription *settingPtr = getSettingPtr(settingElement);
  if (!(settingPtr->type == ESP3DSettingType::string_t)) {
    return false;
  }
  size_t len = strlen(value);
  // cannot be over max size
  if (len > settingPtr->size) {
    return false;
  }
  // only printable char allowed
  for (uint i = 0; i < strlen(value); i++) {
    if (!isPrintable(value[i])) {
      return false;
    }
  }
  switch (settingElement) {
    case ESP_SETTINGS_VERSION:
      return (!strcmp(value, DEFAULT_SETTINGS_VERSION) == 0);
      break;
    case ESP_HOSTNAME:
      // only letter and digit
      for (uint i = 0; i < strlen(value); i++) {
        char c = value[i];
        if (!(isdigit(c) || isalpha(c) || c == '-')) {
          return false;
        }
        if (c == ' ') {
          return false;
        }
      }
      return (len >= 1);  // at least 1 char for hostname
      break;
    case ESP_STA_SSID:
    case ESP_AP_SSID:
      return (len >= 1);  // at least 1 char for ssid
      break;
    case ESP_ADMIN_PWD:
    case ESP_USER_PWD:
      for (uint i = 0; i < strlen(value); i++) {
        if (value[i] == ' ') {  // no space allowed
          return false;
        }
      }
      return (len >= 1);  // at least 1 char for password
      break;
    case ESP_STA_PASSWORD:
    case ESP_AP_PASSWORD:
      return (len == 0 ||
              (len >= 8));  // no password or at least 8 char for password
#if defined(NOTIFICATION_FEATURE)
    case ESP_NOTIFICATION_TOKEN1:
    case ESP_NOTIFICATION_TOKEN2:
    case ESP_NOTIFICATION_SETTINGS:
      return true;  // no more check for notification
      break;
#endif  // NOTIFICATION_FEATURE
#if defined(TIMESTAMP_FEATURE)
    case ESP_TIME_SERVER1:
    case ESP_TIME_SERVER2:
    case ESP_TIME_SERVER3:
      return true;  // no more check for time server
      break;
    case ESP_TIME_ZONE:
      if (len != settingPtr->size - 1) {
        return false;
      }
      for (uint8_t i = 0; i < SupportedTimeZonesSize; i++) {
        if (strcmp(value, SupportedTimeZones[i]) == 0) {
          return true;
        }
      }
      break;
#endif  // TIMESTAMP_FEATURE
    default:
      return false;
  }
  return false;
}
bool Settings_ESP3D::isValidIntegerSetting(uint32_t value,
                                           ESP3DSettingIndex settingElement) {
  const ESP3DSettingDescription *settingPtr = getSettingPtr(settingElement);
  if (!settingPtr) {
    return false;
  }
  if (settingPtr->type != ESP3DSettingType::integer_t) {
    return false;
  }
  switch (settingElement) {
    case ESP_SERIAL_BRIDGE_BAUD:
    case ESP_BAUD_RATE:
      for (uint8_t i = 0; i < SupportedBaudListSize; i++) {
        if (value == SupportedBaudList[i]) {
          return true;
        }
      }
      break;
    case ESP_WEBDAV_PORT:
    case ESP_HTTP_PORT:
    case ESP_TELNET_PORT:
    case ESP_FTP_CTRL_PORT:
    case ESP_FTP_DATA_ACTIVE_PORT:
    case ESP_FTP_DATA_PASSIVE_PORT:
    case ESP_WEBSOCKET_PORT:
      if (value >= 1 && value <= 65535) {
        return true;
      }
      break;
    case ESP_SENSOR_INTERVAL:
      if (value >= MIN_SENSOR_INTERVAL && value <= MAX_SENSOR_INTERVAL) {
        return true;
      }
      break;
    case ESP_BOOT_DELAY:
      if (value >= MIN_BOOT_DELAY && value <= MAX_BOOT_DELAY) {
        return true;
      }
      break;
    case ESP_CALIBRATION_1:
    case ESP_CALIBRATION_2:
    case ESP_CALIBRATION_3:
    case ESP_CALIBRATION_4:
    case ESP_CALIBRATION_5:
      // no check for calibration currently
      return true;
      break;
    default:
      return false;
  }
  return false;
}
bool Settings_ESP3D::isValidByteSetting(uint8_t value,
                                        ESP3DSettingIndex settingElement) {
  const ESP3DSettingDescription *settingPtr = getSettingPtr(settingElement);
  if (!settingPtr) {
    return false;
  }
  if (settingPtr->type != ESP3DSettingType::byte_t) {
    return false;
  }

  switch (settingElement) {
    case ESP_BUZZER:
    case ESP_INTERNET_TIME:
    case ESP_SERIAL_FLAG:
    case ESP_REMOTE_SCREEN_FLAG:
    case ESP_HTTP_ON:
    case ESP_TELNET_ON:
    case ESP_WEBSOCKET_ON:
    case ESP_WEBSOCKET_FLAG:
    case ESP_SD_CHECK_UPDATE_AT_BOOT:
    case ESP_SETUP:
    case ESP_TELNET_FLAG:
    case ESP_BT_FLAG:
    case ESP_SCREEN_FLAG:
    case ESP_FTP_ON:
    case ESP_AUTO_NOTIFICATION:
    case ESP_SERIAL_BRIDGE_FLAG:
    case ESP_SERIAL_BRIDGE_ON:
    case ESP_VERBOSE_BOOT:
    case ESP_WEBDAV_ON:
    case ESP_SECURE_SERIAL:
    case ESP_CALIBRATION:
    case ESP_BOOT_RADIO_STATE:
      if (value == (uint8_t)ESP3DState::off ||
          value == (uint8_t)ESP3DState::on) {
        return true;
      }
      break;
#if defined(NOTIFICATION_FEATURE)
    case ESP_NOTIFICATION_TYPE:
      if (value == ESP_NO_NOTIFICATION || value == ESP_PUSHOVER_NOTIFICATION ||
          value == ESP_EMAIL_NOTIFICATION || value == ESP_LINE_NOTIFICATION ||
          value == ESP_TELEGRAM_NOTIFICATION ||
          value == ESP_IFTTT_NOTIFICATION) {
        return true;
      }

      break;
#endif  // NOTIFICATION_FEATURE
    case ESP_RADIO_MODE:
      if (value == ESP_NO_NETWORK
#if defined(WIFI_FEATURE)
          || value == ESP_WIFI_STA || value == ESP_WIFI_AP ||
          value == ESP_AP_SETUP
#endif  // WIFI_FEATURE
#if defined(ETH_FEATURE)
          || value == ESP_ETH
#endif  // ETH_FEATURE
#if defined(BLUETOOTH_FEATURE)
          || value == ESP_BT
#endif  // BLUETOOTH_FEATURE
      ) {
        return true;
      }
      break;
    case ESP_STA_IP_MODE:
      if (value == DHCP_MODE || value == STATIC_IP_MODE) {
        return true;
      }
      break;
#if defined(WIFI_FEATURE)
    case ESP_AP_CHANNEL:
      for (uint8_t i = 0; i < SupportedApChannelsSize; i++) {
        if (value == SupportedApChannels[i]) {
          return true;
        }
      }
      break;
#endif  // WIFI_FEATURE
#ifdef SD_DEVICE
    case ESP_SD_SPEED_DIV:
      for (uint8_t i = 0; i < SupportedSPIDividerSize; i++) {
        if (value == SupportedSPIDivider[i]) {
          return true;
        }
      }
      break;
    case ESP_SD_MOUNT:
      if (value == ESP_SD_ROOT || value == ESP_SD_SUB_SD ||
          value == ESP_SD_SUB_EXT) {
        return true;
      }
      break;
#endif  // #ifdef SD_DEVICE
#ifdef SENSOR_DEVICE
    case ESP_SENSOR_TYPE:
      if (value == 0) {
        return true;
      }
      for (uint8_t p = 0; p < esp3d_sensor.nbType(); p++) {
        if (value == esp3d_sensor.GetModel(p)) {
          return true;
        }
      }
      break;
#endif  // SENSOR_DEVICE
    case ESP_TARGET_FW:
      if (value == REPETIER || value == MARLIN || value == UNKNOWN_FW ||
          value == SMOOTHIEWARE || value == GRBL) {
        return true;
      }
      break;
    case ESP_SESSION_TIMEOUT:
      // TODO: what should be the upper limit for session timeout?
      // 0 means no timeout so it is ok to have 0
      return true;
      break;
    case ESP_STA_FALLBACK_MODE:
      if (value == ESP_NO_NETWORK
#if defined(WIFI_FEATURE)
          || value == ESP_AP_SETUP
#endif  // WIFI_FEATURE
#if defined(BT_FEATURE)
          || value == ESP_BT
#endif  // BT_FEATURE
      ) {
        return true;
      }
      break;

    default:
      return false;
  }

  return true;
}

uint32_t Settings_ESP3D::getDefaultIntegerSetting(
    ESP3DSettingIndex settingElement) {
  const ESP3DSettingDescription *query = getSettingPtr(settingElement);
  if (getSettingPtr) {
    if (query->type == ESP3DSettingType::ip_t) {
      return _stringToIP(query->default_val);
    }
    if (query->type == ESP3DSettingType::integer_t)
      return (uint32_t)strtoul(query->default_val, NULL, 0);
    else {
      log_esp3d_e("Error invalid type %d for %d", query->type, settingElement);
    }
  }
  return 0;
}

const char *Settings_ESP3D::getDefaultStringSetting(
    ESP3DSettingIndex settingElement) {
  const ESP3DSettingDescription *query = getSettingPtr(settingElement);
  if (getSettingPtr) {
    if (query->type == ESP3DSettingType::string_t ||
        query->type == ESP3DSettingType::ip_t)
      return query->default_val;
    else {
      log_esp3d_e("Error invalid type %d for %d", query->type, settingElement);
    }
  }
  return NULL;
}

uint8_t Settings_ESP3D::getDefaultByteSetting(
    ESP3DSettingIndex settingElement) {
  const ESP3DSettingDescription *query = getSettingPtr(settingElement);
  if (getSettingPtr) {
    if (query->type == ESP3DSettingType::byte_t)
      return (uint8_t)strtoul(query->default_val, NULL, 0);
    else {
      log_esp3d_e("Error invalid type %d for %d", query->type, settingElement);
    }
  }
  return 0;
}

// get the description of a setting
// Unlike esp32 esp8266 does not have lot of memory so we hard code the
// settings and just generate one setting on demand
const ESP3DSettingDescription *Settings_ESP3D::getSettingPtr(
    const ESP3DSettingIndex index) {
  static ESP3DSettingDescription setting;
  memset(&setting, 0, sizeof(setting));
  // index of setting
  setting.index = index;
  // type of setting
  switch (index) {
    case ESP_RADIO_MODE:
    case ESP_NOTIFICATION_TYPE:
    case ESP_CALIBRATION:
    case ESP_AP_CHANNEL:
    case ESP_BUZZER:
    case ESP_INTERNET_TIME:
    case ESP_SERIAL_FLAG:
    case ESP_HTTP_ON:
    case ESP_TELNET_ON:
    case ESP_WEBSOCKET_ON:
    case ESP_SD_SPEED_DIV:
    case ESP_SENSOR_TYPE:
    case ESP_TARGET_FW:
    case ESP_TIME_IS_DST:
    case ESP_REMOTE_SCREEN_FLAG:
    case ESP_SD_MOUNT:
    case ESP_SESSION_TIMEOUT:
    case ESP_WEBSOCKET_FLAG:
    case ESP_SD_CHECK_UPDATE_AT_BOOT:
    case ESP_SETUP:
    case ESP_TELNET_FLAG:
    case ESP_BT_FLAG:
    case ESP_SCREEN_FLAG:
    case ESP_FTP_ON:
    case ESP_AUTO_NOTIFICATION:
    case ESP_VERBOSE_BOOT:
    case ESP_WEBDAV_ON:
    case ESP_SECURE_SERIAL:
    case ESP_BOOT_RADIO_STATE:
    case ESP_STA_FALLBACK_MODE:
    case ESP_SERIAL_BRIDGE_FLAG:
    case ESP_SERIAL_BRIDGE_ON:
    case ESP_STA_IP_MODE:
      setting.type = ESP3DSettingType::byte_t;  // byte
      break;

    case ESP_SETTINGS_VERSION:
    case ESP_HOSTNAME:
    case ESP_STA_PASSWORD:
    case ESP_STA_SSID:
    case ESP_ADMIN_PWD:
    case ESP_USER_PWD:
    case ESP_AP_SSID:
    case ESP_AP_PASSWORD:
    case ESP_NOTIFICATION_TOKEN1:
    case ESP_NOTIFICATION_TOKEN2:
    case ESP_NOTIFICATION_SETTINGS:
    case ESP_TIME_SERVER1:
    case ESP_TIME_SERVER2:
    case ESP_TIME_SERVER3:
    case ESP_TIME_ZONE:
      setting.type = ESP3DSettingType::string_t;  // string
      break;

    case ESP_STA_IP_VALUE:
    case ESP_STA_GATEWAY_VALUE:
    case ESP_STA_MASK_VALUE:
    case ESP_STA_DNS_VALUE:
    case ESP_AP_IP_VALUE:

      setting.type = ESP3DSettingType::ip_t;  // ip = 4 bytes
      break;

    case ESP_BAUD_RATE:
    case ESP_HTTP_PORT:
    case ESP_TELNET_PORT:
    case ESP_SENSOR_INTERVAL:
    case ESP_BOOT_DELAY:
    case ESP_WEBSOCKET_PORT:
    case ESP_CALIBRATION_1:
    case ESP_CALIBRATION_2:
    case ESP_CALIBRATION_3:
    case ESP_CALIBRATION_4:
    case ESP_CALIBRATION_5:
    case ESP_FTP_CTRL_PORT:
    case ESP_FTP_DATA_ACTIVE_PORT:
    case ESP_FTP_DATA_PASSIVE_PORT:
    case ESP_WEBDAV_PORT:
    case ESP_SERIAL_BRIDGE_BAUD:

      setting.type = ESP3DSettingType::integer_t;  // integer = 4 bytes
      break;
    default:
      setting.type = ESP3DSettingType::unknown_t;
      break;
  }

  // size of setting
  switch (index) {
    case ESP_RADIO_MODE:
    case ESP_NOTIFICATION_TYPE:
    case ESP_CALIBRATION:
    case ESP_AP_CHANNEL:
    case ESP_BUZZER:
    case ESP_INTERNET_TIME:
    case ESP_SERIAL_FLAG:
    case ESP_HTTP_ON:
    case ESP_TELNET_ON:
    case ESP_WEBSOCKET_ON:
    case ESP_SD_SPEED_DIV:
    case ESP_SENSOR_TYPE:
    case ESP_TARGET_FW:
    case ESP_TIME_IS_DST:
    case ESP_REMOTE_SCREEN_FLAG:
    case ESP_SD_MOUNT:
    case ESP_SESSION_TIMEOUT:
    case ESP_WEBSOCKET_FLAG:
    case ESP_SD_CHECK_UPDATE_AT_BOOT:
    case ESP_SETUP:
    case ESP_TELNET_FLAG:
    case ESP_BT_FLAG:
    case ESP_SCREEN_FLAG:
    case ESP_FTP_ON:
    case ESP_AUTO_NOTIFICATION:
    case ESP_VERBOSE_BOOT:
    case ESP_WEBDAV_ON:
    case ESP_SECURE_SERIAL:
    case ESP_BOOT_RADIO_STATE:
    case ESP_STA_FALLBACK_MODE:
    case ESP_SERIAL_BRIDGE_FLAG:
    case ESP_SERIAL_BRIDGE_ON:
    case ESP_STA_IP_MODE:
      setting.size = 1;  // 1 byte
      break;
    case ESP_STA_IP_VALUE:
    case ESP_STA_GATEWAY_VALUE:
    case ESP_STA_MASK_VALUE:
    case ESP_STA_DNS_VALUE:
    case ESP_AP_IP_VALUE:
    case ESP_BAUD_RATE:
    case ESP_HTTP_PORT:
    case ESP_TELNET_PORT:
    case ESP_SENSOR_INTERVAL:
    case ESP_BOOT_DELAY:
    case ESP_WEBSOCKET_PORT:
    case ESP_CALIBRATION_1:
    case ESP_CALIBRATION_2:
    case ESP_CALIBRATION_3:
    case ESP_CALIBRATION_4:
    case ESP_CALIBRATION_5:
    case ESP_FTP_CTRL_PORT:
    case ESP_FTP_DATA_ACTIVE_PORT:
    case ESP_FTP_DATA_PASSIVE_PORT:
    case ESP_WEBDAV_PORT:
    case ESP_SERIAL_BRIDGE_BAUD:
      setting.size = 4;  // 4 bytes
      break;
    // Note for string size is the max size of the string, in EEPROM it use
    // the size + 1 for the ending 0x00
    case ESP_STA_SSID:
      setting.size = 32;  // 32 bytes, warning does not support multibyte char
                          // like chinese chars
      break;
    case ESP_SETTINGS_VERSION:
      setting.size = 7;  // 7 bytes
      break;
    case ESP_HOSTNAME:
      setting.size = 32;  // 32 bytes, warning does not support multibyte char
                          // like chinese chars
      break;
    case ESP_STA_PASSWORD:
      setting.size = 64;  // 64 bytes, warning does not support multibyte char
                          // like chinese chars
      break;
    case ESP_ADMIN_PWD:
    case ESP_USER_PWD:
      setting.size = 20;  // 20 bytes
      break;
    case ESP_AP_SSID:
      setting.size = 32;  // 32 bytes, warning does not support multibyte char
                          // like chinese chars
      break;
    case ESP_AP_PASSWORD:
      setting.size = 64;  // 64 bytes, warning does not support multibyte char
                          // like chinese chars
      break;
    case ESP_NOTIFICATION_TOKEN1:
    case ESP_NOTIFICATION_TOKEN2:
      setting.size = 63;  // 63 bytes
      break;
    case ESP_NOTIFICATION_SETTINGS:
      setting.size = 128;  // 128 bytes
      break;
    case ESP_TIME_SERVER1:
    case ESP_TIME_SERVER2:
    case ESP_TIME_SERVER3:
      setting.size = 128;  // 128 bytes
      break;
    case ESP_TIME_ZONE:
      setting.size = 6;  // 6 bytes
      break;

    default:
      break;
  }

  // default value of setting in string
  switch (index) {
    case ESP_STA_IP_MODE:
      setting.default_val = DEFAULT_STA_IP_MODE;
      break;
    case ESP_RADIO_MODE:
      setting.default_val = DEFAULT_ESP_RADIO_MODE;
      break;
    case ESP_STA_SSID:
      setting.default_val = DEFAULT_STA_SSID;
      break;
    case ESP_NOTIFICATION_TYPE:
      setting.default_val = DEFAULT_NOTIFICATION_TYPE;
      break;
    case ESP_CALIBRATION:
      setting.default_val = DEFAULT_CALIBRATION_DONE;
      break;
    case ESP_AP_CHANNEL:
      setting.default_val = DEFAULT_AP_CHANNEL;
      break;
    case ESP_BUZZER:
      setting.default_val = DEFAULT_BUZZER_STATE;
      break;
    case ESP_INTERNET_TIME:
      setting.default_val = DEFAULT_INTERNET_TIME;
      break;
    case ESP_SERIAL_FLAG:
      setting.default_val = DEFAULT_SERIAL_OUTPUT_FLAG;
      break;
    case ESP_HTTP_ON:
      setting.default_val = DEFAULT_HTTP_ON;
      break;
    case ESP_TELNET_ON:
      setting.default_val = DEFAULT_TELNET_ON;
      break;
    case ESP_WEBSOCKET_ON:
      setting.default_val = DEFAULT_WEBSOCKET_ON;
      break;
    case ESP_SD_SPEED_DIV:
      setting.default_val = DEFAULT_SD_SPI_DIV;
      break;
    case ESP_SENSOR_TYPE:
      setting.default_val = DEFAULT_SENSOR_TYPE;
      break;
    case ESP_TARGET_FW:
      setting.default_val = DEFAULT_FW;
      break;
    case ESP_TIME_IS_DST:
      setting.default_val = DEFAULT_TIME_DST;
      break;
    case ESP_REMOTE_SCREEN_FLAG:
      setting.default_val = DEFAULT_REMOTE_SCREEN_FLAG;
      break;
    case ESP_SD_MOUNT:
      setting.default_val = DEFAULT_SD_MOUNT;
      break;
    case ESP_SESSION_TIMEOUT:
      setting.default_val = DEFAULT_SESSION_TIMEOUT;
      break;
    case ESP_WEBSOCKET_FLAG:
      setting.default_val = DEFAULT_WEBSOCKET_FLAG;
      break;
    case ESP_SD_CHECK_UPDATE_AT_BOOT:
      setting.default_val = DEFAULT_SD_CHECK_UPDATE_AT_BOOT;
      break;
    case ESP_SETUP:
      setting.default_val = DEFAULT_SETUP;
      break;
    case ESP_TELNET_FLAG:
      setting.default_val = DEFAULT_TELNET_FLAG;
      break;
    case ESP_BT_FLAG:
      setting.default_val = DEFAULT_BT_FLAG;
      break;
    case ESP_SCREEN_FLAG:
      setting.default_val = DEFAULT_SCREEN_FLAG;
      break;
    case ESP_FTP_ON:
      setting.default_val = DEFAULT_FTP_ON;
      break;
    case ESP_AUTO_NOTIFICATION:
      setting.default_val = DEFAULT_AUTO_NOTIFICATION_STATE;
      break;
    case ESP_VERBOSE_BOOT:
      setting.default_val = DEFAULT_VERBOSE_BOOT;
      break;
    case ESP_WEBDAV_ON:
      setting.default_val = DEFAULT_WEBDAV_ON;
      break;
    case ESP_SECURE_SERIAL:
      setting.default_val = DEFAULT_SECURE_SERIAL;
      break;
    case ESP_BOOT_RADIO_STATE:
      setting.default_val = DEFAULT_BOOT_RADIO_STATE;
      break;
    case ESP_STA_FALLBACK_MODE:
      setting.default_val = DEFAULT_STA_FALLBACK_MODE;
      break;
    case ESP_SERIAL_BRIDGE_FLAG:
      setting.default_val = DEFAULT_SERIAL_BRIDGE_FLAG;
      break;
    case ESP_SERIAL_BRIDGE_ON:
      setting.default_val = DEFAULT_SERIAL_BRIDGE_ON;
      break;
    case ESP_SERIAL_BRIDGE_BAUD:
      setting.default_val = DEFAULT_SERIAL_BRIDGE_BAUD_RATE;
      break;
    case ESP_SETTINGS_VERSION:
      setting.default_val = DEFAULT_SETTINGS_VERSION;
      break;
    case ESP_HOSTNAME:
      setting.default_val = DEFAULT_HOSTNAME;
      break;
    case ESP_STA_PASSWORD:
      setting.default_val = DEFAULT_STA_PASSWORD;
      break;
    case ESP_ADMIN_PWD:
      setting.default_val = DEFAULT_ADMIN_PWD;
      break;
    case ESP_USER_PWD:
      setting.default_val = DEFAULT_USER_PWD;
      break;
    case ESP_AP_SSID:
      setting.default_val = DEFAULT_AP_SSID;
      break;
    case ESP_AP_PASSWORD:
      setting.default_val = DEFAULT_AP_PASSWORD;
      break;
    case ESP_NOTIFICATION_TOKEN1:
      setting.default_val = DEFAULT_NOTIFICATION_TOKEN1;
      break;
    case ESP_NOTIFICATION_TOKEN2:
      setting.default_val = DEFAULT_NOTIFICATION_TOKEN2;
      break;
    case ESP_NOTIFICATION_SETTINGS:
      setting.default_val = DEFAULT_NOTIFICATION_SETTINGS;
      break;
    case ESP_TIME_SERVER1:
      setting.default_val = DEFAULT_TIME_SERVER1;
      break;
    case ESP_TIME_SERVER2:
      setting.default_val = DEFAULT_TIME_SERVER2;
      break;
    case ESP_TIME_SERVER3:
      setting.default_val = DEFAULT_TIME_SERVER3;
      break;
    case ESP_TIME_ZONE:
      setting.default_val = DEFAULT_TIME_ZONE;
      break;
    case ESP_STA_IP_VALUE:
      setting.default_val = DEFAULT_STA_IP_VALUE;
      break;
    case ESP_STA_GATEWAY_VALUE:
      setting.default_val = DEFAULT_STA_GATEWAY_VALUE;
      break;
    case ESP_STA_MASK_VALUE:
      setting.default_val = DEFAULT_STA_MASK_VALUE;
      break;
    case ESP_STA_DNS_VALUE:
      setting.default_val = DEFAULT_STA_DNS_VALUE;
      break;
    case ESP_AP_IP_VALUE:
      setting.default_val = DEFAULT_AP_IP_VALUE;
      break;
    case ESP_BAUD_RATE:
      setting.default_val = DEFAULT_BAUD_RATE;
      break;
    case ESP_HTTP_PORT:
      setting.default_val = DEFAULT_HTTP_PORT;
      break;
    case ESP_TELNET_PORT:
      setting.default_val = DEFAULT_TELNET_PORT;
      break;
    case ESP_SENSOR_INTERVAL:
      setting.default_val = DEFAULT_SENSOR_INTERVAL;
      break;
    case ESP_BOOT_DELAY:
      setting.default_val = DEFAULT_BOOT_DELAY;
      break;
    case ESP_WEBSOCKET_PORT:
      setting.default_val = DEFAULT_WEBSOCKET_PORT;
      break;
    case ESP_CALIBRATION_1:
    case ESP_CALIBRATION_2:
    case ESP_CALIBRATION_3:
    case ESP_CALIBRATION_4:
    case ESP_CALIBRATION_5:
      setting.default_val = DEFAULT_CALIBRATION_VALUE;
      break;
    case ESP_FTP_CTRL_PORT:
      setting.default_val = DEFAULT_FTP_CTRL_PORT;
      break;
    case ESP_FTP_DATA_ACTIVE_PORT:
      setting.default_val = DEFAULT_FTP_ACTIVE_PORT;
      break;
    case ESP_FTP_DATA_PASSIVE_PORT:
      setting.default_val = DEFAULT_FTP_PASSIVE_PORT;
      break;
    case ESP_WEBDAV_PORT:
      setting.default_val = DEFAULT_WEBDAV_PORT;
      break;
    default:
      log_esp3d_e("Invalid setting %d", index);
      return NULL;
      break;
  }

  return &setting;
}
