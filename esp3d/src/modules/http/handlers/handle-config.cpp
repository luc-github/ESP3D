/*
 handle-config.cpp - ESP3D http handle

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
#include "../../../include/esp3d_config.h"
#if defined(HTTP_FEATURE)
#include "../http_server.h"
#if defined(ARDUINO_ARCH_ESP32)
#include <WebServer.h>
#endif  // ARDUINO_ARCH_ESP32
#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WebServer.h>
#endif  // ARDUINO_ARCH_ESP8266
#include "../../../core/esp3d_commands.h"
#include "../../../core/esp3d_message.h"
#include "../../authentication/authentication_service.h"

// Handle web command query [ESP420]plain and send
// answer//////////////////////////////
void HTTP_Server::handle_config() {
  ESP3DAuthenticationLevel auth_level =
      AuthenticationService::getAuthenticatedLevel();
  String cmd = "[ESP420]addPreTag";
  if (_webserver->hasArg("json")) {
    cmd += " json=" + _webserver->arg("json");
  }
  ESP3DMessage *msg = esp3d_message_manager.newMsg(
      ESP3DClientType::http, ESP3DClientType::http, (uint8_t *)cmd.c_str(),
      cmd.length(), auth_level);
  if (msg) {
    msg->type = ESP3DMessageType::unique;
    msg->request_id.code = 200;
    // process command
    esp3d_commands.process(msg);
  } else {
    esp3d_log_e("Cannot create message");
  }
  return;
}
#endif  // HTTP_FEATURE
