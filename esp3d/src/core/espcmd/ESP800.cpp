/*
 ESP800.cpp - ESP3D command class

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
#include "../../modules/network/netconfig.h"
#include "../../modules/authentication/authentication_service.h"
#ifdef FILESYSTEM_FEATURE
#include "../../modules/filesystem/esp_filesystem.h"
#endif //FILESYSTEM_FEATURE
#if defined (WIFI_FEATURE) || defined(ETH_FEATURE) ||defined(BLUETOOTH_FEATURE)
#include "../../modules/network/netconfig.h"
#if defined (WIFI_FEATURE)
#include "../../modules/wifi/wificonfig.h"
#endif //WIFI_FEATURE
#endif //WIFI_FEATURE || ETH_FEATURE || BLUETOOTH_FEATURE
#ifdef HTTP_FEATURE
#include "../../modules/http/http_server.h"
#include "../../modules/websocket/websocket_server.h"
#endif //HTTP_FEATURE
#ifdef TIMESTAMP_FEATURE
#include "../../modules/time/time_server.h"
#endif //TIMESTAMP_FEATURE
#ifdef CAMERA_DEVICE
#include "../../modules/camera/camera.h"
#endif //CAMERA_DEVICE
#define COMMANDID   800
//get fw version firmare target and fw version
//eventually set time with pc time
//output is JSON or plain text according parameter
//[ESP800]json=<no><time=YYYY-MM-DDTHH:mm:ss> <version=3.0.0-a11> <setup=0/1>
bool Commands::ESP800(const char* cmd_params, level_authenticate_type auth_type, ESP3DOutput * output)
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
        parameter = get_param (cmd_params, "setup=");
        if(parameter.length() > 0) {
            if (!Settings_ESP3D::write_byte (ESP_SETUP, parameter =="0"?0:1)) {
                response = format_response(COMMANDID, json, false, "Save setup flag failed");
                noError = false;
            }
        }
    }
    if (noError) {
#ifdef TIMESTAMP_FEATURE
        String newtime = get_param (cmd_params, "time=");
        String tparm = (timeserver.is_internet_time())?"Auto":"Manual";
        if (!timeserver.is_internet_time() && (newtime.length() > 0)) {
            if (!timeserver.setTime(newtime.c_str())) {
                tparm="Failed to set";
            }
        } else {
            if (!timeserver.is_internet_time() && (newtime.length() == 0)) {
                tparm="Not set";
            }
        }
#endif //TIMESTAMP_FEATURE

        String line = "";
        if(json) {
            line = "{\"cmd\":\"800\",\"status\":\"ok\",\"data\":{";
        }
        //FW version
        if (json) {
            line+="\"FWVersion\":\"";
        } else {
            line+="FW version:";
        }
#if defined (SHORT_BUILD_VERSION)
        line+=SHORT_BUILD_VERSION;
        line+="-";
#endif //SHORT_BUILD_VERSION
        line+=FW_VERSION;
        if (json) {
            line +="\"";
            output->print (line.c_str());
        } else {
            output->printMSGLine(line.c_str());
        }
        line="";
        //FW target
        if (json) {
            line+=",\"FWTarget\":\"";
        } else {
            line+="FW target:";
        }
        line+=Settings_ESP3D::GetFirmwareTargetShortName();
        if (json) {
            line +="\"";
            output->print (line.c_str());
        } else {
            output->printMSGLine(line.c_str());
        }
        line="";
        //FW ID
        if (json) {
            line+=",\"FWTargetID\":\"";
        } else {
            line+="FW ID:";
        }
        line+=Settings_ESP3D::GetFirmwareTarget();
        if (json) {
            line +="\"";
            output->print (line.c_str());
        } else {
            output->printMSGLine(line.c_str());
        }
        line="";
        //Setup done
        if (json) {
            line+=",\"Setup\":\"";
        } else {
            line+= "Setup:";
        }
        line+=Settings_ESP3D::read_byte (ESP_SETUP) == 0?F("Enabled"):F("Disabled");
        if (json) {
            line +="\"";
            output->print (line.c_str());
        } else {
            output->printMSGLine(line.c_str());
        }
        line="";

        //SD connection
        if (json) {
            line+=",\"SDConnection\":\"";
        } else {
            line+= "SD connection:";
        }
        if (Settings_ESP3D::GetSDDevice() == ESP_DIRECT_SD) {
            line+="direct";
        } else if (Settings_ESP3D::GetSDDevice() == ESP_SHARED_SD) {
            line+="shared";
        } else {
            line+="none";
        }
        if (json) {
            line +="\"";
            output->print (line.c_str());
        } else {
            output->printMSGLine(line.c_str());
        }
        line="";
        //Serial protocol
        if (json) {
            line+=",\"SerialProtocol\":\"";
        } else {
            line+= "Serial protocol:";
        }

#if COMMUNICATION_PROTOCOL ==  MKS_SERIAL
        line+="MKS";
#endif //COMMUNICATION_PROTOCOL ==  MKS_SERIAL
#if COMMUNICATION_PROTOCOL ==  RAW_SERIAL
        line+="Raw";
#endif //COMMUNICATION_PROTOCOL ==  RAW_SERIAL
#if COMMUNICATION_PROTOCOL ==  SOCKET_SERIAL
        line+="Socket";
#endif //COMMUNICATION_PROTOCOL ==  SOCKET_SERIAL
        if (json) {
            line +="\"";
            output->print (line.c_str());
        } else {
            output->printMSGLine(line.c_str());
        }
        line="";
        //Authentication
        if (json) {
            line+=",\"Authentication\":\"";
        } else {
            line+= "Authentication:";
        }

#ifdef AUTHENTICATION_FEATURE
        line+="Enabled";
#else
        line+="Disabled";
#endif //AUTHENTICATION_FEATURE
        if (json) {
            line +="\"";
            output->print (line.c_str());
        } else {
            output->printMSGLine(line.c_str());
        }
        line="";
#if (defined(WIFI_FEATURE) || defined(ETH_FEATURE)) && defined(HTTP_FEATURE)
        //Web Communication
        if (json) {
            line+=",\"WebCommunication\":\"";
        } else {
            line+= "Web Communication:";
        }
#if defined (ASYNCWEBSERVER_FEATURE)
        line+="Asynchronous";
#else
        line+="Synchronous";
#endif //ASYNCWEBSERVER_FEATURE
        if (json) {
            line +="\"";
            output->print (line.c_str());
        } else {
            output->printMSGLine(line.c_str());
        }
        line="";
        //WebSocket IP
        if (json) {
            line+=",\"WebSocketIP\":\"";
        } else {
            line+= "Web Socket IP:";
        }
        line+=NetConfig::localIP().c_str();
        if (json) {
            line +="\"";
            output->print (line.c_str());
        } else {
            output->printMSGLine(line.c_str());
        }
        line="";
        //WebSocket Port
        if (json) {
            line+=",\"WebSocketPort\":\"";
        } else {
            line+= "Web Socket Port:";
        }
#if defined (ASYNCWEBSERVER_FEATURE)
        line+=HTTP_Server::port();
#else
        line+=websocket_terminal_server.getPort();
#endif
        if (json) {
            line +="\"";
            output->print (line.c_str());
        } else {
            output->printMSGLine(line.c_str());
        }
        line="";

#endif // (WIFI_FEATURE) || ETH_FEATURE) && HTTP_FEATURE)
#if defined(WIFI_FEATURE) || defined(ETH_FEATURE) || defined(BT_FEATURE)
        //Hostname
        if (json) {
            line+=",\"Hostname\":\"";
        } else {
            line+= "Hostname:";
        }
        line+=NetConfig::hostname();
        if (json) {
            line +="\"";
            output->print (line.c_str());
        } else {
            output->printMSGLine(line.c_str());
        }
        line="";
#endif //WIFI_FEATURE|| ETH_FEATURE || BT_FEATURE
#if defined(WIFI_FEATURE)
        if (WiFiConfig::started()) {
            //WiFi mode
            if (json) {
                line+=",\"WiFiMode\":\"";
            } else {
                line+= "WiFi mode:";
            }
            line+=(WiFi.getMode() == WIFI_AP)?"AP":"STA";
            if (json) {
                line +="\"";
                output->print (line.c_str());
            } else {
                output->printMSGLine(line.c_str());
            }
            line="";
        }
#endif //WIFI_FEATURE
#if defined(WIFI_FEATURE) || defined(ETH_FEATURE)
        //Update
        if (json) {
            line+=",\"WebUpdate\":\"";
        } else {
            line+= "Web update:";
        }
#ifdef WEB_UPDATE_FEATURE
        if (ESP_FileSystem::max_update_size()!=0) {
            line+="Enabled";
        } else {
            line+="Disabled";
        }
#else
        line+="Disabled";
#endif //WEB_UPDATE_FEATURE
        if (json) {
            line +="\"";
            output->print (line.c_str());
        } else {
            output->printMSGLine(line.c_str());
        }
        line="";
#endif //WIFI_FEATURE|| ETH_FEATURE
        //FS
        if (json) {
            line+=",\"FlashFileSystem\":\"";
        } else {
            line+= "Flash File System:";
        }
#if defined(FILESYSTEM_FEATURE)
        line+=ESP_FileSystem::FilesystemName();
#else
        line+="none";
#endif //FILESYSTEM_FEATURE
        if (json) {
            line +="\"";
            output->print (line.c_str());
        } else {
            output->printMSGLine(line.c_str());
        }
        line="";
//      Host path
        if (json) {
            line+=",\"HostPath\":\"";
        } else {
            line+= "Host Path:";
        }

        line+= ESP3D_HOST_PATH;
        if (json) {
            line +="\"";
            output->print (line.c_str());
        } else {
            output->printMSGLine(line.c_str());
        }
        line="";
        //time server
        if (json) {
            line+=",\"Time\":\"";
        } else {
            line+= "Time:";
        }
#ifdef TIMESTAMP_FEATURE
        line+=tparm;
#else
        line+="none";
#endif //TIMESTAMP_FEATURE
        if (json) {
            line +="\"";
            output->print (line.c_str());
        } else {
            output->printMSGLine(line.c_str());
        }
        line="";
#ifdef CAMERA_DEVICE
        //camera ID
        if (json) {
            line+=",\"CameraID\":\"";
        } else {
            line+= "Camera ID:";
        }
        line+=esp3d_camera.GetModel();
        if (json) {
            line +="\"";
            output->print (line.c_str());
        } else {
            output->printMSGLine(line.c_str());
        }
        line="";
        //camera Name
        if (json) {
            line+=",\"CameraName\":\"";
        } else {
            line+= "Camera name:";
        }
        line+=esp3d_camera.GetModelString();
        if (json) {
            line +="\"";
            output->print (line.c_str());
        } else {
            output->printMSGLine(line.c_str());
        }
        line="";
#endif //CAMERA_DEVICE

        if(json) {
            output->printLN("}}");
        }
        return true;
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

