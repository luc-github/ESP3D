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
#include "../esp3d_message.h"
#include "../esp3d_settings.h"

#define COMMANDID 202
// Get/Set SD card Speed factor 1 2 4 6 8 16 32
//[ESP202]SPEED=<value> json=<no> pwd=<user/admin password>
bool ESP3DCommands::ESP202(const char* cmd_params,
                           ESP3DAuthenticationLevel auth_type,
                           ESP3D_Message* esp3dmsg) {
  bool noError = true;
  bool json = has_tag(cmd_params, "json");
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
    parameter = clean_param(get_param(cmd_params, ""));
    // get
    if (parameter.length() == 0) {
      response = format_response(
          COMMANDID, json, true,
          String(ESP3DSettings::read_byte(ESP_SD_SPEED_DIV)).c_str());
    } else {  // set
      parameter = get_param(cmd_params, "SPEED=");
      if (ESP3DSettings::isValidByteSetting(parameter.toInt(),
                                            ESP_SD_SPEED_DIV)) {
        if (!ESP3DSettings::write_byte(ESP_SD_SPEED_DIV, parameter.toInt())) {
          response = format_response(COMMANDID, json, false, "Set failed");
          noError = false;

        } else {
          ESP_SD::setSPISpeedDivider(parameter.toInt());
          response = format_response(COMMANDID, json, true, "ok");
        }
      } else {
        response = format_response(COMMANDID, json, false, "Invalid parameter");
        noError = false;
      }
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
  return noError;
}

#endif  // SD_DEVICE
