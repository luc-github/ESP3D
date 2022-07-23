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
//#define ESP_DEBUG_FEATURE DEBUG_OUTPUT_SERIAL0
#include "../../include/esp3d_config.h"
#if  defined(GCODE_HOST_FEATURE)
#include "../commands.h"
#include "../esp3doutput.h"
#include "../settings_esp3d.h"
#include "../../modules/authentication/authentication_service.h"
#include "../../modules/gcode_host/gcode_host.h"
#define COMMANDID   701

//Query and Control ESP700 stream
//[ESP701]action=<PAUSE/RESUME/ABORT>
bool Commands::ESP701(const char* cmd_params, level_authenticate_type auth_type, ESP3DOutput * output)
{
    bool noError = true;
    bool json = has_tag (cmd_params, "json");
    String response;
    String parameter;
    int errorCode = 200; //unless it is a server error use 200 as default and set error in json instead
#ifdef AUTHENTICATION_FEATURE
    if (auth_type != LEVEL_ADMIN) {
        response = format_response(COMMANDID, json, false, "Wrong authentication level");
        noError = false;
        errorCode = 401;
    }
#else
    (void)auth_type;
#endif //AUTHENTICATION_FEATURE
    if (noError) {
        parameter = (get_param (cmd_params, "action="));
        if (parameter.length() != 0) {
            if (parameter.equalsIgnoreCase("PAUSE")) {
                if (esp3d_gcode_host.pause()) {
                    response = format_response(COMMANDID, json, true, "Stream paused");
                } else {
                    response = format_response(COMMANDID, json, false, "No stream to pause");
                    noError = false;
                }
            } else if (parameter.equalsIgnoreCase("RESUME")) {
                if (esp3d_gcode_host.resume()) {
                    response = format_response(COMMANDID, json, true, "Stream resumed");
                } else {
                    response = format_response(COMMANDID, json, false, "No stream to resume");
                    noError = false;
                }
            } else if (parameter.equalsIgnoreCase("ABORT")) {
                if (esp3d_gcode_host.abort()) {
                    response = format_response(COMMANDID, json, true, "Stream aborted");
                } else {
                    response = format_response(COMMANDID, json, false, "No stream to abort");
                    noError = false;
                }
            }
            if (parameter.equalsIgnoreCase("CLEAR_ERROR")) {
                esp3d_gcode_host.setErrorNum(ERROR_NO_ERROR);
                response = format_response(COMMANDID, json, true, "Error cleared");
            } else {
                response = format_response(COMMANDID, json, false, "Unknown action");
                noError = false;
            }

        } else {
            String resp;
            bool noError = true;
            switch (esp3d_gcode_host.getStatus()) {
            case HOST_START_STREAM:
            case HOST_READ_LINE:
            case HOST_PROCESS_LINE:
            case HOST_WAIT4_ACK:
                //TODO add % of progress and filename if any
                //totalSize / processedSize / fileName
                if (json) {
                    resp = "{\"status\":\"processing\",\"total\":\"" + String(esp3d_gcode_host.totalSize()) + "\",\"processed\":\"" + String(esp3d_gcode_host.processedSize()) + "\",\"type\":\"" + String(esp3d_gcode_host.getFSType());
                    if(esp3d_gcode_host.getFSType() !=TYPE_SCRIPT_STREAM) {
                        resp+="\",\"name\":\""+String(esp3d_gcode_host.fileName());
                    }
                    resp+="\"}";
                } else {
                    resp = "processing";
                }
                response = format_response(COMMANDID, json, true, resp.c_str());
                break;
            case HOST_PAUSE_STREAM:
                response = format_response(COMMANDID, json, true, "pause");
                break;
            case HOST_RESUME_STREAM:
                response = format_response(COMMANDID, json, true, "resume stream");
                break;
            case  HOST_NO_STREAM:
                log_esp3d("No stream %d", esp3d_gcode_host.getErrorNum());
                if (esp3d_gcode_host.getErrorNum()!=ERROR_NO_ERROR) {
                    noError = false;
                    if(json) {
                        resp= "{\"status\":\"no stream\",\"code\":\"" + String(esp3d_gcode_host.getErrorNum()) + "\"}";
                    } else {
                        resp = "no stream, last error " + String(esp3d_gcode_host.getErrorNum());
                    }
                } else {
                    resp = "no stream";
                }
                response = format_response(COMMANDID, json, noError, resp.c_str());
                break;
            default:
                response = format_response(COMMANDID, json, false, String(esp3d_gcode_host.getStatus()).c_str());
                noError = false;
                break;
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

#endif //GCODE_HOST_FEATURE
