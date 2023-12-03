/*
 ESP150.cpp - ESP3D command class

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
#include "../../modules/authentication/authentication_service.h"
#include "../esp3d_commands.h"
#include "../esp3d_settings.h"

#define COMMAND_ID 150
// Get/Set display/set boot delay in ms / Verbose boot
//[ESP150]<delay=time in milliseconds><verbose=ON/OFF>[pwd=<admin password>]
void ESP3DCommands::ESP150(int cmd_params_pos, ESP3DMessage* msg) {
  ESP3DClientType target = msg->origin;
  ESP3DRequest requestId = msg->request_id;
  (void)requestId;
  msg->target = target;
  msg->origin = ESP3DClientType::command;
  bool hasError = false;
  String error_msg = "Invalid parameters";
  String ok_msg = "ok";
  String tmpstr;
  bool json = hasTag(msg, cmd_params_pos, "json");
  bool hasParam = false;
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
      ok_msg = "{\"delay\":\"";
    } else {
      ok_msg = "delay=";
    }
    ok_msg += String(ESP3DSettings::readUint32(ESP_BOOT_DELAY));
    if (json) {
      ok_msg += "\",\"verbose\":\"";
    } else {
      ok_msg += ", verbose=";
    }
    ok_msg += ESP3DSettings::isVerboseBoot(true) ? "ON" : "OFF";
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
    tmpstr = get_param(msg, cmd_params_pos, "delay=");
    if (tmpstr.length() != 0) {
      hasParam = true;
      if (ESP3DSettings::isValidIntegerSetting(tmpstr.toInt(),
                                               ESP_BOOT_DELAY)) {
        if (!ESP3DSettings::writeUint32(ESP_BOOT_DELAY, tmpstr.toInt())) {
          hasError = true;
          error_msg = "Set delay failed";
          esp3d_log_e("%s", error_msg.c_str());
        }
      } else {
        hasError = true;
        error_msg = "Incorrect delay";
        esp3d_log_e("%s", error_msg.c_str());
      }
    }
    if (!hasError) {
      tmpstr = get_param(msg, cmd_params_pos, "verbose=");
      if (tmpstr.length() != 0) {
        hasParam = true;
        tmpstr.toUpperCase();
        if (tmpstr == "ON" || tmpstr == "OFF" || tmpstr == "1" ||
            tmpstr == "0" || tmpstr == "TRUE" || tmpstr == "FALSE" ||
            tmpstr == "YES" || tmpstr == "NO") {
          uint8_t val = 0;
          if (tmpstr == "ON" || tmpstr == "1" || tmpstr == "TRUE" ||
              tmpstr == "YES") {
            val = 1;
          }
          if (!ESP3DSettings::writeByte(ESP_VERBOSE_BOOT, val)) {
            hasError = true;
            error_msg = "Set delay failed";
            esp3d_log_e("%s", error_msg.c_str());
          }
        } else {
          hasError = true;
          error_msg = "Incorrect verbose setting";
          esp3d_log_e("%s", error_msg.c_str());
        }
      }
    }
    if (!hasError && !hasParam) {
      hasError = true;
      error_msg = "Incorrect parameter";
      esp3d_log_e("%s", error_msg.c_str());
    }
  }

  if (!dispatchAnswer(msg, COMMAND_ID, json, hasError,
                      hasError ? error_msg.c_str() : ok_msg.c_str())) {
    esp3d_log_e("Error sending response to clients");
  }
}
