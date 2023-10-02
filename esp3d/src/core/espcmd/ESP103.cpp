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
#define COMMANDID   103
//Change STA IP/Mask/GW
//[ESP103]IP=<IP> MSK=<IP> GW=<IP> [json=no] [pwd=<admin password>
bool Commands::ESP103(const char* cmd_params, level_authenticate_type auth_type, ESP3DOutput * output)
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
            String res;
            if(json) {
                res+= "{\"ip\":\"";
            } else {
                res+= "IP:";
            }
            res += Settings_ESP3D::read_IP_String(ESP_STA_IP_VALUE);
            if(json) {
                res+= "\",\"gw\":\"";
            } else {
                res += ", GW:";
            }
            res += Settings_ESP3D::read_IP_String(ESP_STA_GATEWAY_VALUE);
            if(json) {
                res+= "\",\"msk\":\"";
            } else {
                res += ", MSK:";
            }
            res += Settings_ESP3D::read_IP_String(ESP_STA_MASK_VALUE);
            if(json) {
                res+= "\",\"dns\":\"";
            } else {
                res += ", DNS:";
            }
            res += Settings_ESP3D::read_IP_String(ESP_STA_DNS_VALUE);
            if(json) {
                res+= "\"}";
            }
            response = format_response(COMMANDID, json, true, res.c_str());
        } else { //set
#ifdef AUTHENTICATION_FEATURE
            if (auth_type != LEVEL_ADMIN) {
                response = format_response(COMMANDID, json, false, "Wrong authentication level");
                noError = false;
                errorCode = 401;
            }
#endif //AUTHENTICATION_FEATURE
            if (noError) {
                String IP = get_param (cmd_params, "IP=");
                String GW = get_param (cmd_params, "GW=");
                String MSK = get_param (cmd_params, "MSK=");
                String DNS = get_param (cmd_params, "DNS=");
                if ( !NetConfig::isValidIP(IP.c_str())) {
                    response = format_response(COMMANDID, json, false, "Incorrect IP");
                    noError = false;
                }
                if ( !NetConfig::isValidIP(GW.c_str())) {
                    response = format_response(COMMANDID, json, false, "Incorrect gateway");
                    noError = false;
                }
                if ( !NetConfig::isValidIP(MSK.c_str())) {
                    response = format_response(COMMANDID, json, false, "Incorrect mask");
                    noError = false;
                }
                if ( !NetConfig::isValidIP(DNS.c_str())) {
                    response = format_response(COMMANDID, json, false, "Incorrect dns");
                    noError = false;
                }
                if(noError) {

                    if ( !Settings_ESP3D::write_IP_String(ESP_STA_IP_VALUE, IP.c_str()) ||
                            !Settings_ESP3D::write_IP_String(ESP_STA_GATEWAY_VALUE, GW.c_str()) ||
                            !Settings_ESP3D::write_IP_String(ESP_STA_DNS_VALUE, DNS.c_str()) ||
                            !Settings_ESP3D::write_IP_String(ESP_STA_MASK_VALUE, MSK.c_str())) {
                        response = format_response(COMMANDID, json, false, "Set failed");
                        noError = false;
                    } else {
                        response = format_response(COMMANDID, json, true, "ok");
                    }
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

#endif //WIFI_FEATURE || ETH_FEATURE
