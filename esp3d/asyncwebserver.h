/*
  asyncwebserver.h - ESP3D sync functions file

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

#ifndef ASYNCWEBSERVER_H
#define ASYNCWEBSERVER_H
#include "config.h"
class AsyncWebServerRequest;

extern bool filterOnRoot (AsyncWebServerRequest *request);
extern void handle_login(AsyncWebServerRequest *request);
extern void SPIFFSFileupload (AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
extern void handleFileList (AsyncWebServerRequest *request);
extern void handle_not_found (AsyncWebServerRequest *request);
extern void handle_web_command (AsyncWebServerRequest *request);
extern void handle_web_command_silent (AsyncWebServerRequest *request);
extern void handle_serial_SDFileList (AsyncWebServerRequest *request);
extern void SDFile_serial_upload (AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
extern void handle_Websocket_Event(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len);
extern void handle_onevent_connect(AsyncEventSourceClient *client);
extern bool can_process_serial;

#ifdef SSDP_FEATURE
extern void handle_SSDP (AsyncWebServerRequest *request);
#endif

#ifdef WEB_UPDATE_FEATURE
extern void handleUpdate (AsyncWebServerRequest *request);
extern void WebUpdateUpload (AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
#endif


#endif
