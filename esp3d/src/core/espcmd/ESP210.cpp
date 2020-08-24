/*
 ESP210.cpp - ESP3D command class

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
#if defined (SENSOR_DEVICE)
#include "../commands.h"
#include "../esp3doutput.h"
#include "../settings_esp3d.h"
#include "../../modules/sensor/sensor.h"
#include "../../modules/authentication/authentication_service.h"
//Get Sensor Value / type/Set Sensor type
//[ESP210]<type=NONE/xxx> <interval=XXX in millisec>
bool Commands::ESP210(const char* cmd_params, level_authenticate_type auth_type, ESP3DOutput * output)
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
    if (parameter.length() == 0) {
        String s;
        if(esp3d_sensor.started()) {
            s = esp3d_sensor.GetData();
            s += " ";
            s += esp3d_sensor.GetModelString();
            s += " ";
            s += esp3d_sensor.interval();
            s += "ms";

        } else {
            s = "NONE";
        }
        output->printMSG(s.c_str());
    } else {
#ifdef AUTHENTICATION_FEATURE
        if (auth_type != LEVEL_ADMIN) {
            output->printERROR("Wrong authentication!", 401);
            return false;
        }
#endif //AUTHENTICATION_FEATURE
        parameter = get_param (cmd_params, "type=");
        if (parameter.length() != 0) {
            parameter.toUpperCase();
            int8_t v = -1;
            if (parameter == "NONE") {
                v = 0;
            } else if(esp3d_sensor.isModelValid(esp3d_sensor.getIDFromString(parameter.c_str()))) {
                v = esp3d_sensor.getIDFromString(parameter.c_str());
            }  else {
                output->printERROR ("Invalid parameter!");
                return false;
            }
            if (v!=-1) {
                if (!Settings_ESP3D::write_byte(ESP_SENSOR_TYPE,v)) {
                    output->printERROR ("Set failed!");
                    return false;
                }
                if (!esp3d_sensor.begin()) {
                    output->printERROR ("Set failed!");
                    return false;
                }
            }
        }
        parameter = get_param (cmd_params, "interval=");
        if (parameter.length() != 0) {
            if (!Settings_ESP3D::write_uint32(ESP_SENSOR_INTERVAL,parameter.toInt())) {
                output->printERROR ("Set failed!");
                return false;
            }
            esp3d_sensor.setInterval(parameter.toInt());
        }
        output->printMSG ("ok");
    }
    return response;
}

#endif //SENSOR_DEVICE
