/*
 ESP444.cpp - ESP3D command class

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
#include "../esp3d.h"
#include "../commands.h"
#include "../esp3doutput.h"
#include "../../modules/authentication/authentication_service.h"
//Set ESP State
//cmd are RESTART / RESET
//[ESP444]<cmd> json=<no> <pwd=admin>
#define COMMANDID   444
bool Commands::ESP444(const char* cmd_params, level_authenticate_type auth_type, ESP3DOutput * output)
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
        if (has_tag(cmd_params,"RESET")) {
            if (Esp3D::reset()) {
                response = format_response(COMMANDID, json, true, "ok");
            } else {
                response = format_response(COMMANDID, json, false, "Reset failed");
                noError = false;
            }
        }
        if (noError && has_tag(cmd_params,"RESTART")) {
            if (!json) {
                output->printMSG ("Restart ongoing");
            } else {
                response = format_response(COMMANDID, json, true, "Restart ongoing");
                output->printLN (response.c_str());
            }
            output->flush();
            Hal::wait(100);
            Esp3D::restart_esp();
        }
        if (noError && !has_tag(cmd_params,"RESTART") && !has_tag(cmd_params,"RESET")) {
            response = format_response(COMMANDID, json, false, "Invalid parameter");
            noError = false;
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
