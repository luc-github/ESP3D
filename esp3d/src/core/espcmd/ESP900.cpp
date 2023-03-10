/*
 ESP900.cpp - ESP3D command class

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
#if COMMUNICATION_PROTOCOL != SOCKET_SERIAL
#include "../commands.h"
#include "../esp3doutput.h"
#include "../settings_esp3d.h"
#include "../../modules/authentication/authentication_service.h"
#include "../../modules/serial/serial_service.h"
#define COMMANDID   900
//Get state / Set Enable / Disable Serial Communication
//[ESP900]<ENABLE/DISABLE> json=<no> [pwd=<admin password>]
bool Commands::ESP900(const char* cmd_params, level_authenticate_type auth_type, ESP3DOutput * output)
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
        String r;
        if (parameter.length() == 0) {
            if (serial_service.started()) {
                r="ENABLED";
            } else {
                r="DISABLED";
            }
            r+=" - Serial" + String(serial_service.serialIndex());
            response = format_response(COMMANDID, json, true, r.c_str());
        } else { //set
            if (parameter == "ENABLE" ) {
                if (serial_service.begin(ESP_SERIAL_OUTPUT)) {
                    response = format_response(COMMANDID, json, true, "ok");
                } else {
                    response = format_response(COMMANDID, json, false, "Cannot enable serial communication");
                    noError = false;
                }
            } else if (parameter == "DISABLE" ) {
                response = format_response(COMMANDID, json, true, "ok");
                serial_service.end();
            } else {
                response = format_response(COMMANDID, json, false, "Incorrect command");
                noError = false;
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
#endif