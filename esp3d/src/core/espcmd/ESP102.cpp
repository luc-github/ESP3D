/*
 ESP102.cpp - ESP3D command class

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
//Change STA IP mode (DHCP/STATIC)
//[ESP102]<mode>pwd=<admin password>
bool Commands::ESP102(const char* cmd_params, level_authenticate_type auth_type, ESP3DOutput * output)
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
        int8_t resp = Settings_ESP3D::read_byte(ESP_STA_IP_MODE);
        if (resp == DHCP_MODE) {
            output->printMSG("DHCP");
        } else if (resp == STATIC_IP_MODE) {
            output->printMSG("STATIC");
        } else {
            output->printMSG("Unknown");
        }
    } else { //set
#ifdef AUTHENTICATION_FEATURE
        if (auth_type != LEVEL_ADMIN) {
            output->printERROR("Wrong authentication!", 401);
            return false;
        }
#endif //AUTHENTICATION_FEATURE
        parameter.toUpperCase();
        if (!((parameter == "STATIC") || (parameter == "DHCP"))) {
            output->printERROR ("only STATIC or DHCP mode supported!");
            return false;
        } else {
            uint8_t bbuf = (parameter == "DHCP")?DHCP_MODE:STATIC_IP_MODE;
            if (!Settings_ESP3D::write_byte(ESP_STA_IP_MODE, bbuf)) {
                output->printERROR ("Set failed!");
                response = false;
            } else {
                output->printMSG ("ok");
            }
        }
    }
    return response;
}

#endif //WIFI_FEATURE || ETH_FEATURE
