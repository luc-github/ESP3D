/*
 ESP900.cpp - ESP3D command class

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
#if COMMUNICATION_PROTOCOL != SOCKET_SERIAL
#include "../../modules/authentication/authentication_service.h"
#include "../../modules/serial/serial_service.h"
#include "../esp3d_commands.h"
#include "../esp3d_settings.h"

#define COMMAND_ID 900
// Get state / Set Enable / Disable Serial Communication
//[ESP900]<ENABLE/DISABLE> json=<no> [pwd=<admin password>]
void ESP3DCommands::ESP900(int cmd_params_pos, ESP3DMessage* msg) {
  ESP3DClientType target = msg->origin;
  ESP3DRequest requestId = msg->request_id;
  (void)requestId;
  msg->target = target;
  msg->origin = ESP3DClientType::command;
  bool hasError = false;
  String error_msg = "Invalid parameters";
  String ok_msg = "ok";
  bool json = hasTag(msg, cmd_params_pos, "json");
  bool enabled = hasTag(msg, cmd_params_pos, "ENABLE");
  bool disabled = hasTag(msg, cmd_params_pos, "DISABLE");
  String tmpstr;
#if defined(AUTHENTICATION_FEATURE)
  if (msg->authentication_level == ESP3DAuthenticationLevel::guest) {
    msg->authentication_level = ESP3DAuthenticationLevel::not_authenticated;
    dispatchAuthenticationError(msg, COMMAND_ID, json);
    return;
  }
#endif  // AUTHENTICATION_FEATURE
  tmpstr = get_clean_param(msg, cmd_params_pos);
  if (tmpstr.length() == 0) {
    if (esp3d_serial_service.started()) {
      ok_msg = "ENABLED";
    } else {
      ok_msg = "DISABLED";
    }
    ok_msg += " - Serial" + String(esp3d_serial_service.serialIndex());
  } else {
    if (enabled || disabled) {
      if (enabled) {
        if (!esp3d_serial_service.begin(ESP_SERIAL_OUTPUT)) {
          hasError = true;
          error_msg = "Cannot enable serial communication";
        }
      } else {
        esp3d_serial_service.end();
      }
    } else {
      hasError = true;
      error_msg = "Invalid parameters";
      esp3d_log_e("%s", error_msg.c_str());
    }
  }
  if (!dispatchAnswer(msg, COMMAND_ID, json, hasError,
                      hasError ? error_msg.c_str() : ok_msg.c_str())) {
    esp3d_log_e("Error sending response to clients");
  }
}
#endif