/*
  commands.cpp - ESP3D commands class

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
#include "../include/esp3d_config.h"
#include "esp3d.h"
#include "commands.h"
#include "esp3doutput.h"
#include "settings_esp3d.h"

Commands esp3d_commands;

Commands::Commands()
{
}
Commands::~Commands()
{
}

//dispatch the command
void Commands::process(uint8_t * sbuf, size_t len, ESP3DOutput * output, level_authenticate_type auth, ESP3DOutput * outputonly )
{
    if(is_esp_command(sbuf,len)) {

        uint8_t cmd[4];
        cmd[0] = sbuf[4];
        cmd[1] = sbuf[5];
        cmd[2] = sbuf[6];
        cmd[3] = 0x0;
        //log_esp3d("Authentication = %d client %d", auth, output->client());
        execute_internal_command (String((const char *)cmd).toInt(), (len > 8)?(const char*)&sbuf[8]:"", auth, (outputonly == nullptr)?output:outputonly);
    } else {
        //Dispatch to all clients but current or to define output
        if ((output->client() == ESP_HTTP_CLIENT) && (outputonly == nullptr)) {
            if (auth != LEVEL_GUEST) {
                output->printMSG("");
            } else {
                output->printERROR("Wrong authentication!", 401);
                return;
            }
        }
        if (outputonly == nullptr) {
            output->dispatch(sbuf, len);
        } else {
            outputonly->write(sbuf, len);
        }
    }
}

//check if current line is an [ESPXXX] command
bool Commands::is_esp_command(uint8_t * sbuf, size_t len)
{
    if (len < 8) {
        return false;
    }
    if ((char(sbuf[0]) == '[') && (char(sbuf[1]) == 'E') && (char(sbuf[2]) == 'S') && (char(sbuf[3]) == 'P') && (char(sbuf[7]) == ']')) {
        return true;
    }
    return false;
}

//find space in string
//if space is has \ before it is ignored
int Commands::get_space_pos(const char * string, uint from)
{
    uint len = strlen(string);
    if (len < from) {
        return -1;
    }
    for (uint i = from; i < len; i++) {
        if (string[i] == ' ') {
            //if it is first char
            if (i == from) {
                return from;
            }
            //if not first one and previous char is not '\'
            if (string[i-1] != '\\') {
                return (i);
            }
        }
    }
    return  -1;
}

//extract parameter with corresponding label
//if label is empty give whole line without authentication label/parameter
const char * Commands::get_param (const char * cmd_params, const char * label)
{
    static String res;
    res = "";
    int start = 1;
    int end = -1;
    String tmp = "";
    String slabel = " ";
    res = cmd_params;
    res.replace("\r ", "");
    res.replace("\n ", "");
    if (res.length() == 0) {
        return res.c_str();
    }
    res.trim();
    tmp = " " + res;
    slabel += label;
    if (strlen(label) > 0) {
        start = tmp.indexOf(slabel);
        if (start == -1) {
            return res.c_str();
        }
        start+=slabel.length();
        end = get_space_pos(tmp.c_str(),start);
    }
    if (end == -1) {
        end = tmp.length();
    }
    //extract parameter
    res = tmp.substring (start, end);


#ifdef AUTHENTICATION_FEATURE
    //if no label remove authentication parameters
    if (strlen(label) == 0) {

        tmp = " " + res;
        start = tmp.indexOf (" pwd=");
        if (start != -1) {
            end = get_space_pos(tmp.c_str(),start+1);
            res = "";
            if (start != 0) {
                res = tmp.substring(0, start);
            }
            if (end != -1) {
                res += " " + tmp.substring(end+1, tmp.length());
            }
        }
    }
#endif //AUTHENTICATION_FEATURE
    //remove space format
    res.replace("\\ ", " ");
    //be sure no extra space
    res.trim();
    return res.c_str();

}

bool Commands::hastag (const char * cmd_params, const char *tag)
{
    String tmp = "";
    String stag = " ";
    if ((strlen(cmd_params) == 0) || (strlen(tag) == 0)) {
        return false;
    }
    stag += tag;
    tmp = cmd_params;
    tmp.trim();
    tmp = " " + tmp;
    if (tmp.indexOf(stag) == -1) {
        return false;
    }
    return true;
}


//execute internal command
bool Commands::execute_internal_command (int cmd, const char* cmd_params, level_authenticate_type auth_level, ESP3DOutput * output)
{
#ifndef SERIAL_COMMAND_FEATURE
    if (output->client() == ESP_SERIAL_CLIENT) {
        output->printMSG("Feature disabled");
        return false;
    }
#endif //SERIAL_COMMAND_FEATURE
    bool response = true;
    level_authenticate_type auth_type = auth_level;
    //log_esp3d("Authentication = %d", auth_type);
//override if parameters
#ifdef AUTHENTICATION_FEATURE

    //do not overwrite previous authetication level
    if (auth_type == LEVEL_GUEST) {
        String pwd=get_param (cmd_params, "pwd=");
        auth_type = AuthenticationService::authenticated_level(pwd.c_str());
    }
#endif //AUTHENTICATION_FEATURE
    //log_esp3d("Authentication = %d", auth_type);
    String parameter;
    switch (cmd) {
#if defined (WIFI_FEATURE)
    //STA SSID
    //[ESP100]<SSID>[pwd=<admin password>]
    case 100:
        response = ESP100(cmd_params, auth_type, output);
        break;
    //STA Password
    //[ESP101]<Password>[pwd=<admin password>]
    case 101:
        response = ESP101(cmd_params, auth_type, output);
        break;
#endif //WIFI_FEATURE
#if defined (WIFI_FEATURE) || defined (ETH_FEATURE)
    //Change STA IP mode (DHCP/STATIC)
    //[ESP102]<mode>pwd=<admin password>
    case 102:
        response = ESP102(cmd_params, auth_type, output);
        break;
    //Change STA IP/Mask/GW
    //[ESP103]IP=<IP> MSK=<IP> GW=<IP> pwd=<admin password>
    case 103:
        response = ESP103(cmd_params, auth_type, output);
        break;
#endif //WIFI_FEATURE ||ETH_FEATURE
#if defined (WIFI_FEATURE)
    //AP SSID
    //[ESP105]<SSID>[pwd=<admin password>]
    case 105:
        response = ESP105(cmd_params, auth_type, output);
        break;
    //AP Password
    //[ESP106]<Password>[pwd=<admin password>]
    case 106:
        response = ESP106(cmd_params, auth_type, output);
        break;
    //Change AP IP
    //[ESP107]<IP> pwd=<admin password>
    case 107:
        response = ESP107(cmd_params, auth_type, output);
        break;
    //Change AP channel
    //[ESP108]<channel>pwd=<admin password>
    case 108:
        response = ESP108(cmd_params, auth_type, output);
        break;
#endif //WIFI_FEATURE    

#if defined( WIFI_FEATURE) ||  defined( BLUETOOTH_FEATURE) || defined (ETH_FEATURE)
    //Set radio state at boot which can be BT, WIFI-STA, WIFI-AP, ETH-STA, OFF
    //[ESP110]<state>pwd=<admin password>
    case 110:
        response = ESP110(cmd_params, auth_type, output);
        break;
#endif //WIFI_FEATURE || BLUETOOTH_FEATURE || ETH_FEATURE)

#if defined(WIFI_FEATURE) || defined (ETH_FEATURE)
    //Get current IP
    //[ESP111]
    case 111:
        response = ESP111(cmd_params, auth_type, output);
        break;
#endif  //WIFI_FEATURE || ETH_FEATURE)

#if defined(WIFI_FEATURE) || defined(ETH_FEATURE) || defined(BT_FEATURE)
    //Get/Set hostname
    //[ESP112]<Hostname> pwd=<admin password>
    case 112:
        response = ESP112(cmd_params, auth_type, output);
        break;
    //Get/Set immediate Network (WiFi/BT/Ethernet) state which can be ON, OFF
    //[ESP115]<state>pwd=<admin password>
    case 115:
        response = ESP115(cmd_params, auth_type, output);
        break;
#endif //WIFI_FEATURE|| ETH_FEATURE || BT_FEATURE

#ifdef HTTP_FEATURE
    //Set HTTP state which can be ON, OFF
    //[ESP120]<state>pwd=<admin password>
    case 120:
        response = ESP120(cmd_params, auth_type, output);
        break;
    //Set HTTP port
    //[ESP121]<port>pwd=<admin password>
    case 121:
        response = ESP121(cmd_params, auth_type, output);
        break;
#endif HTTP_FEATURE
#ifdef TELNET_FEATURE
    //Set TELNET state which can be ON, OFF
    //[ESP130]<state>pwd=<admin password>
    case 130:
        response = ESP130(cmd_params, auth_type, output);
        break;
    //Set TELNET port
    //[ESP131]<port>pwd=<admin password>
    case 131:
        response = ESP131(cmd_params, auth_type, output);
        break;
#endif //TELNET_FEATURE

#ifdef DIRECT_PIN_FEATURE
    //Get/Set pin value
    //[ESP201]P<pin> V<value> [PULLUP=YES RAW=YES]pwd=<admin password>
    case 201:
        response = ESP201(cmd_params, auth_type, output);
        break;
#endif //DIRECT_PIN_FEATURE

    //Get full ESP3D settings
    //[ESP400]<pwd=admin>
    case 400:
        response = ESP400(cmd_params, auth_type, output);
        break;
    //Set EEPROM setting
    //[ESP401]P=<position> T=<type> V=<value> pwd=<user/admin password>
    case 401:
        response = ESP401(cmd_params, auth_type, output);
        break;
#if defined (WIFI_FEATURE)
    //Get available AP list (limited to 30)
    //output is JSON or plain text according parameter
    //[ESP410]<plain>
    case 410:
        response = ESP410(cmd_params, auth_type, output);
        break;
#endif //WIFI_FEATURE    
    //Get ESP current status
    //output is JSON or plain text according parameter
    //[ESP420]<plain>
    case 420:
        response = ESP420(cmd_params, auth_type, output);
        break;
    //Set ESP State
    //cmd are RESTART / RESET
    //[ESP444]<cmd><pwd=admin>
    case 444:
        response = ESP444(cmd_params, auth_type, output);
        break;
#ifdef AUTHENTICATION_FEATURE
    //Change admin password
    //[ESP550]<password>pwd=<admin password>
    case 550:
        response = ESP550(cmd_params, auth_type, output);
        break;
    //Change user password
    //[ESP555]<password>pwd=<admin/user password>
    case 555:
        response = ESP555(cmd_params, auth_type, output);
        break;
#endif  //AUTHENTICATION_FEATURE
#ifdef FILESYSTEM_FEATURE
    //Format ESP Filesystem
    //[ESP710]FORMAT pwd=<admin password>
    case 710:
        response = ESP710(cmd_params, auth_type, output);
        break;

    //List ESP Filesystem
    //[ESP720]<Root> pwd=<admin password>
    case 720:
        response = ESP720(cmd_params, auth_type, output);
        break;
#endif //FILESYSTEM_FEATURE

    //get fw version firmare target and fw version
    //output is JSON or plain text according parameter
    //[ESP800]<plain>
    case 800:
        response = ESP800(cmd_params, auth_type, output);
        break;
    default:
        output->printERROR ("Invalid Command");
        response = false;
    }
    return response;
}
