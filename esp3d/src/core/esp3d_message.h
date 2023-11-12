/*
  esp3d_message.h -  output functions class

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

#define ESP_NO_CLIENT 0
#define ESP_SERIAL_CLIENT 1
#define ESP_TELNET_CLIENT 2
#define ESP_HTTP_CLIENT 4
#define ESP_WEBSOCKET_TERMINAL_CLIENT 8
#define ESP_REMOTE_SCREEN_CLIENT 16
#define ESP_STREAM_HOST_CLIENT 30
#define ESP_BT_CLIENT 32
#define ESP_SCREEN_CLIENT 64
#define ESP_WEBSOCKET_CLIENT 128
#define ESP_SOCKET_SERIAL_CLIENT 129
#define ESP_ECHO_SERIAL_CLIENT 130
#define ESP_SERIAL_BRIDGE_CLIENT 150
#define ESP_ALL_CLIENTS 255

#define ESP_OUTPUT_IP_ADDRESS 0
#define ESP_OUTPUT_STATUS 1
#define ESP_OUTPUT_PROGRESS 2
#define ESP_OUTPUT_STATE 3

#define ESP_STATE_DISCONNECTED 0

#ifndef _ESP3DOUTPUT_H
#define _ESP3DOUTPUT_H

#include "../include/esp3d_config.h"
#include "Print.h"

#ifdef HTTP_FEATURE
#if defined(ARDUINO_ARCH_ESP32)
class WebServer;
#define WEBSERVER WebServer
#endif  // ARDUINO_ARCH_ESP32
#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WebServer.h>
#define WEBSERVER ESP8266WebServer
#endif  // ARDUINO_ARCH_ESP8266
#endif  // HTTP_FEATURE

#include "../modules/authentication/authentication_level_types.h"
#include "esp3d_client_types.h"

enum class ESP3DMessageType : uint8_t { head, core, tail, unique };

union ESP3DRequest {
  uint id;
#ifdef HTTP_FEATURE
  WEBSERVER *http_request;
#endif  // HTTP_FEATURE
};

extern ESP3DRequest no_id;

struct ESP3DMessage {
  uint8_t *data;
  size_t size;
  ESP3DClientType origin;
  ESP3DClientType target;
  ESP3DAuthenticationLevel authentication_level;
  ESP3DRequest request_id;
  ESP3DMessageType type;
};

class ESP3DMessageManager final {
 public:
  static ESP3DMessage *newMsg();
  static ESP3DMessage *newMsg(ESP3DRequest requestId);
  static bool deleteMsg(ESP3DMessage *message);
  static bool copyMsgInfos(ESP3DMessage *newMsgPtr, ESP3DMessage msg);
  static ESP3DMessage *copyMsgInfos(ESP3DMessage msg);
  static ESP3DMessage *copyMsg(ESP3DMessage msg);
  static ESP3DMessage *newMsg(ESP3DClientType origin, ESP3DClientType target,
                              const uint8_t *data, size_t length,
                              ESP3DAuthenticationLevel authentication_level);
  static ESP3DMessage *newMsg(ESP3DClientType origin, ESP3DClientType target,
                              ESP3DAuthenticationLevel authentication_level);
  static bool setDataContent(ESP3DMessage *msg, const uint8_t *data,
                             size_t length);
};

class ESP3D_Message : public Print {
 public:
  ESP3D_Message(uint8_t target, uint8_t origin = 0);
#ifdef HTTP_FEATURE
  ESP3D_Message(WEBSERVER *webserver, uint8_t target = ESP_HTTP_CLIENT,
                uint8_t origin = 0);
#endif  // HTTP_FEATURE
  ~ESP3D_Message();
  // size_t write(uint8_t c);
  size_t write(const uint8_t *buffer, size_t size);
  inline size_t write(const char *s) { return write((uint8_t *)s, strlen(s)); }
  // inline size_t write(unsigned long n) { return write((uint8_t)n); }
  // inline size_t write(long n) { return write((uint8_t)n); }
  // inline size_t write(unsigned int n) { return write((uint8_t)n); }
  // inline size_t write(int n) { return write((uint8_t)n); }
  uint8_t target(uint8_t target);
  uint8_t origin(uint8_t origin);
  uint8_t getTarget() { return _target; }
  uint8_t getOrigin() { return _origin; }
  size_t dispatch(const uint8_t *sbuf, size_t len, uint8_t ignoreClient = 0);
  size_t printMSG(const char *s, bool withNL = true);
  size_t printMSGLine(const char *s);
  size_t printERROR(const char *s, int code_error = 500);
  size_t printLN(const char *s);
  void flush();
  int availableforwrite();
  static void toScreen(uint8_t output_type, const char *s);
  static const char *encodeString(const char *s);

#ifdef HTTP_FEATURE
  bool footerSent() { return _footerSent; }
#endif  // HTTP_FEATURE
 private:
  uint8_t _target;
  uint8_t _origin;
#ifdef HTTP_FEATURE
  int _code;
  bool _headerSent;
  bool _footerSent;
  WEBSERVER *_webserver;
#endif  // HTTP_FEATURE
};

#endif  //_ESP3DOUTPUT_H
