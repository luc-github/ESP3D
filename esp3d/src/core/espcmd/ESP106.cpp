/*
 ESP106.cpp - ESP3D command class

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
#if defined(WIFI_FEATURE)
#include "../../modules/authentication/authentication_service.h"
#include "../../modules/wifi/wificonfig.h"
#include "../esp3d_commands.h"
#include "../esp3d_message.h"
#include "../esp3d_settings.h"

#define COMMANDID 106
// AP Password
//[ESP106]<Password> [json=no] [pwd=<admin password>]
bool Commands::ESP106(const char* cmd_params,
                      ESP3DAuthenticationLevel auth_type,
                      ESP3D_Message* esp3dmsg) {
  bool noError = true;
  bool json = has_tag(cmd_params, "json");
  bool clearSetting = has_tag(cmd_params, "NOPASSWORD");
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
    parameter = clean_param(get_param(cmd_params, ""));
    if (parameter.length() == 0) {
      response =
          format_response(COMMANDID, json, false, "Password not displayable");
      noError = false;
    } else {
      if (clearSetting) {
        parameter = "";
      }
      if (!Settings_ESP3D::isValidStringSetting(parameter.c_str(),
                                                ESP_AP_PASSWORD)) {
        response = format_response(COMMANDID, json, false, "Set failed");
        noError = false;
      } else {
        if (!Settings_ESP3D::write_string(ESP_AP_PASSWORD, parameter.c_str())) {
          response = format_response(COMMANDID, json, false, "Set failed");
          noError = false;
        } else {
          response = format_response(COMMANDID, json, true, "ok");
        }
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

#endif  // WIFI_FEATURE
