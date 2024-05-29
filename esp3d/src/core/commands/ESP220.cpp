/*
 ESP220.cpp - ESP3D command class

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
#include "../../modules/authentication/authentication_service.h"
#include "../esp3d_commands.h"
#include "../esp3d_settings.h"

#define COMMAND_ID 220

// Get ESP pins definition
// output is JSON or plain text according parameter
//[ESP220]json=<no>
void ESP3DCommands::ESP220(int cmd_params_pos, ESP3DMessage* msg) {
  ESP3DClientType target = msg->origin;
  ESP3DRequest requestId = msg->request_id;
  (void)requestId;
  msg->target = target;
  msg->origin = ESP3DClientType::command;
  bool hasError = false;
  String error_msg = "Invalid parameters";
  String ok_msg = "ok";
  bool json = hasTag(msg, cmd_params_pos, "json");
#if defined(AUTHENTICATION_FEATURE)
  if (msg->authentication_level == ESP3DAuthenticationLevel::guest) {
    msg->authentication_level = ESP3DAuthenticationLevel::not_authenticated;
    dispatchAuthenticationError(msg, COMMAND_ID, json);
    return;
  }
#endif  // AUTHENTICATION_FEATURE
  String tmpstr = get_clean_param(msg, cmd_params_pos);
  if (tmpstr.length() != 0) {
    hasError = true;
    error_msg = "This command doesn't take parameters";
    esp3d_log_e("%s", error_msg.c_str());
  } else {
    ok_msg = "";
    if (json) {
      ok_msg = "{\"cmd\":\"220\",\"status\":\"ok\",\"data\":[";
    } else {
      ok_msg = "Pins:\n";
    }
    msg->type = ESP3DMessageType::head;
    if (!dispatch(msg, ok_msg.c_str())) {
      esp3d_log_e("Error sending response to clients");
      return;
    }
    bool isFirst = true;
    bool hasPin = false;
#if defined(SD_DEVICE) && SD_DEVICE != ESP_SDIO

    //   SD CS
    tmpstr = String(ESP_SD_CS_PIN == -1 ? SS : ESP_SD_CS_PIN);
    if (!dispatchIdValue(json, " SD CS", tmpstr.c_str(), target, requestId,
                         isFirst)) {
      return;
    }
    isFirst = false;
    hasPin = true;
    //   SD MOSI
    tmpstr = String(ESP_SD_MOSI_PIN == -1 ? MOSI : ESP_SD_MOSI_PIN);
    if (!dispatchIdValue(json, " SD MOSI", tmpstr.c_str(), target, requestId,
                         isFirst)) {
      return;
    }

    //   SD MISO
    tmpstr = String(ESP_SD_MISO_PIN == -1 ? MISO : ESP_SD_MISO_PIN);
    if (!dispatchIdValue(json, " SD MISO", tmpstr.c_str(), target, requestId,
                         isFirst)) {
      return;
    }

    //   SD SCK
    tmpstr = String(ESP_SD_SCK_PIN == -1 ? SCK : ESP_SD_SCK_PIN);
    if (!dispatchIdValue(json, " SD SCK", tmpstr.c_str(), target, requestId,
                         isFirst)) {
      return;
    }

    //   SD DETECT
    tmpstr = String(ESP_SD_DETECT_PIN);
    if (!dispatchIdValue(json, " SD DETECT", tmpstr.c_str(), target, requestId,
                         isFirst)) {
      return;
    }
#if ESP_SD_DETECT_PIN != -1
    //   SD DETECT STATE
    tmpstr = String(ESP_SD_DETECT_VALUE);
    if (!dispatchIdValue(json, " SD DETECT STATE", tmpstr.c_str(), target,
                         requestId, isFirst)) {
      return;
    }
#endif  // ESP_SD_DETECT_PIN !=-1
#endif  // defined(SD_DEVICE) && SD_DEVICE != ESP_SDIO
#if SD_DEVICE_CONNECTION == ESP_SHARED_SD && defined(ESP_FLAG_SHARED_SD_PIN)
    //   SD SWITCH
    tmpstr = String(ESP_FLAG_SHARED_SD_PIN);
    if (!dispatchIdValue(json, " SD SWITCH", tmpstr.c_str(), target, requestId,
                         isFirst)) {
      return;
    }
    hasPin = true;
#endif  // SD_DEVICE_CONNECTION == ESP_SHARED_SD
#ifdef BUZZER_DEVICE
    //   BUZZER
    tmpstr = String(ESP3D_BUZZER_PIN);
    if (!dispatchIdValue(json, " BUZZER", tmpstr.c_str(), target, requestId,
                         isFirst)) {
      return;
    }
    hasPin = true;
#endif  // BUZZER_DEVICE
#if defined(PIN_RESET_FEATURE) && defined(ESP3D_RESET_PIN) && \
    ESP3D_RESET_PIN != -1
    //   RESET
    tmpstr = String(ESP3D_RESET_PIN);
    if (!dispatchIdValue(json, " RESET", tmpstr.c_str(), target, requestId,
                         isFirst)) {
      return;
    }
    hasPin = true;
#endif  // PIN_RESET_FEATURE
#if defined(SENSOR_DEVICE) && defined(ESP3D_SENSOR_PIN ) 
    //   SENSOR
    tmpstr = String(ESP3D_SENSOR_PIN);
    if (!dispatchIdValue(json, " SENSOR", tmpstr.c_str(), target, requestId,
                         isFirst)) {
      return;
    }
    hasPin = true;
#endif  // SENSOR_DEVICE
#ifdef DISPLAY_DEVICE
#if (DISPLAY_DEVICE == OLED_I2C_SSD1306) || \
    (DISPLAY_DEVICE == OLED_I2C_SSDSH1106)
    //   SDA
    tmpstr = String(ESP_SDA_PIN);
    if (!dispatchIdValue(json, " SDA", tmpstr.c_str(), target, requestId,
                         isFirst)) {
      return;
    }
    //   SCL
    tmpstr = String(ESP_SCL_PIN);
    if (!dispatchIdValue(json, " SCL", tmpstr.c_str(), target, requestId,
                         isFirst)) {
      return;
    }
    hasPin = true;
#endif  //(DISPLAY_DEVICE == OLED_I2C_SSD1306) || (DISPLAY_DEVICE ==
        // OLED_I2C_SSDSH1106)
#endif  // DISPLAY_DEVICE
    if (!hasPin) {
      tmpstr = "NO PIN";
      if (!dispatchIdValue(json, " PIN", tmpstr.c_str(), target, requestId,
                           isFirst)) {
        return;
      }
    }
    if (json) {
      if (!dispatch("]}", target, requestId, ESP3DMessageType::tail)) {
        esp3d_log_e("Error sending answer to clients");
      }
    } else {
      {
        tmpstr = "ok\n";
        if (!dispatch(tmpstr.c_str(), target, requestId,
                      ESP3DMessageType::tail)) {
          esp3d_log_e("Error sending answer to clients");
        }
      }
    }
    return;
  }
  if (!dispatchAnswer(msg, COMMAND_ID, json, hasError,
                      hasError ? error_msg.c_str() : ok_msg.c_str())) {
    esp3d_log_e("Error sending response to clients");
  }
}
