/*
 ESP201.cpp - ESP3D command class

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
#if defined (DIRECT_PIN_FEATURE)
#include "../commands.h"
#include "../esp3doutput.h"
#include "../settings_esp3d.h"
#include "../hal.h"
#include "../../modules/authentication/authentication_service.h"
//Get/Set pin value
//[ESP201]P<pin> V<value> [PULLUP=YES RAW=YES ANALOG=NO ANALOG_RANGE=255 CLEARCHANNELS=NO]pwd=<admin password>
//Range can be 255 / 1024 / 2047 / 4095 / 8191
bool Commands::ESP201(const char* cmd_params, level_authenticate_type auth_type, ESP3DOutput * output)
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

    //check if have pin
    parameter = get_param (cmd_params, "P");
    log_esp3d("Pin %s", parameter.c_str());
    if (parameter == "") {
        output->printERROR ("Invalid parameter!");
        return false;
    }
    int pin = parameter.toInt();
    //check pin is valid and not serial used pins
    if ( Hal::is_pin_usable(pin)) {
        bool isdigital = true;
        parameter = get_param (cmd_params, "ANALOG=");
        if (parameter == "YES") {
            log_esp3d ("Set as analog");
            isdigital=false;
            parameter = get_param (cmd_params, "CLEARCHANNELS=");
            if (parameter == "YES") {
                Hal::clearAnalogChannels();
            }
        }
        //check if is set or get
        parameter = get_param (cmd_params, "V");
        //it is a get
        if (parameter == "") {
            //this is to not set pin mode
            int value = 0;
            if(isdigital) {
                parameter = get_param (cmd_params, "RAW=");
                if (parameter != "YES") {
                    parameter = get_param (cmd_params, "PULLUP=");
                    if (parameter != "YES") {
                        Hal::pinMode (pin, INPUT);
                    } else {
                        Hal::pinMode (pin, INPUT_PULLUP);
                    }
                }
                value = digitalRead (pin);
            } else {
                value = Hal::analogRead(pin);
            }

            output->printMSG (String(value).c_str());
        } else {
            //it is a set
            int value = parameter.toInt();
            Hal::pinMode (pin, OUTPUT);
            if (isdigital) {
                //verify it is a '0' or a '1'
                if ( (value == 0) || (value == 1) ) {
                    digitalWrite (pin, (value == 0) ? LOW : HIGH);
                    output->printMSG ("ok");
                } else {
                    output->printERROR ("Invalid parameter!");
                    response = false;
                }
            } else {
                int analog_range= 255;
                parameter = get_param (cmd_params, "ANALOG_RANGE=");
                if (parameter.length() > 0) {
                    analog_range = parameter.toInt();
                }
                if ( (value >= 0) || (value <= analog_range+1) ) {
                    Hal::analogWriteRange(analog_range);
                    Hal::analogWriteFreq(1000);
                    if (!Hal::analogWrite(pin, value)) {
                        output->printERROR ("Invalid value!");
                        response = false;
                    }
                } else {
                    output->printERROR ("Invalid parameter!");
                    response = false;
                }
            }
        }
    } else {
        output->printERROR ("Invalid parameter!");
        response = false;
    }
    return response;
}

#endif //DIRECT_PIN_FEATURE
