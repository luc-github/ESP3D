/*
 ESP550.cpp - ESP3D command class

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
#if defined(AUTHENTICATION_FEATURE)
#include "../../modules/authentication/authentication_service.h"
#include "../esp3d_commands.h"
#include "../esp3d_settings.h"

#define COMMAND_ID 500
// Get connection status
//[ESP500] json=<no> pwd=<admin password>
void ESP3DCommands::ESP500(int cmd_params_pos, ESP3DMessage* msg) {
  ESP3DClientType target = msg->origin;
  ESP3DRequest requestId = msg->request_id;
  (void)requestId;
  msg->target = target;
  msg->origin = ESP3DClientType::command;
  bool hasError = false;
  String error_msg = "Invalid parameters";
  String ok_msg = "ok";
  bool json = hasTag(msg, cmd_params_pos, "json");

  String tmpstr = get_clean_param(msg, cmd_params_pos);
  if (tmpstr.length() == 0) {
    switch (msg->authentication_level) {
      case ESP3DAuthenticationLevel::admin:
        ok_msg = "admin";
        break;
      case ESP3DAuthenticationLevel::user:
        ok_msg = "user";
        break;
      case ESP3DAuthenticationLevel::not_authenticated:
        ok_msg = "Not authenticated";
        break;
      default:
        ok_msg = "guest";
        break;
    }
  } else {
    esp3d_log_e("Got %s", tmpstr.c_str());
    hasError = true;
  }

  if (!dispatchAnswer(msg, COMMAND_ID, json, hasError,
                      hasError ? error_msg.c_str() : ok_msg.c_str())) {
    esp3d_log_e("Error sending response to clients");
  }
}

#endif  // AUTHENTICATION_FEATURE
