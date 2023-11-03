/*
 ESP160.cpp - ESP3D command class

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
#if defined(WS_DATA_FEATURE)
#include "../../modules/authentication/authentication_service.h"
#include "../../modules/websocket/websocket_server.h"
#include "../commands.h"
#include "../esp3doutput.h"
#include "../settings_esp3d.h"

#define COMMANDID 160
// Set WebSocket state which can be ON, OFF, CLOSE
//[ESP160]<state> json=<no> pwd=<admin password>
bool Commands::ESP160(const char* cmd_params, level_authenticate_type auth_type,
                      ESP3DOutput* output) {
  bool noError = true;
  bool json = has_tag(cmd_params, "json");
  String response;
  String parameter;
  int errorCode = 200;  // unless it is a server error use 200 as default and
                        // set error in json instead
#ifdef AUTHENTICATION_FEATURE
  if (auth_type == LEVEL_GUEST) {
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
      response = format_response(
          COMMANDID, json, true,
          (Settings_ESP3D::read_byte(ESP_WEBSOCKET_ON) == 0) ? "OFF" : "ON");
    } else {  // set
#ifdef AUTHENTICATION_FEATURE
      if (auth_type != LEVEL_ADMIN) {
        response = format_response(COMMANDID, json, false,
                                   "Wrong authentication level");
        noError = false;
        errorCode = 401;
      }
#endif  // AUTHENTICATION_FEATURE
      if (noError) {
        parameter.toUpperCase();
        if (!((parameter == "ON") || (parameter == "OFF") ||
              (parameter == "CLOSE"))) {
          response = format_response(COMMANDID, json, false,
                                     "Only ON or OFF or CLOSE mode supported!");
          noError = false;
        } else {
          if (parameter == "CLOSE") {
            websocket_data_server.closeClients();
          } else {
            if (!Settings_ESP3D::write_byte(ESP_WEBSOCKET_ON,
                                            (parameter == "ON") ? 1 : 0)) {
              response = format_response(COMMANDID, json, false, "Set failed");
              noError = false;
            }
          }
          if (noError) {
            response = format_response(COMMANDID, json, true, "ok");
          }
        }
      }
    }
  }
  if (json) {
    output->printLN(response.c_str());
  } else {
    if (noError) {
      output->printMSG(response.c_str());
    } else {
      output->printERROR(response.c_str(), errorCode);
    }
  }
  return noError;
}

#endif  // WS_DATA_FEATURE
