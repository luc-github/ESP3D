/*
 ESP181.cpp - ESP3D command class

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
#if defined(FTP_FEATURE)
#include "../../modules/authentication/authentication_service.h"
#include "../esp3d_commands.h"
#include "../esp3d_message.h"
#include "../esp3d_settings.h"

#define COMMANDID 181
// Set/Get Ftp ports
//[ESP181]ctrl=<port> active=<port> passive=<port> json=<no> pwd=<admin
// password>
void ESP3DCommands::ESP181(int cmd_params_pos, ESP3DMessage* msg) {
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
      String s = "";
      if (json) {
        s += "{\"ctrl\":\"";
      } else {
        s += "ctrl=";
      }
      s += String(ESP3DSettings::read_uint32(ESP_FTP_CTRL_PORT));
      if (json) {
        s += "\",\"active\":\"";
      } else {
        s += ", active=";
      }
      s += String(ESP3DSettings::read_uint32(ESP_FTP_DATA_ACTIVE_PORT));
      if (json) {
        s += "\",\"passive\":\"";
      } else {
        s += ", passive=";
      }
      s += String(ESP3DSettings::read_uint32(ESP_FTP_DATA_PASSIVE_PORT));
      if (json) {
        s += "\"}";
      }
      response = format_response(COMMANDID, json, true, s.c_str());
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
        parameter = get_param(cmd_params, "ctrl=");
        uint ibuf;
        bool hasParam = false;
        if (parameter.length() > 0) {
          hasParam = true;
          ibuf = parameter.toInt();
          if (!ESP3DSettings::isValidIntegerSetting(ibuf, ESP_FTP_CTRL_PORT)) {
            response =
                format_response(COMMANDID, json, false, "Incorrect ctrl port");
            noError = false;
          } else {
            if (!ESP3DSettings::write_uint32(ESP_FTP_CTRL_PORT, ibuf)) {
              response = format_response(COMMANDID, json, false, "Set failed");
              noError = false;
            }
          }
        }
        if (noError) {
          parameter = get_param(cmd_params, "active=");
          if (parameter.length() > 0) {
            ibuf = parameter.toInt();
            hasParam = true;
            if (!ESP3DSettings::isValidIntegerSetting(
                    ibuf, ESP_FTP_DATA_ACTIVE_PORT)) {
              response = format_response(COMMANDID, json, false,
                                         "Incorrect active port");
              noError = false;
            } else {
              if (!ESP3DSettings::write_uint32(ESP_FTP_DATA_ACTIVE_PORT,
                                               ibuf)) {
                response =
                    format_response(COMMANDID, json, false, "Set failed");
                noError = false;
              }
            }
          }
        }
        if (noError) {
          parameter = get_param(cmd_params, "passive=");
          if (parameter.length() > 0) {
            hasParam = true;
            ibuf = parameter.toInt();
            if (!ESP3DSettings::isValidIntegerSetting(
                    ibuf, ESP_FTP_DATA_PASSIVE_PORT)) {
              response = format_response(COMMANDID, json, false,
                                         "Incorrect passive port");
              noError = false;
            } else {
            }
            if (!ESP3DSettings::write_uint32(ESP_FTP_DATA_PASSIVE_PORT, ibuf)) {
              response = format_response(COMMANDID, json, false, "Set failed");
              noError = false;
            }
          }
        }
        if (noError && !hasParam) {
          response = format_response(
              COMMANDID, json, false,
              "Only ctrl, active and passive settings are supported!");
          noError = false;
        } else {
          if (noError) {
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

#endif  // TELNET_FEATURE
