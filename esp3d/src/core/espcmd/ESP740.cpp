/*
 ESP740.cpp - ESP3D command class

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
#include "../../modules/authentication/authentication_service.h"
#include "../../modules/filesystem/esp_sd.h"
#ifdef SD_TIMESTAMP_FEATURE
#include "../../modules/time/time_server.h"
#endif //SD_TIMESTAMP_FEATURE
//List SD Filesystem
//[ESP740]<Root> pwd=<admin password>
bool Commands::ESP740(const char* cmd_params, level_authenticate_type auth_type, ESP3DOutput * output)
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
    if (parameter.length() == 0) {
        parameter = "/";
    }
    int8_t state = ESP_SD::getState(false);
    if (state != ESP_SDCARD_IDLE) {
        state = ESP_SD::getState(true);
    }
    if (state == ESP_SDCARD_NOT_PRESENT) {
        output->printERROR ("No SD card");
        return false;
    } else if (state != ESP_SDCARD_IDLE) {
        output->printERROR ("Busy");
        return false;
    }
    output->printf("Directory on SD : %s", parameter.c_str());
    output->printLN("");
    if (ESP_SD::exists(parameter.c_str())) {
        ESP_SDFile f = ESP_SD::open(parameter.c_str(), ESP_FILE_READ);
        uint countf = 0;
        uint countd = 0;
        if (f) {
            //Check directories
            ESP_SDFile sub = f.openNextFile();
            while (sub) {
                if (sub.isDirectory()) {
                    countd++;
                    output->print("<DIR> \t");
                    output->print(sub.name());
                    output->print(" \t");
                    output->printLN("");
                }
                sub.close();
                sub = f.openNextFile();
            }
            f.close();
            f = ESP_SD::open(parameter.c_str(), ESP_FILE_READ);
            //Check files
            sub = f.openNextFile();
            while (sub) {
                if (!sub.isDirectory()) {
                    countf++;
                    output->print("      \t");
                    output->print(sub.name());
                    output->print(" \t");
                    output->print(ESP_SD::formatBytes(sub.size()).c_str());
                    output->print(" \t");
#ifdef SD_TIMESTAMP_FEATURE
                    output->print(timeserver.current_time(sub.getLastWrite()));
                    output->print(" \t");
#endif //SD_TIMESTAMP_FEATURE              
                    output->printLN("");
                }
                sub.close();
                sub = f.openNextFile();
            }
            f.close();
            output->printf("%d file%s, %d dir%s", countf, (countf > 1)?"(s)":"", countd, (countd > 1)?"(s)":"");
            output->printLN("");
            String t = ESP_SD::formatBytes(ESP_SD::totalBytes());
            String u = ESP_SD::formatBytes(ESP_SD::usedBytes());
            String f = ESP_SD::formatBytes(ESP_SD::freeBytes());
            output->printf("Total %s, Used %s, Available: %s", t.c_str(), u.c_str(),f.c_str());
            output->printLN("");
        } else {
            output->printERROR ("Invalid directory!");
        }
    } else {
        output->printERROR ("Invalid directory!");
        response = false;
    }
    return response;
}

#endif //SD_DEVICE
