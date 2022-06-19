/*
 ESP220.cpp - ESP3D command class

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
#define COMMANDID   220

//Get ESP pins definition
//output is JSON or plain text according parameter
//[ESP220]json=<no>
bool Commands::ESP220(const char* cmd_params, level_authenticate_type auth_type, ESP3DOutput * output)
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
        parameter = clean_param(get_param (cmd_params, ""));
        if (parameter.length() == 0) {
            String line = "";
            if(json) {
                line = "{\"cmd\":\"220\",\"status\":\"ok\",\"data\":[";
            }
            bool hasPin = false;
#ifdef SD_DEVICE
            hasPin = true;
            if (json) {
                line += "{\"id\":\"";
            }
            line +="SD CS";
            if (json) {
                line +="\",\"value\":\"";
            } else {
                line +=": ";
            }
            line +=String(ESP_SD_CS_PIN==-1?SS:ESP_SD_CS_PIN);
            if (json) {
                line +="\"}";
                output->print (line.c_str());
            } else {
                output->printMSGLine(line.c_str());
            }
            line="";
            if (json) {
                line += "{\"id\":\"";
            }
            line +="SD MOSI";
            if (json) {
                line +="\",\"value\":\"";
            } else {
                line +=": ";
            }
            line +=String(ESP_SD_MOSI_PIN==-1?MOSI:ESP_SD_MOSI_PIN);
            if (json) {
                line +="\"}";
                output->print (line.c_str());
            } else {
                output->printMSGLine(line.c_str());
            }
            line="";
            if (json) {
                line += "{\"id\":\"";
            }
            line +="SD MISO";
            if (json) {
                line +="\",\"value\":\"";
            } else {
                line +=": ";
            }
            line +=String(ESP_SD_MISO_PIN==-1?MISO:ESP_SD_MISO_PIN);
            if (json) {
                line +="\"}";
                output->print (line.c_str());
            } else {
                output->printMSGLine(line.c_str());
            }
            line="";
            if (json) {
                line += "{\"id\":\"";
            }
            line +="SD SCK";
            if (json) {
                line +="\",\"value\":\"";
            } else {
                line +=": ";
            }
            line +=String(ESP_SD_SCK_PIN==-1?SCK:ESP_SD_SCK_PIN);
            if (json) {
                line +="\"}";
                output->print (line.c_str());
            } else {
                output->printMSGLine(line.c_str());
            }
            line="";
            if (json) {
                line += "{\"id\":\"";
            }
            line +="SD DETECT";
            if (json) {
                line +="\",\"value\":\"";
            } else {
                line +=": ";
            }
            line +=String(ESP_SD_DETECT_PIN);
            if (json) {
                line +="\"}";
                output->print (line.c_str());
            } else {
                output->printMSGLine(line.c_str());
            }
            line="";
#if ESP_SD_DETECT_PIN !=-1
            if (json) {
                line += "{\"id\":\"";
            }
            line +="SD DETECT STATE";
            if (json) {
                line +="\",\"value\":\"";
            } else {
                line +=": ";
            }
            line +=String(ESP_SD_DETECT_VALUE);
            if (json) {
                line +="\"}";
                output->print (line.c_str());
            } else {
                output->printMSGLine(line.c_str());
            }
            line="";
#endif //ESP_SD_DETECT_PIN !=-1
#if SD_DEVICE_CONNECTION == ESP_SHARED_SD && defined(ESP_FLAG_SHARED_SD_PIN)
            if (json) {
                line += "{\"id\":\"";
            }
            line +="SD SWITCH";
            if (json) {
                line +="\",\"value\":\"";
            } else {
                line +=": ";
            }
            line +=String(ESP_FLAG_SHARED_SD_PIN);
            if (json) {
                line +="\"}";
                output->print (line.c_str());
            } else {
                output->printMSGLine(line.c_str());
            }
            line="";
#endif //SD_DEVICE_CONNECTION == ESP_SHARED_SD
#endif //SD_DEVICE
#ifdef BUZZER_DEVICE
            hasPin = true;
            if (json) {
                line += "{\"id\":\"";
            }
            line +="BUZZER";
            if (json) {
                line +="\",\"value\":\"";
            } else {
                line +=": ";
            }
            line +=String(ESP3D_BUZZER_PIN);
            if (json) {
                line +="\"}";
                output->print (line.c_str());
            } else {
                output->printMSGLine(line.c_str());
            }
            line="";
#endif //BUZZER_DEVICE
#ifdef PIN_RESET_FEATURE
            hasPin = true;
            if (json) {
                line += "{\"id\":\"";
            }
            line +="RESET";
            if (json) {
                line +="\",\"value\":\"";
            } else {
                line +=": ";
            }
            line +=String(ESP3D_RESET_PIN);
            if (json) {
                line +="\"}";
                output->print (line.c_str());
            } else {
                output->printMSGLine(line.c_str());
            }
            line="";
#endif //PIN_RESET_FEATURE 
#ifdef SENSOR_DEVICE
            hasPin = true;
            if (json) {
                line += "{\"id\":\"";
            }
            line +="SENSOR";
            if (json) {
                line +="\",\"value\":\"";
            } else {
                line +=": ";
            }
            line +=String(ESP3D_SENSOR_PIN);
            if (json) {
                line +="\"}";
                output->print (line.c_str());
            } else {
                output->printMSGLine(line.c_str());
            }
            line="";
#endif //SENSOR_DEVICE
#ifdef DISPLAY_DEVICE
#if (DISPLAY_DEVICE == OLED_I2C_SSD1306) || (DISPLAY_DEVICE == OLED_I2C_SSDSH1106)
            hasPin = true;
            if (json) {
                line += "{\"id\":\"";
            }
            line +="SDA";
            if (json) {
                line +="\",\"value\":\"";
            } else {
                line +=": ";
            }
            line +=String(ESP_SDA_PIN);
            if (json) {
                line +="\"}";
                output->print (line.c_str());
            } else {
                output->printMSGLine(line.c_str());
            }
            line="";
            if (json) {
                line += "{\"id\":\"";
            }
            line +="SCL";
            if (json) {
                line +="\",\"value\":\"";
            } else {
                line +=": ";
            }
            line +=String(ESP_SCL_PIN);
            if (json) {
                line +="\"}";
                output->print (line.c_str());
            } else {
                output->printMSGLine(line.c_str());
            }
            line="";
#endif //(DISPLAY_DEVICE == OLED_I2C_SSD1306) || (DISPLAY_DEVICE == OLED_I2C_SSDSH1106)
#endif //DISPLAY_DEVICE
            if (!hasPin) {
                if (json) {
                    line += "{\"id\":\"";
                }
                line +="NO PIN";
                if (json) {
                    line +="\",\"value\":\"";
                } else {
                    line +=": ";
                }
                line +="-";
                if (json) {
                    line +="\"}";
                    output->print (line.c_str());
                } else {
                    output->printMSGLine(line.c_str());
                }
                line="";
            }
            if (json) {
                output->printLN ("]}");
            }
            return true;
        } else {
            response = format_response(COMMANDID, json, false, "This command doesn't take parameters");
            noError = false;
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
