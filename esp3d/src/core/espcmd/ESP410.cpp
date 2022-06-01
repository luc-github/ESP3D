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
//[ESP410]json=<no>
#define COMMANDID   410
bool Commands::ESP410(const char* cmd_params, level_authenticate_type auth_type, ESP3DOutput * output)
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
        if (parameter.length() == 0) {
            //Backup current mode
            uint8_t currentmode = WiFi.getMode();
            int n = 0;
            uint8_t total = 0;
            if (!json) {
                output->printMSGLine ("Start Scan");
            }
            if(currentmode==WIFI_AP) {
                WiFi.mode(WIFI_AP_STA);
            }
            n = WiFi.scanNetworks ();
            if(currentmode==WIFI_AP) {
                WiFi.mode((WiFiMode_t)currentmode);
            }
            if (json) {
                output->print ("{\"cmd\":\"410\",\"status\":\"ok\",\"data\":[");
            }
            String line;
            for (int i = 0; i < n; ++i) {
                line = "";
                if (WiFi.RSSI (i)>= MIN_RSSI) {
                    if (total > 0) {
                        if (json) {
                            line+=",";
                        }
                    }
                    total++;
                    if (json) {
                        line += "{\"SSID\":\"";
                        line +=ESP3DOutput::encodeString(WiFi.SSID (i).c_str());
                    } else {
                        line +=WiFi.SSID (i).c_str();
                    }
                    if (json) {
                        line +="\",\"SIGNAL\":\"";
                    } else {
                        line +="\t";
                    }
                    line += String(WiFiConfig::getSignal (WiFi.RSSI (i) ));
                    if (!json) {
                        line +="%";
                    }
                    if (json) {
                        line +="\",\"IS_PROTECTED\":\"";
                    }
                    if (WiFi.encryptionType (i) == ENC_TYPE_NONE) {
                        if (json) {
                            line +="0";
                        } else {
                            line +="\tOpen";
                        }
                    } else {
                        if (json) {
                            line +="1";
                        } else {
                            line +="\tSecure";
                        }
                    }
                    if (json) {
                        line +="\"}";
                    }
                    if (json) {
                        output->print (line.c_str());
                    } else {
                        output->printMSGLine (line.c_str());
                    }
                }
            }
            WiFi.scanDelete();
            if (json) {
                output->printLN ("]}");
            } else {
                output->printMSGLine ("End Scan");
            }
            return true;
        } else {
            response = format_response(COMMANDID, json, false, "This command doesn't take parameters");
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

#endif //WIFI_FEATURE
