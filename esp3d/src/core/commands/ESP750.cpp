/*
 ESP750.cpp - ESP3D command class

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
#include "../esp3d_message.h"
#include "../esp3d_settings.h"

#define COMMAND_ID 750
// Action on SD Filesystem
// rmdir / remove / mkdir / exists /create
//[ESP750]<Action>=<path> json=<no> pwd=<admin password>
void ESP3DCommands::ESP750(int cmd_params_pos, ESP3DMessage* msg) {
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
#if ESP3D_AUTHENTICATION_FEATURE
  if (msg->authentication_level == ESP3DAuthenticationLevel::guest) {
    msg->authentication_level = ESP3DAuthenticationLevel::not_authenticated;
    dispatchAuthenticationError(msg, COMMAND_ID, json);
    return;
  }
#endif  // ESP3D_AUTHENTICATION_FEATURE
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
    if (ESP_SD::accessFS()) {
      if (ESP_SD::getState(true) == ESP_SDCARD_NOT_PRESENT) {
        hasError = true;
        error_msg = "No SD card";
        esp3d_log_e("%s", error_msg.c_str());
      } else {
        switch (i) {
          case 0:
            if (!ESP_SD::rmdir(tmpstr.c_str())) {
              hasError = true;
              error_msg = "rmdir failed";
            }
            break;
          case 1:
            if (!ESP_SD::remove(tmpstr.c_str())) {
              hasError = true;
              error_msg = "remove failed";
            }
            break;
          case 2:
            if (!ESP_SD::mkdir(tmpstr.c_str())) {
              hasError = true;
              error_msg = "mkdir failed";
            }
            break;
          case 3:
            if (ESP_SD::exists(tmpstr.c_str())) {
              ok_msg = "yes";
            } else {
              ok_msg = "no";
            }
            break;
          case 4: {
            ESP_SDFile f = ESP_SD::open(tmpstr.c_str(), ESP_FILE_WRITE);
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
      }
      ESP_SD::releaseFS();
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
  /*
  bool noError = true;
  bool json = has_tag(cmd_params, "json");
  String response;
  String parameter;
  bool hasParam = false;
  int errorCode = 200;  // unless it is a server error use 200 as default and
                        // set error in json instead;
#ifdef AUTHENTICATION_FEATURE
  if (auth_type != admin) {
    response =
        format_response(COMMANDID, json, false, "Wrong authentication level");
    noError = false;
    errorCode = 401;
  }
#else
  (void)auth_type;
#endif  // AUTHENTICATION_FEATURE
  if (noError) {
    if (!ESP_SD::accessFS()) {
      response = format_response(COMMANDID, json, false, "Not available");
      noError = false;
    } else {
      if (ESP_SD::getState(true) == ESP_SDCARD_NOT_PRESENT) {
        response = format_response(COMMANDID, json, false, "No SD card");
        noError = false;
      } else {
        ESP_SD::setState(ESP_SDCARD_BUSY);
        parameter = get_param(cmd_params, "mkdir=");
        if (parameter.length() != 0) {
          hasParam = true;
          if (!ESP_SD::mkdir(parameter.c_str())) {
            response = format_response(COMMANDID, json, false, "mkdir failed");
            noError = false;
          }
        }
        if (noError && !hasParam) {
          parameter = get_param(cmd_params, "rmdir=");
          if (parameter.length() != 0) {
            hasParam = true;
            if (!ESP_SD::rmdir(parameter.c_str())) {
              response =
                  format_response(COMMANDID, json, false, "rmdir failed");
              noError = false;
            }
          }
        }
        if (noError && !hasParam) {
          parameter = get_param(cmd_params, "remove=");
          if (parameter.length() != 0) {
            hasParam = true;
            if (ESP_SD::remove(parameter.c_str())) {
              response =
                  format_response(COMMANDID, json, false, "remove failed");
              noError = false;
            }
          }
        }
        if (noError && !hasParam) {
          parameter = get_param(cmd_params, "exists=");
          if (parameter.length() != 0) {
            hasParam = true;
            if (ESP_SD::exists(parameter.c_str())) {
              response = format_response(COMMANDID, json, true, "yes");
            } else {
              response = format_response(COMMANDID, json, false, "no");
            }
          }
        }
        if (noError && !hasParam) {
          parameter = get_param(cmd_params, "create=");
          if (parameter.length() != 0) {
            hasParam = true;
            ESP_SDFile f = ESP_SD::open(parameter.c_str(), ESP_FILE_WRITE);
            if (!f.isOpen()) {
              response =
                  format_response(COMMANDID, json, false, "create failed");
              noError = false;
            } else {
              f.close();
            }
          }
        }
        if (hasParam && noError && response.length() == 0) {
          response = format_response(COMMANDID, json, true, "ok");
        }
        if (!hasParam) {
          response =
              format_response(COMMANDID, json, false, "Missing parameter");
          noError = false;
        }
      }
      ESP_SD::releaseFS();
    }
  }
  if (json) {
    esp3dmsg->printLN(response.c_str());
  } else {
    if (noError) {
      esp3dmsg->printMSG(response.c_str());
    } else {
      esp3dmsg->printERROR(response.c_str(), errorCode);
    }
  }
  return noError;*/
}

#endif  // SD_DEVICE
