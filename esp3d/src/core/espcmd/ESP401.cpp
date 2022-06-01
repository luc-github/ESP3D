/*
 ESP401.cpp - ESP3D command class

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
#define COMMANDID   401
#if COMMUNICATION_PROTOCOL != SOCKET_SERIAL
#include "../../modules/serial/serial_service.h"
#endif // COMMUNICATION_PROTOCOL != SOCKET_SERIAL
#ifdef SENSOR_DEVICE
#include "../../modules/sensor/sensor.h"
#endif //SENSOR_DEVICE
#ifdef BUZZER_DEVICE
#include "../../modules/buzzer/buzzer.h"
#endif //BUZZER_DEVICE
#ifdef TIMESTAMP_FEATURE
#include "../../modules/time/time_server.h"
#endif //TIMESTAMP_FEATURE
#ifdef NOTIFICATION_FEATURE
#include "../../modules/notifications/notifications_service.h"
#endif //NOTIFICATION_FEATURE
#ifdef SD_DEVICE
#include "../../modules/filesystem/esp_sd.h"
#endif //SD_DEVICE
//Set EEPROM setting
//[ESP401]P=<position> T=<type> V=<value> json=<no> pwd=<user/admin password>
bool Commands::ESP401(const char* cmd_params, level_authenticate_type auth_type, ESP3DOutput * output)
{
    bool noError = true;
    bool json = has_tag (cmd_params, "json");
    String spos= "";
    String response;
    String parameter;
    int errorCode = 200; //unless it is a server error use 200 as default and set error in json instead
#ifdef AUTHENTICATION_FEATURE
    if (auth_type != LEVEL_ADMIN) {
        response = "Wrong authentication level";
        noError = false;
        errorCode = 401;
    }
#else
    (void)auth_type;
#endif //AUTHENTICATION_FEATURE
    if (noError) {
        //check validity of parameters
        spos = get_param (cmd_params, "P=");
        String styp = get_param (cmd_params, "T=");
        String sval = get_param (cmd_params, "V=");
        if (spos.length() == 0) {
            response = "Invalid parameter P";
            noError = false;
        } else if (styp.length() == 0) {
            response = "Invalid parameter T";
            noError = false;
        } else if (sval.length() == 0) {
            response = "Invalid parameter V";
            noError = false;
        } else {
            if (! (styp == "B" || styp == "S" || styp == "A" || styp == "I") ) {
                response = "Invalid value for T";
                noError = false;
            }
        }
        if (noError) {

            if (response) {
                //Byte value
                if (styp == "B") {
                    if (!Settings_ESP3D::write_byte (spos.toInt(), sval.toInt())) {
                        response = false;
                    } else {
                        //dynamique refresh is better than restart the boards
                        switch(spos.toInt()) {
                        case ESP_SERIAL_FLAG:
                        case ESP_REMOTE_SCREEN_FLAG:
                        case ESP_WEBSOCKET_FLAG:
                        case ESP_TELNET_FLAG:
                        case ESP_SCREEN_FLAG:
                        case ESP_BT_FLAG:
                            ESP3DOutput::isOutput(ESP_ALL_CLIENTS,true);
                            break;
                        case ESP_VERBOSE_BOOT:
                            Settings_ESP3D::isVerboseBoot(true);
                            break;
                        case ESP_TARGET_FW:
                            Settings_ESP3D::GetFirmwareTarget(true);
                            break;
#if COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL == MKS_SERIAL
                        case ESP_SECURE_SERIAL:
                            serial_service.setParameters();
                            break;
#endif // COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL == MKS_SERIAL
#ifdef AUTHENTICATION_FEATURE
                        case ESP_SESSION_TIMEOUT:
                            AuthenticationService::setSessionTimeout(1000*60*sval.toInt());
                            break;
#endif //AUTHENTICATION_FEATURE
#ifdef SD_DEVICE
                        case ESP_SD_SPEED_DIV:
                            ESP_SD::setSPISpeedDivider(sval.toInt());
                            break;
#endif //SD_DEVICE
#ifdef TIMESTAMP_FEATURE
                        case ESP_INTERNET_TIME:
                            timeserver.begin();
                            break;
#endif //TIMESTAMP_FEATURE
#ifdef NOTIFICATION_FEATURE
                        case ESP_AUTO_NOTIFICATION:
                            notificationsservice.setAutonotification((sval.toInt() == 0)?false:true);
                            break;
#endif //NOTIFICATION_FEATURE
#ifdef SENSOR_DEVICE
                        case ESP_SENSOR_TYPE:
                            esp3d_sensor.begin();
                            break;
#endif //SENSOR_DEVICE
#ifdef BUZZER_DEVICE
                        case ESP_BUZZER:
                            if (sval.toInt() == 1) {
                                esp3d_buzzer.begin();
                            } else if (sval.toInt() == 0) {
                                esp3d_buzzer.end();
                            }
                            break;
#endif //BUZZER_DEVICE
                        default:
                            break;
                        }
                    }
                }
                //Integer value
                if (styp == "I") {
                    if (!Settings_ESP3D::write_uint32 (spos.toInt(), sval.toInt())) {
                        response = "Set failed";
                        noError = false;
                    } else {
                        //dynamique refresh is better than restart the board
                        switch(spos.toInt()) {
#ifdef SENSOR_DEVICE
                        case ESP_SENSOR_INTERVAL:
                            esp3d_sensor.setInterval(sval.toInt());
                            break;
#endif //SENSOR_DEVICE
#if COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL == MKS_SERIAL
                        case ESP_BAUD_RATE:
                            serial_service.updateBaudRate(sval.toInt());
                            break;
#endif // COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL == MKS_SERIAL
                        default:
                            break;
                        }
                    }
                }
                //String value
                if (styp == "S") {
                    if (!Settings_ESP3D::write_string (spos.toInt(), sval.c_str())) {
                        response = "Set failed";
                        noError = false;
                    } else {
                        //dynamique refresh is better than restart the board
                        switch(spos.toInt()) {
#ifdef AUTHENTICATION_FEATURE
                        case ESP_ADMIN_PWD:
                        case ESP_USER_PWD:
                            AuthenticationService::update();
                            break;
#endif //AUTHENTICATION_FEATURE 
                        default:
                            break;
                        }
                    }
                }
#if defined (WIFI_FEATURE)
                //IP address
                if (styp == "A") {
                    if (!Settings_ESP3D::write_IP_String (spos.toInt(), sval.c_str())) {
                        response = "Set failed";
                        noError = false;
                    } else {
                        //dynamique refresh is better than restart the board
                        //TBD
                    }
                }
#endif //WIFI_FEATURE
            }
        }
    }
    if (noError) {
        if (json) {
            response = format_response(COMMANDID, json, true, String(spos).c_str());
            output->printLN (response.c_str() );
        } else {
            response = format_response(COMMANDID, json, true, "ok");
            output->printMSG (response.c_str() );
        }
    } else {
        if (json) {
            String tmp = "{\"error\":\"";
            tmp += response;
            tmp += "\"";
            if (spos.length() > 0) {
                tmp += ",\"position\":\"";
                tmp += spos;
                tmp += "\"";
            }
            tmp += "}";
            response = tmp;
        } else {
            response+= spos.length() > 0 ? " for P=" + spos : "";
        }
        response = format_response(COMMANDID, json, false, response.c_str());
        output->printERROR(response.c_str(), errorCode);
    }
    return noError;
}
