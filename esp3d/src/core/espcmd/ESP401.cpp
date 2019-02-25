/*
 ESP401.cpp - ESP3D command class

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
#include "../commands.h"
#include "../esp3doutput.h"
#include "../settings_esp3d.h"
#include "../../modules/authentication/authentication_service.h"
//Set EEPROM setting
//[ESP401]P=<position> T=<type> V=<value> pwd=<user/admin password>
bool Commands::ESP401(const char* cmd_params, level_authenticate_type auth_type, ESP3DOutput * output)
{
    bool response = true;
    String parameter;
#ifdef AUTHENTICATION_FEATURE
    if (auth_type != LEVEL_ADMIN) {
        output->printERROR("Wrong authentication!", 401);
        return false;
    }
#endif //AUTHENTICATION_FEATURE
    //check validity of parameters
    String spos = get_param (cmd_params, "P=");
    String styp = get_param (cmd_params, "T=");
    String sval = get_param (cmd_params, "V=");
    if (spos.length() == 0) {
        response = false;
    }
    if (! (styp == "B" || styp == "S" || styp == "A" || styp == "I" || styp == "F") ) {
        response = false;
    }

    if (response) {
        //Byte value
        if ((styp == "B")  ||  (styp == "F")) {
            if (!Settings_ESP3D::write_byte (spos.toInt(), sval.toInt())) {
                response = false;
            } else {
                //dynamique refresh is better than restart the boards
                switch(spos.toInt()) {
                case ESP_TARGET_FW:
                    Settings_ESP3D::GetFirmwareTarget(true);
                    break;
                default:
                    break;
                }
            }
        }
        //Integer value
        if (styp == "I") {
            if (!Settings_ESP3D::write_uint32 (spos.toInt(), sval.toInt())) {
                response = false;
            } else {
                //dynamique refresh is better than restart the board
                //TBD
            }
        }
        //String value
        if (styp == "S") {
            if (!Settings_ESP3D::write_string (spos.toInt(), sval.c_str())) {
                response = false;
            } else {
                //dynamique refresh is better than restart the board
                switch(spos.toInt()) {
#ifdef AUTHENTICATION_FEATURE
                case ESP_ADMIN_PWD:
                case ESP_USER_PWD:
                    AuthenticationService::update();
                    break;
#endif //AUTHENTICATION_FEATURE 
                default:
                    break;
                }
            }
        }
#if defined (WIFI_FEATURE)
        //IP address
        if (styp == "A") {
            if (!Settings_ESP3D::write_IP_String (spos.toInt(), sval.c_str())) {
                response = false;
            } else {
                //dynamique refresh is better than restart the board
                //TBD
            }
        }
#endif //WIFI_FEATURE
    }
    if (!response) {
        output->printERROR ("Incorrect command!");
    } else {
        output->printMSG("ok");
    }

    return response;
}
