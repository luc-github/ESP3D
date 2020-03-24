/*
 ESP103.cpp - ESP3D command class

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
#if defined (WIFI_FEATURE) || defined (ETH_FEATURE)
#include "../commands.h"
#include "../esp3doutput.h"
#include "../settings_esp3d.h"
#include "../../modules/network/netconfig.h"
#if defined (WIFI_FEATURE)
#include "../../modules/wifi/wificonfig.h"
#endif //WIFI_FEATURE 
#if defined (ETH_FEATURE)
#include "../../modules/ethernet/ethconfig.h"
#endif //ETH_FEATURE
#include "../../modules/authentication/authentication_service.h"
//Change STA IP/Mask/GW
//[ESP103]IP=<IP> MSK=<IP> GW=<IP> pwd=<admin password>
bool Commands::ESP103(const char* cmd_params, level_authenticate_type auth_type, ESP3DOutput * output)
{
    bool response = true;
    String parameter;
#ifdef AUTHENTICATION_FEATURE
    if (auth_type == LEVEL_GUEST) {
        output->printERROR("Wrong authentication!", 401);
        return false;
    }
#else
    (void)auth_type;
#endif //AUTHENTICATION_FEATURE
    parameter = get_param (cmd_params, "");
    //get
    if (parameter.length() == 0) {
        String res = "IP:";
        res += Settings_ESP3D::read_IP_String(ESP_STA_IP_VALUE);
        res += ", GW:";
        res += Settings_ESP3D::read_IP_String(ESP_STA_GATEWAY_VALUE);
        res += ", MSK:";
        res += Settings_ESP3D::read_IP_String(ESP_STA_MASK_VALUE);
        output->printMSG (res.c_str());
    } else { //set
#ifdef AUTHENTICATION_FEATURE
        if (auth_type != LEVEL_ADMIN) {
            output->printERROR("Wrong authentication!", 401);
            return false;
        }
#endif //AUTHENTICATION_FEATURE

        String IP = get_param (cmd_params, "IP=");
        String GW = get_param (cmd_params, "GW=");
        String MSK = get_param (cmd_params, "MSK=");
        if ( !NetConfig::isValidIP(IP.c_str())) {
            output->printERROR ("Incorrect IP!");
            return false;
        }
        if ( !NetConfig::isValidIP(GW.c_str())) {
            output->printERROR ("Incorrect gateway!");
            return false;
        }
        if ( !NetConfig::isValidIP(MSK.c_str())) {
            output->printERROR ("Incorrect mask!");
            return false;
        }
        if ( !Settings_ESP3D::write_IP_String(ESP_STA_IP_VALUE, IP.c_str()) ||
                !Settings_ESP3D::write_IP_String(ESP_STA_GATEWAY_VALUE, GW.c_str()) ||
                !Settings_ESP3D::write_IP_String(ESP_STA_MASK_VALUE, MSK.c_str())) {
            output->printERROR ("Set failed!");
            response = false;
        } else {
            output->printMSG ("ok");
        }
    }
    return response;
}

#endif //WIFI_FEATURE || ETH_FEATURE
