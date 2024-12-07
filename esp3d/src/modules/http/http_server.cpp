/*
  http_server.cpp -  http server functions class

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

#if defined(HTTP_FEATURE)
#if defined(ARDUINO_ARCH_ESP32)
#include <WebServer.h>
#define DOWNLOAD_PACKET_SIZE 2048
#endif  // ARDUINO_ARCH_ESP32
#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WebServer.h>
#define DOWNLOAD_PACKET_SIZE 1024
#endif  // ARDUINO_ARCH_ESP8266
#include "../../core/esp3d_settings.h"
#include "../authentication/authentication_service.h"
#include "../filesystem/esp_filesystem.h"
#include "../network/netconfig.h"
#include "../websocket/websocket_server.h"
#include "http_server.h"

#if defined(SD_DEVICE)
#include "../filesystem/esp_sd.h"

#endif  // SD_DEVICE
#ifdef ESP_BENCHMARK_FEATURE
#include "../../core/esp3d_benchmark.h"
#endif  // ESP_BENCHMARK_FEATURE

ESP3DRequest code_200{.code = 200};
ESP3DRequest code_500{.code = 500};
ESP3DRequest code_404{.code = 404};
ESP3DRequest code_401{.code = 401};

bool HTTP_Server::_started = false;
uint16_t HTTP_Server::_port = 0;
WEBSERVER* HTTP_Server::_webserver = nullptr;
uint8_t HTTP_Server::_upload_status = UPLOAD_STATUS_NONE;

void HTTP_Server::init_handlers() {
  _webserver->on("/", HTTP_ANY, handle_root);
  // Page not found handler
  _webserver->onNotFound(handle_not_found);
  // web commands
  _webserver->on("/command", HTTP_ANY, handle_web_command);
  // config
  _webserver->on("/config", HTTP_ANY, handle_config);
  // need to be there even no authentication to say to UI no authentication
  _webserver->on("/login", HTTP_ANY, handle_login);
#ifdef FILESYSTEM_FEATURE
  _webserver->on("/files", HTTP_ANY, handleFSFileList, FSFileupload);
#endif  // FILESYSTEM_FEATURE
#if COMMUNICATION_PROTOCOL == MKS_SERIAL
  // MKS_SERIAL
  _webserver->on("/upload", HTTP_ANY, handleMKSUpload, MKSFileupload);
#endif  // COMMUNICATION_PROTOCOL == MKS_SERIAL
#ifdef SD_DEVICE
  // SD
  _webserver->on("/sdfiles", HTTP_ANY, handleSDFileList, SDFileupload);
#endif  // SD_DEVICE
#ifdef WEB_UPDATE_FEATURE
  // web update
  _webserver->on("/updatefw", HTTP_ANY, handleUpdate, WebUpdateUpload);
#endif  // WEB_UPDATE_FEATURE
#ifdef CAMERA_DEVICE
  _webserver->on("/snap", HTTP_GET, handle_snap);
#endif  // CAMERA_DEVICE
#ifdef SSDP_FEATURE
  if (WiFi.getMode() != WIFI_AP) {
    _webserver->on("/description.xml", HTTP_GET, handle_SSDP);
  }
#endif  // SSDP_FEATURE
#ifdef CAPTIVE_PORTAL_FEATURE
  if (NetConfig::getMode() == ESP_AP_SETUP) {
    _webserver->on("/generate_204", HTTP_ANY, handle_root);
    _webserver->on("/gconnectivitycheck.gstatic.com", HTTP_ANY, handle_root);
    // do not forget the / at the end
    _webserver->on("/fwlink/", HTTP_ANY, handle_root);
  }
#endif  // CAPTIVE_PORTAL_FEATURE
}

bool HTTP_Server::StreamFSFile(const char* filename, const char* contentType) {
  ESP_File datafile = ESP_FileSystem::open(filename);
  uint64_t last_WS_update = millis();
  if (!datafile) {
    return false;
  }
  size_t totalFileSize = datafile.size();
  size_t i = 0;
  bool done = false;
  _webserver->setContentLength(totalFileSize);
  _webserver->send(200, contentType, "");
  uint8_t buf[DOWNLOAD_PACKET_SIZE];
  while (!done && _webserver->client().connected()) {
    ESP3DHal::wait(0);
    int v = datafile.read(buf, DOWNLOAD_PACKET_SIZE);
    if ((v == -1) || (v == 0)) {
      done = true;
    } else {
      _webserver->client().write(buf, v);
      i += v;
    }
    if (i >= totalFileSize) {
      done = true;
    }
    // update websocket every 2000 ms
    if (millis() - last_WS_update > 2000) {
      websocket_terminal_server.handle();
      last_WS_update = millis();
    }
  }
  datafile.close();
  if (i != totalFileSize) {
    return false;
  }
  return true;
}

#if defined(SD_DEVICE)
bool HTTP_Server::StreamSDFile(const char* filename, const char* contentType) {
  ESP_SDFile datafile = ESP_SD::open(filename);
  uint64_t last_WS_update = millis();
#ifdef ESP_BENCHMARK_FEATURE
  uint64_t bench_start = millis();
  size_t bench_transfered = 0;
#endif  // ESP_BENCHMARK_FEATURE
  if (!datafile) {
    return false;
  }
  size_t totalFileSize = datafile.size();
  size_t i = 0;
  bool done = false;
  _webserver->setContentLength(totalFileSize);
  _webserver->send(200, contentType, "");
  uint8_t buf[DOWNLOAD_PACKET_SIZE];
  while (!done && _webserver->client().connected()) {
    ESP3DHal::wait(0);
    int v = datafile.read(buf, DOWNLOAD_PACKET_SIZE);
    if ((v == -1) || (v == 0)) {
      done = true;
    } else {
      _webserver->client().write(buf, v);
      i += v;
#ifdef ESP_BENCHMARK_FEATURE
      bench_transfered += v;
#endif  // ESP_BENCHMARK_FEATURE
    }
    if (i >= totalFileSize) {
      done = true;
    }

    // update websocket every 2000 ms
    if (millis() - last_WS_update > 2000) {
      websocket_terminal_server.handle();
      last_WS_update = millis();
    }
  }
  datafile.close();
  if (i != totalFileSize) {
    return false;
  }
#ifdef ESP_BENCHMARK_FEATURE
  benchMark("SD download", bench_start, millis(), bench_transfered);
#endif  // ESP_BENCHMARK_FEATURE
  return true;
}
#endif  // SD_DEVICE

void HTTP_Server::pushError(int code, const char* st, uint16_t web_error,
                            uint16_t timeout) {
  esp3d_log("%s:%d", st, web_error);
  if (websocket_terminal_server.started() && st) {
    String s = "ERROR:" + String(code) + ":";
    s += st;
    websocket_terminal_server.pushMSG(websocket_terminal_server.get_currentID(),
                                      s.c_str());
    if (web_error != 0) {
      if (_webserver) {
        if (_webserver->client().available() > 0) {
          _webserver->send(web_error, "text/xml", st);
        }
      }
    }
    uint32_t t = millis();
    while (millis() - t < timeout) {
      websocket_terminal_server.handle();
      ESP3DHal::wait(10);
    }
  }
}

void HTTP_Server::cancelUpload() {
  HTTPUpload& upload = _webserver->upload();
  upload.status = UPLOAD_FILE_ABORTED;
#if defined(ARDUINO_ARCH_ESP8266)
  _webserver->client().stopAll();
#else
  errno = ECONNABORTED;
  _webserver->client().stop();
#endif
  ESP3DHal::wait(100);
}

bool HTTP_Server::begin() {
  bool no_error = true;
  end();
  if (ESP3DSettings::readByte(ESP_HTTP_ON) != 1) {
    return no_error;
  }
  _port = ESP3DSettings::readUint32(ESP_HTTP_PORT);
  _webserver = new WEBSERVER(_port);
  if (!_webserver) {
    return false;
  }

  init_handlers();
  // here the list of headers to be recorded
  // Autorization is already added
  // ask server to track these headers
#ifdef AUTHENTICATION_FEATURE
  const char* headerkeys[] = {"Cookie", "Content-Length"};
#else
  const char* headerkeys[] = {"Content-Length"};
#endif
  size_t headerkeyssize = sizeof(headerkeys) / sizeof(char*);
  _webserver->collectHeaders(headerkeys, headerkeyssize);
  _webserver->begin();
#ifdef AUTHENTICATION_FEATURE
  AuthenticationService::begin(_webserver);
#endif  // AUTHENTICATION_FEATURE

  _started = no_error;
  return no_error;
}

bool HTTP_Server::dispatch(ESP3DMessage* msg) {
  if (!msg || !_started || !_webserver) {
    return false;
  }
  if ((msg->size > 0 && msg->data) || (msg->type == ESP3DMessageType::tail)) {
    if (msg->type == ESP3DMessageType::head ||
        msg->type == ESP3DMessageType::unique ||
        msg->type == ESP3DMessageType::realtimecmd) {
      set_http_headers();
      int code = 200;
      if (msg->request_id.code != 0) {
        code = msg->request_id.code;
      }
#if defined(AUTHENTICATION_FEATURE)
      if (msg->authentication_level ==
          ESP3DAuthenticationLevel::not_authenticated) {
        code = 401;
      }
#endif  // AUTHENTICATION_FEATURE
      esp3d_log("Code is %d", code);
      if (msg->type == ESP3DMessageType::head) {
        _webserver->setContentLength(CONTENT_LENGTH_UNKNOWN);
        // in case of no request id set

        _webserver->send(code);
        _webserver->sendContent((const char*)msg->data, msg->size);
      } else {  // unique
        _webserver->send(code, "text/plain", (char*)msg->data);
      }
    } else {
      // core or tail
      if (msg->data && msg->size > 0) {
        _webserver->sendContent((const char*)msg->data, msg->size);
      }
      // this is tail so we send end of data
      if (msg->type == ESP3DMessageType::tail) {
        _webserver->sendContent("");
      }
    }
    esp3d_message_manager.deleteMsg(msg);
    return true;
  }
  return false;
}

void HTTP_Server::set_http_headers() {
  /*
  User-Agent: ESP3D-WebServer/1.0 (ESP8266; Firmware/3.0.0; Platform/arduino;
Embedded; http://www.esp3d.io) Host: http://192.168.0.1

User-Agent: ESP3D-WebdavServer/1.0 (ESP8266; Firmware/3.0.0; Platform/arduino;
Embedded; http://www.esp3d.io) Host: http://192.168.0.1:8181
  */
  static String ua = "";
  static String host = "";
  if (ua.length() == 0) {
    ua = "ESP3D-WebServer/1.0 (";
    ua += ESP3DSettings::TargetBoard();
    ua += "; Firmware/";
    ua += FW_VERSION;
    ua += "; Platform/arduino; Embedded; http://www.esp3d.io)";
  }
  if (host.length() == 0) {
    host = "Host: http://";
    host += NetConfig::localIP();
    if (_port != 80) {
      host += ":";
      host += String(_port);
    }
  }
  if (_webserver) {
    _webserver->sendHeader("User-Agent", ua.c_str());
    _webserver->sendHeader("Host", host.c_str());
    _webserver->sendHeader("Cache-Control", "no-cache");
#ifdef ESP_ACCESS_CONTROL_ALLOW_ORIGIN
    _webserver->sendHeader("Access-Control-Allow-Origin", "*");
#endif  // ESP_ACCESS_CONTROL_ALLOw_ORIGIN
  }
}

void HTTP_Server::end() {
  _started = false;
  _upload_status = UPLOAD_STATUS_NONE;
#ifdef AUTHENTICATION_FEATURE
  AuthenticationService::end();
#endif  // AUTHENTICATION_FEATURE
  if (_webserver) {
    _webserver->stop();
    delete _webserver;
    _webserver = NULL;
  }
}

void HTTP_Server::handle() {
  if (_started) {
    if (_webserver) {
      _webserver->handleClient();
    }
  }
}

const char* HTTP_Server::get_Splited_Value(String data, char separator,
                                           int index) {
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length() - 1;
  static String s;
  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  if (found > index) {
    s = data.substring(strIndex[0], strIndex[1]).c_str();
  } else {
    s = "";
  }
  return s.c_str();
}

#endif  // Enable HTTP
