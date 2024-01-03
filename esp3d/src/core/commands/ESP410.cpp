/*
 ESP410.cpp - ESP3D command class

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
#if defined(WIFI_FEATURE)
#include "../../modules/authentication/authentication_service.h"
#include "../../modules/wifi/wificonfig.h"
#include "../esp3d_commands.h"
#include "../esp3d_settings.h"
#include "../esp3d_string.h"

// Get available AP list (limited to 30)
// output is JSON or plain text according parameter
//[ESP410]json=<no>
#define COMMAND_ID 410
void ESP3DCommands::ESP410(int cmd_params_pos, ESP3DMessage* msg) {
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
#if defined(AUTHENTICATION_FEATURE)
  if (msg->authentication_level == ESP3DAuthenticationLevel::guest) {
    msg->authentication_level = ESP3DAuthenticationLevel::not_authenticated;
    dispatchAuthenticationError(msg, COMMAND_ID, json);
    return;
  }
#endif  // AUTHENTICATION_FEATURE
  tmpstr = get_clean_param(msg, cmd_params_pos);
  if (tmpstr.length() != 0) {
    hasError = true;
    error_msg = "This command doesn't take parameters";
    esp3d_log_e("%s", error_msg.c_str());

  } else {
    if (WiFi.getMode() == WIFI_STA || WiFi.getMode() == WIFI_AP) {
      uint8_t currentmode = WiFi.getMode();
      int n = 0;
      uint8_t total = 0;
      if (json) {
        tmpstr = "{\"cmd\":\"410\",\"status\":\"ok\",\"data\":[";

      } else {
        tmpstr = "Start Scan\n";
      }
      msg->type = ESP3DMessageType::head;
      if (!dispatch(msg, tmpstr.c_str())) {
        esp3d_log_e("Error sending response to clients");
        return;
      }
      // Set Sta Mode if necessary
      if (currentmode == WIFI_AP) {
        WiFi.mode(WIFI_AP_STA);
      }
      // Scan
      n = WiFi.scanNetworks();

      for (int i = 0; i < n; ++i) {
        tmpstr = "";
        if (WiFi.RSSI(i) >= MIN_RSSI) {
          if (total > 0) {
            if (json) {
              tmpstr += ",";
            }
          }
          total++;
          if (json) {
            tmpstr += "{\"SSID\":\"";
            tmpstr += esp3d_string::encodeString(WiFi.SSID(i).c_str());
          } else {
            tmpstr += WiFi.SSID(i).c_str();
          }
          if (json) {
            tmpstr += "\",\"SIGNAL\":\"";
          } else {
            tmpstr += "\t";
          }
          tmpstr += String(WiFiConfig::getSignal(WiFi.RSSI(i)));
          if (!json) {
            tmpstr += "%";
          }
          if (json) {
            tmpstr += "\",\"IS_PROTECTED\":\"";
          }
          if (WiFi.encryptionType(i) == ENC_TYPE_NONE) {
            if (json) {
              tmpstr += "0";
            } else {
              tmpstr += "\tOpen";
            }
          } else {
            if (json) {
              tmpstr += "1";
            } else {
              tmpstr += "\tSecure";
            }
          }
          if (json) {
            tmpstr += "\"}";
          } else {
            tmpstr += "\n";
          }
          if (!dispatch(tmpstr.c_str(), target, requestId,
                        ESP3DMessageType::core)) {
            esp3d_log_e("Error sending answer to clients");
          }
        }
      }

      // Restore mode
      if (currentmode == WIFI_AP) {
        WiFi.mode((WiFiMode_t)currentmode);
      }
      WiFi.scanDelete();
      if (json) {
        tmpstr = "]}";
      } else {
        tmpstr = "End Scan\n";
      }
      if (!dispatch(tmpstr.c_str(), target, requestId,
                    ESP3DMessageType::tail)) {
        esp3d_log_e("Error sending answer to clients");
      }
      return;
    } else {
      hasError = true;
      error_msg = "WiFi not enabled";
      esp3d_log_e("%s", error_msg.c_str());
    }
  }
  if (!dispatchAnswer(msg, COMMAND_ID, json, hasError,
                      hasError ? error_msg.c_str() : ok_msg.c_str())) {
    esp3d_log_e("Error sending response to clients");
  }
}

#endif  // WIFI_FEATURE
