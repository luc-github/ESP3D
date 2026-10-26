/*
 ESP109.cpp - ESP3D command class

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
#include "../esp3d_commands.h"
#include "../esp3d_settings.h"

// Get/Set STA rejoin from the setup fallback which can be ON, OFF
//[ESP109]<state> json=<no> pwd=<user/admin password>
#define COMMAND_ID 109
void ESP3DCommands::ESP109(int cmd_params_pos, ESP3DMessage* msg) {
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
  uint8_t byteValue = (uint8_t)-1;
#if defined(AUTHENTICATION_FEATURE)
  if (msg->authentication_level == ESP3DAuthenticationLevel::guest) {
    msg->authentication_level = ESP3DAuthenticationLevel::not_authenticated;
    dispatchAuthenticationError(msg, COMMAND_ID, json);
    return;
  }
#endif  // AUTHENTICATION_FEATURE
  tmpstr = get_clean_param(msg, cmd_params_pos);
  if (tmpstr.length() == 0) {
    byteValue = ESP3DSettings::readByte(ESP_STA_FALLBACK_RETRY);
    if (byteValue == (uint8_t)ESP3DState::off) {
      ok_msg = "OFF";
    } else if (byteValue == (uint8_t)ESP3DState::on) {
      ok_msg = "ON";
    } else {
      ok_msg = "Unknown";
    }
  } else {
#if defined(AUTHENTICATION_FEATURE)
    if (msg->authentication_level != ESP3DAuthenticationLevel::admin) {
      dispatchAuthenticationError(msg, COMMAND_ID, json);
      return;
    }
#endif  // AUTHENTICATION_FEATURE
    if (tmpstr == "OFF") {
      byteValue = (uint8_t)ESP3DState::off;
    } else if (tmpstr == "ON") {
      byteValue = (uint8_t)ESP3DState::on;
    } else {
      byteValue = (uint8_t)-1;  // unknow flag so put outof range value
    }
    esp3d_log("got %s param for a value of %d", tmpstr.c_str(), byteValue);
    // isValidByteSetting() accepts any value for on/off settings, so only
    // ON and OFF are written
    if (byteValue == (uint8_t)ESP3DState::off ||
        byteValue == (uint8_t)ESP3DState::on) {
      if (!ESP3DSettings::writeByte(ESP_STA_FALLBACK_RETRY, byteValue)) {
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
}

#endif  // WIFI_FEATURE