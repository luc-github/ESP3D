/*
 ESP610.cpp - ESP3D command class

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
#if defined(NOTIFICATION_FEATURE)
#include "../../modules/authentication/authentication_service.h"
#include "../../modules/notifications/notifications_service.h"
#include "../esp3d_commands.h"
#include "../esp3d_settings.h"

#define COMMAND_ID 610
// Set/Get Notification settings
//[ESP610]
// Get will give type and settings
// but not T1/T2 chich are protected
void ESP3DCommands::ESP610(int cmd_params_pos, ESP3DMessage* msg) {
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
  const char* cmdList[] = {"type=", "T1=", "T2=", "TS="};
  uint8_t cmdListSize = sizeof(cmdList) / sizeof(char*);
  const char* notificationStr[] = {"NONE", "PUSHOVER", "EMAIL",
                                   "LINE", "TELEGRAM", "IFTTT", "HOMEASSISTANT"};
  uint8_t notificationStrSize = sizeof(notificationStr) / sizeof(char*);
  const ESP3DSettingIndex settingIndex[] = {
      ESP_NOTIFICATION_TYPE, ESP_NOTIFICATION_TOKEN1, ESP_NOTIFICATION_TOKEN2,
      ESP_NOTIFICATION_SETTINGS};
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
      ok_msg = "{\"type\":\"";
    } else {
      ok_msg = "type: ";
    }
    uint8_t Ntype = ESP3DSettings::readByte(ESP_NOTIFICATION_TYPE);
    ok_msg += Ntype < notificationStrSize ? notificationStr[Ntype] : "Unknown";

    if (json) {
      ok_msg += "\",\"TS\":\"";
    } else {
      ok_msg += ", TS: ";
    }
    ok_msg += ESP3DSettings::readString(ESP_NOTIFICATION_SETTINGS);

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
    bool hasParam = false;
    for (uint8_t i = 0; i < cmdListSize; i++) {
      bool isPresent = false;
      tmpstr = get_param(msg, cmd_params_pos, cmdList[i], &isPresent);
      if (tmpstr.length() != 0 || isPresent) {
        hasParam = true;
        if (i > 0) {
          if (ESP3DSettings::isValidStringSetting(tmpstr.c_str(),
                                                  settingIndex[i])) {
            esp3d_log("Value %s is valid", tmpstr.c_str());
            if (!ESP3DSettings::writeString(settingIndex[i], tmpstr.c_str())) {
              hasError = true;
              error_msg = "Set value failed";
            }
          } else {
            hasError = true;
            error_msg = "Invalid parameter";
          }
        } else {
          tmpstr.toUpperCase();
          int id = -1;
          for (uint8_t j = 0; j < notificationStrSize; j++) {
            if (tmpstr == notificationStr[j]) {
              id = j;
              break;
            }
          }
          if (id != -1) {
            if (!ESP3DSettings::writeByte(ESP_NOTIFICATION_TYPE, id)) {
              hasError = true;
              error_msg = "Set value failed";
            }
          } else {
            hasError = true;
            error_msg = "Invalid parameter";
            esp3d_log_e("%s", error_msg.c_str());
          }
        }
      }
    }
    if (!hasParam && !hasError) {
      hasError = true;
      error_msg = "Invalid parameter";
    }
    if (!hasError) {
      notificationsservice.begin();
    }
  }
  if (!dispatchAnswer(msg, COMMAND_ID, json, hasError,
                      hasError ? error_msg.c_str() : ok_msg.c_str())) {
    esp3d_log_e("Error sending response to clients");
  }
}

#endif  // NOTIFICATION_FEATURE
