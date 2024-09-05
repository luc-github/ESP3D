/*
 ESP301.cpp - ESP3D command class

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
#if defined(ESP_LUA_INTERPRETER_FEATURE)
#include "../../modules/authentication/authentication_service.h"
#include "../../modules/lua_interpreter/lua_interpreter_service.h"
#include "../esp3d_commands.h"
#include "../esp3d_settings.h"
#include "../esp3d_string.h"


#define COMMAND_ID 301

// Query and Control ESP300 execution
//[ESP301]action=<PAUSE/RESUME/ABORT>
void ESP3DCommands::ESP301(int cmd_params_pos, ESP3DMessage* msg) {
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
  if (tmpstr.length() == 0) {
    String error = esp3d_lua_interpreter.getLastError();
    if (!esp3d_lua_interpreter.isScriptRunning()) {
      if (json) {
        ok_msg = "{\"status\":\"idle\"";
        if (error.length() > 0) {
          ok_msg += ",\"error\":\"" + error + "\"";
        }
        ok_msg+="}";
      } else {
        ok_msg = "idle";
        if (error.length() > 0) {
          ok_msg += ", error: " + error;
        }
      }
    } else {
      String status =
          esp3d_lua_interpreter.isScriptPaused() ? "paused" : "running";
      
      if (error.length() > 0) {
        status = "error";
      }
      String scriptName = esp3d_lua_interpreter.getCurrentScriptName();
      String duration = esp3d_string::formatDuration(
          esp3d_lua_interpreter.getExecutionTime());
      if (json) {
        String errorMsg = error.length() > 0 ? ",\"error\":\"" + error + "\"" : "";
        ok_msg = "{\"status\":\"" + status + "\",\"script\":\"" + scriptName +
                 "\",\"duration\":\"" + duration + "\"" + errorMsg + "}";
      } else {
        ok_msg = status;
        if (error.length() > 0) {
          ok_msg += ": " + error;
        }
        ok_msg += ", " + scriptName + ", duration " + duration;
      }
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
        if (esp3d_lua_interpreter.isScriptRunning()) {
          if (tmpstr == "PAUSE") {
            if (esp3d_lua_interpreter.isScriptPaused()) {
              hasError = true;
              error_msg = "Script already paused";
              esp3d_log_e("%s", error_msg.c_str());
            } else {
              if (esp3d_lua_interpreter.pauseScript()) {
                ok_msg = "Script paused";
              } else {
                hasError = true;
                error_msg = "Pause failed";
                esp3d_log_e("%s", error_msg.c_str());
              }
            }
          } else if (tmpstr == "RESUME") {
            if (!esp3d_lua_interpreter.isScriptPaused()) {
              hasError = true;
              error_msg = "Script not paused";
              esp3d_log_e("%s", error_msg.c_str());
            } else {
              if (esp3d_lua_interpreter.resumeScript()) {
                ok_msg = "Script resumed";
              } else {
                hasError = true;
                error_msg = "Resume failed";
                esp3d_log_e("%s", error_msg.c_str());
              }
            }
          } else if (tmpstr == "ABORT") {
            esp3d_lua_interpreter.abortScript();
            ok_msg = "Script aborted";
          }
        } else {
          hasError = true;
          error_msg = "No script running";
          esp3d_log_e("%s", error_msg.c_str());
        }
      }
    }
  }
  if (!dispatchAnswer(msg, COMMAND_ID, json, hasError,
                      hasError ? error_msg.c_str() : ok_msg.c_str())) {
    esp3d_log_e("Error sending response to clients");
  }
}

#endif  // ESP_LUA_INTERPRETER_FEATURE
