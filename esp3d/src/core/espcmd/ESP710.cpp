/*
 ESP710.cpp - ESP3D command class

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
#include "../commands.h"
#include "../esp3d_message.h"
#include "../settings_esp3d.h"

#define COMMANDID 710
// Format ESP Filesystem
//[ESP710]FORMATFS json=<no> pwd=<admin password>
bool Commands::ESP710(const char* cmd_params, level_authenticate_type auth_type,
                      ESP3DMessage* esp3dmsg) {
  bool noError = true;
  bool json = has_tag(cmd_params, "json");
  String response;
  String parameter;
  int errorCode = 200;  // unless it is a server error use 200 as default and
                        // set error in json instead
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
    if (has_tag(cmd_params, "FORMATFS")) {
      if (!json) {
        esp3dmsg->printMSGLine("Start Formating");
      }
      ESP_FileSystem::format();
      response = format_response(COMMANDID, json, true, "ok");
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
  return noError;
}

#endif  // FILESYSTEM_FEATURE
