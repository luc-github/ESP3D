/*
 ESP700.cpp - ESP3D command class

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
#if defined(GCODE_HOST_FEATURE)
#include "../../modules/authentication/authentication_service.h"
#include "../../modules/gcode_host/gcode_host.h"
#include "../esp3d_commands.h"
#include "../esp3d_settings.h"

#define COMMAND_ID 700
// TODO :
//  - on ESP3DLib or GRBL_ESP32 the file/line must be processed like a SD gcode
//  file
//  - on ESP3D the file/line must be processed and/or streamed like a SD gcode
//  file

// process local file
//[ESP700]<filename>
void ESP3DCommands::ESP700(int cmd_params_pos, ESP3DMessage* msg) {
  ESP3DClientType target = msg->origin;
  ESP3DRequest requestId = msg->request_id;
  (void)requestId;
  msg->target = target;
  msg->origin = ESP3DClientType::command;
  bool hasError = false;
  String error_msg = "Invalid parameters";
  String ok_msg = "ok";
  bool json = hasTag(msg, cmd_params_pos, "json");
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
    hasError = true;
    error_msg = "Missing parameter";
    esp3d_log_e("%s", error_msg.c_str());
  } else {
    if (esp3d_gcode_host.getStatus() == HOST_NO_STREAM) {
      if (esp3d_gcode_host.processFile(tmpstr.c_str(),
                                       msg->authentication_level)) {
        esp3d_log("Processing %s", tmpstr.c_str());
      } else {
        hasError = true;
        error_msg = "Error processing file";
        esp3d_log_e("%s", error_msg.c_str());
      }
    } else {
      hasError = true;
      error_msg = "Streaming already in progress";
    }
  }
  if (!dispatchAnswer(msg, COMMAND_ID, json, hasError,
                      hasError ? error_msg.c_str() : ok_msg.c_str())) {
    esp3d_log_e("Error sending response to clients");
  }
}

#endif  // GCODE_HOST_FEATURE
