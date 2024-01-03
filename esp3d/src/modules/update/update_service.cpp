/*
  update_service.cpp -  update services functions class

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

#include "../../include/esp3d_config.h"
#ifdef SD_UPDATE_FEATURE
#include "../../core/esp3d_commands.h"
#include "../../core/esp3d_message.h"
#include "../../core/esp3d_settings.h"
#include "../filesystem/esp_filesystem.h"
#include "../filesystem/esp_sd.h"
#include "esp_config_file.h"
#include "update_service.h"

#if defined(ARDUINO_ARCH_ESP32)
#include <Update.h>
#define U_FS U_SPIFFS
#endif  // ARDUINO_ARCH_ESP32
#if defined(ARDUINO_ARCH_ESP8266)

#endif  // ARDUINO_ARCH_ESP8266

UpdateService update_service;

#define CONFIG_FILE "/esp3dcnf.ini"
#define FW_FILE "/esp3dfw.bin"
#define FS_FILE "/esp3dfs.bin"

const char* NetstringKeysVal[] = {"hostname", "STA_SSID", "STA_Password",
                                  "AP_SSID", "AP_Password"};

const uint16_t NetstringKeysPos[] = {
    ESP_HOSTNAME, ESP_STA_SSID, ESP_STA_PASSWORD, ESP_AP_SSID, ESP_AP_PASSWORD};

const char* ServstringKeysVal[] = {
    "Time_zone",    "Time_server1",   "Time_server2",
    "Time_server3", "ADMIN_PASSWORD", "USER_PASSWORD",
    "NOTIF_TOKEN1", "NOTIF_TOKEN2",   "NOTIF_TOKEN_Settings"};

const uint16_t ServstringKeysPos[] = {ESP_TIME_ZONE,
                                      ESP_TIME_SERVER1,
                                      ESP_TIME_SERVER2,
                                      ESP_TIME_SERVER3,
                                      ESP_ADMIN_PWD,
                                      ESP_USER_PWD,
                                      ESP_NOTIFICATION_TOKEN1,
                                      ESP_NOTIFICATION_TOKEN2,
                                      ESP_NOTIFICATION_SETTINGS};

const char* IPKeysVal[] = {"STA_IP", "STA_GW", "STA_MSK", "STA_DNS", "AP_IP"};

const uint16_t IPKeysPos[] = {ESP_STA_IP_VALUE, ESP_STA_GATEWAY_VALUE,
                              ESP_STA_MASK_VALUE, ESP_STA_DNS_VALUE,
                              ESP_AP_IP_VALUE};

const char* ServintKeysVal[] = {
    "Serial_Bridge_Baud"
    "HTTP_Port",
    "TELNET_Port",
    "SENSOR_INTERVAL",
    "WebSocket_Port",
    "WebDav_Port",
    "FTP_Control_Port",
    "FTP_Active_Port ",
    "FTP_Passive_Port"};

const uint16_t ServintKeysPos[] = {
    ESP_SERIAL_BRIDGE_BAUD,   ESP_HTTP_PORT,
    ESP_TELNET_PORT,          ESP_SENSOR_INTERVAL,
    ESP_WEBSOCKET_PORT,       ESP_WEBDAV_PORT,
    ESP_FTP_CTRL_PORT,        ESP_FTP_DATA_ACTIVE_PORT,
    ESP_FTP_DATA_PASSIVE_PORT};

const char* SysintKeysVal[] = {"Baud_rate", "Boot_delay"};

const uint16_t SysintKeysPos[] = {ESP_BAUD_RATE, ESP_BOOT_DELAY};

const char* ServboolKeysVal[] = {"Serial_Bridge_active", "AUTONOTIFICATION",
                                 "HTTP_active",          "TELNET_active",
                                 "WebSocket_active",     "WebDav_active",
                                 "CHECK_FOR_UPDATE",     "Active_buzzer",
                                 "Active_Internet_time", "Radio_enabled"};

const uint16_t ServboolKeysPos[] = {ESP_SERIAL_BRIDGE_ON,
                                    ESP_AUTO_NOTIFICATION,
                                    ESP_HTTP_ON,
                                    ESP_TELNET_ON,
                                    ESP_WEBSOCKET_ON,
                                    ESP_WEBDAV_ON,
                                    ESP_SD_CHECK_UPDATE_AT_BOOT,
                                    ESP_BUZZER,
                                    ESP_INTERNET_TIME,
                                    ESP_BOOT_RADIO_STATE};

const char* SysboolKeysVal[] = {"Boot_verbose", "Secure_serial"};

const uint16_t SysboolKeysPos[] = {ESP_VERBOSE_BOOT, ESP_SECURE_SERIAL};

const char* NetbyteKeysVal[] = {"AP_channel"};

const uint16_t NetbyteKeysPos[] = {ESP_AP_CHANNEL};
const char* ServbyteKeysVal[] = {"Session_timeout", "SD_SPEED"};

const uint16_t ServbyteKeysPos[] = {ESP_SESSION_TIMEOUT, ESP_SD_SPEED_DIV};

bool processString(const char** keysval, const uint16_t* keypos,
                   const size_t size, const char* key, const char* value,
                   char& T, int& P) {
  for (uint i = 0; i < size; i++) {
    if (strcasecmp(keysval[i], key) == 0) {
      // if it is a previouly saved scrambled password ignore it
      if (strcasecmp(value, "********") != 0) {
        T = 'S';
        P = keypos[i];
        return true;
      }
    }
  }
  return false;
}

bool processInt(const char** keysval, const uint16_t* keypos, const size_t size,
                const char* key, const char* value, char& T, int& P,
                uint32_t& v) {
  for (uint i = 0; i < size; i++) {
    if (strcasecmp(keysval[i], key) == 0) {
      T = 'I';
      P = keypos[i];
      v = String(value).toInt();
      return true;
    }
  }
  return false;
}

bool processBool(const char** keysval, const uint16_t* keypos,
                 const size_t size, const char* key, const char* value, char& T,
                 int& P, byte& b) {
  for (uint i = 0; i < size; i++) {
    if (strcasecmp(keysval[i], key) == 0) {
      T = 'B';
      P = keypos[i];
      if ((strcasecmp("yes", value) == 0) || (strcasecmp("on", value) == 0) ||
          (strcasecmp("true", value) == 0) || (strcasecmp("1", value) == 0)) {
        b = 1;
      } else if ((strcasecmp("no", value) == 0) ||
                 (strcasecmp("off", value) == 0) ||
                 (strcasecmp("false", value) == 0) ||
                 (strcasecmp("0", value) == 0)) {
        b = 0;
      } else {
        P = -1;
      }
      return true;
    }
  }
  return false;
}

// Parsing all entries of file once is faster that checking all possible
// parameters for each line of file
bool processingFileFunction(const char* section, const char* key,
                            const char* value) {
  bool res = true;
  char T = '\0';
  int P = -1;
  uint32_t v = 0;
  byte b = 0;
  bool done = false;
  esp3d_log("[%s]%s=%s", section, key, value);
  // network / services / system sections
  if (strcasecmp("network", section) == 0) {
    if (!done) {
      done = processString(NetstringKeysVal, NetstringKeysPos,
                           sizeof(NetstringKeysVal) / sizeof(char*), key, value,
                           T, P);
    }
    if (!done) {
      done = processString(IPKeysVal, IPKeysPos,
                           sizeof(IPKeysVal) / sizeof(char*), key, value, T, P);
      if (done) {
        T = 'A';
      }
    }
    if (!done) {
      done = processInt(NetbyteKeysVal, NetbyteKeysPos,
                        sizeof(NetbyteKeysVal) / sizeof(char*), key, value, T,
                        P, v);
      if (done) {
        T = 'B';
        b = v;
      }
    }
    // Radio mode BT, WIFI-STA, WIFI-AP, ETH-STA, OFF
    if (!done) {
      if (strcasecmp("radio_mode", key) == 0) {
        T = 'B';
        P = ESP_RADIO_MODE;
        done = true;
        if (strcasecmp("BT", value) == 0) {
          b = ESP_BT;
        } else if (strcasecmp("WIFI-STA", value) == 0) {
          b = ESP_WIFI_STA;
        } else if (strcasecmp("WIFI-AP", value) == 0) {
          b = ESP_WIFI_AP;
        } else if (strcasecmp("WIFI-SETUP", value) == 0) {
          b = ESP_AP_SETUP;
        } else if (strcasecmp("ETH-STA", value) == 0) {
          b = ESP_ETH_STA;
        } else if (strcasecmp("OFF", value) == 0) {
          b = ESP_NO_NETWORK;
        } else {
          P = -1;  // invalide value
        }
      }
    }
    // STA fallback mode BT, WIFI-AP, OFF
    if (!done) {
      if (strcasecmp("sta_fallback", key) == 0) {
        T = 'B';
        P = ESP_STA_FALLBACK_MODE;
        done = true;
        if (strcasecmp("BT", value) == 0) {
          b = ESP_BT;
        } else if (strcasecmp("WIFI-SETUP", value) == 0) {
          b = ESP_AP_SETUP;
        } else if (strcasecmp("OFF", value) == 0) {
          b = ESP_NO_NETWORK;
        } else {
          P = -1;  // invalide value
        }
      }
    }

    // STA IP Mode DHCP / STATIC
    if (!done) {
      if (strcasecmp("STA_IP_mode", key) == 0) {
        T = 'B';
        P = ESP_STA_IP_MODE;
        done = true;
        if (strcasecmp("DHCP", value) == 0) {
          b = DHCP_MODE;
        } else if (strcasecmp("STATIC", key) == 0) {
          b = STATIC_IP_MODE;
        } else {
          P = -1;  // invalide value
        }
      }
    }
  } else if (strcasecmp("services", section) == 0) {
    if (!done) {
      done = processString(ServstringKeysVal, ServstringKeysPos,
                           sizeof(ServstringKeysVal) / sizeof(char*), key,
                           value, T, P);
    }
    if (!done) {
      done = processInt(ServintKeysVal, ServintKeysPos,
                        sizeof(ServintKeysVal) / sizeof(char*), key, value, T,
                        P, v);
    }
    if (!done) {
      done = processBool(ServboolKeysVal, ServboolKeysPos,
                         sizeof(ServboolKeysVal) / sizeof(char*), key, value, T,
                         P, b);
    }
    if (!done) {
      done = processInt(ServbyteKeysVal, ServbyteKeysPos,
                        sizeof(ServbyteKeysVal) / sizeof(char*), key, value, T,
                        P, v);
      if (done) {
        T = 'B';
        b = v;
      }
    }
    // Notification type None / PushOver / Line / Email / Telegram / IFTTT / HomeAssistant
    if (!done) {
      if (strcasecmp("NOTIF_TYPE", key) == 0) {
        T = 'B';
        P = ESP_NOTIFICATION_TYPE;
        done = true;
        if (strcasecmp("None", value) == 0) {
          b = ESP_NO_NOTIFICATION;
        } else if (strcasecmp("PushOver", value) == 0) {
          b = ESP_PUSHOVER_NOTIFICATION;
        } else if (strcasecmp("Line", value) == 0) {
          b = ESP_LINE_NOTIFICATION;
        } else if (strcasecmp("Email", value) == 0) {
          b = ESP_EMAIL_NOTIFICATION;
        } else if (strcasecmp("Telegram", value) == 0) {
          b = ESP_TELEGRAM_NOTIFICATION;
        } else if (strcasecmp("IFTTT", value) == 0) {
          b = ESP_IFTTT_NOTIFICATION;
        } else if (strcasecmp("HOMEASSISTANT", value) == 0) {
          b = ESP_HOMEASSISTANT_NOTIFICATION;
        } else {
          P = -1;  // invalide value
        }
      }
    }
    // Sensor type if enabled None / DHT11 / DHT22 / ANALOG / BMP280 / BME280
    if (!done) {
      if (strcasecmp("SENSOR_TYPE", key) == 0) {
        T = 'B';
        P = ESP_SENSOR_TYPE;
        done = true;
        if (strcasecmp("None", value) == 0) {
          b = NO_SENSOR_DEVICE;
        } else if (strcasecmp("DHT11", key) == 0) {
          b = DHT11_DEVICE;
        } else if (strcasecmp("DHT22", key) == 0) {
          b = DHT22_DEVICE;
        } else if (strcasecmp("ANALOG", key) == 0) {
          b = ANALOG_DEVICE;
        } else if (strcasecmp("BMP280", key) == 0) {
          b = BMP280_DEVICE;
        } else if (strcasecmp("BME280", key) == 0) {
          b = BME280_DEVICE;
        } else {
          P = -1;  // invalide value
        }
      }
    }
  } else if (strcasecmp("system", section) == 0) {
    if (!done) {
      done = processInt(SysintKeysVal, SysintKeysPos,
                        sizeof(SysintKeysVal) / sizeof(char*), key, value, T, P,
                        v);
    }
    if (!done) {
      done = processBool(SysboolKeysVal, SysboolKeysPos,
                         sizeof(SysboolKeysVal) / sizeof(char*), key, value, T,
                         P, b);
    }
    // Target Firmware None / Marlin / Repetier / MarlinKimbra / Smoothieware /
    // GRBL
    if (!done) {
      if (strcasecmp("TargetFW", key) == 0) {
        T = 'B';
        P = ESP_TARGET_FW;
        done = true;
        if (strcasecmp("None", value) == 0) {
          b = UNKNOWN_FW;
        } else if (strcasecmp("MARLIN", value) == 0) {
          b = MARLIN;
        } else if (strcasecmp("GRBL", value) == 0) {
          b = GRBL;
        } else if (strcasecmp("REPETIER", value) == 0) {
          b = REPETIER;
        } else if (strcasecmp("SMOOTHIEWARE", value) == 0) {
          b = SMOOTHIEWARE;
        } else {
          P = -1;  // invalide value
        }
      }
    }
  }

  // now we save -handle saving status
  // if setting is not recognized it is not a problem
  // but if save is fail - that is a problem - so report it
  if (P != -1) {
    switch (T) {
      case 'S':
        esp3d_log("Saving setting to ESP3D");
        res = ESP3DSettings::writeString(P, value);
        break;
      case 'B':
      case 'F':
        res = ESP3DSettings::writeByte(P, b);
        break;
      case 'I':
        res = ESP3DSettings::writeUint32(P, v);
        break;
      case 'A':
        res = ESP3DSettings::writeIPString(P, value);
        break;
      default:
        esp3d_log("Unknown flag");
    }
  }
  return res;
}

UpdateService::UpdateService() {}
UpdateService::~UpdateService() {}

bool UpdateService::flash(const char* filename, int type) {
  bool res = false;
  if (ESP_SD::exists(filename)) {
    esp3d_log("Update found");
    bool issucess = false;
    ESP_SDFile sdfile;
    String finalName = filename;
    sdfile = ESP_SD::open(filename);
    if (sdfile) {
      size_t s = sdfile.size();
      size_t rs = 0;
      uint8_t v[1];
      if (Update.begin(s, type)) {
        esp3d_log("Update started");
        while (sdfile.available() && (rs <= (s + 1))) {
          rs++;
          v[0] = sdfile.read();
          Update.write(v, 1);
          ESP3DHal::wait(0);
        }
        if (rs == s) {
          esp3d_log("Update done");
          if (Update.end(true)) {
            esp3d_log("Update success");
            issucess = true;
          }
        } else {
          Update.end();
          esp3d_log_e("Wrong size");
        }
      }
      sdfile.close();
    } else {
      esp3d_log_e("Cannot open file");
    }
    if (issucess) {
      res = true;
      finalName.replace(".bin", ".ok");
    } else {
      finalName.replace(".bin", ".bad");
    }
    if (ESP_SD::exists(finalName.c_str())) {
      String name = filename;
      uint8_t n = 1;
      esp3d_log("Final name already exists, backup existing");
      name.replace("bin", String(n).c_str());
      while (ESP_SD::exists(name.c_str())) {
        n++;
        name.replace("bin", String(n).c_str());
      }
      ESP_SD::rename(finalName.c_str(), name.c_str());
    }
    ESP_SD::rename(filename, finalName.c_str());
  }
  return res;
}

bool UpdateService::begin() {
  bool res = false;
  if (ESP3DSettings::readByte(ESP_SD_CHECK_UPDATE_AT_BOOT) != 0) {
    if (ESP_SD::accessFS()) {
      esp3d_log("Update SD for update requestest");
      if (ESP_SD::getState(true) != ESP_SDCARD_NOT_PRESENT) {
        ESP_SD::setState(ESP_SDCARD_BUSY);
        ESP_ConfigFile updateConfig(CONFIG_FILE, processingFileFunction);
        if (updateConfig.processFile()) {
          esp3d_log("Processing ini file done");
          if (updateConfig.revokeFile()) {
            esp3d_log("Revoking ini file done");
            res = true;
          } else {
            esp3d_log_e("Revoking ini file failed");
          }
        } else {
          esp3d_log_e("Processing ini file failed");
        }
        if (flash(FW_FILE, U_FLASH)) {
          res = true;
        } else {
          if (flash(FS_FILE, U_FS)) {
            res = true;
          }
        }
      }
      ESP_SD::releaseFS();
    }
  } else {
    esp3d_log("No need to check for update");
  }

  return res;
}
void UpdateService::end() {}

void UpdateService::handle() {}

#endif  // SD_UPDATE_FEATURE
