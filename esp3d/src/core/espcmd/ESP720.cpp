/*
 ESP720.cpp - ESP3D command class

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
#if defined (FILESYSTEM_FEATURE)
#include "../commands.h"
#include "../esp3doutput.h"
#include "../settings_esp3d.h"
#include "../../modules/authentication/authentication_service.h"
#include "../../modules/filesystem/esp_filesystem.h"
#ifdef FILESYSTEM_TIMESTAMP_FEATURE
#include "../../modules/time/time_server.h"
#endif //FILESYSTEM_TIMESTAMP_FEATURE
#define COMMANDID   720
//List ESP Filesystem
//[ESP720]<Root> json=<no> pwd=<admin password>
bool Commands::ESP720(const char* cmd_params, level_authenticate_type auth_type, ESP3DOutput * output)
{
    bool noError = true;
    bool json = has_tag (cmd_params, "json");
    String response;
    String parameter;
    int errorCode = 200; //unless it is a server error use 200 as default and set error in json instead
#ifdef AUTHENTICATION_FEATURE
    if (auth_type != LEVEL_ADMIN) {
        response = format_response(COMMANDID, json, false, "Wrong authentication level");
        noError = false;
        errorCode = 401;
    }
#else
    (void)auth_type;
#endif //AUTHENTICATION_FEATURE
    if (noError) {
        parameter = clean_param(get_param (cmd_params, ""));
        if (parameter.length() == 0) {
            parameter = "/";
        }

        if (ESP_FileSystem::exists(parameter.c_str())) {
            String line = "";
            ESP_File f ;
            f = ESP_FileSystem::open(parameter.c_str(), ESP_FILE_READ);
            uint countf = 0;
            uint countd = 0;
            if (f) {
                if(json) {
                    line = "{\"cmd\":\"720\",\"status\":\"ok\",\"data\":{\"path\":\"" + parameter + "\",\"files\":[";
                    output->print (line.c_str());
                } else {
                    line = "Directory on FS : " + parameter;
                    output->printMSGLine(line.c_str());
                }
                //Check directories
                ESP_File sub;
                sub = f.openNextFile();
                while (sub) {
                    if (sub.isDirectory()) {
                        line="";
                        countd++;
                        if (json) {
                            line="";
                            if (countd > 1) {
                                line += ",";
                            }
                            line += "{\"name\":\"" ;
                            line+=sub.name() ;
                            line+= "\",\"size\":\"-1\"}";
                        } else {
                            line = "[DIR] \t";
                            line+= sub.name();
                        }
                        if (json) {
                            output->print (line.c_str());
                        } else {
                            output->printMSGLine(line.c_str());
                        }
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
                        String time = "";
                        line="";
#ifdef FILESYSTEM_TIMESTAMP_FEATURE
                        time = timeserver.current_time(sub.getLastWrite());
#endif //FILESYSTEM_TIMESTAMP_FEATURE
                        if (json) {
                            line="";
                            if (countd > 0 || countf>1) {
                                line += ",";
                            }
                            line+= "{\"name\":\"";
                            line+=sub.name() ;
                            line+="\",\"size\":\"";
                            line+=ESP_FileSystem::formatBytes(sub.size());
                            if (time.length() > 0) {
                                line += "\",\"time\":\"";
                                line += time;
                            }
                            line+="\"}";
                        } else {
                            line+="     \t ";
                            line+=sub.name();
                            line+=" \t";
                            line+=ESP_FileSystem::formatBytes(sub.size());
                            line+=" \t";
                            line+=time;
                        }
                        if (json) {
                            output->print (line.c_str());
                        } else {
                            output->printMSGLine(line.c_str());
                        }
                    }
                    sub.close();
                    sub = f.openNextFile();
                }
                f.close();

                if (json) {
                    line = "], \"total\":\"";
                    line += ESP_FileSystem::formatBytes(ESP_FileSystem::totalBytes());
                    line += "\",\"used\":\"";
                    line += ESP_FileSystem::formatBytes(ESP_FileSystem::usedBytes());
                    line+="\",\"occupation\":\"";
                    uint64_t total =ESP_FileSystem::totalBytes();
                    if (total==0) {
                        total=1;
                    }
                    float occupation = 100.0*ESP_FileSystem::usedBytes()/total;
                    if ((occupation < 1) && (ESP_FileSystem::usedBytes()>0)) {
                        occupation=1;
                    }
                    line+= String((int)round(occupation));
                    line+="\"}}";
                    output->printLN (line.c_str());
                } else {
                    line =String(countf) + " file";
                    if (countf > 1) {
                        line += "s";
                    }
                    line += " , " + String(countd) + " dir";
                    if (countd > 1) {
                        line += "s";
                    }
                    output->printMSGLine(line.c_str());
                    line = "Total ";
                    line+=ESP_FileSystem::formatBytes(ESP_FileSystem::totalBytes());
                    line+=", Used ";
                    line+=ESP_FileSystem::formatBytes(ESP_FileSystem::usedBytes());
                    line+=", Available: ";
                    line+=ESP_FileSystem::formatBytes(ESP_FileSystem::freeBytes());
                    output->printMSGLine(line.c_str());
                }

                return true;
            } else {
                response = format_response(COMMANDID, json, false, "Invalid directory");
                noError = false;
            }
        } else {
            response = format_response(COMMANDID, json, false, "Invalid directory");
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

#endif //FILESYSTEM_FEATURE
