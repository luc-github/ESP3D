/* 
  webinterface.cpp - esp8266 configuration class

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

#include "config.h"
#include "webinterface.h"
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <ESP8266WebServer.h>

//cannot put it in class then cast it as std::function<void(void)> so put outside
void handle_web_interface_root()
{
  web_interface.WebServer.send(200, "text/plain", "hello from esp8266 from port 80!");
}

WEBINTERFACE_CLASS::WEBINTERFACE_CLASS (int port):WebServer(port)
{
  //init what will handle "/"
  WebServer.on("/",HTTP_GET, handle_web_interface_root);
}

WEBINTERFACE_CLASS web_interface(80);

