/*
 ESP720.cpp - ESP3D command class

 Copyright (c) 2014 Luc Lebosse. All rights reserved.

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#include "../../include/esp3d_config.h"
#if defined (FILESYSTEM_FEATURE)
#include "../commands.h"
#include "../esp3doutput.h"
#include "../settings_esp3d.h"
#include "../../modules/authentication/authentication_service.h"
#include "../../modules/filesystem/esp_filesystem.h"
//List ESP Filesystem
//[ESP720]<Root> pwd=<admin password>
bool Commands::ESP720(const char* cmd_params, level_authenticate_type auth_type, ESP3DOutput * output)
{
    bool response = true;
    String parameter;
    parameter = get_param (cmd_params, "");
#ifdef AUTHENTICATION_FEATURE
    if (auth_type != LEVEL_ADMIN) {
        output->printERROR("Wrong authentication!", 401);
        return false;
    }
#endif //AUTHENTICATION_FEATURE
    if (parameter.length() == 0) {
        parameter = "/";
    }
    output->printf("Directory : %s", parameter.c_str());
    output->printLN("");
    if (ESP_FileSystem::exists(parameter.c_str())) {
        ESP_File f = ESP_FileSystem::open(parameter.c_str(), ESP_FILE_READ);
        uint countf = 0;
        uint countd = 0;
        if (f) {
            //Check directories
            ESP_File sub = f.openNextFile();
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
            f = ESP_FileSystem::open(parameter.c_str(), ESP_FILE_READ);
            //Check files
            sub = f.openNextFile();
            while (sub) {
                if (!sub.isDirectory()) {
                    countf++;
                    output->print("      \t");
                    output->print(sub.name());
                    output->print(" \t");
                    output->print(ESP_FileSystem::formatBytes(sub.size()).c_str());
                    output->print(" \t");
#ifdef FILESYSTEM_TIMESTAMP_FEATURE
                    time_t t = sub.getLastWrite();
                    struct tm * tmstruct = localtime(&t);
                    output->printf("%d-%02d-%02d %02d:%02d:%02d",(tmstruct->tm_year)+1900,( tmstruct->tm_mon)+1, tmstruct->tm_mday,tmstruct->tm_hour, tmstruct->tm_min, tmstruct->tm_sec);
                    output->print(" \t");
#endif //FILESYSTEM_TIMESTAMP_FEATURE              
                    output->printLN("");
                }
                sub.close();
                sub = f.openNextFile();
            }
            f.close();
            output->printf("%d file%s, %d dir%s", countf, (countf > 1)?"(s)":"", countd, (countd > 1)?"(s)":"");
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

#endif //FILESYSTEM_FEATURE
