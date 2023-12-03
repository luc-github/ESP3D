/*
 ESP202.cpp - ESP3D command class

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
#if defined(SD_DEVICE) && SD_DEVICE != ESP_SDIO
#include "../../modules/authentication/authentication_service.h"
#include "../../modules/filesystem/esp_sd.h"
#include "../esp3d_commands.h"
#include "../esp3d_settings.h"

#define COMMAND_ID 202
// Get/Set SD card Speed factor 1 2 4 6 8 16 32
//[ESP202]SPEED=<value> json=<no> pwd=<user/admin password>
void ESP3DCommands::ESP202(int cmd_params_pos, ESP3DMessage* msg) {
  ESP3DClientType target = msg->origin;
  ESP3DRequest requestId = msg->request_id;
  (void)requestId;
  msg->target = target;
  msg->origin = ESP3DClientType::command;
  bool hasError = false;
  String error_msg = "Invalid parameters";
  String ok_msg = "ok";
  String speed = get_param(msg, cmd_params_pos, "SPEED=");
  bool json = hasTag(msg, cmd_params_pos, "json");
  String tmpstr;
#if defined(AUTHENTICATION_FEATURE)
  if (msg->authentication_level == ESP3DAuthenticationLevel::guest) {
    msg->authentication_level = ESP3DAuthenticationLevel::not_authenticated;
    dispatchAuthenticationError(msg, COMMAND_ID, json);
    return;
  }
#endif  // AUTHENTICATION_FEATURE

  if (speed.length() == 0) {  // Get
    esp3d_log("Reading SD speed divider");
    uint8_t s = ESP3DSettings::readByte(ESP_SD_SPEED_DIV);
    esp3d_log("SD speed divider is %d", s);
    ok_msg = String(s);
  } else {
#if defined(AUTHENTICATION_FEATURE)
    if (msg->authentication_level != ESP3DAuthenticationLevel::admin) {
      dispatchAuthenticationError(msg, COMMAND_ID, json);
      return;
    }
#endif  // AUTHENTICATION_FEATURE
        // Set
    esp3d_log("Setting SD speed divider");
    uint8_t s = speed.toInt();
    if (ESP3DSettings::isValidByteSetting(s, ESP_SD_SPEED_DIV)) {
      if (!ESP3DSettings::writeByte(ESP_SD_SPEED_DIV, s)) {
        hasError = true;
        error_msg = "Set failed";
        esp3d_log_e("Error setting SD speed divider");
      } else {
        ESP_SD::setSPISpeedDivider(s);
      }
    } else {
      hasError = true;
      error_msg = "Invalid parameter";
    }
  }

  if (!dispatchAnswer(msg, COMMAND_ID, json, hasError,
                      hasError ? error_msg.c_str() : ok_msg.c_str())) {
    esp3d_log_e("Error sending response to clients");
  }
}

#endif  // SD_DEVICE
