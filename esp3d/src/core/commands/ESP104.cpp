/*
 ESP104.cpp - ESP3D command class

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
#if defined(WIFI_FEATURE) || defined(BLUETOOTH_FEATURE) || defined(ETH_FEATURE)
#include "../../modules/authentication/authentication_service.h"
#include "../../modules/network/netconfig.h"
#include "../esp3d_commands.h"
#include "../esp3d_settings.h"

#define COMMAND_ID 104
// Set STA fallback mode state at boot which can be BT, WIFI-SETUP,  OFF
//[ESP104]<state> json=<no> pwd=<admin password>
void ESP3DCommands::ESP104(int cmd_params_pos, ESP3DMessage* msg) {
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
    byteValue = ESP3DSettings::readByte(ESP_STA_FALLBACK_MODE);
#if defined(BLUETOOTH_FEATURE)
    if (byteValue == (uint8_t)ESP_BT) {
      ok_msg = "BT";
    } else
#endif  // BLUETOOTH_FEATURE
#if defined(WIFI_FEATURE)
        if (byteValue == (uint8_t)ESP_AP_SETUP) {
      ok_msg = "CONFIG";
    } else
#endif  // WIFI_FEATURE
      if (byteValue == (uint8_t)ESP_NO_NETWORK) {
        ok_msg = "OFF";
      } else {
        ok_msg = "Unknown";
      }
  } else {
#if defined(BLUETOOTH_FEATURE)
    if (tmpstr == "BT") {
      byteValue = (uint8_t)ESP_BT;
    } else
#endif  // BLUETOOTH_FEATURE
#if defined(WIFI_FEATURE)
        if (tmpstr == "CONFIG") {
      byteValue = (uint8_t)ESP_AP_SETUP;
    } else
#endif  // WIFI_FEATURE
      if (tmpstr == "OFF") {
        byteValue = (uint8_t)ESP_NO_NETWORK;
      } else {
        byteValue = (uint8_t)-1;  // unknow flag so put outof range value
      }
    esp3d_log(
        "got %s param for a value of %d, is valid %d", tmpstr.c_str(),
        byteValue,
        ESP3DSettings::isValidByteSetting(byteValue, ESP_STA_FALLBACK_MODE));
    if (ESP3DSettings::isValidByteSetting(byteValue, ESP_STA_FALLBACK_MODE)) {
      esp3d_log("Value %d is valid", byteValue);
      if (!ESP3DSettings::writeByte(ESP_STA_FALLBACK_MODE, byteValue)) {
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
  String response;
  String parameter;
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
      int8_t wifiMode = ESP3DSettings::readByte(ESP_STA_FALLBACK_MODE);
      if (wifiMode == ESP_NO_NETWORK) {
        response = format_response(COMMANDID, json, true, "OFF");
      } else if (wifiMode == ESP_BT) {
        response = format_response(COMMANDID, json, true, "BT");
      } else if (wifiMode == ESP_AP_SETUP) {
        response = format_response(COMMANDID, json, true, "WIFI-SETUP");
      } else {
        response = format_response(COMMANDID, json, true, "???");
      }
    } else {  // set
#ifdef AUTHENTICATION_FEATURE
      if (auth_type != admin) {
        response = format_response(COMMANDID, json, false,
                                   "Wrong authentication level");
        noError = false;
        errorCode = 401;
      }
#endif  // AUTHENTICATION_FEATURE
      if (noError) {
        parameter.toUpperCase();
        if (!(
#if defined(BLUETOOTH_FEATURE)
                (parameter == "BT") ||
#endif  // BLUETOOTH_FEATURE
#if defined(WIFI_FEATURE)
                (parameter == "WIFI-SETUP") ||
#endif  // WIFI_FEATURE
#if defined(ETH_FEATURE)
                (parameter == "ETH-STA") ||  //(parameter == "ETH-SRV") ||
#endif                                       // ETH_FEATURE
                (parameter == "OFF"))) {

          String res =
              "Only "
#ifdef BLUETOOTH_FEATURE
              "BT or "
#endif  // BLUETOOTH_FEATURE
#ifdef WIFI_FEATURE
              "WIFI-SETUP or "
#endif  // WIFI_FEATURE
              "OFF mode supported!";
          response = format_response(COMMANDID, json, false, res.c_str());
          noError = false;
        }
        if (noError) {
          int8_t bbuf = ESP_NO_NETWORK;
#ifdef WIFI_FEATURE
          if (parameter == "WIFI-SETUP") {
            bbuf = ESP_AP_SETUP;
          }
#endif  // WIFI_FEATURE

#ifdef BLUETOOTH_FEATURE
          if (parameter == "BT") {
            bbuf = ESP_BT;
          }
#endif  // BLUETOOTH_FEATURE
          if (!ESP3DSettings::writeByte(ESP_STA_FALLBACK_MODE, bbuf)) {
            response = format_response(COMMANDID, json, false, "Set failed");
            noError = false;
          } else {
            response = format_response(COMMANDID, json, true, "ok");
          }
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