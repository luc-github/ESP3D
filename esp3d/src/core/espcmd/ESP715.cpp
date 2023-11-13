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
#include "../esp3d_message.h"
#include "../esp3d_settings.h"

#define COMMANDID 715
// Format SD Filesystem
//[ESP715]FORMATSD json=<no> pwd=<admin password>
void ESP3DCommands::ESP715(int cmd_params_pos, ESP3DMessage* msg) {
  /*
  bool noError = true;
  bool json = has_tag(cmd_params, "json");
  String response;
  String parameter;
  int errorCode = 200;  // unless it is a server error use 200 as default and
                        // set error in json instead
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
    if (has_tag(cmd_params, "FORMATSD")) {
      if (!ESP_SD::accessFS()) {
        response = format_response(COMMANDID, json, false, "Not available");
        noError = false;
      } else {
        ESP_SD::setState(ESP_SDCARD_BUSY);
        if (!json) {
          esp3dmsg->printMSGLine("Start Formating");
        }
        if (ESP_SD::format(esp3dmsg)) {
          response = format_response(COMMANDID, json, true, "ok");
        } else {
          response = format_response(COMMANDID, json, false, "Format failed");
          noError = false;
        }
        ESP_SD::releaseFS();
      }
    } else {
      response = format_response(COMMANDID, json, false, "Invalid parameter");
      noError = false;
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
