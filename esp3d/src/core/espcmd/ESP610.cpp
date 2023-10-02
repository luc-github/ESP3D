/*
 ESP610.cpp - ESP3D command class

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
#if defined (NOTIFICATION_FEATURE)
#include "../commands.h"
#include "../esp3doutput.h"
#include "../settings_esp3d.h"
#include "../../modules/authentication/authentication_service.h"
#include "../../modules/notifications/notifications_service.h"
#define COMMANDID   610
//Set/Get Notification settings
//[ESP610]type=<NONE/PUSHOVER/EMAIL/LINE/IFTTT> T1=<token1> T2=<token2> TS=<Settings> json=<no> [pwd=<admin password>]
//Get will give type and settings only not the protected T1/T2
bool Commands::ESP610(const char* cmd_params, level_authenticate_type auth_type, ESP3DOutput * output)
{
    bool noError = true;
    bool json = has_tag (cmd_params, "json");
    String response;
    String parameter;
    bool hasParam = false;
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
            uint8_t Ntype =  Settings_ESP3D::read_byte(ESP_NOTIFICATION_TYPE);
            String tmp;
            if (json) {
                tmp = "{\"type\":\"";
            } else {
                tmp = "type=";
            }
            switch(Ntype) {
            case ESP_PUSHOVER_NOTIFICATION:
                tmp += "PUSHOVER";
                break;
            case ESP_EMAIL_NOTIFICATION:
                tmp += "EMAIL";
                break;
            case ESP_LINE_NOTIFICATION:
                tmp += "LINE";
                break;
            case ESP_TELEGRAM_NOTIFICATION:
                tmp += "TELEGRAM";
                break;
            case ESP_IFTTT_NOTIFICATION:
                tmp += "IFTTT";
                break;
            default:
                tmp+= "NONE";
            }
            if (json) {
                tmp +="\"";
            }
            String ts = Settings_ESP3D::read_string(ESP_NOTIFICATION_SETTINGS);
            if (ts.length() > 0 && ts!="NONE") {
                if (json) {
                    tmp += ",\"TS\":\"";
                } else {
                    tmp += ", TS=";
                }
                tmp+= ts;
                if (json) {
                    tmp += "\"}";
                }
            }
            if (json) {
                tmp += "}";
            }
            response = format_response(COMMANDID, json, true, tmp.c_str());
        } else {
            //type
            parameter = get_param (cmd_params, "type=");
            if (parameter.length() > 0) {
                hasParam = true;
                uint8_t Ntype;
                parameter.toUpperCase();
                if (parameter == "NONE") {
                    Ntype = 0;
                } else if (parameter == "PUSHOVER") {
                    Ntype = ESP_PUSHOVER_NOTIFICATION;
                } else if (parameter == "EMAIL") {
                    Ntype = ESP_EMAIL_NOTIFICATION;
                } else if (parameter == "LINE") {
                    Ntype = ESP_LINE_NOTIFICATION;
                } else if (parameter == "TELEGRAM") {
                    Ntype = ESP_TELEGRAM_NOTIFICATION;
                } else if (parameter == "IFTTT") {
                    Ntype = ESP_IFTTT_NOTIFICATION;
                } else {
                    response = format_response(COMMANDID, json, false, "Only NONE, PUSHOVER, EMAIL, LINE, IFTTT are supported");
                    noError = false;
                }
                if (noError) {
                    if(!Settings_ESP3D::write_byte(ESP_NOTIFICATION_TYPE, Ntype)) {
                        response = format_response(COMMANDID, json, false, "Set failed");
                        noError = false;
                    }
                }
            }
            //Settings
            if (noError) {
                parameter = get_param (cmd_params, "TS=");
                if (parameter.length() > 0) {
                    hasParam = true;
                    if(!Settings_ESP3D::write_string(ESP_NOTIFICATION_SETTINGS, parameter.c_str())) {
                        response = format_response(COMMANDID, json, false, "Set TS failed");
                        noError = false;
                    }
                }
            }
            //Token1
            if (noError) {
                parameter = get_param (cmd_params, "T1=");
                if (parameter.length() > 0) {
                    hasParam = true;
                    if(!Settings_ESP3D::write_string(ESP_NOTIFICATION_TOKEN1, parameter.c_str())) {
                        response = format_response(COMMANDID, json, false, "Set T1 failed");
                        noError = false;
                    }
                }
            }
            //Token2
            if (noError) {
                parameter = get_param (cmd_params, "T2=");
                if (parameter.length() > 0) {
                    hasParam = true;
                    if(!Settings_ESP3D::write_string(ESP_NOTIFICATION_TOKEN2, parameter.c_str())) {
                        response = format_response(COMMANDID, json, false, "Set T2 failed");
                        noError = false;
                    } else {
                        response = true;
                    }
                }
            }
            if (noError) {
                if (hasParam) {
                    //Restart service
                    notificationsservice.begin();
                    response = format_response(COMMANDID, json, true, "ok");
                } else {
                    response = format_response(COMMANDID, json, false, "Only type, T1, T2 and TS not empty are supported");
                    noError = false;
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

#endif //NOTIFICATION_FEATURE
