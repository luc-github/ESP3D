/*
 ESP210.cpp - ESP3D command class

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
#if defined (SENSOR_DEVICE)
#include "../commands.h"
#include "../esp3doutput.h"
#include "../settings_esp3d.h"
#include "../../modules/sensor/sensor.h"
#include "../../modules/authentication/authentication_service.h"
#define COMMANDID   210
//Get Sensor Value / type/Set Sensor type
//[ESP210]<type=NONE/xxx> <interval=XXX in millisec> json=<no> pwd=<admin password>
bool Commands::ESP210(const char* cmd_params, level_authenticate_type auth_type, ESP3DOutput * output)
{
    bool noError = true;
    bool json = has_tag (cmd_params, "json");
    String response;
    String parameter;
    bool hasParam = false;
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
        parameter = clean_param(get_param (cmd_params, ""));
        //get
        if (parameter.length() == 0) {
            String s;
            if (json) {
                s="{\"type\":\"";
            } else {
                s="type=";
            }
            if(esp3d_sensor.started()) {
                s+=esp3d_sensor.GetModelString()
            } else {
                s+="NONE";
            }
            if (json) {
                s=":\",\"interval\":";
            } else {
                s=", interval=";
            }
            s += esp3d_sensor.interval();
            if (json) {
                s=":\",\"value\":";
            } else {
                s="ms, value=";
            }
            s += esp3d_sensor.GetData();
            if (json) {
                s+="\"}";
            }
            response = format_response(COMMANDID, json, true,s.c_str());
        } else {
#ifdef AUTHENTICATION_FEATURE
            if (auth_type != LEVEL_ADMIN) {
                response = format_response(COMMANDID, json, false, "Wrong authentication level");
                noError = false;
                errorCode = 401;
            }
#endif //AUTHENTICATION_FEATURE
            if (noError) {
                parameter = get_param (cmd_params, "type=");
                if (parameter.length() != 0) {
                    hasParam=true;
                    parameter.toUpperCase();
                    int8_t v = -1;
                    if (parameter == "NONE") {
                        v = 0;
                    } else if(esp3d_sensor.isModelValid(esp3d_sensor.getIDFromString(parameter.c_str()))) {
                        v = esp3d_sensor.getIDFromString(parameter.c_str());
                    }  else {
                        response = format_response(COMMANDID, json, false, "Invalid parameter");
                        noError = false;
                    }
                    if (v!=-1) {
                        if (!Settings_ESP3D::write_byte(ESP_SENSOR_TYPE,v)) {
                            response = format_response(COMMANDID, json, false, "Set failed");
                            noError = false;
                        }
                        if(noError) {
                            if (!esp3d_sensor.begin()) {
                                response = format_response(COMMANDID, json, false, "Starting failed");
                                noError = false;
                            }
                        }

                    } else {
                        response = format_response(COMMANDID, json, false, "Invalid type");
                        noError = false;
                    }
                }
            }
            if (noError) {
                parameter = get_param (cmd_params, "interval=");
                if (parameter.length() != 0) {
                    hasParam = true;
                    if (!Settings_ESP3D::write_uint32(ESP_SENSOR_INTERVAL,parameter.toInt())) {
                        response = format_response(COMMANDID, json, false, "Set failed");
                        noError = false;
                    }
                    esp3d_sensor.setInterval(parameter.toInt());
                }
            }
            if (noError) {
                if (hasParam) {
                    response = format_response(COMMANDID, json, true, "ok");
                } else {
                    response = format_response(COMMANDID, json, false, "No parameter");
                    noError = false;
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

#endif //SENSOR_DEVICE
