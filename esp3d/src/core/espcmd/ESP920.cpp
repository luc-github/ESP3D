/*
 ESP910.cpp - ESP3D command class

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
#define COMMANDID   920
//Get state / Set state of output message clients
//[ESP920]<SERIAL / SCREEN / REMOTE_SCREEN/ WEBSOCKET / TELNET /BT / ALL>=<ON/OFF> json=<no> [pwd=<admin password>]
bool Commands::ESP920(const char* cmd_params, level_authenticate_type auth_type, ESP3DOutput * output)
{
    bool noError = true;
    bool json = has_tag (cmd_params, "json");
    String response;
    String parameter;
    bool hasParam = false;
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
        //get
        if (parameter.length() == 0) {
            String s = "";
            if (json) {
                s += "{";
            }
            bool hasData=false;
#if COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL == MKS_SERIAL || COMMUNICATION_PROTOCOL == SOCKET_SERIAL
            hasData=true;
            if (json) {
                s += "\"";
            }
            s += "SERIAL";
            if (json) {
                s += "\":\"";
            } else {
                s += ":";
            }
            s += ESP3DOutput::isOutput(ESP_SERIAL_CLIENT)?"ON":"OFF";
            if (json) {
                s += "\"";
            }
#endif //COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL == MKS_SERIAL || COMMUNICATION_PROTOCOL == SOCKET_SERIAL
#if !defined(ESP3DLIB_ENV) || (defined (ESP3DLIB_ENV) && (HAS_DISPLAY || defined (HAS_SERIAL_DISPLAY)))
            if (hasData) {
                if (json) {
                    s += ",";
                } else {
                    s += ", ";
                }
            } else {
                hasData = true;
            }
            if (json) {
                s += "\"";
            }
            s += "REMOTE_SCREEN";
            if (json) {
                s += "\":\"";
            } else {
                s += ":";
            }
            s += ESP3DOutput::isOutput(ESP_REMOTE_SCREEN_CLIENT)?"ON":"OFF";
            if (json) {
                s += "\"";
            }
#endif //!defined(ESP3DLIB_ENV) || (defined (ESP3DLIB_ENV) && HAS_DISPLAY)
#ifdef DISPLAY_DEVICE
            if (hasData) {
                if (json) {
                    s += ",";
                } else {
                    s += ", ";
                }
            } else {
                hasData = true;
            }
            if (json) {
                s += "\"";
            }
            s += "REMOTE_SCREEN";
            if (json) {
                s += "\":\"";
            } else {
                s += ":";
            }
            s += ESP3DOutput::isOutput(ESP_SCREEN_CLIENT)?"ON":"OFF";
            if (json) {
                s += "\"";
            }
#endif //DISPLAY_DEVICE
#ifdef WS_DATA_FEATURE
            if (hasData) {
                if (json) {
                    s += ",";
                } else {
                    s += ", ";
                }
            } else {
                hasData = true;
            }
            if (json) {
                s += "\"";
            }
            s += "WEBSOCKET";
            if (json) {
                s += "\":\"";
            } else {
                s += ":";
            }
            s += ESP3DOutput::isOutput(ESP_WEBSOCKET_CLIENT)?"ON":"OFF";
            if (json) {
                s += "\"";
            }
#endif //WS_DATA_FEATURE
#ifdef BLUETOOTH_FEATURE
            if (hasData) {
                if (json) {
                    s += ",";
                } else {
                    s += ", ";
                }
            } else {
                hasData = true;
            }
            if (json) {
                s += "\"";
            }
            s += "BT";
            if (json) {
                s += "\":\"";
            } else {
                s += ":";
            }
            s += ESP3DOutput::isOutput(ESP_BT_CLIENT)?"ON":"OFF";
            if (json) {
                s += "\"";
            }
#endif //BLUETOOTH_FEATURE
#ifdef TELNET_FEATURE
            if (hasData) {
                if (json) {
                    s += ",";
                } else {
                    s += ", ";
                }
            } else {
                hasData = true;
            }
            if (json) {
                s += "\"";
            }
            s += "TELNET";
            if (json) {
                s += "\":\"";
            } else {
                s += ":";
            }
            s += ESP3DOutput::isOutput(ESP_TELNET_CLIENT)?"ON":"OFF";
            if (json) {
                s += "\"";
            }
#endif //TELNET_FEATURE
            if (json) {
                s += "}";
            }
            response = format_response(COMMANDID, json, true, s.c_str());
        } else { //set

#if COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL == MKS_SERIAL || COMMUNICATION_PROTOCOL == SOCKET_SERIAL
            parameter = get_param (cmd_params, "SERIAL=");
            if (parameter.length() != 0) {
                hasParam=true;
                if ((parameter == "ON")|| (parameter == "OFF")) {
                    if (!Settings_ESP3D::write_byte (ESP_SERIAL_FLAG, (parameter == "ON")?1:0)) {
                        response = format_response(COMMANDID, json, false, "Set failed");
                        noError = false;
                    }
                } else {
                    response = format_response(COMMANDID, json, false, "Incorrect value");
                    noError = false;
                }
            }
#endif //COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL == MKS_SERIAL || COMMUNICATION_PROTOCOL == SOCKET_SERIAL
#if !defined(ESP3DLIB_ENV) || (defined (ESP3DLIB_ENV) && (HAS_DISPLAY || defined (HAS_SERIAL_DISPLAY)))
            if (noError && !hasParam) {
                parameter = get_param (cmd_params, "REMOTE_SCREEN=");
                if (parameter.length() != 0) {
                    hasParam=true;
                    if ((parameter == "ON")|| (parameter == "OFF")) {
                        if (!Settings_ESP3D::write_byte (ESP_REMOTE_SCREEN_FLAG, (parameter == "ON")?1:0)) {
                            response = format_response(COMMANDID, json, false, "Set failed");
                            noError = false;
                        }
                    } else {
                        response = format_response(COMMANDID, json, false, "Incorrect value");
                        noError = false;
                    }
                }
            }
#endif //!defined(ESP3DLIB_ENV) || (defined (ESP3DLIB_ENV) && HAS_DISPLAY)
            if (noError && !hasParam) {
                parameter = get_param (cmd_params, "ALL=");
                if (parameter.length() != 0) {
                    hasParam=true;
                    if ((parameter == "ON")|| (parameter == "OFF")) {
                        if (
#if COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL == MKS_SERIAL || COMMUNICATION_PROTOCOL == SOCKET_SERIAL
                            !Settings_ESP3D::write_byte (ESP_SERIAL_FLAG, (parameter == "ON")?1:0)||
#endif //COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL == MKS_SERIAL || COMMUNICATION_PROTOCOL == SOCKET_SERIAL
#ifdef DISPLAY_DEVICE
                            !Settings_ESP3D::write_byte (ESP_SCREEN_FLAG, (parameter == "ON")?1:0)||
#endif //DISPLAY_DEVICE
#ifdef WS_DATA_FEATURE
                            !Settings_ESP3D::write_byte (ESP_WEBSOCKET_FLAG, (parameter == "ON")?1:0)||
#endif //WS_DATA_FEATURE
#ifdef BLUETOOTH_FEATURE
                            !Settings_ESP3D::write_byte (ESP_BT_FLAG, (parameter == "ON")?1:0)||
#endif //BLUETOOTH_FEATURE
#ifdef TELNET_FEATURE
                            !Settings_ESP3D::write_byte (ESP_TELNET_FLAG, (parameter == "ON")?1:0)||
#endif //TELNET_FEATURE
                            !Settings_ESP3D::write_byte (ESP_REMOTE_SCREEN_FLAG, (parameter == "ON")?1:0)) {
                            response = format_response(COMMANDID, json, false, "Set failed");
                            noError = false;
                        }
                    } else {
                        response = format_response(COMMANDID, json, false, "Incorrect value");
                        noError = false;
                    }
                }
            }
#ifdef DISPLAY_DEVICE
            if (noError && !hasParam) {
                parameter = get_param (cmd_params, "SCREEN=");
                if (parameter.length() != 0) {
                    hasParam=true;
                    if ((parameter == "ON")|| (parameter == "OFF")) {
                        if (!Settings_ESP3D::write_byte (ESP_SCREEN_FLAG, (parameter == "ON")?1:0)) {
                            response = format_response(COMMANDID, json, false, "Set failed");
                            noError = false;
                        }
                    } else {
                        response = format_response(COMMANDID, json, false, "Incorrect value");
                        noError = false;
                    }
                }
            }
#endif //DISPLAY_DEVICE
#ifdef WS_DATA_FEATURE
            if (noError && !hasParam) {
                parameter = get_param (cmd_params, "WEBSOCKET=");
                if (parameter.length() != 0) {
                    hasParam=true;
                    if ((parameter == "ON")|| (parameter == "OFF")) {
                        if (!Settings_ESP3D::write_byte (ESP_WEBSOCKET_FLAG, (parameter == "ON")?1:0)) {
                            response = format_response(COMMANDID, json, false, "Set failed");
                            noError = false;
                        }
                    } else {
                        response = format_response(COMMANDID, json, false, "Incorrect value");
                        noError = false;
                    }
                }
            }
#endif //WS_DATA_FEATURE
#ifdef BLUETOOTH_FEATURE
            if (noError && !hasParam) {
                parameter = get_param (cmd_params, "BT=");
                if (parameter.length() != 0) {
                    if ((parameter == "ON")|| (parameter == "OFF")) {
                        if (!Settings_ESP3D::write_byte (ESP_BT_FLAG, (parameter == "ON")?1:0)) {
                            response = format_response(COMMANDID, json, false, "Set failed");
                            noError = false;
                        }
                    } else {
                        response = format_response(COMMANDID, json, false, "Incorrect value");
                        noError = false;
                    }
                }
            }
#endif //BLUETOOTH_FEATURE
#ifdef TELNET_FEATURE
            if (noError && !hasParam) {
                parameter = get_param (cmd_params, "TELNET=");
                if (parameter.length() != 0) {
                    hasParam=true;
                    if ((parameter == "ON")|| (parameter == "OFF")) {
                        if (!Settings_ESP3D::write_byte (ESP_TELNET_FLAG, (parameter == "ON")?1:0)) {
                            response = format_response(COMMANDID, json, false, "Set failed");
                            noError = false;
                        }
                    } else {
                        response = format_response(COMMANDID, json, false, "Incorrect value");
                        noError = false;
                    }
                }
            }
#endif //TELNET_FEATURE
            //all ok we do the hot change
            if(noError) {
                if ( hasParam) {
                    ESP3DOutput::isOutput(ESP_ALL_CLIENTS,true);
                    response = format_response(COMMANDID, json, true, "ok");
                } else {
                    response = format_response(COMMANDID, json, false, "Incorrect parameter");
                    noError = false;
                }
            }
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
