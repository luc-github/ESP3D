/*
 ESP931.cpp - ESP3D command class

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
#if defined(ESP_SERIAL_BRIDGE_OUTPUT)
#include "../../modules/authentication/authentication_service.h"
#include "../../modules/serial/serial_service.h"
#include "../esp3d_commands.h"
#include "../esp3d_settings.h"

#define COMMAND_ID 931
// Set Serial bridge baudrate
//[ESP931]<baude rate> json=<no> pwd=<admin password>
void ESP3DCommands::ESP931(int cmd_params_pos, ESP3DMessage* msg) {
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
#if defined(AUTHENTICATION_FEATURE)
  if (msg->authentication_level == ESP3DAuthenticationLevel::guest) {
    msg->authentication_level = ESP3DAuthenticationLevel::not_authenticated;
    dispatchAuthenticationError(msg, COMMAND_ID, json);
    return;
  }
#endif  // AUTHENTICATION_FEATURE
  tmpstr = get_clean_param(msg, cmd_params_pos);
  if (tmpstr.length() == 0) {
    const ESP3DSettingDescription* settingPtr =
        ESP3DSettings::getSettingPtr(ESP_SERIAL_BRIDGE_BAUD);
    if (settingPtr) {
      ok_msg = String(ESP3DSettings::readUint32(ESP_SERIAL_BRIDGE_BAUD));
    } else {
      hasError = true;
      error_msg = "This setting is unknown";
    }
  } else {
#if defined(AUTHENTICATION_FEATURE)
    if (msg->authentication_level != ESP3DAuthenticationLevel::admin) {
      dispatchAuthenticationError(msg, COMMAND_ID, json);
      return;
    }
#endif  // AUTHENTICATION_FEATURE
    esp3d_log("got %s param for a value of %s, is valid %d", tmpstr.c_str(),
              tmpstr.c_str(),
              ESP3DSettings::isValidIntegerSetting(tmpstr.toInt(),
                                                   ESP_SERIAL_BRIDGE_BAUD));
    if (ESP3DSettings::isValidIntegerSetting(tmpstr.toInt(),
                                             ESP_SERIAL_BRIDGE_BAUD)) {
      esp3d_log("Value %s is valid", tmpstr.c_str());
      if (!ESP3DSettings::writeUint32(ESP_SERIAL_BRIDGE_BAUD, tmpstr.toInt())) {
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

#endif  // ESP_SERIAL_BRIDGE_OUTPUT
