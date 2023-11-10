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
#include "../esp3d_message.h"
#include "../esp3d_settings.h"

#define COMMANDID 111
// Get current IP
//[ESP111] [json=no]
bool ESP3DCommands::ESP111(const char* cmd_params,
                           ESP3DAuthenticationLevel auth_type,
                           ESP3D_Message* esp3dmsg) {
  esp3d_log("Client is %d", output ? esp3dmsg->getTarget() : 0);
  (void)auth_type;
  bool noError = true;
  bool json = has_tag(cmd_params, "json");
  String response;
  String parameter = clean_param(get_param(cmd_params, ""));
  if (parameter.length() == 0) {
    response =
        format_response(COMMANDID, json, true, NetConfig::localIP().c_str());
  } else {
    parameter = get_param(cmd_params, "OUTPUT=");
    if (parameter != "PRINTER") {
      response = format_response(COMMANDID, json, false, "Unknown parameter");
    }
  }

  if (noError) {
    parameter = get_param(cmd_params, "OUTPUT=");
    if (json) {
      esp3dmsg->printLN(response.c_str());
    } else {
      esp3dmsg->printMSG(response.c_str());
      if (parameter == "PRINTER") {
        ESP3D_Message printerOutput(ESP_REMOTE_SCREEN_CLIENT,
                                    esp3dmsg->getOrigin());
        printerOutput.printMSG(NetConfig::localIP().c_str());
      }
    }
  } else {
    if (json) {
      esp3dmsg->printLN(response.c_str());
    } else {
      esp3dmsg->printERROR(response.c_str(), 200);
    }
  }
  return noError;
}

#endif  // WIFI_FEATURE
