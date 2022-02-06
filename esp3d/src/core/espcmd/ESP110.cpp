/*
 ESP110.cpp - ESP3D command class

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
#if defined( WIFI_FEATURE) ||  defined( BLUETOOTH_FEATURE) || defined (ETH_FEATURE)
#include "../commands.h"
#include "../esp3doutput.h"
#include "../settings_esp3d.h"
#include "../../modules/network/netconfig.h"
#include "../../modules/authentication/authentication_service.h"
//Set radio state at boot which can be BT, WIFI-STA, WIFI-AP, ETH-STA, OFF
//[ESP110]<state>pwd=<admin password>
bool Commands::ESP110(const char* cmd_params, level_authenticate_type auth_type, ESP3DOutput * output)
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
        int8_t wifiMode = Settings_ESP3D::read_byte(ESP_RADIO_MODE);
        if (wifiMode == ESP_NO_NETWORK) {
            output->printMSG("OFF");
        } else if (wifiMode == ESP_BT) {
            output->printMSG("BT");
        } else if (wifiMode == ESP_WIFI_AP) {
            output->printMSG("WIFI-AP");
        } else if (wifiMode == ESP_WIFI_STA) {
            output->printMSG("WIFI-STA");
//           } else if (wifiMode == ESP_ETH_SRV) {
//               output->printMSG("ETH-SRV");
        } else if (wifiMode == ESP_ETH_STA) {
            output->printMSG("ETH-STA");
        } else if (wifiMode == ESP_AP_SETUP) {
            output->printMSG("WIFI-SETUP");
        } else {
            output->printMSG("??");
        }
    } else { //set
#ifdef AUTHENTICATION_FEATURE
        if (auth_type != LEVEL_ADMIN) {
            output->printERROR("Wrong authentication!", 401);
            return false;
        }
#endif //AUTHENTICATION_FEATURE
        parameter.toUpperCase();
        if (!(
#if defined( BLUETOOTH_FEATURE)
                    (parameter == "BT") ||
#endif //BLUETOOTH_FEATURE     
#if defined( WIFI_FEATURE)
                    (parameter == "WIFI-STA") || (parameter == "WIFI-AP") || (parameter == "WIFI-SETUP") ||
#endif //WIFI_FEATURE
#if defined( ETH_FEATURE)
                    (parameter == "ETH-STA") || //(parameter == "ETH-SRV") ||
#endif //ETH_FEATURE
                    (parameter == "OFF"))) {

            output->printERROR ("Only "
#ifdef BLUETOOTH_FEATURE
                                "BT or "
#endif //BLUETOOTH_FEATURE
#ifdef WIFI_FEATURE
                                "WIFI-STA or WIFI-AP or WIFI-SETUP or "
#endif //WIFI_FEATURE
#ifdef ETH_FEATURE
                                "ETH-STA or "
#endif //ETH_FEATURE
                                "OFF mode supported!");
            return false;
        }

        int8_t bbuf = ESP_NO_NETWORK;
#ifdef WIFI_FEATURE
        if(parameter == "WIFI-STA") {
            bbuf = ESP_WIFI_STA;
        }
        if(parameter == "WIFI-AP") {
            bbuf = ESP_WIFI_AP;
        }
        if(parameter == "WIFI-SETUP") {
            bbuf = ESP_AP_SETUP;
        }
#endif //WIFI_FEATURE
#ifdef ETH_FEATURE
        if(parameter == "ETH-STA") {
            bbuf = ESP_ETH_STA;
        }
//            if(parameter == "ETH-SRV") {
//                bbuf = ESP_ETH_SRV;
//            }
#endif //ETH_FEATURE
#ifdef BLUETOOTH_FEATURE
        if(parameter == "BT") {
            bbuf = ESP_BT;
        }
#endif //BLUETOOTH_FEATURE
        if (!Settings_ESP3D::write_byte(ESP_RADIO_MODE, bbuf)) {
            output->printERROR ("Set failed!");
            response = false;
        } else {
            if (!NetConfig::begin()) {
                output->printERROR ("Cannot setup network");
                response = false;
            }
        }
    }
    return response;
}

#endif //WIFI_FEATURE
