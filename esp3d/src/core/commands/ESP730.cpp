/*
 ESP730.cpp - ESP3D command class

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
#if defined(FILESYSTEM_FEATURE)
#include "../../modules/authentication/authentication_service.h"
#include "../../modules/filesystem/esp_filesystem.h"
#include "../esp3d_commands.h"
#include "../esp3d_settings.h"

#define COMMAND_ID 730
// Action on ESP Filesystem
// rmdir / remove / mkdir / exists / create
//[ESP730]<Action>=<path> json=<no> pwd=<admin password>
void ESP3DCommands::ESP730(int cmd_params_pos, ESP3DMessage* msg) {
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
  const char* cmdList[] = {"rmdir=", "remove=", "mkdir=", "exists=", "create="};
  uint8_t cmdListSize = sizeof(cmdList) / sizeof(char*);
#if defined(AUTHENTICATION_FEATURE)
  if (msg->authentication_level == ESP3DAuthenticationLevel::guest) {
    msg->authentication_level = ESP3DAuthenticationLevel::not_authenticated;
    dispatchAuthenticationError(msg, COMMAND_ID, json);
    return;
  }
#endif  // AUTHENTICATION_FEATURE
  uint8_t i;
  for (i = 0; i < cmdListSize && !hasError; i++) {
    tmpstr = get_param(msg, cmd_params_pos, cmdList[i]);
    if (tmpstr.length() != 0) {
      break;
    }
  }
  if (i >= cmdListSize || tmpstr.length() == 0) {
    hasError = true;
  } else {
    if (ESP_FileSystem::accessFS()) {
      switch (i) {
        case 0:
          if (!ESP_FileSystem::rmdir(tmpstr.c_str())) {
            hasError = true;
            error_msg = "rmdir failed";
          }
          break;
        case 1:
          if (!ESP_FileSystem::remove(tmpstr.c_str())) {
            hasError = true;
            error_msg = "remove failed";
          }
          break;
        case 2:
          if (!ESP_FileSystem::mkdir(tmpstr.c_str())) {
            hasError = true;
            error_msg = "mkdir failed";
          }
          break;
        case 3:
          if (ESP_FileSystem::exists(tmpstr.c_str())) {
            ok_msg = "yes";
          } else {
            ok_msg = "no";
          }
          break;
        case 4: {
          ESP_File f = ESP_FileSystem::open(tmpstr.c_str(), ESP_FILE_WRITE);
          if (f.isOpen()) {
            f.close();
          } else {
            hasError = true;
            error_msg = "creation failed";
          }
        } break;
        default:
          hasError = true;
          error_msg = "Invalid parameters";
          break;
      }
      ESP_FileSystem::releaseFS();
    } else {
      hasError = true;
      error_msg = "Flash not available";
      esp3d_log_e("%s", error_msg.c_str());
    }
  }

  if (!dispatchAnswer(msg, COMMAND_ID, json, hasError,
                      hasError ? error_msg.c_str() : ok_msg.c_str())) {
    esp3d_log_e("Error sending response to clients");
  }
}

#endif  // FILESYSTEM_FEATURE
