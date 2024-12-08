/*
 handle-command.cpp - ESP3D http handle

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
#include "../../../core/esp3d_settings.h"
#include "../../../core/esp3d_string.h"
#include "../../authentication/authentication_service.h"


// Handle web command query and send answer//////////////////////////////
void HTTP_Server::handle_web_command() {
  ESP3DAuthenticationLevel auth_level =
      AuthenticationService::getAuthenticatedLevel();
  if (auth_level == ESP3DAuthenticationLevel::guest) {
    _webserver->send(401, "text/plain", "Wrong authentication!");
    return;
  }
  // esp3d_log("Authentication = %d", auth_level);
  String cmd = "";
  bool isRealTimeCommand = false;
  if (_webserver->hasArg("cmd")) {
    cmd = _webserver->arg("cmd");
    esp3d_log_d("Command is %s", cmd.c_str());
    if (!cmd.endsWith("\n")) {
      esp3d_log_d("Command is not ending with \\n");
      if (ESP3DSettings::GetFirmwareTarget() == GRBL || ESP3DSettings::GetFirmwareTarget() == GRBLHAL) {
        uint len = cmd.length();
        if (!((len == 1 && esp3d_string::isRealTimeCommand(cmd[0])) ||
              (len == 2 && esp3d_string::isRealTimeCommand(cmd[1])))) {
          cmd += "\n";
          esp3d_log_d("Command is not realtime, adding \\n");
        } else {  // no need \n for realtime command
          esp3d_log_d("Command is realtime, no need to add \\n");
          isRealTimeCommand = true;
          // remove the 0XC2 that should not be there
          if (len == 2 && esp3d_string::isRealTimeCommand(cmd[1]) && cmd[1] == 0xC2) {
            cmd[0] = cmd[1];
            cmd[1] = 0x0;
            esp3d_log_d("Command is realtime, removing 0xC2");
          }
        }
      } else {
        esp3d_log_d("Command is not realtime, adding \\n");
        cmd += "\n";  // need to validate command
      }
    } else {
      esp3d_log_d("Command is ending with \\n");
    }
    esp3d_log_d("Message type is %s for %s", isRealTimeCommand ? "realtimecmd" : "unique", cmd.c_str());
    if (esp3d_commands.is_esp_command((uint8_t *)cmd.c_str(), cmd.length())) {
      ESP3DMessage *msg = esp3d_message_manager.newMsg(
          ESP3DClientType::http, esp3d_commands.getOutputClient(),
          (uint8_t *)cmd.c_str(), cmd.length(), auth_level);
      if (msg) {
        msg->type = ESP3DMessageType::unique; //ESP3D command is always unique
        msg->request_id.code = 200;
        // process command
        esp3d_commands.process(msg);
      } else {
        esp3d_log_e("Cannot create message");
      }
    } else {
      HTTP_Server::set_http_headers();
      // the command is not ESP3D so it will be forwarded to the output client
      // no need to wait to answer then
      _webserver->send(200, "text/plain", "ESP3D says: command forwarded");
      esp3d_commands.dispatch(cmd.c_str(), esp3d_commands.getOutputClient(),
                              no_id, isRealTimeCommand ? ESP3DMessageType::realtimecmd :ESP3DMessageType::unique,
                              ESP3DClientType::http, auth_level);
    }
  } else if (_webserver->hasArg("ping")) {
    _webserver->send(200);
  } else {
    _webserver->send(400, "text/plain", "Invalid command");
  }
  return;
}
#endif  // HTTP_FEATURE
