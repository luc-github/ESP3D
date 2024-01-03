/*
 ESP140.cpp - ESP3D command class

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
#if defined(TIMESTAMP_FEATURE)
#include "../../modules/authentication/authentication_service.h"
#include "../../modules/time/time_service.h"
#include "../esp3d_commands.h"
#include "../esp3d_settings.h"

#define COMMAND_ID 140
// Sync / Set / Get current time
//[ESP140]<SYNC> <srv1=XXXXX> <srv2=XXXXX> <srv3=XXXXX> <tzone=+HH:SS>
//<ntp=YES/NO> <time=YYYY-MM-DDTHH:mm:ss> NOW json=<no> pwd=<admin password>
void ESP3DCommands::ESP140(int cmd_params_pos, ESP3DMessage* msg) {
  ESP3DClientType target = msg->origin;
  ESP3DRequest requestId = msg->request_id;
  (void)requestId;
  msg->target = target;
  msg->origin = ESP3DClientType::command;
  bool hasError = false;
  String error_msg = "Invalid parameters";
  String ok_msg = "ok";
  bool json = hasTag(msg, cmd_params_pos, "json");
  bool sync = hasTag(msg, cmd_params_pos, "sync");
  bool now = hasTag(msg, cmd_params_pos, "now");
  String tmpstr;
  const char* cmdList[] = {"srv1", "srv2", "srv3", "tzone", "ntp", "time"};
  uint8_t cmdListSize = sizeof(cmdList) / sizeof(char*);
  const ESP3DSettingIndex settingIndex[] = {
      ESP_TIME_SERVER1, ESP_TIME_SERVER2,  ESP_TIME_SERVER3,
      ESP_TIME_ZONE,    ESP_INTERNET_TIME, (ESP3DSettingIndex)-1,
  };
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
      ok_msg = "{";
    } else {
      ok_msg = "";
    }
    // no need to show time right now
    for (uint8_t i = 0; i < cmdListSize - 2; i++) {
      if (json) {
        if (i > 0) {
          ok_msg += ",";
        }
        ok_msg += "\"";
      } else {
        if (i > 0) {
          ok_msg += ", ";
        }
      }
      ok_msg += cmdList[i];

      if (json) {
        ok_msg += "\":\"";
      } else {
        ok_msg += ": ";
      }
      ok_msg += ESP3DSettings::readString(settingIndex[i]);
      if (json) {
        ok_msg += "\"";
      } else {
      }
    }
    if (json) {
      ok_msg += ",\"ntp\":\"";
    } else {
      ok_msg += ", ntp: ";
    }
    ok_msg += ESP3DSettings::readByte(ESP_INTERNET_TIME) ? "yes" : "no";
    if (json) {
      ok_msg += "\"";
    } else {
    }

    if (json) {
      ok_msg += "}";
    }
  } else {
    bool hasParam = false;
    String tmpkey;
    for (uint8_t i = 0; i < cmdListSize; i++) {
      tmpkey = cmdList[i];
      tmpkey += "=";
      tmpstr = get_param(msg, cmd_params_pos, tmpkey.c_str());
      if (tmpstr.length() != 0) {
#if defined(AUTHENTICATION_FEATURE)
        if (msg->authentication_level != ESP3DAuthenticationLevel::admin) {
          dispatchAuthenticationError(msg, COMMAND_ID, json);
          return;
        }
#endif  // AUTHENTICATION_FEATURE
        hasParam = true;
        if (settingIndex[i] == (ESP3DSettingIndex)-1) {
          // set time
          if (!timeService.setTime(tmpstr.c_str())) {
            hasError = true;
            error_msg = "Set time failed";
          }
        } else if (settingIndex[i] == ESP_INTERNET_TIME) {
          tmpstr.toLowerCase();
          if (tmpstr == "yes" || tmpstr == "no" || tmpstr == "1" ||
              tmpstr == "0" || tmpstr == "true" || tmpstr == "false") {
            uint8_t val = 0;
            if (tmpstr == "yes" || tmpstr == "1" || tmpstr == "true") {
              val = 1;
            }
            if (!ESP3DSettings::writeByte(settingIndex[i], val)) {
              hasError = true;
              error_msg = "Set value failed";
            }
          } else {
            hasError = true;
            error_msg = "Invalid token parameter";
          }

        } else {
          if (ESP3DSettings::isValidStringSetting(tmpstr.c_str(),
                                                  settingIndex[i])) {
            esp3d_log("Value %s is valid", tmpstr.c_str());
            if (!ESP3DSettings::writeString(settingIndex[i], tmpstr.c_str())) {
              hasError = true;
              error_msg = "Set value failed";
            }
          } else {
            hasError = true;
            error_msg = "Invalid token parameter";
            esp3d_log_e("Value %s is invalid", tmpstr.c_str());
          }
        }
      }
    }
    if (!hasError && now) {
      ok_msg = timeService.getCurrentTime();
      ok_msg += "  (";
      ok_msg += timeService.getTimeZone();
      ok_msg += ")";
      hasParam = true;
    }
    if (!hasError && sync) {
      // apply changes without restarting the board
      timeService.begin();
      hasParam = true;
    }
    if (!hasParam && !hasError) {
      hasError = true;
      error_msg = "Invalid parameter";
    }
  }
  if (!dispatchAnswer(msg, COMMAND_ID, json, hasError,
                      hasError ? error_msg.c_str() : ok_msg.c_str())) {
    esp3d_log_e("Error sending response to clients");
  }
}

#endif  // TIMESTAMP_FEATURE
