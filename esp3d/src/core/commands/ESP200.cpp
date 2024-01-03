/*
 ESP200.cpp - ESP3D command class

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
#if defined(SD_DEVICE)
#include "../../modules/authentication/authentication_service.h"
#include "../../modules/filesystem/esp_sd.h"
#include "../esp3d_commands.h"
#include "../esp3d_settings.h"

#define COMMAND_ID 200
// Get SD Card Status
//[ESP200] json=<YES/NO> <RELEASESD> <REFRESH> pwd=<user/admin password>
void ESP3DCommands::ESP200(int cmd_params_pos, ESP3DMessage* msg) {
  ESP3DClientType target = msg->origin;
  ESP3DRequest requestId = msg->request_id;
  (void)requestId;
  msg->target = target;
  msg->origin = ESP3DClientType::command;
  bool hasError = false;
  String error_msg = "Invalid parameters";
  String ok_msg = "ok";
  bool releasesd = hasTag(msg, cmd_params_pos, "RELEASESD");
  bool refreshsd = hasTag(msg, cmd_params_pos, "REFRESH");
  bool json = hasTag(msg, cmd_params_pos, "json");
  String tmpstr;
#if defined(AUTHENTICATION_FEATURE)
  if (msg->authentication_level == ESP3DAuthenticationLevel::guest) {
    msg->authentication_level = ESP3DAuthenticationLevel::not_authenticated;
    dispatchAuthenticationError(msg, COMMAND_ID, json);
    return;
  }
#endif  // AUTHENTICATION_FEATURE
  if (releasesd) {
    ESP_SD::releaseFS();
    ok_msg = "SD card released";
  }

  if (!ESP_SD::accessFS()) {
    if (ESP_SD::getState() == ESP_SDCARD_BUSY) {
      ok_msg = "Busy";
    } else {
      ok_msg = "Not available";
    }
  } else {
    int8_t state = ESP_SD::getState(true);
    if (state == ESP_SDCARD_IDLE) {
      ok_msg = "SD card ok";
      if (refreshsd) {
        ESP_SD::refreshStats(true);
      }
    }
    ESP_SD::releaseFS();
  }

  if (!dispatchAnswer(msg, COMMAND_ID, json, hasError,
                      hasError ? error_msg.c_str() : ok_msg.c_str())) {
    esp3d_log_e("Error sending response to clients");
  }
}

#endif  // SD_DEVICE
