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

#define ESP_OUTPUT_IP_ADDRESS 0
#define ESP_OUTPUT_STATUS 1
#define ESP_OUTPUT_PROGRESS 2
#define ESP_OUTPUT_STATE 3

#define ESP_STATE_DISCONNECTED 0

#ifndef _ESP3DOUTPUT_H
#define _ESP3DOUTPUT_H

#include "../include/esp3d_config.h"

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
  uint code;
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
                              ESP3DAuthenticationLevel authentication_level =
                                  ESP3DAuthenticationLevel::guest);
  static ESP3DMessage *newMsg(ESP3DClientType origin, ESP3DClientType target,
                              ESP3DAuthenticationLevel authentication_level =
                                  ESP3DAuthenticationLevel::guest);
  static bool setDataContent(ESP3DMessage *msg, const uint8_t *data,
                             size_t length);
};

#endif  //_ESP3DOUTPUT_H
