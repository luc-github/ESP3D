/*
 ESP111.cpp - ESP3D command class

 Copyright (c) 2014 Luc Lebosse. All rights reserved.

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#include "../../include/esp3d_config.h"
#if defined( WIFI_FEATURE) || defined (ETH_FEATURE)
#include "../commands.h"
#include "../esp3doutput.h"
#include "../settings_esp3d.h"
#if defined (WIFI_FEATURE)
#include "../../modules/wifi/wificonfig.h"
#endif //WIFI_FEATURE
#if defined (ETH_FEATURE)
#include "../../modules/ethernet/ethconfig.h"
#endif //ETH_FEATURE
#include "../../modules/authentication/authentication_service.h"
//Get current IP
//[ESP111]
bool Commands::ESP111(const char* cmd_params, level_authenticate_type auth_type, ESP3DOutput * output)
{
    bool response = true;
    String parameter;
    String res = get_param (cmd_params, "");
    String currentIP="";
#if defined( WIFI_FEATURE)
    if (WiFi.getMode() == WIFI_STA) {
        currentIP = WiFi.localIP().toString();
    } else if (WiFi.getMode() == WIFI_AP) {
        currentIP = WiFi.softAPIP().toString();
    }
#endif //WIFI_FEATURE
#if defined (ETH_FEATURE)
    if (EthConfig::started()) {
        currentIP = ETH.localIP().toString();
    }
#endif //ETH_FEATURE
    if (currentIP.length() == 0) {
        currentIP = "0.0.0.0";
    }
    res += currentIP;
    //log_esp3d("Client %d", output->client());
    output->printMSG (res.c_str());
    return response;
}

#endif //WIFI_FEATURE
