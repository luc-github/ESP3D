/*
 ESP111.cpp - ESP3D command class

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
// #define ESP_LOG_FEATURE LOG_OUTPUT_SERIAL0
#include "../../include/esp3d_config.h"
#if defined(WIFI_FEATURE) || defined(ETH_FEATURE)
#include "../../modules/authentication/authentication_service.h"
#include "../../modules/network/netconfig.h"
#include "../esp3d_commands.h"
#include "../esp3d_settings.h"

#define COMMAND_ID 111
// Get current IP
//[ESP111] OUTPUT=PRINTER ALL [json=no]
void ESP3DCommands::ESP111(int cmd_params_pos, ESP3DMessage* msg) {
  ESP3DClientType target = msg->origin;
  ESP3DRequest requestId = msg->request_id;
  (void)requestId;
  msg->target = target;
  msg->origin = ESP3DClientType::command;
  bool hasError = false;
  String error_msg = "Invalid parameters";
  String ok_msg = "ok";
  bool json = hasTag(msg, cmd_params_pos, "json");
  bool respjson = json;
  bool showAll = hasTag(msg, cmd_params_pos, "ALL");
  String tmpstr;
#if defined(AUTHENTICATION_FEATURE)
  if (msg->authentication_level == ESP3DAuthenticationLevel::guest) {
    msg->authentication_level = ESP3DAuthenticationLevel::not_authenticated;
    dispatchAuthenticationError(msg, COMMAND_ID, json);
    return;
  }
#endif  // AUTHENTICATION_FEATURE
  tmpstr = get_param(msg, cmd_params_pos, "OUTPUT=");
  if (tmpstr == "PRINTER") {
    msg->target = ESP3DClientType::remote_screen;
    json = false;
  }
  if (!showAll) {
    ok_msg = NetConfig::localIP().c_str();
  } else {
    if (json) {
      ok_msg = "{\"ip\":\"";
    } else {
      ok_msg = "IP: ";
    }
    ok_msg += NetConfig::localIP().c_str();
    if (json) {
      ok_msg += "\",\"gw\":\"";
    } else {
      ok_msg += "\nGW: ";
    }
    ok_msg += NetConfig::localGW().c_str();
    if (json) {
      ok_msg += "\",\"msk\":\"";
    } else {
      ok_msg += "\nMSK: ";
    }
    ok_msg += NetConfig::localMSK().c_str();
    if (json) {
      ok_msg += "\",\"dns\":\"";
    } else {
      ok_msg += "\nDNS: ";
    }
    ok_msg += NetConfig::localDNS().c_str();
    if (json) {
      ok_msg += "\"}";
    }
  }

  if (msg->target == ESP3DClientType::remote_screen) {
    if (respjson) {
      tmpstr = "{\"cmd\":\"111\",\"status\":\"ok\",\"data\":\"ok\"}";
    } else {
      tmpstr = "ok\n";
    }
    // send response to original client
    if (!dispatch(tmpstr.c_str(), target, msg->request_id,
                  ESP3DMessageType::unique)) {
      esp3d_log_e("Error sending response to original client");
    }
  }

  // Printer does not support json just normal serial
  if (!dispatchAnswer(msg, COMMAND_ID, json, hasError,
                      hasError ? error_msg.c_str() : ok_msg.c_str())) {
    esp3d_log_e("Error sending response to clients");
  }
}

#endif  // WIFI_FEATURE
