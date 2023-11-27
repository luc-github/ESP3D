/*
 ESP701.cpp - ESP3D command class

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
#if defined(GCODE_HOST_FEATURE)
#include "../../modules/authentication/authentication_service.h"
#include "../../modules/gcode_host/gcode_host.h"
#include "../esp3d_commands.h"
#include "../esp3d_settings.h"

#define COMMAND_ID 701

// Query and Control ESP700 stream
//[ESP701]action=<PAUSE/RESUME/ABORT> CLEAR_ERROR
void ESP3DCommands::ESP701(int cmd_params_pos, ESP3DMessage* msg) {
  ESP3DClientType target = msg->origin;
  ESP3DRequest requestId = msg->request_id;
  (void)requestId;
  msg->target = target;
  msg->origin = ESP3DClientType::command;
  bool hasError = false;
  String error_msg = "Invalid parameters";
  String ok_msg = "ok";
  bool json = hasTag(msg, cmd_params_pos, "json");
  bool hasClearError = hasTag(msg, cmd_params_pos, "CLEAR_ERROR");
  String tmpstr;
#if defined(AUTHENTICATION_FEATURE)
  if (msg->authentication_level == ESP3DAuthenticationLevel::guest) {
    msg->authentication_level = ESP3DAuthenticationLevel::not_authenticated;
    dispatchAuthenticationError(msg, COMMAND_ID, json);
    return;
  }
#endif  // AUTHENTICATION_FEATURE
  tmpstr = get_clean_param(msg, cmd_params_pos);
  if (tmpstr.length() == 0) {
    // give status
    switch (esp3d_gcode_host.getStatus()) {
      case HOST_START_STREAM:
      case HOST_READ_LINE:
      case HOST_PROCESS_LINE:
      case HOST_WAIT4_ACK:
        // TODO add % of progress and filename if any
        // totalSize / processedSize / fileName
        if (json) {
          ok_msg = "{\"status\":\"processing\",\"total\":\"" +
                   String(esp3d_gcode_host.totalSize()) +
                   "\",\"processed\":\"" +
                   String(esp3d_gcode_host.processedSize()) + "\",\"type\":\"" +
                   String(esp3d_gcode_host.getFSType());
          if (esp3d_gcode_host.getFSType() != TYPE_SCRIPT_STREAM) {
            ok_msg += "\",\"name\":\"" + String(esp3d_gcode_host.fileName());
          }
          ok_msg += "\"}";
        } else {
          ok_msg = "processing";
        }
        break;
      case HOST_PAUSE_STREAM:
        ok_msg = "pause";
        break;
      case HOST_NO_STREAM:
        esp3d_log("No stream %d", esp3d_gcode_host.getErrorNum());
        if (esp3d_gcode_host.getErrorNum() != ERROR_NO_ERROR) {
          hasError = true;
          if (json) {
            error_msg = "{\"status\":\"no stream\",\"code\":\"" +
                        String(esp3d_gcode_host.getErrorNum()) + "\"}";
          } else {
            error_msg = "no stream, last error " +
                        String(esp3d_gcode_host.getErrorNum());
          }
        } else {
          ok_msg = "no stream";
        }
        break;
      default:
        error_msg = String(esp3d_gcode_host.getStatus());
        hasError = true;
        break;
    }

  } else {
    tmpstr = get_param(msg, cmd_params_pos, "action=");
    if (tmpstr.length() == 0) {
      hasError = true;
      error_msg = "Missing parameter";
      esp3d_log_e("%s", error_msg.c_str());
    } else {
      tmpstr.toUpperCase();
      if (tmpstr != "PAUSE" && tmpstr != "ABORT" && tmpstr != "RESUME") {
        hasError = true;
        error_msg = "Unknown action";
        esp3d_log_e("%s", error_msg.c_str());
      } else {
        if (tmpstr == "PAUSE") {
          if (esp3d_gcode_host.pause()) {
            ok_msg = "Stream paused";
          } else {
            hasError = true;
            error_msg = "No stream to pause";
          }
        } else if (tmpstr == "RESUME") {
          if (esp3d_gcode_host.resume()) {
            ok_msg = "Stream resumed";
          } else {
            error_msg = "No stream to resume";
            hasError = true;
          }
        } else if (tmpstr == "ABORT") {
          if (esp3d_gcode_host.abort()) {
            ok_msg = "Stream aborted";
          } else {
            error_msg = "No stream to abort";
            hasError = true;
          }
        }
      }
      if (hasClearError) {
        esp3d_gcode_host.setErrorNum(ERROR_NO_ERROR);
      }
    }
  }
  if (!dispatchAnswer(msg, COMMAND_ID, json, hasError,
                      hasError ? error_msg.c_str() : ok_msg.c_str())) {
    esp3d_log_e("Error sending response to clients");
  }
}

#endif  // GCODE_HOST_FEATURE
