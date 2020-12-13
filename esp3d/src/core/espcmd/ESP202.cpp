/*
 ESP202.cpp - ESP3D command class

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
#if defined (SD_DEVICE)
#include "../commands.h"
#include "../esp3doutput.h"
#include "../settings_esp3d.h"
#include "../../modules/filesystem/esp_sd.h"
#include "../../modules/authentication/authentication_service.h"
//Get/Set SD card Speed factor 1 2 4 6 8 16 32
//[ESP202]SPEED=<value>pwd=<user/admin password>
bool Commands::ESP202(const char* cmd_params, level_authenticate_type auth_type, ESP3DOutput * output)
{
    (void)cmd_params;
#ifdef AUTHENTICATION_FEATURE
    if (auth_type == LEVEL_GUEST) {
        output->printERROR("Wrong authentication!", 401);
        return false;
    }
#else
    (void)auth_type;
#endif //AUTHENTICATION_FEATURE
    bool response = true;
    String parameter;
    parameter = get_param (cmd_params, "");
    //get
    if (parameter.length() == 0) {
        String r = "SPEED="  + String(Settings_ESP3D::read_byte (ESP_SD_SPEED_DIV));
        output->printMSG (r.c_str());
    } else { //set
        parameter = get_param (cmd_params, "SPEED=");
        if ((parameter == "1") || (parameter == "2") || (parameter == "4")|| (parameter == "6")|| (parameter == "8")|| (parameter == "16")|| (parameter == "32")) {
            if (!Settings_ESP3D::write_byte (ESP_SD_SPEED_DIV, parameter.toInt())) {
                response = false;
                output->printERROR ("Set failed!");
            } else {
                ESP_SD::setSPISpeedDivider(parameter.toInt());
                output->printMSG ("ok");
            }
        } else {
            output->printERROR ("Invalid parameter!");
            response = false;
        }
    }
    return response;
}

#endif //SD_DEVICE
