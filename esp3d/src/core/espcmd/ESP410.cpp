/*
 ESP410.cpp - ESP3D command class

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
#if defined (WIFI_FEATURE)
#include "../commands.h"
#include "../esp3doutput.h"
#include "../settings_esp3d.h"
#include "../../modules/wifi/wificonfig.h"
#include "../../modules/authentication/authentication_service.h"
//Get available AP list (limited to 30)
//output is JSON or plain text according parameter
//[ESP410]<plain>
bool Commands::ESP410(const char* cmd_params, level_authenticate_type auth_type, ESP3DOutput * output)
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
    //Backup current mode
    uint8_t currentmode = WiFi.getMode();
    bool plain = hastag(cmd_params,"plain");
    int n = 0;
    if (plain) {
        output->printLN ("Start Scan");
        output->flush();
    }
    n = WiFi.scanNetworks ();
    if (!plain) {
        output->print ("{\"AP_LIST\":[");
    }
    for (int i = 0; i < n; ++i) {
        if (i > 0) {
            if (!plain) {
                output->print (",");
            } else {
                output->printLN ("");
            }
        }
        if (!plain) {
            output->print ("{\"SSID\":\"");
        }
        output->print (WiFi.SSID (i).c_str());
        if (!plain) {
            output->print ("\",\"SIGNAL\":\"");
        } else {
            output->print ("\t");
        }
        output->print (String(WiFiConfig::getSignal (WiFi.RSSI (i) )));
        if (plain) {
            output->print("%");
        }
        if (!plain) {
            output->print ("\",\"IS_PROTECTED\":\"");
        }
        if (WiFi.encryptionType (i) == ENC_TYPE_NONE) {
            if (!plain) {
                output->print ("0");
            } else {
                output->print ("\tOpen");
            }
        } else {
            if (!plain) {
                output->print ("1");
            } else {
                output->print ("\tSecure");
            }
        }
        if (!plain) {
            output->print ("\"}");
        }
    }
    WiFi.scanDelete();
    if (!plain) {
        output->printLN ("]}");
    } else {
        output->printLN ("");
        output->printLN ("End Scan");
    }
    WiFi.mode((WiFiMode_t)currentmode);
    return response;
}

#endif //WIFI_FEATURE
