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
#include "../esp3d_settings.h"

#define COMMAND_ID 201
// Get/Set pin value
//[ESP201]P=<pin> V=<value> PULLUP=YES RAW=YES ANALOG=NO
// ANALOG_RANGE=255]pwd=<admin password> Range can be 255 / 1024 / 2047 / 4095 /
// 8191
void ESP3DCommands::ESP201(int cmd_params_pos, ESP3DMessage* msg) {
  ESP3DClientType target = msg->origin;
  ESP3DRequest requestId = msg->request_id;
  (void)requestId;
  msg->target = target;
  msg->origin = ESP3DClientType::command;
  bool hasError = false;
  String error_msg = "Invalid parameters";
  String ok_msg = "ok";
  String P = get_param(msg, cmd_params_pos, "P=");
  String V = get_param(msg, cmd_params_pos, "V=");
  bool isPULLUP = hasTag(msg, cmd_params_pos, "PULLUP");
  bool isRAW = hasTag(msg, cmd_params_pos, "RAW");
  bool isANALOG = hasTag(msg, cmd_params_pos, "ANALOG");
  int analog_range = 255;
  String ANALOG_RANGE = get_param(msg, cmd_params_pos, "ANALOG_RANGE=");
  if (ANALOG_RANGE.length() > 0) {
    analog_range = ANALOG_RANGE.toInt();
  }
  bool found = false;
  get_param(msg, cmd_params_pos, "PULLUP=", &found);
  if (!found && !isPULLUP) {
    esp3d_log("Missing PULLUP parameter");
    isPULLUP = true;  // default value
  }
  found = false;
  int value = 0;
  get_param(msg, cmd_params_pos, "RAW=", &found);
  if (!found && !isRAW) {
    esp3d_log("Missing RAW parameter");
    isRAW = true;  // default value
  }
  found = false;
  get_param(msg, cmd_params_pos, "ANALOG=", &found);
  if (!found && !isANALOG) {
    esp3d_log("Missing ANALOG parameter");
    isANALOG = false;  // default value
  }
  bool json = hasTag(msg, cmd_params_pos, "json");
  String tmpstr;
#if defined(AUTHENTICATION_FEATURE)
  if (msg->authentication_level == ESP3DAuthenticationLevel::guest) {
    msg->authentication_level = ESP3DAuthenticationLevel::not_authenticated;
    dispatchAuthenticationError(msg, COMMAND_ID, json);
    return;
  }
#endif  // AUTHENTICATION_FEATURE
  if (P.length() == 0) {
    hasError = true;
    error_msg = "Missing pin";
    esp3d_log_e("%s", error_msg.c_str());
  } else {
    int pin = P.toInt();
    esp3d_log("P=%d V=%s RAW=%d PULLUP=%d ANALOG=%d RANGE=%d", pin, V.c_str(),
              isRAW, isPULLUP, isANALOG, analog_range);
    if (!ESP3DHal::is_pin_usable(pin)) {
      hasError = true;
      error_msg = "Invalid pin";
      esp3d_log_e("%s", error_msg.c_str());
    } else {
      if (V.length() > 0) {  // Set
        value = V.toInt();
        ESP3DHal::pinMode(pin, OUTPUT);
        if (isANALOG) {
          if ((value >= 0) || (value <= analog_range + 1)) {
            ESP3DHal::analogRange(analog_range);
            ESP3DHal::analogWriteFreq(1000);
            analogWrite(pin, value);
          } else {
            hasError = true;
            error_msg = "Invalid value";
            esp3d_log_e("%s", error_msg.c_str());
          }
        } else {
          if ((value == 0) || (value == 1)) {
            digitalWrite(pin, (value == 0) ? LOW : HIGH);
          } else {
            hasError = true;
            error_msg = "Invalid value";
            esp3d_log_e("%s", error_msg.c_str());
          }
        }
      } else {  // Get
        if (isANALOG) {
          value = ESP3DHal::analogRead(pin);
        } else {
          if (!isRAW) {
            if (isPULLUP) {
              ESP3DHal::pinMode(pin, INPUT_PULLUP);
            } else {
              ESP3DHal::pinMode(pin, INPUT);
            }
          }
          value = digitalRead(pin);
          ok_msg = String(value);
        }
      }
    }
  }

  if (!dispatchAnswer(msg, COMMAND_ID, json, hasError,
                      hasError ? error_msg.c_str() : ok_msg.c_str())) {
    esp3d_log_e("Error sending response to clients");
  }
}

#endif  // DIRECT_PIN_FEATURE
