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
                                   "LINE", "TELEGRAM", "IFTTT"};
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
  /*
  bool noError = true;
  bool json = has_tag(cmd_params, "json");
  String response;
  String parameter;
  bool hasParam = false;
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
      uint8_t Ntype = ESP3DSettings::readByte(ESP_NOTIFICATION_TYPE);
      String tmp;
      if (json) {
        tmp = "{\"type\":\"";
      } else {
        tmp = "type=";
      }
      switch (Ntype) {
        case ESP_PUSHOVER_NOTIFICATION:
          tmp += "PUSHOVER";
          break;
        case ESP_EMAIL_NOTIFICATION:
          tmp += "EMAIL";
          break;
        case ESP_LINE_NOTIFICATION:
          tmp += "LINE";
          break;
        case ESP_TELEGRAM_NOTIFICATION:
          tmp += "TELEGRAM";
          break;
        case ESP_IFTTT_NOTIFICATION:
          tmp += "IFTTT";
          break;
        default:
          tmp += "NONE";
      }
      if (json) {
        tmp += "\"";
      }
      String ts = ESP3DSettings::readString(ESP_NOTIFICATION_SETTINGS);
      if (ts.length() > 0 && ts != "NONE") {
        if (json) {
          tmp += ",\"TS\":\"";
        } else {
          tmp += ", TS=";
        }
        tmp += ts;
        if (json) {
          tmp += "\"}";
        }
      }
      if (json) {
        tmp += "}";
      }
      response = format_response(COMMANDID, json, true, tmp.c_str());
    } else {
      // type
      parameter = get_param(cmd_params, "type=");
      if (parameter.length() > 0) {
        hasParam = true;
        uint8_t Ntype;
        parameter.toUpperCase();
        if (parameter == "NONE") {
          Ntype = 0;
        } else if (parameter == "PUSHOVER") {
          Ntype = ESP_PUSHOVER_NOTIFICATION;
        } else if (parameter == "EMAIL") {
          Ntype = ESP_EMAIL_NOTIFICATION;
        } else if (parameter == "LINE") {
          Ntype = ESP_LINE_NOTIFICATION;
        } else if (parameter == "TELEGRAM") {
          Ntype = ESP_TELEGRAM_NOTIFICATION;
        } else if (parameter == "IFTTT") {
          Ntype = ESP_IFTTT_NOTIFICATION;
        } else {
          response = format_response(
              COMMANDID, json, false,
              "Only NONE, PUSHOVER, EMAIL, LINE, IFTTT are supported");
          noError = false;
        }
        if (noError) {
          if (!ESP3DSettings::writeByte(ESP_NOTIFICATION_TYPE, Ntype)) {
            response = format_response(COMMANDID, json, false, "Set failed");
            noError = false;
          }
        }
      }
      // Settings
      if (noError) {
        parameter = get_param(cmd_params, "TS=");
        if (parameter.length() > 0) {
          hasParam = true;
          if (!ESP3DSettings::writeString(ESP_NOTIFICATION_SETTINGS,
                                           parameter.c_str())) {
            response = format_response(COMMANDID, json, false, "Set TS failed");
            noError = false;
          }
        }
      }
      // Token1
      if (noError) {
        parameter = get_param(cmd_params, "T1=");
        if (parameter.length() > 0) {
          hasParam = true;
          if (!ESP3DSettings::writeString(ESP_NOTIFICATION_TOKEN1,
                                           parameter.c_str())) {
            response = format_response(COMMANDID, json, false, "Set T1 failed");
            noError = false;
          }
        }
      }
      // Token2
      if (noError) {
        parameter = get_param(cmd_params, "T2=");
        if (parameter.length() > 0) {
          hasParam = true;
          if (!ESP3DSettings::writeString(ESP_NOTIFICATION_TOKEN2,
                                           parameter.c_str())) {
            response = format_response(COMMANDID, json, false, "Set T2 failed");
            noError = false;
          } else {
            response = true;
          }
        }
      }
      if (noError) {
        if (hasParam) {
          // Restart service
          notificationsservice.begin();
          response = format_response(COMMANDID, json, true, "ok");
        } else {
          response = format_response(
              COMMANDID, json, false,
              "Only type, T1, T2 and TS not empty are supported");
          noError = false;
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

#endif  // NOTIFICATION_FEATURE
