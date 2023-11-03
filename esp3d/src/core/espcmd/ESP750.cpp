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
#include "../commands.h"
#include "../esp3doutput.h"
#include "../settings_esp3d.h"

#define COMMANDID 750
// Action on SD Filesystem
// rmdir / remove / mkdir / exists /create
//[ESP750]<Action>=<path> json=<no> pwd=<admin password>
bool Commands::ESP750(const char* cmd_params, level_authenticate_type auth_type,
                      ESP3DOutput* output) {
  bool noError = true;
  bool json = has_tag(cmd_params, "json");
  String response;
  String parameter;
  bool hasParam = false;
  int errorCode = 200;  // unless it is a server error use 200 as default and
                        // set error in json instead;
#ifdef AUTHENTICATION_FEATURE
  if (auth_type != LEVEL_ADMIN) {
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
    output->printLN(response.c_str());
  } else {
    if (noError) {
      output->printMSG(response.c_str());
    } else {
      output->printERROR(response.c_str(), errorCode);
    }
  }
  return noError;
}

#endif  // SD_DEVICE
