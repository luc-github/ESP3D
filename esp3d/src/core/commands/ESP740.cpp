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
#include "../esp3d_message.h"
#include "../esp3d_settings.h"

#ifdef SD_TIMESTAMP_FEATURE
#include "../../modules/time/time_service.h"
#endif  // SD_TIMESTAMP_FEATURE
#define COMMANDID 740
// List SD Filesystem
//[ESP740]<Root> json=<no> pwd=<admin password>
void ESP3DCommands::ESP740(int cmd_params_pos, ESP3DMessage* msg) {
  /*
  bool noError = true;
  bool json = has_tag(cmd_params, "json");
  String response;
  String parameter;
  int errorCode = 200;  // unless it is a server error use 200 as default and
                        // set error in json instead
#ifdef AUTHENTICATION_FEATURE
  if (auth_type != admin) {
    response =
        format_response(COMMANDID, json, false, "Wrong authentication level");
    noError = false;
    errorCode = 401;
  }
#else
  (void)auth_type;
#endif  // AUTHENTICATION_FEATURE
  if (noError) {
    parameter = clean_param(get_param(cmd_params, ""));
    if (parameter.length() == 0) {
      parameter = "/";
    }
    if (!ESP_SD::accessFS()) {
      response = format_response(COMMANDID, json, false, "Not available");
      noError = false;
    } else {
      if (ESP_SD::getState(true) == ESP_SDCARD_NOT_PRESENT) {
        response = format_response(COMMANDID, json, false, "No SD card");
        noError = false;
      } else {
        ESP_SD::setState(ESP_SDCARD_BUSY);
        if (ESP_SD::exists(parameter.c_str())) {
          String line = "";
          ESP_SDFile f = ESP_SD::open(parameter.c_str(), ESP_FILE_READ);
          uint countf = 0;
          uint countd = 0;
          if (f) {
            if (json) {
              line =
                  "{\"cmd\":\"720\",\"status\":\"ok\",\"data\":{\"path\":\"" +
                  parameter + "\",\"files\":[";
              esp3dmsg->print(line.c_str());
            } else {
              line = "Directory on SD : " + parameter;
              esp3dmsg->printMSGLine(line.c_str());
            }
            // Check directories
            ESP_SDFile sub = f.openNextFile();
            while (sub) {
              if (sub.isDirectory()) {
                line = "";
                countd++;
                if (json) {
                  line = "";
                  if (countd > 1) {
                    line += ",";
                  }
                  line += "{\"name\":\"";
                  line += sub.name();
                  line += "\",\"size\":\"-1\"}";
                } else {
                  line = "[DIR] \t";
                  line += sub.name();
                }
                if (json) {
                  esp3dmsg->print(line.c_str());
                } else {
                  esp3dmsg->printMSGLine(line.c_str());
                }
              }
              sub.close();
              sub = f.openNextFile();
            }
            f.close();
            f = ESP_SD::open(parameter.c_str(), ESP_FILE_READ);
            // Check files
            sub = f.openNextFile();
            while (sub) {
              if (!sub.isDirectory()) {
                String time = "";
                line = "";
                countf++;
#ifdef SD_TIMESTAMP_FEATURE
                time = timeService.getDateTime((time_t)sub.getLastWrite());
#endif  // SD_TIMESTAMP_FEATURE
                if (json) {
                  if (countd > 0 || countf > 1) {
                    line += ",";
                  }
                  line += "{\"name\":\"";
                  line += sub.name();
                  line += "\",\"size\":\"";
                  line += ESP_SD::formatBytes(sub.size());
                  if (time.length() > 0) {
                    line += "\",\"time\":\"";
                    line += time;
                  }
                  line += "\"}";
                } else {
                  line += "     \t ";
                  line += sub.name();
                  line += " \t";
                  line += ESP_SD::formatBytes(sub.size());
                  line += " \t";
                  line += time;
                }
                if (json) {
                  esp3dmsg->print(line.c_str());
                } else {
                  esp3dmsg->printMSGLine(line.c_str());
                }
              }
              sub.close();
              sub = f.openNextFile();
            }
            f.close();
            if (json) {
              line = "], \"total\":\"";
              line += ESP_SD::formatBytes(ESP_SD::totalBytes());
              line += "\",\"used\":\"";
              line += ESP_SD::formatBytes(ESP_SD::usedBytes());
              line += "\",\"occupation\":\"";
              uint64_t total = ESP_SD::totalBytes();
              if (total == 0) {
                total = 1;
              }
              float occupation = 100.0 * ESP_SD::usedBytes() / total;
              if ((occupation < 1) && (ESP_SD::usedBytes() > 0)) {
                occupation = 1;
              }
              line += String((int)round(occupation));
              line += "\"}}";
              esp3dmsg->printLN(line.c_str());
            } else {
              line = String(countf) + " file";
              if (countf > 1) {
                line += "s";
              }
              line += " , " + String(countd) + " dir";
              if (countd > 1) {
                line += "s";
              }
              esp3dmsg->printMSGLine(line.c_str());
              line = "Total ";
              line += ESP_SD::formatBytes(ESP_SD::totalBytes());
              line += ", Used ";
              line += ESP_SD::formatBytes(ESP_SD::usedBytes());
              line += ", Available: ";
              line += ESP_SD::formatBytes(ESP_SD::freeBytes());
              esp3dmsg->printMSGLine(line.c_str());
            }
            ESP_SD::releaseFS();
            return true;
          } else {
            response =
                format_response(COMMANDID, json, false, "Invalid directory");
            noError = false;
          }
        } else {
          response =
              format_response(COMMANDID, json, false, "Invalid directory");
          noError = false;
        }
      }
      ESP_SD::releaseFS();
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

#endif  // SD_DEVICE
