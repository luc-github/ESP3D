/*
 ESP140.cpp - ESP3D command class

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
#if defined (TIMESTAMP_FEATURE)
#include "../commands.h"
#include "../esp3doutput.h"
#include "../settings_esp3d.h"
#include "../../modules/authentication/authentication_service.h"
#include "../../modules/time/time_server.h"
//Sync / Set / Get current time
//[ESP140]<SYNC> <srv1=XXXXX> <srv2=XXXXX> <srv3=XXXXX> <zone=xxx> <dst=YES/NO> <time=YYYY-MM-DD#H24:MM:SS> pwd=<admin password>
bool Commands::ESP140(const char* cmd_params, level_authenticate_type auth_type, ESP3DOutput * output)
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
    if (parameter.length() != 0) {
#ifdef AUTHENTICATION_FEATURE
        if (auth_type != LEVEL_ADMIN) {
            output->printERROR("Wrong authentication!", 401);
            return false;
        }
#endif //AUTHENTICATION_FEATURE
        String s = get_param (cmd_params, "srv1=");
        if (s.length() > 0 && s.length() < Settings_ESP3D::get_max_string_size(ESP_TIME_SERVER1)) {
            if (!Settings_ESP3D::write_string (ESP_TIME_SERVER1, s.c_str())) {
                output->printERROR("Set server 1 failed!");
            }
        }
        s = get_param (cmd_params, "srv2=");
        if (s.length() > 0 && s.length() < Settings_ESP3D::get_max_string_size(ESP_TIME_SERVER2)) {
            if (!Settings_ESP3D::write_string (ESP_TIME_SERVER2, s.c_str())) {
                output->printERROR("Set server 2 failed!");
            }
        }
        s = get_param (cmd_params, "srv3=");
        if (s.length() > 0 && s.length() < Settings_ESP3D::get_max_string_size(ESP_TIME_SERVER3)) {
            if (!Settings_ESP3D::write_string (ESP_TIME_SERVER3, s.c_str())) {
                output->printERROR("Set server 2 failed!");
            }
        }
        s = get_param (cmd_params, "zone=");
        if (s.length() > 0 && (s.toInt() <= (int8_t)Settings_ESP3D::get_max_byte(ESP_TIMEZONE)) && (s.toInt() >= (int8_t)Settings_ESP3D::get_min_byte(ESP_TIMEZONE))) {
            if (!Settings_ESP3D::write_byte (ESP_TIMEZONE, s.toInt())) {
                output->printERROR("Set time zone failed!");
            }
        }
        s = get_param (cmd_params, "dst=");
        s.toUpperCase();
        if (s.length() > 0 ) {
            if (!Settings_ESP3D::write_byte (ESP_TIME_IS_DST, (s == "NO")?0:1)) {
                output->printERROR("Set dayligh failed!");
            }
        }
        s = get_param (cmd_params, "time=");
        s.toUpperCase();
        if (s.length() > 0 ) {
            output->printMSG("Setting time");
            if(!timeserver.setTime(s.c_str())) {
                output->printERROR("Set time failed!");
                response = false;
            }
        }

        if (hastag(parameter.c_str(), "SYNC")) {
            if (timeserver.is_internet_time()) {
                output->printMSG("Contacting time servers");
                if(!timeserver.begin()) {
                    output->printERROR("Init time failed!");
                    response = false;
                }
            } else {
                output->printERROR("Time is manual!");
                response = false;
            }
        }
    }

    output->printMSG(timeserver.current_time());
    return response;
}

#endif //TIMESTAMP_FEATURE
