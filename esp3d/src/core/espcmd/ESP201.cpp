/*
 ESP201.cpp - ESP3D command class

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
#if defined(DIRECT_PIN_FEATURE)
#include "../../modules/authentication/authentication_service.h"
#include "../esp3d_commands.h"
#include "../esp3d_hal.h"
#include "../esp3d_message.h"
#include "../esp3d_settings.h"

#define COMMANDID 201
// Get/Set pin value
//[ESP201]P<pin> V<value> [PULLUP=YES RAW=YES ANALOG=NO
// ANALOG_RANGE=255]pwd=<admin password> Range can be 255 / 1024 / 2047 / 4095 /
// 8191
bool ESP3DCommands::ESP201(const char* cmd_params,
                           ESP3DAuthenticationLevel auth_type,
                           ESP3D_Message* esp3dmsg) {
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
    // check if have pin
    parameter = get_param(cmd_params, "P=");
    esp3d_log("Pin %s", parameter.c_str());
    if (parameter.length() == 0) {
      response = format_response(COMMANDID, json, false, "Missing pin");
      noError = false;
    } else {
      int pin = parameter.toInt();
      // check pin is valid and not serial used pins
      if (ESP3DHal::is_pin_usable(pin)) {
        bool isdigital = true;
        parameter = get_param(cmd_params, "ANALOG=");
        if (parameter == "YES") {
          esp3d_log("Set as analog");
          isdigital = false;
        }
        // check if is set or get
        parameter = get_param(cmd_params, "V=");
        // it is a get
        if (parameter.length() == 0) {
          // this is to not set pin mode
          int value = 0;
          if (isdigital) {
            parameter = get_param(cmd_params, "RAW=");
            if (parameter != "YES") {
              parameter = get_param(cmd_params, "PULLUP=");
              if (parameter != "YES") {
                ESP3DHal::pinMode(pin, INPUT);
              } else {
                ESP3DHal::pinMode(pin, INPUT_PULLUP);
              }
            }
            value = digitalRead(pin);
          } else {
            value = ESP3DHal::analogRead(pin);
          }
          response =
              format_response(COMMANDID, json, true, String(value).c_str());
        } else {
          // it is a set
          int value = parameter.toInt();
          ESP3DHal::pinMode(pin, OUTPUT);
          if (isdigital) {
            // verify it is a '0' or a '1'
            if ((value == 0) || (value == 1)) {
              digitalWrite(pin, (value == 0) ? LOW : HIGH);
              response = format_response(COMMANDID, json, true, "ok");
            } else {
              response =
                  format_response(COMMANDID, json, false, "Invalid value");
              noError = false;
            }
          } else {
            int analog_range = 255;
            parameter = get_param(cmd_params, "ANALOG_RANGE=");
            if (parameter.length() > 0) {
              analog_range = parameter.toInt();
            }
            if ((value >= 0) || (value <= analog_range + 1)) {
              ESP3DHal::analogRange(analog_range);
              ESP3DHal::analogWriteFreq(1000);
              analogWrite(pin, value);
              response =
                  format_response(COMMANDID, json, false, "Invalid value");
              noError = false;
            } else {
              response =
                  format_response(COMMANDID, json, false, "Invalid parameter");
              noError = false;
            }
          }
        }
      } else {
        response = format_response(COMMANDID, json, false, "Invalid pin");
        noError = false;
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
  return noError;
}

#endif  // DIRECT_PIN_FEATURE
