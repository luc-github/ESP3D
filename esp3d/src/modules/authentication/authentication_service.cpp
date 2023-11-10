/*
  authentication_service.cpp - ESP3D authentication service class

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

#include "authentication_service.h"

#include "../../core/esp3d_message.h"
#include "../../core/esp3d_settings.h"

#if defined(AUTHENTICATION_FEATURE)
#if defined(HTTP_FEATURE)
#if defined(ARDUINO_ARCH_ESP32)
#include <WebServer.h>
#endif  // ARDUINO_ARCH_ESP32
#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WebServer.h>
#endif  // ARDUINO_ARCH_ESP8266
Authwebserver *AuthenticationService::_webserver = nullptr;
#endif  // HTTP_FEATURE
#endif  // AUTHENTICATION_FEATURE

#if defined(AUTHENTICATION_FEATURE)
String AuthenticationService::_adminpwd = "";
String AuthenticationService::_userpwd = "";
#if defined(HTTP_FEATURE)
uint32_t AuthenticationService::_sessionTimeout = 360000;
auth_ip *AuthenticationService::_head = nullptr;
uint8_t AuthenticationService::_current_nb_ip = 0;
#endif  // HTTP_FEATURE
#endif  // AUTHENTICATION_FEATURE

#define MAX_AUTH_IP 10
// #define ALLOW_MULTIPLE_SESSIONS

// check authentification
ESP3DAuthenticationLevel AuthenticationService::authenticated_level(
    const char *pwd, ESP3D_Message *esp3dmsg) {
#ifdef AUTHENTICATION_FEATURE
  ESP3DAuthenticationLevel auth_type = guest;
  if (pwd != nullptr) {
    if (isadmin(pwd)) {
      auth_type = admin;
    }
    if (isuser(pwd) && (auth_type != admin)) {
      auth_type = user;
    }
    return auth_type;
  } else {
    if (esp3dmsg) {
      if (esp3dmsg->getTarget() != ESP_HTTP_CLIENT) {
        return auth_type;
      }
    }
#if defined(HTTP_FEATURE)
    if (_webserver) {
      if (_webserver->hasHeader("Authorization")) {
        // esp3d_log("Check authorization %",(_webserver->uri()).c_str());
        if (_webserver->authenticate(DEFAULT_ADMIN_LOGIN, _adminpwd.c_str())) {
          auth_type = admin;
        } else {
          if (_webserver->authenticate(DEFAULT_USER_LOGIN, _userpwd.c_str())) {
            auth_type = user;
          }
        }
      }
      if (_webserver->hasHeader("Cookie")) {
        // esp3d_log("Check Cookie %s",(_webserver->uri()).c_str());
        String cookie = _webserver->header("Cookie");
        int pos = cookie.indexOf("ESPSESSIONID=");
        if (pos != -1) {
          int pos2 = cookie.indexOf(";", pos);
          String sessionID =
              cookie.substring(pos + strlen("ESPSESSIONID="), pos2);
          IPAddress ip = _webserver->getTarget().remoteIP();
          // check if cookie can be reset and clean table in same time
          auth_type = ResetAuthIP(ip, sessionID.c_str());
          // esp3d_log("Authentication = %d", auth_type);
        }
      }
    }
#endif  // HTTP_FEATURE
  }
  return auth_type;
#else
  (void)pwd;
  (void)esp3dmsg;
  return admin;
#endif  // AUTHENTICATION_FEATURE
}
#ifdef AUTHENTICATION_FEATURE

#if defined(HTTP_FEATURE)
uint32_t AuthenticationService::setSessionTimeout(uint32_t timeout) {
  if (timeout >= 0) {
    _sessionTimeout = timeout;
  }
  return _sessionTimeout;
}
uint32_t AuthenticationService::getSessionTimeout() { return _sessionTimeout; }
#endif  // HTTP_FEATURE

bool AuthenticationService::begin(Authwebserver *webserver) {
  end();
  update();
#if defined(HTTP_FEATURE)
  _webserver = webserver;
#endif  // HTTP_FEATURE
  // value is in ms but storage is in min
  _sessionTimeout = 1000 * 60 * ESP3DSettings::read_byte(ESP_SESSION_TIMEOUT);
  return true;
}
void AuthenticationService::end() {
#if defined(HTTP_FEATURE)
  _webserver = nullptr;
  ClearAllSessions();
#endif  // HTTP_FEATURE
}

void AuthenticationService::update() {
  _adminpwd = ESP3DSettings::read_string(ESP_ADMIN_PWD);
  _userpwd = ESP3DSettings::read_string(ESP_USER_PWD);
}

void AuthenticationService::handle() {}

// check admin password
bool AuthenticationService::isadmin(const char *pwd) {
  if (strcmp(_adminpwd.c_str(), pwd) != 0) {
    return false;
  } else {
    return true;
  }
}

// check user password - admin password is also valid
bool AuthenticationService::isuser(const char *pwd) {
  // it is not user password
  if (strcmp(_userpwd.c_str(), pwd) != 0) {
    // check admin password
    return isadmin(pwd);
  } else {
    return true;
  }
}

#if defined(HTTP_FEATURE)
// add the information in the linked list if possible
bool AuthenticationService::AddAuthIP(auth_ip *item) {
  if (_current_nb_ip > MAX_AUTH_IP) {
    return false;
  }
  item->_next = _head;
  _head = item;
  _current_nb_ip++;
  return true;
}

// Session ID based on IP and time using 16 char
char *AuthenticationService::create_session_ID() {
  static char sessionID[17];
  // reset SESSIONID
  for (int i = 0; i < 17; i++) {
    sessionID[i] = '\0';
  }
  // get time
  uint32_t now = millis();
  // get remote IP
  IPAddress remoteIP = _webserver->client().remoteIP();
  // generate SESSIONID
  if (0 > sprintf(sessionID, "%02X%02X%02X%02X%02X%02X%02X%02X", remoteIP[0],
                  remoteIP[1], remoteIP[2], remoteIP[3],
                  (uint8_t)((now >> 0) & 0xff), (uint8_t)((now >> 8) & 0xff),
                  (uint8_t)((now >> 16) & 0xff),
                  (uint8_t)((now >> 24) & 0xff))) {
    strcpy(sessionID, "NONE");
  }
  return sessionID;
}

bool AuthenticationService::ClearAllSessions() {
  while (_head) {
    auth_ip *current = _head;
    _head = _head->_next;
    delete current;
  }
  _current_nb_ip = 0;
  _head = nullptr;

  return true;
}

bool AuthenticationService::ClearCurrentSession() {
  String cookie = _webserver->header("Cookie");
  int pos = cookie.indexOf("ESPSESSIONID=");
  String sessionID;
  if (pos != -1) {
    int pos2 = cookie.indexOf(";", pos);
    sessionID = cookie.substring(pos + strlen("ESPSESSIONID="), pos2);
  }
  return ClearAuthIP(_webserver->client().remoteIP(), sessionID.c_str());
}

bool AuthenticationService::CreateSession(ESP3DAuthenticationLevel auth_level,
                                          const char *username,
                                          const char *session_ID) {
  auth_ip *current_auth = new auth_ip;
  current_auth->level = auth_level;
  current_auth->ip = _webserver->client().remoteIP();
  strcpy(current_auth->sessionID, session_ID);
  strcpy(current_auth->userID, username);
  current_auth->last_time = millis();
#ifndef ALLOW_MULTIPLE_SESSIONS
  // if not multiple session no need to keep all session, current one is enough
  ClearAllSessions();
#endif  // ALLOW_MULTIPLE_SESSIONS
  if (AddAuthIP(current_auth)) {
    return true;
  } else {
    delete current_auth;
    return false;
  }
}

bool AuthenticationService::ClearAuthIP(IPAddress ip, const char *sessionID) {
  auth_ip *current = _head;
  auth_ip *previous = NULL;
  bool done = false;
  while (current) {
    if ((ip == current->ip) && (strcmp(sessionID, current->sessionID) == 0)) {
      // remove
      done = true;
      if (current == _head) {
        _head = current->_next;
        _current_nb_ip--;
        delete current;
        current = _head;
      } else {
        previous->_next = current->_next;
        _current_nb_ip--;
        delete current;
        current = previous->_next;
      }
    } else {
      previous = current;
      current = current->_next;
    }
  }
  return done;
}

// Get info
auth_ip *AuthenticationService::GetAuth(IPAddress ip, const char *sessionID) {
  auth_ip *current = _head;
  while (current) {
    if (ip == current->ip) {
      if (strcmp(sessionID, current->sessionID) == 0) {
        // found
        return current;
      }
    }
    // previous = current;
    current = current->_next;
  }
  return NULL;
}

// Get time left for specific session
uint32_t AuthenticationService::getSessionRemaining(const char *sessionID) {
  auth_ip *current = _head;
  if ((sessionID == nullptr) || (strlen(sessionID) == 0)) {
    return 0;
  }
  while (current) {
    if (strcmp(sessionID, current->sessionID) == 0) {
      // found
      uint32_t now = millis();
      if ((now - current->last_time) > _sessionTimeout) {
        return 0;
      }
      return _sessionTimeout - (now - current->last_time);
    }
    // previous = current;
    current = current->_next;
  }
  return 0;
}

// Review all IP to reset timers
ESP3DAuthenticationLevel AuthenticationService::ResetAuthIP(
    IPAddress ip, const char *sessionID) {
  auth_ip *current = _head;
  auth_ip *previous = NULL;
  // get time
  // uint32_t now = millis();
  while (current) {
    // if time out is reached and time out is not disabled
    // if IP is not current one and time out is disabled
    if ((((millis() - current->last_time) > _sessionTimeout) &&
         (_sessionTimeout != 0)) ||
        ((ip != current->ip) && (_sessionTimeout == 0))) {
      // remove
      if (current == _head) {
        _head = current->_next;
        _current_nb_ip--;
        delete current;
        current = _head;
      } else {
        previous->_next = current->_next;
        _current_nb_ip--;
        delete current;
        current = previous->_next;
      }
    } else {
      if (ip == current->ip) {
        if (strcmp(sessionID, current->sessionID) == 0) {
          // reset time
          current->last_time = millis();
          return (ESP3DAuthenticationLevel)current->level;
        }
      }
      previous = current;
      current = current->_next;
    }
  }
  return guest;
}
#endif  // HTTP_FEATURE

#endif  // AUTHENTICATION_FEATURE
