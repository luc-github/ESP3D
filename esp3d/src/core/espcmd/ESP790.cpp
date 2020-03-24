/*
 ESP790.cpp - ESP3D command class

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
#if defined (GLOBAL_FILESYSTEM_FEATURE)
#include "../commands.h"
#include "../esp3doutput.h"
#include "../settings_esp3d.h"
#include "../../modules/authentication/authentication_service.h"
#include "../../modules/filesystem/esp_globalFS.h"
// Action on Global Filesystem
//rmdir / remove / mkdir / exists /create
//[ESP790]<Action>=<path> pwd=<admin password>
bool Commands::ESP790(const char* cmd_params, level_authenticate_type auth_type, ESP3DOutput * output)
{
    bool response = true;
    String parameter;
    parameter = get_param (cmd_params, "");
#ifdef AUTHENTICATION_FEATURE
    if (auth_type != LEVEL_ADMIN) {
        output->printERROR("Wrong authentication!", 401);
        return false;
    }
#else
    (void)auth_type;
#endif //AUTHENTICATION_FEATURE
    parameter = get_param (cmd_params, "mkdir=");
    if (parameter.length() != 0) {
        if (ESP_GBFS::mkdir(parameter.c_str())) {
            output->printMSG ("ok");
        } else {
            output->printERROR ("failed!");
            response = false;
        }
        return response;
    }
    parameter = get_param (cmd_params, "rmdir=");
    if (parameter.length() != 0) {
        if (ESP_GBFS::rmdir(parameter.c_str())) {
            output->printMSG ("ok");
        } else {
            output->printERROR ("failed!");
            response = false;
        }
        return response;
    }
    parameter = get_param (cmd_params, "remove=");
    if (parameter.length() != 0) {
        if (ESP_GBFS::remove(parameter.c_str())) {
            output->printMSG ("ok");
        } else {
            output->printERROR ("failed!");
            response = false;
        }
        return response;
    }
    parameter = get_param (cmd_params, "exists=");
    if (parameter.length() != 0) {
        if (ESP_GBFS::exists(parameter.c_str())) {
            output->printMSG ("yes");
        } else {
            output->printMSG ("no");
        }
        return response;
    }
    parameter = get_param (cmd_params, "create=");
    if (parameter.length() != 0) {
        ESP_GBFile f = ESP_GBFS::open(parameter.c_str(), ESP_FILE_WRITE);
        if (f.isOpen()) {
            f.close();
            output->printMSG ("ok");
        } else {
            output->printERROR ("failed!");
            response = false;
        }
        return response;
    }
    output->printERROR ("Incorrect command!");
    return false;
}

#endif //GLOBAL_FILESYSTEM_FEATURE
