/*
 ESP210.cpp - ESP3D command class

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
#if defined(SENSOR_DEVICE)
#include "../../modules/authentication/authentication_service.h"
#include "../../modules/sensor/sensor.h"
#include "../esp3d_commands.h"
#include "../esp3d_settings.h"

#define COMMAND_ID 210
// Get Sensor Value / type/Set Sensor type
//[ESP210]<type=NONE/xxx> <interval=XXX in millisec> json=<no> pwd=<admin
// password>
void ESP3DCommands::ESP210(int cmd_params_pos, ESP3DMessage* msg) {
  ESP3DClientType target = msg->origin;
  ESP3DRequest requestId = msg->request_id;
  (void)requestId;
  msg->target = target;
  msg->origin = ESP3DClientType::command;
  bool hasError = false;
  String error_msg = "Invalid parameters";
  String ok_msg = "ok";
  String stype = get_param(msg, cmd_params_pos, "type=");
  String sint = get_param(msg, cmd_params_pos, "interval=");
  bool json = hasTag(msg, cmd_params_pos, "json");
  String tmpstr;
#if defined(AUTHENTICATION_FEATURE)
  if (msg->authentication_level == ESP3DAuthenticationLevel::guest) {
    msg->authentication_level = ESP3DAuthenticationLevel::not_authenticated;
    dispatchAuthenticationError(msg, COMMAND_ID, json);
    return;
  }
#endif                                              // AUTHENTICATION_FEATURE
  if (stype.length() == 0 && sint.length() == 0) {  // Get
    String s;
    if (json) {
      s = "{\"type\":\"";
    } else {
      s = "type=";
    }
    if (esp3d_sensor.started()) {
      s += esp3d_sensor.GetCurrentModelString();
    } else {
      s += "NONE";
    }
    if (json) {
      s += ":\",\"interval\":";
    } else {
      s += ", interval=";
    }
    s += esp3d_sensor.interval();
    if (json) {
      s += ":\",\"value\":";
    } else {
      s += "ms, value=";
    }
    s += esp3d_sensor.GetData();
    if (json) {
      s += "}";
    }
    ok_msg = s;
  } else {
#if defined(AUTHENTICATION_FEATURE)
    if (msg->authentication_level != ESP3DAuthenticationLevel::admin) {
      dispatchAuthenticationError(msg, COMMAND_ID, json);
      return;
    }
#endif  // AUTHENTICATION_FEATURE
    // Set
    if (stype.length() != 0) {
      stype.toUpperCase();
      int8_t v = -1;
      if (stype == "NONE") {
        v = 0;
      } else if (esp3d_sensor.isModelValid(
                     esp3d_sensor.getIDFromString(stype.c_str()))) {
        v = esp3d_sensor.getIDFromString(stype.c_str());
      } else {
        hasError = true;
        error_msg = "Invalid parameter";
        esp3d_log_e("%s", error_msg.c_str());
      }
      if (v != -1) {
        if (!ESP3DSettings::writeByte(ESP_SENSOR_TYPE, v)) {
          hasError = true;
          error_msg = "Set failed";
          esp3d_log_e("%s", error_msg.c_str());
        }
        if (!esp3d_sensor.begin()) {
          hasError = true;
          error_msg = "Starting failed";
          esp3d_log_e("%s", error_msg.c_str());
        }
      } else {
        hasError = true;
        error_msg = "Invalid type";
        esp3d_log_e("%s", error_msg.c_str());
      }
    }
    if (!hasError && sint.length() != 0 &&
        ESP3DSettings::isValidIntegerSetting(sint.toInt(),
                                             ESP_SENSOR_INTERVAL)) {
      if (!ESP3DSettings::writeUint32(ESP_SENSOR_INTERVAL, sint.toInt())) {
        hasError = true;
        error_msg = "Set failed";
        esp3d_log_e("%s", error_msg.c_str());
      } else {
        esp3d_sensor.setInterval(sint.toInt());
      }
    }
  }
  if (!dispatchAnswer(msg, COMMAND_ID, json, hasError,
                      hasError ? error_msg.c_str() : ok_msg.c_str())) {
    esp3d_log_e("Error sending response to clients");
  }
}
#endif  // SENSOR_DEVICE
