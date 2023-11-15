/*
 ESP122.cpp - ESP3D command class

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
#if defined(CAMERA_DEVICE)
#include <time.h>

#include "../../modules/authentication/authentication_service.h"
#include "../../modules/camera/camera.h"
#include "../esp3d_commands.h"
#include "../esp3d_settings.h"
#include "esp_camera.h"

#define COMMAND_ID 171
// Save frame to target path and filename (default target = today date, default
// name=timestamp.jpg)
//[ESP171]path=<target path> filename=<target filename> pwd=<admin/user
// password>
void ESP3DCommands::ESP171(int cmd_params_pos, ESP3DMessage* msg) {
  ESP3DClientType target = msg->origin;
  ESP3DRequest requestId = msg->request_id;
  (void)requestId;
  msg->target = target;
  msg->origin = ESP3DClientType::command;
  bool hasError = false;
  String error_msg = "Invalid parameters";
  String ok_msg = "ok Snapshot taken";
  String path;
  String filename;
  String tmpstr;
  bool json = hasTag(msg, cmd_params_pos, "json");

#if defined(AUTHENTICATION_FEATURE)
  if (msg->authentication_level == ESP3DAuthenticationLevel::guest) {
    msg->authentication_level = ESP3DAuthenticationLevel::not_authenticated;
    dispatchAuthenticationError(msg, COMMAND_ID, json);
    return;
  }
#endif  // AUTHENTICATION_FEATURE
  if (!esp3d_camera.started()) {
    hasError = true;
    error_msg = "No camera initialized";
    esp3d_log_e("%s", error_msg.c_str());
  } else {
    tmpstr = get_param(msg, cmd_params_pos, "path=");
    if (tmpstr.length() != 0) {
      // get path
      path = tmpstr;
    }
    tmpstr = get_param(msg, cmd_params_pos, "filename=");
    if (tmpstr.length() != 0) {
      // get filename
      filename = tmpstr;
    }
    // if nothing provided, use default filename / path
    if (path.length() == 0) {
      struct tm tmstruct;
      time_t now;
      path = "";
      time(&now);
      localtime_r(&now, &tmstruct);
      path = String((tmstruct.tm_year) + 1900) + "-";
      if (((tmstruct.tm_mon) + 1) < 10) {
        path += "0";
      }
      path += String((tmstruct.tm_mon) + 1) + "-";
      if (tmstruct.tm_mday < 10) {
        path += "0";
      }
      path += String(tmstruct.tm_mday);
    }
    if (filename.length() == 0) {
      struct tm tmstruct;
      time_t now;
      time(&now);
      localtime_r(&now, &tmstruct);
      filename = String(now) + ".jpg";
    }
    if (!esp3d_camera.handle_snap(nullptr, path.c_str(), filename.c_str())) {
      hasError = true;
      error_msg = "Error taking snapshot";
      esp3d_log_e("%s", error_msg.c_str());
    }
  }

  if (!dispatchAnswer(msg, COMMAND_ID, json, hasError,
                      hasError ? error_msg.c_str() : ok_msg.c_str())) {
    esp3d_log_e("Error sending response to clients");
  }
}

#endif  // CAMERA_DEVICE
