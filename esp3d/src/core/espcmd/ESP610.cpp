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
//Set/Get Notification settings
//[ESP610]type=<NONE/PUSHOVER/EMAIL/LINE> T1=<token1> T2=<token2> TS=<Settings> [pwd=<admin password>]
//Get will give type and settings only not the protected T1/T2
bool Commands::ESP610(const char* cmd_params, level_authenticate_type auth_type, ESP3DOutput * output)
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
        uint8_t Ntype =  Settings_ESP3D::read_byte(ESP_NOTIFICATION_TYPE);
        static String tmp;
        tmp = (Ntype == ESP_PUSHOVER_NOTIFICATION)?"PUSHOVER":(Ntype == ESP_EMAIL_NOTIFICATION)?"EMAIL":(Ntype == ESP_LINE_NOTIFICATION)?"LINE":(Ntype == ESP_TELEGRAM_NOTIFICATION)?"TELEGRAM":"NONE";
        tmp+= " ";
        tmp+= Settings_ESP3D::read_string(ESP_NOTIFICATION_SETTINGS);
        output->printMSG (tmp.c_str());
    } else {
        response = false;
        //type
        parameter = get_param (cmd_params, "type=");
        if (parameter.length() > 0) {
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
            } else {
                output->printERROR("Only NONE, PUSHOVER, EMAIL, LINE are supported!");
                return false;
            }
            if(!Settings_ESP3D::write_byte(ESP_NOTIFICATION_TYPE, Ntype)) {
                output->printERROR ("Set failed!");
                return false;
            } else {
                response = true;
            }
        }
        //Settings
        parameter = get_param (cmd_params, "TS=");
        if (parameter.length() > 0) {
            if(!Settings_ESP3D::write_string(ESP_NOTIFICATION_SETTINGS, parameter.c_str())) {
                output->printERROR ("Set failed!");
                return false;
            } else {
                response = true;
            }
        }
        //Token1
        parameter = get_param (cmd_params, "T1=");
        if (parameter.length() > 0) {
            if(!Settings_ESP3D::write_string(ESP_NOTIFICATION_TOKEN1, parameter.c_str())) {
                output->printERROR ("Set failed!");
                return false;
            } else {
                response = true;
            }
        }
        //Token2
        parameter = get_param (cmd_params, "T2=");
        if (parameter.length() > 0) {
            if(!Settings_ESP3D::write_string(ESP_NOTIFICATION_TOKEN2, parameter.c_str())) {
                output->printERROR ("Set failed!");
                return false;
            } else {
                response = true;
            }
        }
        if (response) {
            //Restart service
            notificationsservice.begin();
            output->printMSG ("ok");
        } else {
            output->printERROR ("Invalid parameter! Only type, T1, T2 and TS are supported");
        }
    }
    return response;
}

#endif //NOTIFICATION_FEATURE
