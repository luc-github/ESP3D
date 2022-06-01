/*
 ESP200.cpp - ESP3D command class

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
#if defined (SD_DEVICE)
#include "../commands.h"
#include "../esp3doutput.h"
#include "../settings_esp3d.h"
#include "../../modules/filesystem/esp_sd.h"
#include "../../modules/authentication/authentication_service.h"
#define COMMANDID   200
//Get SD Card Status
//[ESP200] json=<YES/NO> <RELEASESD> <REFRESH> pwd=<user/admin password>
bool Commands::ESP200(const char* cmd_params, level_authenticate_type auth_type, ESP3DOutput * output)
{
    bool noError = true;
    bool json = has_tag (cmd_params, "json");
    bool releaseSD = has_tag (cmd_params, "RELEASE");
    bool refreshSD = has_tag (cmd_params, "REFRESH");
    String response;
    String parameter;
    int errorCode = 200; //unless it is a server error use 200 as default and set error in json instead

#ifdef AUTHENTICATION_FEATURE
    if (auth_type == LEVEL_GUEST) {
        response = format_response(COMMANDID, json, false, "Guest user can't use this command");
        noError = false;
        errorCode = 401;
    }
#else
    (void)auth_type;
#endif //AUTHENTICATION_FEATURE
    if (noError) {
        if (releaseSD) {
            ESP_SD::releaseFS();
            response = format_response(COMMANDID, json, true, " SD card released");
        }

        if (!ESP_SD::accessFS()) {
            if (ESP_SD::getState() == ESP_SDCARD_BUSY) {
                response = format_response(COMMANDID, json, true, "Busy");
            } else {
                response = format_response(COMMANDID, json, true, "Not available");
            }
        } else {
            int8_t state = ESP_SD::getState(true);
            if (state == ESP_SDCARD_IDLE) {
                response = format_response(COMMANDID, json, true, "SD card ok");
                if (refreshSD) {
                    ESP_SD::refreshStats(true);
                }
            }
            ESP_SD::releaseFS();
            parameter = clean_param(get_param (cmd_params, ""));
            if (parameter.length()!=0 && parameter.indexOf("REFRESH")==-1 && parameter.indexOf("RELEASE")==-1) {
                response = format_response(COMMANDID, json, false, "Unknown parameter");
                noError = false;
            }
        }
    }
    if (noError) {
        if (response.length() == 0) {
            response = format_response(COMMANDID, json, true, "No SD card");
        }
        if (json) {
            output->printLN (response.c_str() );
        } else {
            output->printMSG (response.c_str() );
        }
    } else {
        output->printERROR(response.c_str(), errorCode);
    }
    return noError;
}

#endif //SD_DEVICE
