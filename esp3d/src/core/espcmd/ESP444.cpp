/*
 ESP444.cpp - ESP3D command class

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
#include "../esp3d.h"
#include "../commands.h"
#include "../esp3doutput.h"
#include "../../modules/authentication/authentication_service.h"
//Set ESP State
//cmd are RESTART / RESET
//[ESP444]<cmd><pwd=admin>
bool Commands::ESP444(const char* cmd_params, level_authenticate_type auth_type, ESP3DOutput * output)
{
    bool response = true;
    String parameter;
#ifdef AUTHENTICATION_FEATURE
    if (auth_type != LEVEL_ADMIN) {
        output->printERROR("Wrong authentication!", 401);
        return false;
    }
#else
    (void)auth_type;
#endif //AUTHENTICATION_FEATURE
    if (hastag(cmd_params,"RESTART")) {
        output->printMSG ("Restart ongoing");
        Esp3D::restart_esp();
    } else if (hastag(cmd_params,"RESET")) {
        if (Esp3D::reset()) {
            output->printMSG ("Reset done");
        } else {
            output->printERROR ("Reset failed");
        }
    } else {
        response = false;
        output->printERROR ("Invalid parameter!");
    }
    return response;
}
