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
#define COMMANDID   140
//Sync / Set / Get current time
//[ESP140]<SYNC> <srv1=XXXXX> <srv2=XXXXX> <srv3=XXXXX> <zone=xxx> <dst=YES/NO> <time=YYYY-MM-DD#H24:MM:SS> NOW json=<no> pwd=<admin password>
bool Commands::ESP140(const char* cmd_params, level_authenticate_type auth_type, ESP3DOutput * output)
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
        parameter = clean_param(get_param (cmd_params, ""));//get
        if (parameter.length() != 0) {
#ifdef AUTHENTICATION_FEATURE
            if (auth_type != LEVEL_ADMIN) {
                response = format_response(COMMANDID, json, false, "Wrong authentication level");
                noError = false;
                errorCode = 401;
            }
#endif //AUTHENTICATION_FEATURE
            if (noError) {
                bool hasParam = false;
                String s = get_param (cmd_params, "srv1=");
                if (s.length() > 0 ) {
                    hasParam = true;
                    if(s.length() < Settings_ESP3D::get_max_string_size(ESP_TIME_SERVER1)) {
                        if (!Settings_ESP3D::write_string (ESP_TIME_SERVER1, s.c_str())) {
                            response = format_response(COMMANDID, json, false, "Set server 1 failed");
                            noError = false;
                        }
                    }
                }
            }
            if (noError) {
                s = get_param (cmd_params, "srv2=");
                if (s.length() > 0 ) {
                    hasParam = true;
                    if (s.length() < Settings_ESP3D::get_max_string_size(ESP_TIME_SERVER2)) {
                        if (!Settings_ESP3D::write_string (ESP_TIME_SERVER2, s.c_str())) {
                            response = format_response(COMMANDID, json, false, "Set server 2 failed");
                            noError = false;
                        }
                    }
                }
            }
            if (noError) {
                s = get_param (cmd_params, "srv3=");
                if (s.length() > 0 ) {
                    hasParam = true;
                    if ( s.length() < Settings_ESP3D::get_max_string_size(ESP_TIME_SERVER3)) {
                        if (!Settings_ESP3D::write_string (ESP_TIME_SERVER3, s.c_str())) {
                            response = format_response(COMMANDID, json, false, "Set server 3 failed");
                            noError = false;
                        }
                    }
                }
            }
            if (noError) {
                s = get_param (cmd_params, "zone=");
                if (s.length() > 0 ) {
                    hasParam = true;
                    if ((s.toInt() <= (int8_t)Settings_ESP3D::get_max_byte(ESP_TIMEZONE)) && (s.toInt() >= (int8_t)Settings_ESP3D::get_min_byte(ESP_TIMEZONE))) {
                        if (!Settings_ESP3D::write_byte (ESP_TIMEZONE, s.toInt())) {
                            response = format_response(COMMANDID, json, false, "Set time zone failed");
                            noError = false;
                        }
                    }
                }
            }
            if (noError) {
                s = get_param (cmd_params, "dst=");
                if (s.length() > 0 ) {
                    hasParam = true;
                    s.toUpperCase();
                    if (s.length() > 0 ) {
                        if (!Settings_ESP3D::write_byte (ESP_TIME_IS_DST, (s == "NO")?0:1)) {
                            response = format_response(COMMANDID, json, false, "Set dayligh failed");
                            noError = false;
                        }
                    }
                }
            }
            if (noError) {
                s = get_param (cmd_params, "time=");
                s.toUpperCase();
                if (s.length() > 0 ) {
                    hasParam = true;
                    if(!timeserver.setTime(s.c_str())) {
                        response = format_response(COMMANDID, json, false, "Set time failed");
                        noError = false;
                    }
                }
            }
            if (noError) {
                if (has_tag(parameter.c_str(), "SYNC")) {
                    hasParam=true;
                    if (timeserver.is_internet_time()) {
                        if(!timeserver.begin()) {
                            response = format_response(COMMANDID, json, false, "Init time failed");
                            noError = false;
                        }
                    } else {
                        response = format_response(COMMANDID, json, false, "Time is manual");
                        noError = false;
                    }
                    if (noError) {
                        response = format_response(COMMANDID, json, true, timeserver.current_time());
                    }
                }
            }
            if (noError) {
                if (has_tag(parameter.c_str(), "NOW")) {
                    hasParam = true;
                    response = format_response(COMMANDID, json, true, timeserver.current_time());
                }
            }
            if (noError && !hasParam) {
                response = format_response(COMMANDID, json, false, "No parameter");
                noError = false;
            }
        } else {
            //get display settings
            String tmp = "";
            if (json) {
                tmp += "{\"srv1\":\"";
            } else {
                tmp += "srv1=";
            }
            tmp+= Settings_ESP3D::read_string(ESP_TIME_SERVER1);
            if (json) {
                tmp += "\",\"srv2\":\"";
            } else {
                tmp += ", srv2=";
            }
            tmp+= Settings_ESP3D::read_string(ESP_TIME_SERVER2);
            if (json) {
                tmp += "\",\"srv3\":\"";
            } else {
                tmp += ", srv3=";
            }
            tmp+= Settings_ESP3D::read_string(ESP_TIME_SERVER3);
            if (json) {
                tmp += "\",\"zone\":\"";
            } else {
                tmp += ", zone=";
            }
            tmp+= Settings_ESP3D::READ_byte (ESP_TIMEZONE);
            if (json) {
                tmp += "\",\"dst\":\"";
            } else {
                tmp += ", dst=";
            }
            tmp+= Settings_ESP3D::READ_byte (ESP_TIME_IS_DST)?"YES":"NO";
            if (json) {
                tmp += "\"}";
            }
            response =   format_response(COMMANDID, json, true, tmp.c_str());
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

#endif //TIMESTAMP_FEATURE
