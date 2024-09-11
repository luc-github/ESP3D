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
mDNS_Service esp3d_mDNS;

#define MDNS_SERVICE_NAME "esp3d"
#define MDNS_SERVICE_TYPE "tcp"

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
  _port = port;
  if (WiFi.getMode() == WIFI_AP) {
    return;
  }
  MDNS.addService(MDNS_SERVICE_NAME, MDNS_SERVICE_TYPE, _port);
  MDNS.addServiceTxt(MDNS_SERVICE_NAME, MDNS_SERVICE_TYPE, "firmware",
                     ESP3D_CODE_BASE);
  MDNS.addServiceTxt(MDNS_SERVICE_NAME, MDNS_SERVICE_TYPE, "version",
                     FW_VERSION);
#if defined(HTTP_FEATURE)
  MDNS.addService("http", "tcp", HTTP_Server::port());
#endif  // HTTP_FEATURE
#if defined(FTP_FEATURE)
  MDNS.addService("ftp", "tcp", ftp_server.ctrlport());
#endif  // FTP_FEATURE
#if defined(TELNET_FEATURE)
  MDNS.addService("telnet", "tcp", telnet_server.port());
#endif  // TELNET_FEATURE
#if defined(WEBDAV_FEATURE)
  MDNS.addService("webdav", "tcp", webdav_server.port());
#endif  // WEBDAV_FEATURE
#if defined(WS_DATA_FEATURE)
  MDNS.addService("websocket", "tcp", websocket_data_server.port());
  MDNS.addServiceTxt("websocket", "tcp", "uri", "/");
  MDNS.addServiceTxt("websocket", "tcp", "subprotocol", "arduino");
#endif  // WS_DATA_FEATURE
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
