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
#include "../esp3d_message.h"
#include "../esp3d_settings.h"

#define COMMANDID 200
// Get SD Card Status
//[ESP200] json=<YES/NO> <RELEASESD> <REFRESH> pwd=<user/admin password>
void ESP3DCommands::ESP200(int cmd_params_pos, ESP3DMessage* msg) {
  /*
  bool noError = true;
  bool json = has_tag(cmd_params, "json");
  bool releaseSD = has_tag(cmd_params, "RELEASE");
  bool refreshSD = has_tag(cmd_params, "REFRESH");
  String response;
  String parameter;
  int errorCode = 200;  // unless it is a server error use 200 as default and
                        // set error in json instead

#ifdef AUTHENTICATION_FEATURE
  if (auth_type == guest) {
    response = format_response(COMMANDID, json, false,
                               "Guest user can't use this command");
    noError = false;
    errorCode = 401;
  }
#else
  (void)auth_type;
#endif  // AUTHENTICATION_FEATURE
  if (noError) {
    if (releaseSD) {
      ESP_SD::releaseFS();
      response = format_response(COMMANDID, json, true, " SD card released");
    }

    if (!ESP_SD::accessFS()) {
      log_esp3d_d("No Access to SD card");
      if (ESP_SD::getState() == ESP_SDCARD_BUSY) {
        response = format_response(COMMANDID, json, true, "Busy");
        log_esp3d_d("Busy");
      } else {
        response = format_response(COMMANDID, json, true, "Not available");
        log_esp3d_d("Not available");
      }
    } else {
      log_esp3d_d("Accessing SD card ok");
      int8_t state = ESP_SD::getState(true);
      if (state != ESP_SDCARD_NOT_PRESENT) {
        response = format_response(COMMANDID, json, true, "SD card ok");
        log_esp3d_d("SD card ok");
        if (refreshSD) {
          ESP_SD::refreshStats(true);
        }
      } else {
        log_esp3d_d("SD card state %d", state);
      }
      ESP_SD::releaseFS();
      parameter = clean_param(get_param(cmd_params, ""));
      if (parameter.length() != 0 && parameter.indexOf("REFRESH") == -1 &&
          parameter.indexOf("RELEASE") == -1) {
        response = format_response(COMMANDID, json, false, "Unknown parameter");
        log_esp3d_e("Unknown parameter");
        noError = false;
      }
    }
  }
  if (noError) {
    if (response.length() == 0) {
      log_esp3d_d("No SD card");
      response = format_response(COMMANDID, json, true, "No SD card");
    }
    if (json) {
      esp3dmsg->printLN(response.c_str());
    } else {
      esp3dmsg->printMSG(response.c_str());
    }
  } else {
    if (json) {
      esp3dmsg->printLN(response.c_str());
    } else {
      esp3dmsg->printERROR(response.c_str(), errorCode);
    }
  }
  return noError;*/
}

#endif  // SD_DEVICE
