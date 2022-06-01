/*
 ESP150.cpp - ESP3D command class

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
#include "../commands.h"
#include "../esp3doutput.h"
#include "../settings_esp3d.h"
#include "../../modules/authentication/authentication_service.h"
#define COMMANDID   150
// Get/Set display/set boot delay in ms / Verbose boot
//[ESP150]<delay=time in milliseconds><verbose=ON/OFF>[pwd=<admin password>]
bool Commands::ESP150(const char* cmd_params, level_authenticate_type auth_type, ESP3DOutput * output)
{
    bool noError = true;
    bool json = has_tag (cmd_params, "json");
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
        parameter = clean_param(get_param (cmd_params, ""));
        //get
        if (parameter.length() == 0) {
            String s ="";
            if(json) {
                s += "{\"delay\":\"";
            } else {
                s+= "delay=";
            }
            s+=  String(Settings_ESP3D::read_uint32(ESP_BOOT_DELAY));
            if(json) {
                s += "\",\"verbose\":\"";
            } else {
                s+= ", verbose=";
            }
            s+= Settings_ESP3D::isVerboseBoot(true)?"ON":"OFF";
            if(json) {
                s += "\"}";
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
                bool hasParameter = false;
                parameter = get_param (cmd_params, "delay=");
                if (parameter.length() != 0) {
                    hasParameter = true;
                    uint ibuf = parameter.toInt();
                    if ((ibuf > Settings_ESP3D::get_max_int32_value(ESP_BOOT_DELAY)) || (ibuf < Settings_ESP3D::get_min_int32_value(ESP_BOOT_DELAY))) {
                        response = format_response(COMMANDID, json, false, "Incorrect delay");
                        noError = false;
                    }
                    if (noError) {
                        if (!Settings_ESP3D::write_uint32 (ESP_BOOT_DELAY, ibuf)) {
                            response = format_response(COMMANDID, json, false, "Set failed");
                            noError = false;
                        }
                    }
                }
                if(noError) {
                    parameter = get_param (cmd_params, "verbose=");
                    if (parameter.length() != 0) {
                        hasParameter = true;
                        if ((parameter == "ON")|| (parameter == "OFF")) {
                            if (!Settings_ESP3D::write_byte (ESP_VERBOSE_BOOT, (parameter == "ON")?1:0)) {
                                response = format_response(COMMANDID, json, false, "Set failed");
                                noError = false;
                            } else {
                                Settings_ESP3D::isVerboseBoot(true);
                            }
                        } else {
                            response = format_response(COMMANDID, json, false, "Only verbose +ON/OFF is allowed");
                            noError = false;
                        }
                    }
                }
                if (noError && !hasParameter) {
                    response = format_response(COMMANDID, json, false, "Incorrect command");
                    noError = false;
                } else if(noError) {
                    response = format_response(COMMANDID, json, true, "ok");
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
