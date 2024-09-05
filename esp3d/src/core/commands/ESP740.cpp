/*
 ESP740.cpp - ESP3D command class

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
#if defined(SD_DEVICE)
#include "../../modules/authentication/authentication_service.h"
#include "../../modules/filesystem/esp_sd.h"
#include "../esp3d_commands.h"
#include "../esp3d_settings.h"
#include "../esp3d_string.h"

#ifdef SD_TIMESTAMP_FEATURE
#include "../../modules/time/time_service.h"
#endif  // SD_TIMESTAMP_FEATURE
#define COMMAND_ID 740
// List SD Filesystem
//[ESP740]<Root> json=<no> pwd=<admin password>
void ESP3DCommands::ESP740(int cmd_params_pos, ESP3DMessage* msg) {
  bool hasError = false;
  String error_msg = "Path inccorrect";
  String ok_msg = "ok";
  bool json = hasTag(msg, cmd_params_pos, "json");
  String tmpstr;
  // prepare answer msg
  msg->target = msg->origin;
  msg->origin = ESP3DClientType::command;
#if defined(AUTHENTICATION_FEATURE)
  if (msg->authentication_level == ESP3DAuthenticationLevel::guest) {
    msg->authentication_level = ESP3DAuthenticationLevel::not_authenticated;
    dispatchAuthenticationError(msg, COMMAND_ID, json);
    return;
  }
#endif  // AUTHENTICATION_FEATURE

  ESP3DMessage msgInfo;
  esp3d_message_manager.copyMsgInfos(&msgInfo, *msg);
  tmpstr = get_clean_param(msg, cmd_params_pos);

  if (tmpstr.length() == 0) {
    tmpstr = "/";
  }

  if (ESP_SD::accessFS()) {
    if (ESP_SD::getState(true) == ESP_SDCARD_NOT_PRESENT) {
      hasError = true;
      error_msg = "No SD card";
      esp3d_log_e("%s", error_msg.c_str());
      if (!dispatchAnswer(msg, COMMAND_ID, json, hasError,
                          hasError ? error_msg.c_str() : ok_msg.c_str())) {
        esp3d_log_e("Error sending response to clients");
      }
    } else {
      ESP_SD::setState(ESP_SDCARD_BUSY);
      ESP_SDFile f;
      f = ESP_SD::open(tmpstr.c_str(), ESP_FILE_READ);
      if (f) {
        if (json) {
          ok_msg = "{\"cmd\":\"740\",\"status\":\"ok\",\"data\":{\"path\":\"";
          ok_msg += tmpstr.c_str();
          ok_msg += "\",\"files\":[";

        } else {
          ok_msg = "Directory on SD : ";
          ok_msg += tmpstr.c_str();
          ok_msg += "\n";
        }
        msg->type = ESP3DMessageType::head;
        if (!dispatch(msg, ok_msg.c_str())) {
          esp3d_log_e("Error sending response to clients");
          hasError = true;
        }
        if (ESP_SD::getState(true) == ESP_SDCARD_NOT_PRESENT) {
          hasError = true;
          error_msg = "No SD card";
          esp3d_log_e("%s", error_msg.c_str());
        }
        if (!hasError) {
          uint nbDirs = 0;
          uint nbFiles = 0;
          uint64_t totalSpace = ESP_SD::totalBytes();
          uint64_t usedSpace = ESP_SD::usedBytes();
          uint64_t freeSpace = ESP_SD::freeBytes();

          ESP_SDFile sub;
          sub = f.openNextFile();
          while (sub && !hasError) {
            if (sub.isDirectory()) {
              ok_msg = "";
              nbDirs++;
              if (json) {
                ok_msg = "";
                if (nbDirs > 1) {
                  ok_msg += ",";
                }
                ok_msg += "{\"name\":\"";
                ok_msg += sub.name();
                ok_msg += "\",\"size\":\"-1\"}";
              } else {
                ok_msg = "[DIR] \t";
                ok_msg += sub.name();
              }
              if (!json) {
                ok_msg += "\n";
              }
              ESP3DMessage* newMsg = esp3d_message_manager.copyMsgInfos(msgInfo);
              if (newMsg) {
                newMsg->type = ESP3DMessageType::core;
                if (!dispatch(newMsg, ok_msg.c_str())) {
                  esp3d_log_e("Error sending response to clients");
                  hasError = true;
                }
              } else {
                esp3d_log_e("Error creating message");
                hasError = true;
              }
            }
            sub.close();
            sub = f.openNextFile();
          }
          f.close();
          f = ESP_SD::open(tmpstr.c_str(), ESP_FILE_READ);
          // Check files
          sub = f.openNextFile();
          while (sub && !hasError) {
            if (!sub.isDirectory()) {
              nbFiles++;
              String time = "";
              ok_msg = "";
#ifdef SD_TIMESTAMP_FEATURE
              time = timeService.getDateTime((time_t)sub.getLastWrite());
#endif  // SD_TIMESTAMP_FEATURE
              if (json) {
                ok_msg = "";
                if (nbDirs > 0 || nbFiles > 1) {
                  ok_msg += ",";
                }
                ok_msg += "{\"name\":\"";
                ok_msg += sub.name();
                ok_msg += "\",\"size\":\"";
                ok_msg += esp3d_string::formatBytes(sub.size());
                if (time.length() > 0) {
                  ok_msg += "\",\"time\":\"";
                  ok_msg += time;
                }
                ok_msg += "\"}";
              } else {
                ok_msg += "     \t ";
                ok_msg += sub.name();
                ok_msg += " \t";
                ok_msg += esp3d_string::formatBytes(sub.size());
                ok_msg += " \t";
                ok_msg += time;
              }
              if (!json) {
                ok_msg += "\n";
              }
              ESP3DMessage* newMsg = esp3d_message_manager.copyMsgInfos(msgInfo);
              if (newMsg) {
                newMsg->type = ESP3DMessageType::core;
                if (!dispatch(newMsg, ok_msg.c_str())) {
                  esp3d_log_e("Error sending response to clients");
                  hasError = true;
                }
              } else {
                esp3d_log_e("Error creating message");
                hasError = true;
              }
            }
            sub.close();
            sub = f.openNextFile();
          }
          // end of json
          if (json) {
            ok_msg = "], \"total\":\"";
            ok_msg += esp3d_string::formatBytes(totalSpace);
            ok_msg += "\",\"used\":\"";
            ok_msg += esp3d_string::formatBytes(usedSpace);
            ok_msg += "\",\"occupation\":\"";
            if (totalSpace == 0) {
              totalSpace = 1;
            }
            uint occupation = round(100.0 * usedSpace / totalSpace);
            if ((occupation < 1) && (usedSpace > 0)) {
              occupation = 1;
            }
            ok_msg += String(occupation);
            ok_msg += "\"}}";

          } else {
            ok_msg = "Files: ";
            ok_msg += String(nbFiles);
            ok_msg += ", Dirs :";
            ok_msg += String(nbDirs);
            ok_msg += "\nTotal: ";
            ok_msg += esp3d_string::formatBytes(totalSpace);
            ok_msg += ", Used: ";
            ok_msg += esp3d_string::formatBytes(usedSpace);
            ok_msg += ", Available: ";
            ok_msg += esp3d_string::formatBytes(freeSpace);
            ok_msg += "\n";
          }

          ESP3DMessage* newMsg = esp3d_message_manager.copyMsgInfos(msgInfo);
          newMsg->type = ESP3DMessageType::tail;
          if (!dispatch(newMsg, ok_msg.c_str())) {
            esp3d_log_e("Error sending response to clients");
          }
          f.close();
        }
      } else {
        hasError = true;
        error_msg = "Invalid directory";
        esp3d_log_e("%s", error_msg.c_str());
        if (!dispatchAnswer(msg, COMMAND_ID, json, hasError,
                            hasError ? error_msg.c_str() : ok_msg.c_str())) {
          esp3d_log_e("Error sending response to clients");
        }
      }
    }
    ESP_SD::releaseFS();
  } else {
    hasError = true;
    error_msg = "FS not available";
    esp3d_log_e("%s", error_msg.c_str());
    if (!dispatchAnswer(msg, COMMAND_ID, json, hasError,
                        hasError ? error_msg.c_str() : ok_msg.c_str())) {
      esp3d_log_e("Error sending response to clients");
    }
  }
}

#endif  // SD_DEVICE
