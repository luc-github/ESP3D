/*
 ESP401.cpp - ESP3D command class

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

#define COMMAND_ID 401
#if COMMUNICATION_PROTOCOL != SOCKET_SERIAL || defined(ESP_SERIAL_BRIDGE_OUTPUT)
#include "../../modules/serial/serial_service.h"
#endif  // COMMUNICATION_PROTOCOL != SOCKET_SERIAL
#ifdef SENSOR_DEVICE
#include "../../modules/sensor/sensor.h"
#endif  // SENSOR_DEVICE
#ifdef BUZZER_DEVICE
#include "../../modules/buzzer/buzzer.h"
#endif  // BUZZER_DEVICE
#ifdef TIMESTAMP_FEATURE
#include "../../modules/time/time_service.h"
#endif  // TIMESTAMP_FEATURE
#ifdef NOTIFICATION_FEATURE
#include "../../modules/notifications/notifications_service.h"
#endif  // NOTIFICATION_FEATURE
#ifdef SD_DEVICE
#include "../../modules/filesystem/esp_sd.h"
#endif  // SD_DEVICE
// Set EEPROM setting
//[ESP401]P=<position> T=<type> V=<value> json=<no> pwd=<user/admin password>
void ESP3DCommands::ESP401(int cmd_params_pos, ESP3DMessage* msg) {
  ESP3DClientType target = msg->origin;
  ESP3DRequest requestId = msg->request_id;
  (void)requestId;
  msg->target = target;
  msg->origin = ESP3DClientType::command;
  bool hasError = false;
  String error_msg = "Invalid parameters";
  String ok_msg = "ok";
  String spos = get_param(msg, cmd_params_pos, "P=");
  bool foundV = false;
  String sval = get_param(msg, cmd_params_pos, "V=", &foundV);
  String styp = get_param(msg, cmd_params_pos, "T=");

  bool json = hasTag(msg, cmd_params_pos, "json");
  String tmpstr;
#if defined(AUTHENTICATION_FEATURE)
  if (msg->authentication_level != ESP3DAuthenticationLevel::admin) {
    msg->authentication_level = ESP3DAuthenticationLevel::not_authenticated;
    dispatchAuthenticationError(msg, COMMAND_ID, json);
    return;
  }
#endif  // AUTHENTICATION_FEATURE

  if (spos.length() == 0) {
    error_msg = "Invalid parameter P";
    hasError = true;
  } else if (styp.length() == 0) {
    error_msg = "Invalid parameter T";
    hasError = true;
  } else if (sval.length() == 0 && !foundV) {
    error_msg = "Invalid parameter V";
    hasError = true;
  } else {
    if (!(styp == "B" || styp == "S" || styp == "A" || styp == "I")) {
      error_msg = "Invalid value for T";
      hasError = true;
    }
  }
  if (!hasError) {
    switch (styp[0]) {
      case 'B':  // Byte value
        if (ESP3DSettings::isValidByteSetting((uint8_t)sval.toInt(),
                                              spos.toInt())) {
          if (!ESP3DSettings::writeByte(spos.toInt(), (uint8_t)sval.toInt())) {
            error_msg = "Set failed";
            hasError = true;
            esp3d_log_e("Set failed");
          }
        } else {
          error_msg = "Invalid value for T";
          hasError = true;
          esp3d_log_e("Invalid value for T");
        }
        break;
      case 'I':  // Integer value
        if (ESP3DSettings::isValidIntegerSetting(sval.toInt(), spos.toInt())) {
          if (!ESP3DSettings::writeUint32(spos.toInt(), sval.toInt())) {
            error_msg = "Set failed";
            hasError = true;
            esp3d_log_e("Set failed");
          }
        } else {
          error_msg = "Invalid value for T";
          hasError = true;
          esp3d_log_e("Invalid value for T");
        }
        break;
      case 'S':  // String value
        if (ESP3DSettings::isValidStringSetting(sval.c_str(), spos.toInt())) {
          if (!ESP3DSettings::writeString(spos.toInt(), sval.c_str())) {
            error_msg = "Set failed";
            hasError = true;
            esp3d_log_e("Set failed");
          }
        } else {
          error_msg = "Invalid value for T";
          hasError = true;
          esp3d_log_e("Invalid value for T");
        }
        break;
      case 'A':  // IP address
        if (ESP3DSettings::isValidIPStringSetting(sval.c_str(), spos.toInt())) {
          if (!ESP3DSettings::writeIPString(spos.toInt(), sval.c_str())) {
            error_msg = "Set failed";
            hasError = true;
          }
        } else {
          error_msg = "Invalid value for T";
          hasError = true;
          esp3d_log_e("Invalid value for T");
        }
        break;
      default:
        error_msg = "Invalid value for T";
        hasError = true;
        break;
    }

    if (!hasError) {
      switch (spos.toInt()) {
#if defined(ESP_SERIAL_BRIDGE_OUTPUT)
        case ESP_SERIAL_BRIDGE_ON:
          if (!serial_bridge_service.started()) {
            serial_bridge_service.begin(ESP_SERIAL_BRIDGE_OUTPUT);
          }
          break;
#endif  // ESP_SERIAL_BRIDGE_OUTPUT
        case ESP_VERBOSE_BOOT:
          ESP3DSettings::isVerboseBoot(true);
          break;
        case ESP_TARGET_FW:
          ESP3DSettings::GetFirmwareTarget(true);
          break;
#if COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL == MKS_SERIAL
        case ESP_SECURE_SERIAL:
          esp3d_serial_service.setParameters();
          break;
#endif  // COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL ==
        // MKS_SERIAL
#ifdef AUTHENTICATION_FEATURE
        case ESP_SESSION_TIMEOUT:
          AuthenticationService::setSessionTimeout(1000 * 60 * sval.toInt());
          break;
#endif  // AUTHENTICATION_FEATURE
#ifdef SD_DEVICE
        case ESP_SD_SPEED_DIV:
          ESP_SD::setSPISpeedDivider(sval.toInt());
          break;
#endif  // SD_DEVICE
#ifdef TIMESTAMP_FEATURE
        case ESP_INTERNET_TIME:
          timeService.begin();
          break;
#endif  // TIMESTAMP_FEATURE
#ifdef NOTIFICATION_FEATURE
        case ESP_AUTO_NOTIFICATION:
          notificationsservice.setAutonotification((sval.toInt() == 0) ? false
                                                                       : true);
          break;
#endif  // NOTIFICATION_FEATURE
#ifdef SENSOR_DEVICE
        case ESP_SENSOR_TYPE:
          esp3d_sensor.begin();
          break;
#endif  // SENSOR_DEVICE
#ifdef BUZZER_DEVICE
        case ESP_BUZZER:
          if (sval.toInt() == 1) {
            esp3d_buzzer.begin();
          } else if (sval.toInt() == 0) {
            esp3d_buzzer.end();
          }
          break;
#endif  // BUZZER_DEVICE
#ifdef SENSOR_DEVICE
        case ESP_SENSOR_INTERVAL:
          esp3d_sensor.setInterval(sval.toInt());
          break;
#endif  // SENSOR_DEVICE
#if COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL == MKS_SERIAL
        case ESP_BAUD_RATE:
          esp3d_serial_service.updateBaudRate(sval.toInt());
          break;
#endif  // COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL ==
        // MKS_SERIAL
#if defined(ESP_SERIAL_BRIDGE_OUTPUT)
        case ESP_SERIAL_BRIDGE_BAUD:
          serial_bridge_service.updateBaudRate(sval.toInt());
          break;
#endif  // defined(ESP_SERIAL_BRIDGE_OUTPUT)
#ifdef AUTHENTICATION_FEATURE
        case ESP_ADMIN_PWD:
        case ESP_USER_PWD:
          AuthenticationService::update();
          break;
#endif  // AUTHENTICATION_FEATURE
        default:
          break;
      }
    }
  }

  if (!hasError) {
    if (json) {
      ok_msg = spos;
    } else {
      ok_msg = "ok";
    }
  } else {
    if (json) {
      String tmp = "{\"error\":\"";
      tmp += error_msg;
      tmp += "\"";
      if (spos.length() > 0) {
        tmp += ",\"position\":\"";
        tmp += spos;
        tmp += "\"";
      }
      tmp += "}";
      error_msg = tmp;
    } else {
      error_msg += spos.length() > 0 ? " for P=" + spos : "";
    }
  }
  if (!dispatchAnswer(msg, COMMAND_ID, json, hasError,
                      hasError ? error_msg.c_str() : ok_msg.c_str())) {
    esp3d_log_e("Error sending response to clients");
  }
}
