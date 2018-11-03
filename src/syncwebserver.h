/*
  syncwebserver.h - ESP3D sync functions file

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

#ifndef SYNCWEBSERVER_H
#define SYNCWEBSERVER_H
#include "config.h"
#define NODEBUG_WEBSOCKETS
#include <WebSocketsServer.h>

extern void handle_web_interface_root();
extern void handle_login();
extern void handleFileList();
extern void SPIFFSFileupload();
extern void handle_not_found();
extern void handle_web_command();
extern void handle_web_command_silent();
extern void handle_serial_SDFileList();
extern void SDFile_serial_upload();
extern WebSocketsServer * socket_server;
extern void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length);

#ifdef SSDP_FEATURE
extern void handle_SSDP ();
#endif

#ifdef WEB_UPDATE_FEATURE
extern void handleUpdate ();
extern void WebUpdateUpload ();
#endif

#endif
