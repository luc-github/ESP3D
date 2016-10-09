/*
  webinterface.h - ESP3D configuration class

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

#ifndef WEBINTERFACE_h
#define WEBINTERFACE_h
#include <Arduino.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <ESP8266WebServer.h>
#ifdef SDCARD_FEATURE
#ifndef FS_NO_GLOBALS
#define FS_NO_GLOBALS
#endif
#endif
#include <FS.h>
#include "storestrings.h"

#define MAX_EXTRUDERS 4

typedef enum {
    LEVEL_GUEST = 0,
    LEVEL_USER = 1,
    LEVEL_ADMIN = 2
} level_authenticate_type;

struct auth_ip {
    IPAddress ip;
    level_authenticate_type level;
    char sessionID[17];
    uint32_t last_time;
    auth_ip * _next;
};

class WEBINTERFACE_CLASS
{
public:
    WEBINTERFACE_CLASS (int port = 80);
    ~WEBINTERFACE_CLASS();
    ESP8266WebServer WebServer;
    FSFILE fsUploadFile;
#ifdef TEMP_MONITORING_FEATURE
    String answer4M105;
    uint32_t last_temp;
#endif
#ifdef POS_MONITORING_FEATURE
    String answer4M114;
#endif
#ifdef SPEED_MONITORING_FEATURE
    String answer4M220;
#endif
#ifdef FLOW_MONITORING_FEATURE
    String answer4M221;
#endif
#ifndef DIRECT_SDCARD_FEATURE
    STORESTRINGS_CLASS fileslist;
#endif
#ifdef ERROR_MSG_FEATURE
    STORESTRINGS_CLASS error_msg;
#endif
#ifdef INFO_MSG_FEATURE
    STORESTRINGS_CLASS info_msg;
#endif
#ifdef STATUS_MSG_FEATURE
    STORESTRINGS_CLASS status_msg;
#endif
    bool restartmodule;
    bool processTemplate(const char  * filename, STORESTRINGS_CLASS & KeysList ,  STORESTRINGS_CLASS & ValuesList );
    void GetFreeMem(STORESTRINGS_CLASS & KeysList, STORESTRINGS_CLASS & ValuesList);
    void GeLogin(STORESTRINGS_CLASS & KeysList, STORESTRINGS_CLASS & ValuesList,level_authenticate_type auth_level);
    void GetIpWeb(STORESTRINGS_CLASS & KeysList, STORESTRINGS_CLASS & ValuesList);
    void GetMode(STORESTRINGS_CLASS & KeysList, STORESTRINGS_CLASS & ValuesList);
    void GetPorts(STORESTRINGS_CLASS & KeysList, STORESTRINGS_CLASS & ValuesList);
    void SetPageProp(STORESTRINGS_CLASS & KeysList, STORESTRINGS_CLASS & ValuesList,
                     const __FlashStringHelper *title, const __FlashStringHelper *filename);
    void GetDHCPStatus(STORESTRINGS_CLASS & KeysList, STORESTRINGS_CLASS & ValuesList);
    void ProcessAlertError(STORESTRINGS_CLASS & KeysList, STORESTRINGS_CLASS & ValuesList, String & smsg);
    void ProcessAlertSuccess(STORESTRINGS_CLASS & KeysList, STORESTRINGS_CLASS & ValuesList, String & smsg);
    void ProcessNoAlert(STORESTRINGS_CLASS & KeysList, STORESTRINGS_CLASS & ValuesList);
    String getContentType(String filename);
    level_authenticate_type is_authenticated();
    bool AddAuthIP(auth_ip * item);
    bool blockserial;
#ifdef AUTHENTICATION_FEATURE
    level_authenticate_type ResetAuthIP(IPAddress ip,const char * sessionID);
    char * create_session_ID();
#endif
    uint8_t _upload_status;

private:
    auth_ip * _head;
    uint8_t _nb_ip;
};

extern WEBINTERFACE_CLASS * web_interface;

#endif
