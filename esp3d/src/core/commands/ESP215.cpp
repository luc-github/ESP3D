/*
 ESP215.cpp - ESP3D command class

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
#if defined(DISPLAY_DEVICE) && defined(DISPLAY_TOUCH_DRIVER)
#include "../../modules/authentication/authentication_service.h"
#include "../../modules/display/display.h"
#include "../esp3d_commands.h"
#include "../esp3d_settings.h"

#define COMMAND_ID 215
// Touch Calibration
//[ESP215]<CALIBRATE> json=<no> [pwd=<user password>]
void ESP3DCommands::ESP215(int cmd_params_pos, ESP3DMessage* msg) {
  ESP3DClientType target = msg->origin;
  ESP3DRequest requestId = msg->request_id;
  (void)requestId;
  msg->target = target;
  msg->origin = ESP3DClientType::command;
  bool hasError = false;
  String error_msg = "Invalid parameters";
  String ok_msg = "ok";
  bool json = hasTag(msg, cmd_params_pos, "json");
  bool calibrate = hasTag(msg, cmd_params_pos, "CALIBRATE");
  String tmpstr;
#if defined(AUTHENTICATION_FEATURE)
  if (msg->authentication_level != ESP3DAuthenticationLevel::admin) {
    msg->authentication_level = ESP3DAuthenticationLevel::not_authenticated;
    dispatchAuthenticationError(msg, COMMAND_ID, json);
    return;
  }
#endif  // AUTHENTICATION_FEATURE
  tmpstr = get_clean_param(msg, cmd_params_pos);
  if (tmpstr.length() == 0) {
    const ESP3DSettingDescription* settingPtr =
        ESP3DSettings::getSettingPtr(ESP_CALIBRATION);
    if (settingPtr) {
      ok_msg =
          ESP3DSettings::readByte(ESP_CALIBRATION) == 1 ? "Done" : "Not done";
    } else {
      hasError = true;
      error_msg = "This setting is unknown";
      esp3d_log_e("%s", error_msg.c_str());
    }
  } else {
    tmpstr.toUpperCase();
    if (!calibrate) {
      hasError = true;
      error_msg = "Missing parameter";
      esp3d_log_e("%s", error_msg.c_str());
    } else {
      ok_msg = "ok - Please follow screen instructions";
      esp3d_display.startCalibration();
    }
  }
  if (!dispatchAnswer(msg, COMMAND_ID, json, hasError,
                      hasError ? error_msg.c_str() : ok_msg.c_str())) {
    esp3d_log_e("Error sending response to clients");
  }
}

#endif  // DISPLAY_DEVICE && DISPLAY_TOUCH_DRIVER
