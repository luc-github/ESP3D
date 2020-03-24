/*
 ESP700.cpp - ESP3D command class

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
#if defined (FILESYSTEM_FEATURE) && defined(ESP_GCODE_HOST_FEATURE)
#include "../commands.h"
#include "../esp3doutput.h"
#include "../settings_esp3d.h"
#include "../../modules/authentication/authentication_service.h"
#include "../../modules/filesystem/esp_filesystem.h"
#include "../../modules/gcode_host/gcode_host.h"
////read local file
//[ESP700]<filename>
bool Commands::ESP700(const char* cmd_params, level_authenticate_type auth_type, ESP3DOutput * output)
{
    bool response = true;
    String parameter;
    parameter = get_param (cmd_params, "");
#ifdef AUTHENTICATION_FEATURE
    if (auth_type != LEVEL_ADMIN) {
        output->printERROR("Wrong authentication!", 401);
        response = false;
    } else
#else
    (void)auth_type;
#endif //AUTHENTICATION_FEATURE
    {
        esp3d_gcode_host.processFile(parameter.c_str(), auth_type, output);
        output->printMSG("ok");
    }
    return response;
}

#endif //FILESYSTEM_FEATURE && ESP_GCODE_HOST_FEATURE
