/*
 ESP780.cpp - ESP3D command class

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
#if defined(SD_TIMESTAMP_FEATURE) ||  defined(FILESYSTEM_TIMESTAMP_FEATURE)
#include "../../modules/time/time_server.h"
#endif //SD_TIMESTAMP_FEATURE || FILESYSTEM_TIMESTAMP_FEATURE
//List Global Filesystem
//[ESP780]<Root> pwd=<admin password>
bool Commands::ESP780(const char* cmd_params, level_authenticate_type auth_type, ESP3DOutput * output)
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
    uint8_t fs = ESP_GBFS::getFSType(parameter.c_str());
    output->printf("Directory on Global FS : %s", parameter.c_str());
    output->printLN("");
    if (ESP_GBFS::exists(parameter.c_str())) {
        ESP_GBFile f = ESP_GBFS::open(parameter.c_str(), ESP_FILE_READ);
        uint countf = 0;
        uint countd = 0;
        if (f) {
            //Check directories
            ESP_GBFile sub = f.openNextFile();
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
            f = ESP_GBFS::open(parameter.c_str(), ESP_FILE_READ);
            //Check files
            sub = f.openNextFile();
            while (sub) {
                if (!sub.isDirectory()) {
                    countf++;
                    output->print("      \t");
                    output->print(sub.name());
                    output->print(" \t");
                    output->print(ESP_GBFS::formatBytes(sub.size()).c_str());
                    output->print(" \t");
#if defined(SD_TIMESTAMP_FEATURE) ||  defined(FILESYSTEM_TIMESTAMP_FEATURE)
                    output->print(timeserver.current_time(sub.getLastWrite()));
                    output->print(" \t");
#endif //SD_TIMESTAMP_FEATURE ||  FILESYSTEM_TIMESTAMP_FEATURE              
                    output->printLN("");
                }
                sub.close();
                sub = f.openNextFile();
            }
            f.close();
            output->printf("%d file%s, %d dir%s", countf, (countf > 1)?"(s)":"", countd, (countd > 1)?"(s)":"");
            output->printLN("");
            if (fs != FS_ROOT) {
                String t = ESP_GBFS::formatBytes(ESP_GBFS::totalBytes(fs));
                String u = ESP_GBFS::formatBytes(ESP_GBFS::usedBytes(fs));
                String f = ESP_GBFS::formatBytes(ESP_GBFS::freeBytes(fs));
                output->printf("Total %s, Used %s, Available: %s", t.c_str(), u.c_str(),f.c_str());
                output->printLN("");
            }
        } else {
            output->printERROR ("Invalid directory!");
        }
    } else {
        output->printERROR ("Invalid directory!");
        response = false;
    }
    return response;
}

#endif //GLOBAL_FILESYSTEM_FEATURE
