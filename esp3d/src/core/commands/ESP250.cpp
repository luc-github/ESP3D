/*
 ESP250.cpp - ESP3D command class

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
#if defined(BUZZER_DEVICE)
#include "../../modules/authentication/authentication_service.h"
#include "../../modules/buzzer/buzzer.h"
#include "../esp3d_commands.h"
#include "../esp3d_settings.h"

#define COMMAND_ID 250
// Play sound
//[ESP250]F=<frequency> D=<duration> json=<no> [pwd=<user password>]
void ESP3DCommands::ESP250(int cmd_params_pos, ESP3DMessage* msg) {
  ESP3DClientType target = msg->origin;
  ESP3DRequest requestId = msg->request_id;
  (void)requestId;
  msg->target = target;
  msg->origin = ESP3DClientType::command;
  bool hasError = false;
  String error_msg = "Invalid parameters";
  String ok_msg = "ok";
  String F = get_param(msg, cmd_params_pos, "F=");
  String D = get_param(msg, cmd_params_pos, "D=");
  int freq = -1;
  int duration = 0;
  if (F.length() > 0) {
    freq = F.toInt();
  }
  if (D.length() > 0) {
    duration = D.toInt();
  }
  bool json = hasTag(msg, cmd_params_pos, "json");
  String tmpstr;
#if defined(AUTHENTICATION_FEATURE)
  if (msg->authentication_level == ESP3DAuthenticationLevel::guest) {
    msg->authentication_level = ESP3DAuthenticationLevel::not_authenticated;
    dispatchAuthenticationError(msg, COMMAND_ID, json);
    return;
  }
#endif  // AUTHENTICATION_FEATURE

  if (freq == -1 && duration == 0) {
    esp3d_buzzer.beep();
  } else {
    if (freq == -1) {
      freq = 1000;
    }
    if (duration != 0) {
      esp3d_buzzer.beep(freq, duration);
    }
  }
  if (!dispatchAnswer(msg, COMMAND_ID, json, hasError,
                      hasError ? error_msg.c_str() : ok_msg.c_str())) {
    esp3d_log_e("Error sending response to clients");
  }
}

#endif  // BUZZER_DEVICE
