/*
 ESP150.cpp - ESP3D command class

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
#include "../commands.h"
#include "../esp3doutput.h"
#include "../settings_esp3d.h"
#include "../../modules/authentication/authentication_service.h"
// Get/Set display/set boot delay in ms / Verbose boot
//[ESP150]<delay=time in milliseconds><verbose=ON/OFF>[pwd=<admin password>]
bool Commands::ESP150(const char* cmd_params, level_authenticate_type auth_type, ESP3DOutput * output)
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
        String s = "delay="+String(Settings_ESP3D::read_uint32(ESP_BOOT_DELAY));
        s+=" verbose=";
        s+= Settings_ESP3D::isVerboseBoot(true)?"ON":"OFF";
        output->printMSG(s.c_str());
    } else {
#ifdef AUTHENTICATION_FEATURE
        if (auth_type != LEVEL_ADMIN) {
            output->printERROR("Wrong authentication!", 401);
            return false;
        }
#endif //AUTHENTICATION_FEATURE
        response = false;
        parameter = get_param (cmd_params, "delay=");
        if (parameter.length() != 0) {
            uint ibuf = parameter.toInt();
            if ((ibuf > Settings_ESP3D::get_max_int32_value(ESP_BOOT_DELAY)) || (ibuf < Settings_ESP3D::get_min_int32_value(ESP_BOOT_DELAY))) {
                output->printERROR ("Incorrect delay!");
                return false;
            }
            if (!Settings_ESP3D::write_uint32 (ESP_BOOT_DELAY, ibuf)) {
                output->printERROR ("Set failed!");
                return false;
            } else {
                response = true;
            }
        }
        parameter = get_param (cmd_params, "verbose=");
        if (parameter.length() != 0) {
            if ((parameter == "ON")|| (parameter == "OFF")) {
                if (!Settings_ESP3D::write_byte (ESP_VERBOSE_BOOT, (parameter == "ON")?1:0)) {
                    output->printERROR ("Set failed!");
                    return false;
                } else {
                    Settings_ESP3D::isVerboseBoot(true);
                    response = true;
                }
            } else {
                output->printERROR ("Incorrect command! only ON/OFF is allowed");
                return false;
            }
            response = true;
        }
        if (!response) {
            output->printERROR ("Incorrect command!");
        } else {
            output->printMSG ("ok");
        }
    }
    return response;
}
