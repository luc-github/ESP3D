/*
 ESP122.cpp - ESP3D command class

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
#if defined (CAMERA_DEVICE)
#include "../commands.h"
#include "../esp3doutput.h"
#include "esp_camera.h"
#include "../settings_esp3d.h"
#include "../../modules/authentication/authentication_service.h"
#include "../../modules/camera/camera.h"
#include <time.h>
#define COMMANDID   171
//Save frame to target path and filename (default target = today date, default name=timestamp.jpg)
//[ESP171]path=<target path> filename=<target filename> pwd=<admin/user password>
bool Commands::ESP171(const char* cmd_params, level_authenticate_type auth_type, ESP3DOutput * output)
{
    bool noError = true;
    bool json = has_tag (cmd_params, "json");
    String response;
    String parameter;
    int errorCode = 200; //unless it is a server error use 200 as default and set error in json instead
    String path;
    String filename;
#ifdef AUTHENTICATION_FEATURE
    if (auth_type == LEVEL_GUEST) {
        response = format_response(COMMANDID, json, false, "Guest user can't use this command");
        noError = false;
        errorCode = 401;
    }
#else
    (void)auth_type;
#endif //AUTHENTICATION_FEATURE
    if(noError) {
        if (!esp3d_camera.started()) {
            response = format_response(COMMANDID, json, false, "No camera initialized");
            noError = false;
        } else {
            parameter = clean_param(get_param (cmd_params, "path="));
            //get path
            if (parameter.length() != 0) {

                path = parameter;
            }
            parameter = clean_param(get_param (cmd_params, "filename="));
            //get filename
            if (parameter.length() != 0) {
                filename = parameter;
            }
            //if nothing provided, use default filename / path
            if (path.length()==0) {
                struct tm  tmstruct;
                time_t now;
                path = "";
                time(&now);
                localtime_r(&now, &tmstruct);
                path = String((tmstruct.tm_year)+1900) + "-";
                if (((tmstruct.tm_mon)+1) < 10) {
                    path +="0";
                }
                path += String(( tmstruct.tm_mon)+1) + "-";
                if (tmstruct.tm_mday < 10) {
                    path +="0";
                }
                path += String(tmstruct.tm_mday);
            }
            if(filename.length()==0) {
                struct tm  tmstruct;
                time_t now;
                time(&now);
                localtime_r(&now, &tmstruct);
                filename = String(now) + ".jpg";
            }

            //now send command
            if(noError) {
                noError =  esp3d_camera.handle_snap(nullptr,path.c_str(), filename.c_str());
                if(noError) {
                    response = format_response(COMMANDID, json, true, "Snapshot taken");
                } else {
                    response = format_response(COMMANDID, json, false, "Error taking snapshot");
                }
            }
        }
    }
    if (noError) {
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

#endif //CAMERA_DEVICE
