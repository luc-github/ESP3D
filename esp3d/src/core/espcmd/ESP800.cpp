/*
 ESP800.cpp - ESP3D command class

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
#endif //HTTP_FEATURE
//get fw version firmare target and fw version
//output is JSON or plain text according parameter
//[ESP800]<plain>
bool Commands::ESP800(const char* cmd_params, level_authenticate_type auth_type, ESP3DOutput * output)
{
    bool response = true;
    String parameter;
#ifdef AUTHENTICATION_FEATURE
    if (auth_type == LEVEL_GUEST) {
        output->printERROR("Wrong authentication!", 401);
        return false;
    }
#endif //AUTHENTICATION_FEATURE
    bool plain = hastag(cmd_params,"plain");
    //FW version
    if (plain) {
        output->print("FW version:");
    } else {
        output->print("{\"FWVersion\":\"");
    }
    output->print(FW_VERSION);
    if(plain) {
        output->printLN("");
    } else {
        output->print("\"");
    }

    //FW target
    if (plain) {
        output->print("FW target:");
    } else {
        output->print(",\"FWTarget\":\"");
    }
    output->print(Settings_ESP3D::GetFirmwareTargetShortName());
    if(plain) {
        output->printLN("");
    } else {
        output->print("\"");
    }
    //SD connection
    if (plain) {
        output->print("SD connection:");
    } else {
        output->print(",\"SDConnection\":\"");
    }
    output->print((Settings_ESP3D::isDirectSD())?"Direct":"Serial");
    if(plain) {
        output->printLN("");
    } else {
        output->print("\"");
    }
    if (Settings_ESP3D::isDirectSD()) {
        //TODO
        //Primary SD

        //TODO
        //Secondary SD
    }
    //Authentication
    if (plain) {
        output->print("Authentication:");
    } else {
        output->print(",\"Authentication\":\"");
    }
#ifdef AUTHENTICATION_FEATURE
    output->print("Enabled");
#else
    output->print("Disabled");
#endif //AUTHENTICATION_FEATURE
    if(plain) {
        output->printLN("");
    } else {
        output->print("\"");
    }
#if (defined(WIFI_FEATURE) || defined(ETH_FEATURE)) && defined(HTTP_FEATURE)
    //Web Communication
    if (plain) {
        output->print("Web Communication:");
    } else {
        output->print(",\"WebCommunication\":\"");
    }
#if defined (ASYNCWEBSERVER_FEATURE)
    output->print("Asynchronous");
#else
    output->print("Synchronous");
#endif //ASYNCWEBSERVER_FEATURE
    if(plain) {
        output->printLN("");
    } else {
        output->print("\"");
    }
    //WebSocket Port
    if (plain) {
        output->print("Web Socket port:");
    } else {
        output->print(",\"WebSocketport\":\"");
    }
    output->print(HTTP_Server::port() +1);
    if(plain) {
        output->printLN("");
    } else {
        output->print("\"");
    }
#endif // (WIFI_FEATURE) || ETH_FEATURE) && HTTP_FEATURE)
#if defined(WIFI_FEATURE) || defined(ETH_FEATURE) || defined(BT_FEATURE)
    //Hostname
    if (plain) {
        output->print("Hostname:");
    } else {
        output->print(",\"Hostname\":\"");
    }
    output->print(NetConfig::hostname());
    if(plain) {
        output->printLN("");
    } else {
        output->print("\"");
    }
#endif //WIFI_FEATURE|| ETH_FEATURE || BT_FEATURE
#if defined(WIFI_FEATURE)
    if (WiFiConfig::started()) {
        //WiFi mode
        if (plain) {
            output->print("WiFi mode:");
        } else {
            output->print(",\"WiFiMode\":\"");
        }
        output->print((WiFi.getMode() == WIFI_AP)?"AP":"STA");
        if(plain) {
            output->printLN("");
        } else {
            output->print("\"");
        }
    }
#endif //WIFI_FEATURE
#if defined(WIFI_FEATURE) || defined(ETH_FEATURE)
    //Update
    if (plain) {
        output->print("Web Update:");
    } else {
        output->print(",\"WebUpdate\":\"");
    }
#ifdef WEB_UPDATE_FEATURE
    output->print("Enabled");
#else
    output->print("Disabled");
#endif //WEB_UPDATE_FEATURE
    if(plain) {
        output->printLN("");
    } else {
        output->print("\"");
    }
#endif //WIFI_FEATURE|| ETH_FEATURE

//Hostname
    if (plain) {
        output->print("Filesystem:");
    } else {
        output->print(",\"Filesystem\":\"");
    }
#if defined(FILESYSTEM_FEATURE)
    output->print(ESP_FileSystem::FilesystemName());
#else
    output->print("None");
#endif //FILESYSTEM_FEATURE
    if(plain) {
        output->printLN("");
    } else {
        output->print("\"");
    }

    //final
    if(!plain) {
        output->printLN("}");
    }
    return response;
}

