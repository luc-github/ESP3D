/*
   mDNS.cpp - ESP3D mDNS encapsulation class

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
// #define ESP_LOG_FEATURE LOG_OUTPUT_SERIAL0
#include "../../include/esp3d_config.h"

#ifdef MDNS_FEATURE

#include "../../core/esp3d_commands.h"
#include "../../core/esp3d_settings.h"
#include "mDNS.h"

#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266mDNS.h>
#endif  // ARDUINO_ARCH_ESP8266
#if defined(ARDUINO_ARCH_ESP32)
#include <ESPmDNS.h>
#endif  // ARDUINO_ARCH_ESP32
#if defined(HTTP_FEATURE)
#include "../http/http_server.h"
#endif  // HTTP_FEATURE
#if defined(FTP_FEATURE)
#include "../ftp/FtpServer.h"
#endif  // FTP_FEATURE
#if defined(TELNET_FEATURE)
#include "../telnet/telnet_server.h"
#endif  // TELNET_FEATURE
#if defined(WEBDAV_FEATURE)
#include "../webdav/webdav_server.h"
#endif  // WEBDAV_FEATURE
#if defined(WS_DATA_FEATURE)
#include "../websocket/websocket_server.h"
#endif  // WS_DATA_FEATURE
#include "../../core/esp3d_hal.h"
#include "../../core/esp3d_string.h"
#if defined(FILESYSTEM_FEATURE)
#include "../filesystem/esp_filesystem.h"
#endif  // FILESYSTEM_FEATURE
#if defined(NOTIFICATION_FEATURE)
#include "../notifications/notifications_service.h"
#endif  // NOTIFICATION_FEATURE
#if defined(CAMERA_DEVICE)
#include "../camera/camera.h"
#endif  // CAMERA_DEVICE
#if defined(BLUETOOTH_FEATURE)
#include "../bluetooth/BT_service.h"
#endif  // BLUETOOTH_FEATURE
#if defined(SENSOR_DEVICE)
#include "../sensor/sensor.h"
#endif  // SENSOR_DEVICE
#if defined(TIMESTAMP_FEATURE)
#include "../time/time_service.h"
#endif  // TIMESTAMP_FEATURE
// Helper: addServiceTxt with bool return on ESP32 (const char* overload returns void)
// On ESP8266 addServiceTxt also returns void, so we always return true there.
static bool esp3d_mdns_add_txt(const char* svc, const char* proto,
                                const char* key, const char* val) {
#if defined(ARDUINO_ARCH_ESP32)
  return MDNS.addServiceTxt((char*)svc, (char*)proto, (char*)key, (char*)val);
#else
  MDNS.addServiceTxt(svc, proto, key, val);
  return true;
#endif
}

mDNS_Service esp3d_mDNS;

#define MDNS_SERVICE_NAME "esp3d"
#define MDNS_SERVICE_TYPE "tcp"
#define MDNS_DEVICE_INFO_SERVICE "device-info"

mDNS_Service::mDNS_Service() {
  _started = false;
  _hostname = "";
  _port = 0;
  _currentQueryCount = 0;
  _currentQueryTxtCount = 0;
#if defined(ARDUINO_ARCH_ESP8266)
  _hMDNSServiceQuery = 0;
#endif  // ARDUINO_ARCH_ESP8266
}

bool mDNS_Service::begin(const char* hostname) {
  if (_started) {
    end();
  }
  if (WiFi.getMode() != WIFI_AP) {
    _hostname = hostname;
    _hostname.toLowerCase();
    esp3d_log("Start mdsn for %s", _hostname.c_str());
    if (!MDNS.begin(_hostname.c_str())) {
      esp3d_commands.dispatch("mDNS failed to start",
                              ESP3DClientType::all_clients, no_id,
                              ESP3DMessageType::unique, ESP3DClientType::system,
                              ESP3DAuthenticationLevel::admin);
      _started = false;
    } else {
      String stmp = "mDNS started with '" + _hostname + ".local'";
      if (ESP3DSettings::isVerboseBoot()) {
        esp3d_commands.dispatch(stmp.c_str(), ESP3DClientType::all_clients,
                                no_id, ESP3DMessageType::unique,
                                ESP3DClientType::system,
                                ESP3DAuthenticationLevel::admin);
      }
      _started = true;
    }
  }
  return _started;
}
void mDNS_Service::end() {
  _currentQueryCount = 0;
  _currentQueryTxtCount = 0;
  if (!_started || WiFi.getMode() == WIFI_AP) {
    return;
  }
  _started = false;
#if defined(ARDUINO_ARCH_ESP8266)

  if (_hMDNSServiceQuery) {
    esp3d_log("Remove mdns service for %s", _hostname.c_str());
    if (!MDNS.removeServiceQuery(_hMDNSServiceQuery)) {
      esp3d_log_e("failed");
    }
  }
  _hMDNSServiceQuery = 0;
  esp3d_log("Remove mdns for %s", _hostname.c_str());
  if (!MDNS.removeService(_hostname.c_str(), MDNS_SERVICE_NAME,
                          MDNS_SERVICE_TYPE)) {
    esp3d_log_e("failed");
  }
#if defined(HTTP_FEATURE)
  if (!MDNS.removeService(_hostname.c_str(), "http", "tcp")) {
    esp3d_log_e("failed");
  }
#endif  // HTTP_FEATURE
#if defined(FTP_FEATURE)
  if (!MDNS.removeService(_hostname.c_str(), "ftp", "tcp")) {
    esp3d_log_e("failed");
  }
#endif  // FTP_FEATURE
#if defined(TELNET_FEATURE)
  if (!MDNS.removeService(_hostname.c_str(), "telnet", "tcp")) {
    esp3d_log_e("failed");
  }
#endif  // TELNET_FEATURE
#if defined(WEBDAV_FEATURE)
  if (!MDNS.removeService(_hostname.c_str(), "webdav", "tcp")) {
    esp3d_log_e("failed");
  }
#endif  // WEBDAV_FEATURE
#if defined(WS_DATA_FEATURE)
  if (!MDNS.removeService(_hostname.c_str(), "websocket", "tcp")) {
    esp3d_log_e("failed");
  }
#endif  // WS_DATA_FEATURE
  if (!MDNS.removeService(_hostname.c_str(), MDNS_DEVICE_INFO_SERVICE,
                          MDNS_SERVICE_TYPE)) {
    esp3d_log_e("failed");
  }
#endif  // ARDUINO_ARCH_ESP8266
#if defined(ARDUINO_ARCH_ESP32)
  mdns_service_remove("_" MDNS_SERVICE_NAME, "_" MDNS_SERVICE_TYPE);
#if defined(HTTP_FEATURE)
  mdns_service_remove("_http", "_tcp");
#endif  // HTTP_FEATURE
#if defined(FTP_FEATURE)
  mdns_service_remove("_ftp", "_tcp");
#endif  // FTP_FEATURE
#if defined(TELNET_FEATURE)
  mdns_service_remove("_telnet", "_tcp");
#endif  // TELNET_FEATURE
#if defined(WEBDAV_FEATURE)
  mdns_service_remove("_webdav", "_tcp");
#endif  // WEBDAV_FEATURE
#if defined(WS_DATA_FEATURE)
  mdns_service_remove("_websocket", "_tcp");
#endif  // WS_DATA_FEATURE
  mdns_service_remove("_" MDNS_DEVICE_INFO_SERVICE, "_" MDNS_SERVICE_TYPE);
#endif  // ARDUINO_ARCH_ESP32
  MDNS.end();
  _hostname = "";
  _port = 0;
}
#if defined(ARDUINO_ARCH_ESP8266)
// call back function for mDNS service query
// currently not used, but necessary to setup the service query
void MDNSServiceQueryCallback(MDNSResponder::MDNSServiceInfo serviceInfo,
                              MDNSResponder::AnswerType answerType,
                              bool p_bSetContent) {}
#endif  // ARDUINO_ARCH_ESP8266
void mDNS_Service::addESP3DServices(uint16_t port) {
  // Port 0 means auto-detect: first active service port will be used.
  // Note: the ESP Arduino mDNS stack rejects port 0 at service registration
  // (addService returns false), even though DNS-SD standard allows it.
  _port = port;
  if (WiFi.getMode() == WIFI_AP) {
    return;
  }
  // Register individual services first, capturing the first available port
#if defined(HTTP_FEATURE)
  MDNS.addService("http", "tcp", HTTP_Server::port());
  if (_port == 0) _port = HTTP_Server::port();
#endif  // HTTP_FEATURE
#if defined(FTP_FEATURE)
  MDNS.addService("ftp", "tcp", ftp_server.ctrlport());
  if (_port == 0) _port = ftp_server.ctrlport();
#endif  // FTP_FEATURE
#if defined(TELNET_FEATURE)
  MDNS.addService("telnet", "tcp", telnet_server.port());
  if (_port == 0) _port = telnet_server.port();
#endif  // TELNET_FEATURE
#if defined(WEBDAV_FEATURE)
  MDNS.addService("webdav", "tcp", webdav_server.port());
  if (_port == 0) _port = webdav_server.port();
#endif  // WEBDAV_FEATURE
#if defined(WS_DATA_FEATURE)
  MDNS.addService("websocket", "tcp", websocket_data_server.port());
  if (!esp3d_mdns_add_txt("websocket", "tcp", "uri", "/")) {
    esp3d_log_e("Failed to add TXT websocket/uri");
  }
  if (!esp3d_mdns_add_txt("websocket", "tcp", "subprotocol", "arduino")) {
    esp3d_log_e("Failed to add TXT websocket/subprotocol");
  }
  if (_port == 0) _port = websocket_data_server.port();
#endif  // WS_DATA_FEATURE
  // No active service found - register esp3d and device-info with 8080
  if (_port == 0) {
    _port = 8080; 
    esp3d_log("No active service port, using 8080 for esp3d and device-info registration");
  }
  // Main esp3d service (port = first active service port)
  MDNS.addService(MDNS_SERVICE_NAME, MDNS_SERVICE_TYPE, _port);
  if (!esp3d_mdns_add_txt(MDNS_SERVICE_NAME, MDNS_SERVICE_TYPE, "firmware",
                          ESP3D_CODE_BASE)) {
    esp3d_log_e("Failed to add TXT firmware");
  }
  if (!esp3d_mdns_add_txt(MDNS_SERVICE_NAME, MDNS_SERVICE_TYPE, "version",
                          FW_VERSION)) {
    esp3d_log_e("Failed to add TXT version");
  }
// Device info service - ESP8266 version: minimal records only to avoid OOM.
// The mDNS parser needs heap to process incoming packets; too many TXT records
// exhaust it and cause crashes.
#if defined(ARDUINO_ARCH_ESP8266)
  if (MDNS.addService(MDNS_DEVICE_INFO_SERVICE, MDNS_SERVICE_TYPE, _port)) {
    esp3d_mdns_add_txt(MDNS_DEVICE_INFO_SERVICE, MDNS_SERVICE_TYPE, "board",
                       ESP3DSettings::TargetBoard());
  } else {
    esp3d_log_e("Failed to add service " MDNS_DEVICE_INFO_SERVICE);
  }
#else
  // Device info service - device identification and capabilities
  if (!MDNS.addService(MDNS_DEVICE_INFO_SERVICE, MDNS_SERVICE_TYPE, _port)) {
    esp3d_log_e("Failed to add service " MDNS_DEVICE_INFO_SERVICE);
    return;
  }
  if (!esp3d_mdns_add_txt(MDNS_DEVICE_INFO_SERVICE, MDNS_SERVICE_TYPE, "name",
                          _hostname.c_str())) {
    esp3d_log_e("Failed to add TXT name");
  }
  if (!esp3d_mdns_add_txt(MDNS_DEVICE_INFO_SERVICE, MDNS_SERVICE_TYPE, "board",
                          ESP3DSettings::TargetBoard())) {
    esp3d_log_e("Failed to add TXT board");
  }
#if defined(ARDUINO_ARCH_ESP32)
  if (!esp3d_mdns_add_txt(MDNS_DEVICE_INFO_SERVICE, MDNS_SERVICE_TYPE, "chip",
                          ESP.getChipModel())) {
    esp3d_log_e("Failed to add TXT chip");
  }
#endif  // ARDUINO_ARCH_ESP32
  if (!esp3d_mdns_add_txt(MDNS_DEVICE_INFO_SERVICE, MDNS_SERVICE_TYPE, "fw",
                          ESP3D_CODE_BASE)) {
    esp3d_log_e("Failed to add TXT fw");
  }
  if (!esp3d_mdns_add_txt(MDNS_DEVICE_INFO_SERVICE, MDNS_SERVICE_TYPE, "ver",
                          FW_VERSION)) {
    esp3d_log_e("Failed to add TXT ver");
  }
  if (!esp3d_mdns_add_txt(MDNS_DEVICE_INFO_SERVICE, MDNS_SERVICE_TYPE,
                          "target",
                          ESP3DSettings::GetFirmwareTargetShortName())) {
    esp3d_log_e("Failed to add TXT target");
  }
  if (!esp3d_mdns_add_txt(MDNS_DEVICE_INFO_SERVICE, MDNS_SERVICE_TYPE, "sdk",
                          ESP.getSdkVersion())) {
    esp3d_log_e("Failed to add TXT sdk");
  }
  if (!esp3d_mdns_add_txt(MDNS_DEVICE_INFO_SERVICE, MDNS_SERVICE_TYPE, "core",
                          ESP3DHal::arduinoVersion())) {
    esp3d_log_e("Failed to add TXT core");
  }
  {
    String s = esp3d_string::formatBytes(ESP.getFlashChipSize());
    if (!esp3d_mdns_add_txt(MDNS_DEVICE_INFO_SERVICE, MDNS_SERVICE_TYPE,
                            "flash", s.c_str())) {
      esp3d_log_e("Failed to add TXT flash");
    }
  }
#if defined(ARDUINO_ARCH_ESP32) && defined(BOARD_HAS_PSRAM)
  {
    String s = esp3d_string::formatBytes(ESP.getPsramSize());
    if (!esp3d_mdns_add_txt(MDNS_DEVICE_INFO_SERVICE, MDNS_SERVICE_TYPE,
                            "psram", s.c_str())) {
      esp3d_log_e("Failed to add TXT psram");
    }
  }
#endif  // ARDUINO_ARCH_ESP32 && BOARD_HAS_PSRAM
#if defined(FILESYSTEM_FEATURE)
  {
    String s = esp3d_string::formatBytes(ESP_FileSystem::totalBytes());
    if (!esp3d_mdns_add_txt(MDNS_DEVICE_INFO_SERVICE, MDNS_SERVICE_TYPE, "fs",
                            s.c_str())) {
      esp3d_log_e("Failed to add TXT fs");
    }
  }
#endif  // FILESYSTEM_FEATURE
#if defined(NOTIFICATION_FEATURE)
  if (!esp3d_mdns_add_txt(MDNS_DEVICE_INFO_SERVICE, MDNS_SERVICE_TYPE,
                          "notification",
                          notificationsservice.getTypeString())) {
    esp3d_log_e("Failed to add TXT notification");
  }
#endif  // NOTIFICATION_FEATURE
#if defined(CAMERA_DEVICE)
  if (!esp3d_mdns_add_txt(MDNS_DEVICE_INFO_SERVICE, MDNS_SERVICE_TYPE,
                          "camera", esp3d_camera.GetModelString())) {
    esp3d_log_e("Failed to add TXT camera");
  }
#endif  // CAMERA_DEVICE
#if defined(BLUETOOTH_FEATURE)
  {
    String btinfo = bt_service.hostname();
    btinfo += "/";
    btinfo += BTService::macAddress();
    if (!esp3d_mdns_add_txt(MDNS_DEVICE_INFO_SERVICE, MDNS_SERVICE_TYPE, "bt",
                            btinfo.c_str())) {
      esp3d_log_e("Failed to add TXT bt");
    }
  }
#endif  // BLUETOOTH_FEATURE
#if defined(SENSOR_DEVICE)
  if (!esp3d_mdns_add_txt(MDNS_DEVICE_INFO_SERVICE, MDNS_SERVICE_TYPE,
                          "sensor", esp3d_sensor.GetCurrentModelString())) {
    esp3d_log_e("Failed to add TXT sensor");
  }
#endif  // SENSOR_DEVICE
#if defined(SD_UPDATE_FEATURE)
  if (!esp3d_mdns_add_txt(
          MDNS_DEVICE_INFO_SERVICE, MDNS_SERVICE_TYPE, "sd-update",
          ESP3DSettings::readByte(ESP_SD_CHECK_UPDATE_AT_BOOT) != 0 ? "ON"
                                                                     : "OFF")) {
    esp3d_log_e("Failed to add TXT sd-update");
  }
#endif  // SD_UPDATE_FEATURE
#if defined(WEB_UPDATE_FEATURE)
  if (!esp3d_mdns_add_txt(MDNS_DEVICE_INFO_SERVICE, MDNS_SERVICE_TYPE,
                          "web-update", "enabled")) {
    esp3d_log_e("Failed to add TXT web-update");
  }
#endif  // WEB_UPDATE_FEATURE
#if defined(TIMESTAMP_FEATURE)
  if (!esp3d_mdns_add_txt(MDNS_DEVICE_INFO_SERVICE, MDNS_SERVICE_TYPE, "time",
                          timeService.isInternetTime() ? "ntp" : "manual")) {
    esp3d_log_e("Failed to add TXT time");
  }
#endif  // TIMESTAMP_FEATURE
#if defined(SD_DEVICE)
#if SD_DEVICE == ESP_SDIO
  if (!esp3d_mdns_add_txt(MDNS_DEVICE_INFO_SERVICE, MDNS_SERVICE_TYPE, "sd",
                          "SDIO")) {
    esp3d_log_e("Failed to add TXT sd");
  }
#elif SD_DEVICE == ESP_SDFAT2
  if (!esp3d_mdns_add_txt(MDNS_DEVICE_INFO_SERVICE, MDNS_SERVICE_TYPE, "sd",
                          "SPI-SdFat")) {
    esp3d_log_e("Failed to add TXT sd");
  }
#else
  if (!esp3d_mdns_add_txt(MDNS_DEVICE_INFO_SERVICE, MDNS_SERVICE_TYPE, "sd",
                          "SPI")) {
    esp3d_log_e("Failed to add TXT sd");
  }
#endif
#endif  // SD_DEVICE
#if defined(LUA_INTERPRETER_FEATURE)
  if (!esp3d_mdns_add_txt(MDNS_DEVICE_INFO_SERVICE, MDNS_SERVICE_TYPE, "lua",
                          "enabled")) {
    esp3d_log_e("Failed to add TXT lua");
  }
#endif  // LUA_INTERPRETER_FEATURE
#if defined(USB_SERIAL_FEATURE)
  if (!esp3d_mdns_add_txt(MDNS_DEVICE_INFO_SERVICE, MDNS_SERVICE_TYPE, "usb",
                          "enabled")) {
    esp3d_log_e("Failed to add TXT usb");
  }
#endif  // USB_SERIAL_FEATURE
#if defined(AUTHENTICATION_FEATURE)
  if (!esp3d_mdns_add_txt(MDNS_DEVICE_INFO_SERVICE, MDNS_SERVICE_TYPE, "auth",
                          "enabled")) {
    esp3d_log_e("Failed to add TXT auth");
  }
#else
  if (!esp3d_mdns_add_txt(MDNS_DEVICE_INFO_SERVICE, MDNS_SERVICE_TYPE, "auth",
                          "disabled")) {
    esp3d_log_e("Failed to add TXT auth");
  }
#endif  // AUTHENTICATION_FEATURE
#if defined(SSDP_FEATURE)
  if (!esp3d_mdns_add_txt(MDNS_DEVICE_INFO_SERVICE, MDNS_SERVICE_TYPE, "ssdp",
                          "enabled")) {
    esp3d_log_e("Failed to add TXT ssdp");
  }
#endif  // SSDP_FEATURE
  {
    String loginfo = "";
#if defined(ESP_LOG_FEATURE)
#if ESP_LOG_FEATURE == LOG_OUTPUT_SERIAL0
    loginfo = "serial0/";
#elif ESP_LOG_FEATURE == LOG_OUTPUT_SERIAL1
    loginfo = "serial1/";
#elif ESP_LOG_FEATURE == LOG_OUTPUT_SERIAL2
    loginfo = "serial2/";
#elif ESP_LOG_FEATURE == LOG_OUTPUT_TELNET
    loginfo = "telnet/";
#elif ESP_LOG_FEATURE == LOG_OUTPUT_WEBSOCKET
    loginfo = "websocket/";
#else
    loginfo = "unknown/";
#endif
#if ESP3D_LOG_LEVEL == LOG_LEVEL_VERBOSE
    loginfo += "verbose";
#elif ESP3D_LOG_LEVEL == LOG_LEVEL_DEBUG
    loginfo += "debug";
#elif ESP3D_LOG_LEVEL == LOG_LEVEL_ERROR
    loginfo += "error";
#else
    loginfo += "none";
#endif
#else
    loginfo = "none";
#endif  // ESP_LOG_FEATURE
    if (!esp3d_mdns_add_txt(MDNS_DEVICE_INFO_SERVICE, MDNS_SERVICE_TYPE, "log",
                            loginfo.c_str())) {
      esp3d_log_e("Failed to add TXT log");
    }
  }
#endif  // ARDUINO_ARCH_ESP8266 / else
#if defined(ARDUINO_ARCH_ESP8266)
  _hMDNSServiceQuery = MDNS.installServiceQuery(
      MDNS_SERVICE_NAME, MDNS_SERVICE_TYPE, MDNSServiceQueryCallback);
  if (_hMDNSServiceQuery) {
    esp3d_log("MDNS Service query services installed.");
  } else {
    esp3d_log_e("MDNS Service query services installation failed.");
  }
#endif  // ARDUINO_ARCH_ESP8266
}

void mDNS_Service::handle() {
#if defined(ARDUINO_ARCH_ESP8266)
  if (WiFi.getMode() == WIFI_AP) {
    return;
  }
  MDNS.update();
#endif  // ARDUINO_ARCH_ESP8266
}

uint16_t mDNS_Service::servicesCount() {
  _currentQueryCount = 0;
  if (WiFi.getMode() == WIFI_AP) {
    return _currentQueryCount;
  }
#if defined(ARDUINO_ARCH_ESP32)
  _currentQueryCount = MDNS.queryService(MDNS_SERVICE_NAME, MDNS_SERVICE_TYPE);
#endif  // ARDUINO_ARCH_ESP32
#if defined(ARDUINO_ARCH_ESP8266)
  if (_hMDNSServiceQuery) {
    _currentQueryCount = MDNS.answerCount(_hMDNSServiceQuery);
  }
#endif  // ARDUINO_ARCH_ESP8266
  return _currentQueryCount;
}

const char* mDNS_Service::answerHostname(uint16_t index) {
  static String tmp;
  if (WiFi.getMode() == WIFI_AP || _currentQueryCount == 0 ||
      index >= _currentQueryCount) {
    return "";
  }
#if defined(ARDUINO_ARCH_ESP32)
  tmp = MDNS.hostname(index);

#endif  // ARDUINO_ARCH_ESP32
#if defined(ARDUINO_ARCH_ESP8266)
  tmp = MDNS.answerHostDomain(_hMDNSServiceQuery, index);
#endif  // ARDUINO_ARCH_ESP8266
  return tmp.c_str();
}

const char* mDNS_Service::answerIP(uint16_t index) {
  static String tmp;
  if (WiFi.getMode() == WIFI_AP || _currentQueryCount == 0 ||
      index >= _currentQueryCount) {
    return "";
  }
#if defined(ARDUINO_ARCH_ESP32)
#if ESP_ARDUINO_VERSION_MAJOR == 3
  tmp = MDNS.address(index).toString();
#endif  // ESP_ARDUINO_VERSION_MAJOR == 3
#if ESP_ARDUINO_VERSION_MAJOR == 2
  tmp = MDNS.IP(index).toString();
#endif  // ESP_ARDUINO_VERSION_MAJOR == 2

#endif  // ARDUINO_ARCH_ESP32
#if defined(ARDUINO_ARCH_ESP8266)
  tmp = MDNS.answerIP4Address(_hMDNSServiceQuery, index, 0).toString();
#endif  // ARDUINO_ARCH_ESP8266
  return tmp.c_str();
}

uint16_t mDNS_Service::answerPort(uint16_t index) {
  if (WiFi.getMode() == WIFI_AP || _currentQueryCount == 0 ||
      index >= _currentQueryCount) {
    return 0;
  }
#if defined(ARDUINO_ARCH_ESP32)
  return MDNS.port(index);
#endif  // ARDUINO_ARCH_ESP32
#if defined(ARDUINO_ARCH_ESP8266)
  return MDNS.answerPort(_hMDNSServiceQuery, index);
#endif  // ARDUINO_ARCH_ESP8266
}

uint16_t mDNS_Service::answerTxtCount(uint16_t index) {
  _currentQueryTxtCount = 0;
  if (WiFi.getMode() == WIFI_AP || _currentQueryCount == 0 ||
      index >= _currentQueryCount) {
    return _currentQueryTxtCount;
  }
#if defined(ARDUINO_ARCH_ESP32)
  _currentQueryTxtCount = MDNS.numTxt(index);
  return _currentQueryTxtCount;
#endif  // ARDUINO_ARCH_ESP32
#if defined(ARDUINO_ARCH_ESP8266)
  if (!_hMDNSServiceQuery || !MDNS.hasAnswerTxts(_hMDNSServiceQuery, index)) {
    return _currentQueryTxtCount;
  }
  String txt = MDNS.answerTxts(_hMDNSServiceQuery, index);
  for (uint i = 0; i < txt.length(); i++) {
    if (txt[i] == ';') {
      _currentQueryTxtCount++;
    }
  }
  // there are n+1 number of ';'
  _currentQueryTxtCount++;
  return _currentQueryTxtCount;
#endif  // ARDUINO_ARCH_ESP8266
}

const char* mDNS_Service::answerTxtKey(uint16_t index, uint16_t txtIndex) {
  static String tmp;
  if (WiFi.getMode() == WIFI_AP || _currentQueryCount == 0 ||
      index >= _currentQueryCount || txtIndex >= _currentQueryTxtCount ||
      _currentQueryTxtCount == 0) {
    return "";
  }
#if defined(ARDUINO_ARCH_ESP32)
  tmp = MDNS.txtKey(index, txtIndex);
  return tmp.c_str();
#endif  // ARDUINO_ARCH_ESP32
#if defined(ARDUINO_ARCH_ESP8266)
  String txt = MDNS.answerTxts(_hMDNSServiceQuery, index);
  esp3d_log("txt: %s", txt.c_str());
  String keyValue = "";
  bool found = false;
  if (txt.indexOf(";") == -1) {
    keyValue = txt;
  } else {
    uint currentIndex = 0;
    uint pos = 0;
    while (!found) {
      int posend = txt.indexOf(";", pos);
      if (posend == -1) {
        posend = txt.length();
      }
      keyValue = txt.substring(pos, posend);
      if (currentIndex == txtIndex) {
        found = true;
      } else {
        pos = posend + 1;
        currentIndex++;
      }
    }
  }
  for (uint p = 0; p < keyValue.length(); p++) {
    if (keyValue[p] == '=') {
      tmp = keyValue.substring(0, p);
    }
  }
  return tmp.c_str();
#endif  // ARDUINO_ARCH_ESP8266
}

const char* mDNS_Service::answerTxt(uint16_t index, uint16_t txtIndex) {
  static String tmp;
  if (WiFi.getMode() == WIFI_AP || _currentQueryCount == 0 ||
      index >= _currentQueryCount || txtIndex >= _currentQueryTxtCount ||
      _currentQueryTxtCount == 0) {
    return "";
  }
#if defined(ARDUINO_ARCH_ESP32)
  tmp = MDNS.txt(index, txtIndex);
  return tmp.c_str();
#endif  // ARDUINO_ARCH_ESP32
#if defined(ARDUINO_ARCH_ESP8266)
  String txt = MDNS.answerTxts(_hMDNSServiceQuery, index);
  esp3d_log("txt: %s", txt.c_str());
  String keyValue = "";
  bool found = false;
  if (txt.indexOf(";") == -1) {
    keyValue = txt;
  } else {
    uint currentIndex = 0;
    uint pos = 0;
    while (!found) {
      int posend = txt.indexOf(";", pos);
      if (posend == -1) {
        posend = txt.length();
      }
      keyValue = txt.substring(pos, posend);
      if (currentIndex == txtIndex) {
        found = true;
      } else {
        pos = posend + 1;
        currentIndex++;
      }
    }
  }
  for (uint p = 0; p < keyValue.length(); p++) {
    if (keyValue[p] == '=') {
      tmp = keyValue.substring(p + 1, keyValue.length());
    }
  }
  return tmp.c_str();
#endif  // ARDUINO_ARCH_ESP8266
}

mDNS_Service::~mDNS_Service() { end(); }

#endif  // MDNS_FEATURE
