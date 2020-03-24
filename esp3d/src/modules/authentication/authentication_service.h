/*
  authentication_service.h -  authentication functions class

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



#ifndef _AUTHENTICATION_SERVICE_H
#define _AUTHENTICATION_SERVICE_H
typedef enum {
    LEVEL_GUEST = 0,
    LEVEL_USER = 1,
    LEVEL_ADMIN = 2
} level_authenticate_type;

const char DEFAULT_ADMIN_LOGIN [] =      "admin";
const char DEFAULT_USER_LOGIN [] =       "user";

#include "../../include/esp3d_config.h"
#if defined (AUTHENTICATION_FEATURE)
#if defined (HTTP_FEATURE)
#include <IPAddress.h>
struct auth_ip {
    IPAddress ip;
    level_authenticate_type level;
    char userID[17];
    char sessionID[17];
    uint32_t last_time;
    auth_ip * _next;
};
#if defined (ARDUINO_ARCH_ESP32)
class WebServer;
typedef  WebServer Authwebserver;
#endif //ARDUINO_ARCH_ESP32
#if defined (ARDUINO_ARCH_ESP8266)
#include <ESP8266WebServer.h>
typedef  ESP8266WebServer Authwebserver;
#endif //ARDUINO_ARCH_ESP8266
#else
typedef void Authwebserver;
#endif // HTTP_FEATURE
#endif //AUTHENTICATION_FEATURE
class AuthenticationService
{
public:
    AuthenticationService();
    ~AuthenticationService();
    static level_authenticate_type  authenticated_level(const  char * pwd = nullptr);
#ifdef AUTHENTICATION_FEATURE
    static bool begin(Authwebserver * webserver);
    static void end();
    static void handle();
    static bool isadmin (const char *pwd);
    static void update();
    static bool isuser (const char *pwd);
#if defined (HTTP_FEATURE)
    static uint32_t setSessionTimeout(uint32_t timeout);
    static uint32_t getSessionTimeout();
    static char * create_session_ID();
    static bool ClearCurrentSession ();
    static bool ClearAllSessions ();
    static bool CreateSession(level_authenticate_type auth_level, const char * username, const char* session_ID);
#endif //HTTP_FEATURE
private:
    static String _adminpwd;
    static String _userpwd;
#if defined (HTTP_FEATURE)
    static bool AddAuthIP (auth_ip * item);
    static bool ClearAuthIP (IPAddress ip, const char * sessionID);
    static auth_ip * GetAuth (IPAddress ip, const char * sessionID);
    static level_authenticate_type ResetAuthIP (IPAddress ip, const char * sessionID);
    static Authwebserver * _webserver;
    static uint32_t _sessionTimeout;
    static auth_ip * _head;
    static uint8_t _current_nb_ip;
#endif //HTTP_FEATURE
#endif //AUTHENTICATION_FEATURE
};

#endif //_ESP3DSECURITY_H

