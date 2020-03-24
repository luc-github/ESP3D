/*
 ESP250.cpp - ESP3D command class

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
#if defined (BUZZER_DEVICE)
#include "../commands.h"
#include "../esp3doutput.h"
#include "../settings_esp3d.h"
#include "../../modules/authentication/authentication_service.h"
#include "../../modules/buzzer/buzzer.h"
//Play sound
//[ESP250]F=<frequency> D=<duration> [pwd=<user password>]
bool Commands::ESP250(const char* cmd_params, level_authenticate_type auth_type, ESP3DOutput * output)
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
    if (!esp3d_buzzer.started()) {
        output->printERROR ("Buzzer disabled");
        return false;
    }
    parameter = get_param (cmd_params, "");
    //get
    if (parameter.length() == 0) {
        esp3d_buzzer.beep();
    } else {
        int f,d;
        //frequency
        parameter = get_param (cmd_params, "F=");
        if (parameter.length() == 0) {
            output->printERROR ("No frequency");
            return false;
        }
        f = parameter.toInt();
        parameter = get_param (cmd_params, "D=");
        if (parameter.length() == 0) {
            output->printERROR ("No duration");
            return false;
        }
        d = parameter.toInt();
        esp3d_buzzer.playsound(f,d);
    }
    output->printMSG ("ok");
    return response;
}

#endif //BUZZER_DEVICE
