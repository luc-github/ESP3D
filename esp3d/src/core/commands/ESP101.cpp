/*
 ESP101.cpp - ESP3D command class

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
#include "../esp3d_settings.h"

#define COMMAND_ID 101
// STA Password
//[ESP101]<Password> [json=no] [pwd=<admin password>]
void ESP3DCommands::ESP101(int cmd_params_pos, ESP3DMessage* msg) {
  ESP3DClientType target = msg->origin;
  ESP3DRequest requestId = msg->request_id;
  (void)requestId;
  msg->target = target;
  msg->origin = ESP3DClientType::command;
  bool hasError = false;
  String error_msg = "Invalid parameters";
  String ok_msg = "ok";
  bool json = hasTag(msg, cmd_params_pos, "json");
  bool clearSetting = hasTag(msg, cmd_params_pos, "NOPASSWORD");
  String tmpstr;
#if ESP3D_AUTHENTICATION_FEATURE
  if (msg->authentication_level == ESP3DAuthenticationLevel::guest) {
    msg->authentication_level = ESP3DAuthenticationLevel::not_authenticated;
    dispatchAuthenticationError(msg, COMMAND_ID, json);
    return;
  }
#endif  // ESP3D_AUTHENTICATION_FEATURE
  tmpstr = get_clean_param(msg, cmd_params_pos);
  if (tmpstr.length() == 0) {
    hasError = true;
    error_msg = "Password not displayable";
  } else {
    if (clearSetting) {
      esp3d_log("NOPASSWORD flag detected, set string to empty string");
      tmpstr = "";
    }
    esp3d_log("got %s param for a value of %s, is valid %d", tmpstr.c_str(),
              tmpstr.c_str(),
              ESP3DSettings::isValidStringSetting(
                  tmpstr.c_str(), ESP_STA_PASSWORD));
    if (ESP3DSettings::isValidStringSetting(
            tmpstr.c_str(), ESP_STA_PASSWORD)) {
      esp3d_log("Value %s is valid", tmpstr.c_str());
      if (!ESP3DSettings::writeString(ESP_STA_PASSWORD,
                                        tmpstr.c_str())) {
        hasError = true;
        error_msg = "Set value failed";
      }
    } else {
      hasError = true;
      error_msg = "Invalid parameter";
    }
  }
  if (!dispatchAnswer(msg, COMMAND_ID, json, hasError,
                      hasError ? error_msg.c_str() : ok_msg.c_str())) {
    esp3d_log_e("Error sending response to clients");
  }
  /*
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
      if (!ESP3DSettings::isValidStringSetting(parameter.c_str(),
                                               ESP_STA_PASSWORD)) {
        response =
            format_response(COMMANDID, json, false, "Incorrect password");
        noError = false;
      } else {
        if (!ESP3DSettings::write_string(ESP_STA_PASSWORD, parameter.c_str())) {
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
  return noError;*/
}
#endif  // WIFI_FEATURE
