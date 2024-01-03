/*
 ESP130.cpp - ESP3D command class

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
#if defined(TELNET_FEATURE)
#include "../../modules/authentication/authentication_service.h"
#include "../../modules/telnet/telnet_server.h"
#include "../esp3d_commands.h"
#include "../esp3d_settings.h"

#define COMMAND_ID 130
// Set TELNET state which can be ON, OFF, CLOSE
//[ESP130]<state> json=<no> pwd=<admin password>
void ESP3DCommands::ESP130(int cmd_params_pos, ESP3DMessage* msg) {
  ESP3DClientType target = msg->origin;
  ESP3DRequest requestId = msg->request_id;
  (void)requestId;
  msg->target = target;
  msg->origin = ESP3DClientType::command;
  bool hasError = false;
  String error_msg = "Invalid parameters";
  String ok_msg = "ok";
  bool json = hasTag(msg, cmd_params_pos, "json");
  bool closeClients = hasTag(msg, cmd_params_pos, "CLOSE");
  bool stateON = hasTag(msg, cmd_params_pos, "ON");
  bool stateOFF = hasTag(msg, cmd_params_pos, "OFF");
  bool has_param = false;
  String tmpstr;
#if defined(AUTHENTICATION_FEATURE)
  if (msg->authentication_level == ESP3DAuthenticationLevel::guest) {
    msg->authentication_level = ESP3DAuthenticationLevel::not_authenticated;
    dispatchAuthenticationError(msg, COMMAND_ID, json);
    return;
  }
#endif  // AUTHENTICATION_FEATURE
  tmpstr = get_clean_param(msg, cmd_params_pos);
  ESP3DState setting_mode = (ESP3DState)ESP3DSettings::readByte(ESP_TELNET_ON);
  if (tmpstr.length() == 0) {
    if (setting_mode == ESP3DState::off) {
      ok_msg = "OFF";
    } else {
      ok_msg = "ON";
    }
  } else {
#if defined(AUTHENTICATION_FEATURE)
    if (msg->authentication_level != ESP3DAuthenticationLevel::admin) {
      dispatchAuthenticationError(msg, COMMAND_ID, json);
      return;
    }
#endif  // AUTHENTICATION_FEATURE
    if (stateON || stateOFF) {
      if (!ESP3DSettings::writeByte(ESP_TELNET_ON, stateOFF ? 0 : 1)) {
        hasError = true;
        error_msg = "Set value failed";
      }
      has_param = true;
    }
    if (closeClients) {
      has_param = true;
      telnet_server.closeClient();
    }
    if (!has_param) {
      hasError = true;
      error_msg = "Invalid parameter";
    }
  }
  if (!dispatchAnswer(msg, COMMAND_ID, json, hasError,
                      hasError ? error_msg.c_str() : ok_msg.c_str())) {
    esp3d_log_e("Error sending response to clients");
  }
}

#endif  // TELNET_FEATURE
