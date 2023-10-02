/*
 ESP111.cpp - ESP3D command class

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
#if defined( WIFI_FEATURE) || defined (ETH_FEATURE)
#include "../commands.h"
#include "../esp3doutput.h"
#include "../settings_esp3d.h"
#include "../../modules/network/netconfig.h"
#include "../../modules/authentication/authentication_service.h"
#define COMMANDID   111
//Get current IP
//[ESP111] [json=no]
bool Commands::ESP111(const char* cmd_params, level_authenticate_type auth_type, ESP3DOutput * output)
{
    log_esp3d("Client is %d", output?output->client():0);
    (void)auth_type;
    bool noError = true;
    bool json = has_tag (cmd_params, "json");
    String response ;
    String parameter = clean_param(get_param (cmd_params, ""));
    if (parameter.length() == 0) {
        response = format_response(COMMANDID, json, true, NetConfig::localIP().c_str());
    } else {
        parameter = get_param (cmd_params, "OUTPUT=");
        if (parameter != "PRINTER") {
            response = format_response(COMMANDID, json, false, "Unknown parameter");
        }
    }

    if (noError) {
        parameter = get_param (cmd_params, "OUTPUT=");
        if (json) {
            output->printLN (response.c_str() );
        } else {
            output->printMSG (response.c_str() );
            if (parameter == "PRINTER") {
                ESP3DOutput printerOutput(ESP_REMOTE_SCREEN_CLIENT);
                printerOutput.printMSG (NetConfig::localIP().c_str() );
            }
        }
    } else {
        output->printERROR(response.c_str(), 200);
    }
    return noError;
}

#endif //WIFI_FEATURE
