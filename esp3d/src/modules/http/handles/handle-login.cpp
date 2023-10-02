/*
 handle-login.cpp - ESP3D http handle

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
#include "../../../include/esp3d_config.h"
#if defined (HTTP_FEATURE)
#include "../http_server.h"
#if defined (ARDUINO_ARCH_ESP32)
#include <WebServer.h>
#endif //ARDUINO_ARCH_ESP32
#if defined (ARDUINO_ARCH_ESP8266)
#include <ESP8266WebServer.h>
#endif //ARDUINO_ARCH_ESP8266
#include "../../authentication/authentication_service.h"
#include "../../../core/esp3doutput.h"
#include "../../../core/settings_esp3d.h"

//login status check
void HTTP_Server::handle_login()
{
#ifdef AUTHENTICATION_FEATURE
    int code = 401;
    String status = "Wrong authentication!";
    //Disconnect can be done anytime no need to check credential
    if (_webserver->hasArg("DISCONNECT") && _webserver->arg("DISCONNECT")=="YES") {
        AuthenticationService::ClearCurrentSession();
        _webserver->sendHeader("Set-Cookie","ESPSESSIONID=0");
        _webserver->sendHeader("Cache-Control","no-cache");
        _webserver->send(401, "application/json", "{\"status\":\"disconnected\",\"authentication_lvl\":\"guest\"}");
        return;
    }
    level_authenticate_type auth_level = AuthenticationService::authenticated_level();
    //check is it is a submission or a query
    if (_webserver->hasArg("SUBMIT")) {
        //is there a correct list of query?
        if (_webserver->hasArg("PASSWORD") && _webserver->hasArg("USER")) {
            //User
            String sUser = _webserver->arg("USER");
            //Password
            String sPassword = _webserver->arg("PASSWORD");
            if((((sUser == DEFAULT_ADMIN_LOGIN) && (AuthenticationService::isadmin(sPassword.c_str()))) ||
                    ((sUser == DEFAULT_USER_LOGIN) && (AuthenticationService::isuser(sPassword.c_str()))))) {
                //check if it is to change password  or login
                if (_webserver->hasArg("NEWPASSWORD")) {
                    String newpassword =  _webserver->arg("NEWPASSWORD");
                    //check new password
                    if (Settings_ESP3D::isLocalPasswordValid(newpassword.c_str())) {
                        if (!Settings_ESP3D::write_string (ESP_ADMIN_PWD, newpassword.c_str())) {
                            code = 500;
                            status = "Set failed!";
                        } else {
                            code = 200;
                            status = "ok";
                        }
                    } else {
                        code = 500;
                        status = "Incorrect password!";
                    }
                } else { //do authentication
                    //allow to change session timeout when login
                    if (_webserver->hasArg("TIMEOUT")) {
                        String timeout =  _webserver->arg("TIMEOUT");
                        AuthenticationService::setSessionTimeout(timeout.toInt());
                    }
                    //it is a change or same level
                    if (((auth_level == LEVEL_USER) && (sUser == DEFAULT_USER_LOGIN)) ||
                            ((auth_level == LEVEL_ADMIN)&& (sUser == DEFAULT_ADMIN_LOGIN))) {
                        code = 200;
                        status = "ok";
                    } else { //new authentication
                        String session = AuthenticationService::create_session_ID();
                        if (AuthenticationService::CreateSession((sUser == DEFAULT_ADMIN_LOGIN)?LEVEL_ADMIN:LEVEL_USER,sUser.c_str(), session.c_str())) {
                            AuthenticationService::ClearCurrentSession();
                            code = 200;
                            status = "ok";
                            String tmps ="ESPSESSIONID=";
                            tmps+=session;
                            _webserver->sendHeader("Set-Cookie",tmps);
                        }
                    }
                }
            }
        }
    } else {
        if (auth_level == LEVEL_USER || auth_level == LEVEL_ADMIN) {
            status = "Identified";
            code = 200;
        }
    }
    _webserver->sendHeader("Cache-Control","no-cache");
    String smsg = "{\"status\":\"";
    smsg+=status;
    smsg+="\",\"authentication_lvl\":\"";
    if (auth_level == LEVEL_USER) {
        smsg += "user";
    } else if (auth_level == LEVEL_ADMIN) {
        smsg += "admin";
    } else {
        smsg += "guest";
    }
    smsg += "\"}";
    _webserver->send(code, "application/json", smsg);
    return;
#else // No AUTHENTICATION_FEATURE
    _webserver->sendHeader("Cache-Control","no-cache");
    _webserver->send(200, "application/json", "{\"status\":\"ok\",\"authentication_lvl\":\"admin\"}");
#endif //AUTHENTICATION_FEATURE
}

#endif //HTTP_FEATURE
