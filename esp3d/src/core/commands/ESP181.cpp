/*
 ESP181.cpp - ESP3D command class

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
#if defined(FTP_FEATURE)
#include "../../modules/authentication/authentication_service.h"
#include "../esp3d_commands.h"
#include "../esp3d_settings.h"

#define COMMAND_ID 181
// Set/Get Ftp ports
//[ESP181]ctrl=<port> active=<port> passive=<port> json=<no> pwd=<admin
// password>
void ESP3DCommands::ESP181(int cmd_params_pos, ESP3DMessage* msg) {
  ESP3DClientType target = msg->origin;
  ESP3DRequest requestId = msg->request_id;
  (void)requestId;
  msg->target = target;
  msg->origin = ESP3DClientType::command;
  bool hasError = false;
  String error_msg = "Invalid parameters";
  String ok_msg = "ok";
  bool json = hasTag(msg, cmd_params_pos, "json");
  bool has_param = false;
  String tmpstr;
  uint32_t intValue = 0;
#if defined(AUTHENTICATION_FEATURE)
  if (msg->authentication_level == ESP3DAuthenticationLevel::guest) {
    msg->authentication_level = ESP3DAuthenticationLevel::not_authenticated;
    dispatchAuthenticationError(msg, COMMAND_ID, json);
    return;
  }
#endif  // AUTHENTICATION_FEATURE
  tmpstr = get_clean_param(msg, cmd_params_pos);
  if (tmpstr.length() == 0) {
    if (json) {
      ok_msg = "{\"ctrl\":\"";
    } else {
      ok_msg = "ctrl=";
    }
    intValue = ESP3DSettings::readUint32(ESP_FTP_CTRL_PORT);
    ok_msg += String(intValue);
    if (json) {
      ok_msg += "\",\"active\":\"";
    } else {
      ok_msg += ", active=";
    }
    intValue = ESP3DSettings::readUint32(ESP_FTP_DATA_ACTIVE_PORT);
    ok_msg += String(intValue);
    if (json) {
      ok_msg += "\",\"passive\":\"";
    } else {
      ok_msg += ", passive=";
    }
    intValue = ESP3DSettings::readUint32(ESP_FTP_DATA_PASSIVE_PORT);
    ok_msg += String(intValue);
    if (json) {
      ok_msg += "\"}";
    }

  } else {
#if defined(AUTHENTICATION_FEATURE)
    if (msg->authentication_level != ESP3DAuthenticationLevel::admin) {
      dispatchAuthenticationError(msg, COMMAND_ID, json);
      return;
    }
#endif  // AUTHENTICATION_FEATURE
    tmpstr = get_param(msg, cmd_params_pos, "ctrl=");
    if (tmpstr.length() > 0) {
      has_param = true;
      intValue = atoi(tmpstr.c_str());
      if (ESP3DSettings::isValidIntegerSetting(intValue, ESP_FTP_CTRL_PORT)) {
        esp3d_log("Value %ld is valid", intValue);
        if (!ESP3DSettings::writeUint32(ESP_FTP_CTRL_PORT, intValue)) {
          hasError = true;
          error_msg = "Set value failed";
        }
      } else {
        hasError = true;
        error_msg = "Invalid parameter ctrl";
      }
    }

    if (!hasError) {
      tmpstr = get_param(msg, cmd_params_pos, "active=");
      if (tmpstr.length() > 0) {
        has_param = true;
        intValue = atoi(tmpstr.c_str());
        if (ESP3DSettings::isValidIntegerSetting(intValue,
                                                 ESP_FTP_DATA_ACTIVE_PORT)) {
          esp3d_log("Value %ld is valid", intValue);
          if (!ESP3DSettings::writeUint32(ESP_FTP_DATA_ACTIVE_PORT, intValue)) {
            hasError = true;
            error_msg = "Set value failed";
          }
        } else {
          hasError = true;
          error_msg = "Invalid parameter active";
        }
      }
    }

    if (!hasError) {
      tmpstr = get_param(msg, cmd_params_pos, "passive=");
      if (tmpstr.length() > 0) {
        has_param = true;
        intValue = atoi(tmpstr.c_str());
        if (ESP3DSettings::isValidIntegerSetting(intValue,
                                                 ESP_FTP_DATA_PASSIVE_PORT)) {
          esp3d_log("Value %ld is valid", intValue);
          if (!ESP3DSettings::writeUint32(ESP_FTP_DATA_PASSIVE_PORT,
                                          intValue)) {
            hasError = true;
            error_msg = "Set value failed";
          }
        } else {
          hasError = true;
          error_msg = "Invalid parameter passive";
        }
      }
    }
    if (!hasError && !has_param) {
      hasError = true;
      error_msg = "Invalid parameter";
      esp3d_log_e("%s", error_msg.c_str());
    }
  }

  if (!dispatchAnswer(msg, COMMAND_ID, json, hasError,
                      hasError ? error_msg.c_str() : ok_msg.c_str())) {
    esp3d_log_e("Error sending response to clients");
  }
}

#endif  // TELNET_FEATURE
