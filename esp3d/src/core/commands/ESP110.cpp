/*
 ESP110.cpp - ESP3D command class

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

#define COMMAND_ID 110
// Set radio state at boot which can be BT, WIFI-STA, WIFI-AP, ETH-STA, OFF
//[ESP110]<state>  json=<no> pwd=<admin password>
void ESP3DCommands::ESP110(int cmd_params_pos, ESP3DMessage* msg) {
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
    byteValue = ESP3DSettings::readByte(ESP_RADIO_MODE);
    if (byteValue == ESP_BT) {
      ok_msg = "BT";
    } else
#if defined(WIFI_FEATURE)
        if (byteValue == ESP_WIFI_AP) {
      ok_msg = "WIFI-AP";
    } else if (byteValue == ESP_WIFI_STA) {
      ok_msg = "WIFI-STA";
    } else if (byteValue == ESP_AP_SETUP) {
      ok_msg = "WIFI-SETUP";
    } else if (byteValue == ESP_ETH_STA) {
      ok_msg = "ETH-STA";
    } else
#endif  // WIFI_FEATURE
      if (byteValue == ESP_NO_NETWORK) {
        ok_msg = "OFF";
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
    tmpstr.toUpperCase();
    if (tmpstr == "BT") {
      byteValue = ESP_BT;
    } else
#if defined(WIFI_FEATURE)
        if (tmpstr == "WIFI-AP") {
      byteValue = ESP_WIFI_AP;
    } else if (tmpstr == "WIFI-STA") {
      byteValue = ESP_WIFI_STA;
    } else if (tmpstr == "WIFI-SETUP") {
      byteValue = ESP_AP_SETUP;
    } else
#endif  // WIFI_FEATURE
#if defined(ETH_FEATURE)
        if (tmpstr == "ETH-STA") {
      byteValue = ESP_ETH_STA;
    } else
#endif  // ETH_FEATURE
      if (tmpstr == "OFF") {
        byteValue = ESP_NO_NETWORK;
      } else {
        byteValue = (uint8_t)-1;  // unknow flag so put outof range value
      }
    esp3d_log("got %s param for a value of %d, is valid %d", tmpstr.c_str(),
              byteValue,
              ESP3DSettings::isValidByteSetting(byteValue, ESP_RADIO_MODE));
    if (ESP3DSettings::isValidByteSetting(byteValue, ESP_RADIO_MODE)) {
      esp3d_log("Value %d is valid", byteValue);
      if (!ESP3DSettings::writeByte(ESP_RADIO_MODE, byteValue)) {
        hasError = true;
        error_msg = "Set value failed";
      } else {
        if (!NetConfig::begin()) {
          hasError = true;
          error_msg = "Cannot setup network";
        }
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
