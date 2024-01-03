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
#if defined(HTTP_FEATURE)
#include "../http_server.h"
#if defined(ARDUINO_ARCH_ESP32)
#include <WebServer.h>
#endif  // ARDUINO_ARCH_ESP32
#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WebServer.h>
#endif  // ARDUINO_ARCH_ESP8266
#include "../../../core/esp3d_message.h"
#include "../../../core/esp3d_settings.h"
#include "../../authentication/authentication_service.h"

// login status check
void HTTP_Server::handle_login() {
  HTTP_Server::set_http_headers();

#ifdef AUTHENTICATION_FEATURE
  int code = 401;
  String status = "Wrong authentication!";
  // Disconnect can be done anytime no need to check credential
  if (_webserver->hasArg("DISCONNECT") &&
      _webserver->arg("DISCONNECT") == "YES") {
    AuthenticationService::ClearCurrentHttpSession();
    _webserver->sendHeader("Set-Cookie", "ESPSESSIONID=0");
    _webserver->sendHeader("Cache-Control", "no-cache");
    _webserver->send(
        401, "application/json",
        "{\"status\":\"disconnected\",\"authentication_lvl\":\"guest\"}");
    return;
  }
  ESP3DAuthenticationLevel auth_level =
      AuthenticationService::getAuthenticatedLevel();
  // check is it is a submission or a query
  if (_webserver->hasArg("SUBMIT")) {
    // is there a correct list of query?
    if (_webserver->hasArg("PASSWORD") && _webserver->hasArg("USER")) {
      // User
      String sUser = _webserver->arg("USER");
      // Password
      String sPassword = _webserver->arg("PASSWORD");
      if ((((sUser == DEFAULT_ADMIN_LOGIN) &&
            (AuthenticationService::isadmin(sPassword.c_str()))) ||
           ((sUser == DEFAULT_USER_LOGIN) &&
            (AuthenticationService::isuser(sPassword.c_str()))))) {
        // check if it is to change password  or login
        if (_webserver->hasArg("NEWPASSWORD")) {
          String newpassword = _webserver->arg("NEWPASSWORD");
          // check new password
          if (ESP3DSettings::isValidStringSetting(newpassword.c_str(),
                                                  ESP_ADMIN_PWD)) {
            if (!ESP3DSettings::writeString(ESP_ADMIN_PWD,
                                            newpassword.c_str())) {
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
        } else {  // do authentication
          // allow to change session timeout when login
          if (_webserver->hasArg("TIMEOUT")) {
            String timeout = _webserver->arg("TIMEOUT");
            AuthenticationService::setSessionTimeout(timeout.toInt());
          }
          // it is a change or same level
          if (((auth_level == ESP3DAuthenticationLevel::user) &&
               (sUser == DEFAULT_USER_LOGIN)) ||
              ((auth_level == ESP3DAuthenticationLevel::admin) &&
               (sUser == DEFAULT_ADMIN_LOGIN))) {
            code = 200;
            status = "ok";
          } else {  // new authentication
            String session = AuthenticationService::create_session_ID();
            if (AuthenticationService::CreateSession(
                    (sUser == DEFAULT_ADMIN_LOGIN)
                        ? ESP3DAuthenticationLevel::admin
                        : ESP3DAuthenticationLevel::user,
                    ESP3DClientType::http, session.c_str())) {
              AuthenticationService::ClearCurrentHttpSession();
              code = 200;
              status = "ok";
              String tmps = "ESPSESSIONID=";
              tmps += session;
              _webserver->sendHeader("Set-Cookie", tmps);
            }
          }
        }
      }
    }
  } else {
    if (auth_level == ESP3DAuthenticationLevel::user ||
        auth_level == ESP3DAuthenticationLevel::admin) {
      status = "Identified";
      code = 200;
    }
  }
  _webserver->sendHeader("Cache-Control", "no-cache");
  String smsg = "{\"status\":\"";
  smsg += status;
  smsg += "\",\"authentication_lvl\":\"";
  if (auth_level == ESP3DAuthenticationLevel::user) {
    smsg += "user";
  } else if (auth_level == ESP3DAuthenticationLevel::admin) {
    smsg += "admin";
  } else {
    smsg += "guest";
  }
  smsg += "\"}";
  _webserver->send(code, "application/json", smsg);
  return;
#else   // No AUTHENTICATION_FEATURE
  _webserver->sendHeader("Cache-Control", "no-cache");
  _webserver->send(200, "application/json",
                   "{\"status\":\"ok\",\"authentication_lvl\":\"admin\"}");
#endif  // AUTHENTICATION_FEATURE
}

#endif  // HTTP_FEATURE
