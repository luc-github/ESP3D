/*
 ESP715.cpp - ESP3D command class

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

#define COMMAND_ID 715
// Format SD Filesystem
//[ESP715]FORMATSD json=<no> pwd=<admin password>
void ESP3DCommands::ESP715(int cmd_params_pos, ESP3DMessage* msg) {
  ESP3DClientType target = msg->origin;
  ESP3DRequest requestId = msg->request_id;
  (void)requestId;
  msg->target = target;
  msg->origin = ESP3DClientType::command;
  bool hasError = false;
  String error_msg = "Invalid parameters";
  String ok_msg = "ok";
  bool json = hasTag(msg, cmd_params_pos, "json");
  bool formatsd = hasTag(msg, cmd_params_pos, "FORMATSD");
  String tmpstr;
#if defined(AUTHENTICATION_FEATURE)
  if (msg->authentication_level != ESP3DAuthenticationLevel::admin) {
    msg->authentication_level = ESP3DAuthenticationLevel::not_authenticated;
    dispatchAuthenticationError(msg, COMMAND_ID, json);
    return;
  }
#endif  // AUTHENTICATION_FEATURE
  tmpstr = get_clean_param(msg, cmd_params_pos);
  ESP3DMessage* endMsg = nullptr;
  if (tmpstr.length() == 0) {
    hasError = true;
    error_msg = "Missing parameter";
    esp3d_log_e("%s", error_msg.c_str());

  } else {
    if (formatsd) {
      if (ESP_SD::getState() != ESP_SDCARD_BUSY) {
        ok_msg = "Starting formating...";
       endMsg =  esp3d_message_manager.copyMsgInfos(*msg);
      } else {
        hasError = true;
        error_msg = "SD card busy";
        esp3d_log_e("%s", error_msg.c_str());
      }
    } else {
      hasError = true;
      error_msg = "Invalid parameter";
      esp3d_log_e("%s", error_msg.c_str());
    }
  }
  if (!dispatchAnswer(msg, COMMAND_ID, json, hasError,
                      hasError ? error_msg.c_str() : ok_msg.c_str())) {
    esp3d_log_e("Error sending response to clients");
  }
  if (!hasError && formatsd) {
    if (ESP_SD::accessFS()) {
      ESP_SD::setState(ESP_SDCARD_BUSY);
      bool success = ESP_SD::format();
      if (success) {
        ok_msg = "Formating done";
      } else {
        hasError = true;
        error_msg = "Formating failed";
        esp3d_log_e("%s", error_msg.c_str());
      }

      ESP_SD::releaseFS();
    } else {
      hasError = true;
      error_msg = "SD not available";
      esp3d_log_e("%s", error_msg.c_str());
    }

    if (!dispatchAnswer(endMsg, COMMAND_ID, json, hasError,
                        hasError ? error_msg.c_str() : ok_msg.c_str())) {
      esp3d_log_e("Error sending response to clients");
    }
  }
}

#endif  // SD_DEVICE
