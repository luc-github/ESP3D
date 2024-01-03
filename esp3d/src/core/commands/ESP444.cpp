/*
 ESP444.cpp - ESP3D command class

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
#include "../../modules/authentication/authentication_service.h"
#include "../esp3d.h"
#include "../esp3d_commands.h"

// Set ESP State
// cmd are RESTART / RESET
//[ESP444]<cmd> json=<no> <pwd=admin>
#define COMMAND_ID 444
void ESP3DCommands::ESP444(int cmd_params_pos, ESP3DMessage* msg) {
  ESP3DClientType target = msg->origin;
  ESP3DRequest requestId = msg->request_id;
  (void)requestId;
  msg->target = target;
  msg->origin = ESP3DClientType::command;
  bool json = hasTag(msg, cmd_params_pos, "json");
  String tmpstr;
  bool isRestart = hasTag(msg, cmd_params_pos, "RESTART");
  bool isReset = hasTag(msg, cmd_params_pos, "RESET");
  bool hasError = false;
  String error_msg = "Invalid parameters";
  String ok_msg = "";
#if defined(AUTHENTICATION_FEATURE)
  if (msg->authentication_level != ESP3DAuthenticationLevel::admin) {
    msg->authentication_level = ESP3DAuthenticationLevel::not_authenticated;
    dispatchAuthenticationError(msg, COMMAND_ID, json);
    return;
  }
#endif  // AUTHENTICATION_FEATURE
  if (isReset) {
    Esp3D::reset();
    esp3d_log("Resetting settings");
  }

  if (!(isRestart || isReset)) {
    hasError = true;
    esp3d_log_e("%s", error_msg.c_str());
  } else {
    if (isReset) {
#if defined(AUTHENTICATION_FEATURE)
      if (msg->authentication_level != ESP3DAuthenticationLevel::admin) {
        dispatchAuthenticationError(msg, COMMAND_ID, json);
        return;
      }
#endif  // AUTHENTICATION_FEATURE
      ok_msg += "Reset done";
    }
    if (isRestart) {
      if (ok_msg.length() > 0) {
        ok_msg += ", ";
      }
      ok_msg += "Now restarting...";
    }
  }
  if (!dispatchAnswer(msg, COMMAND_ID, json, hasError,
                      hasError ? error_msg.c_str() : ok_msg.c_str())) {
    esp3d_log_e("Error sending response to clients");
    return;
  }
  if (isRestart) {
    ESP3DHal::wait(100);
    Esp3D::restart_esp();
  }
}
