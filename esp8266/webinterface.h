/* 
  webinterface.h - esp8266 configuration class

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
#include <FS.h>
#include "storestrings.h"

#define MAX_EXTRUDERS 4

class WEBINTERFACE_CLASS
{
  public:
  WEBINTERFACE_CLASS (int port = 80);
  ~WEBINTERFACE_CLASS();
  ESP8266WebServer WebServer;
  File fsUploadFile;
  void urldecode( String & dst, const char *src);
  bool isSSIDValid(const char * ssid);
  bool isPasswordValid(const char * password);
  bool isHostnameValid(const char * hostname);
  bool isIPValid(const char * IP);
  String answer4M105;
  String answer4M114;
  String answer4M220;
  String answer4M221;
  STORESTRINGS_CLASS fileslist;
  uint32_t last_temp;
  STORESTRINGS_CLASS error_msg;
  STORESTRINGS_CLASS info_msg;
  STORESTRINGS_CLASS status_msg;
  bool restartmodule; 
  private:

};

extern WEBINTERFACE_CLASS * web_interface;

#endif
