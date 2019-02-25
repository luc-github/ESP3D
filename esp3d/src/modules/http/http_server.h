/*
  http_server.h -  http server functions class

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


#ifndef _HTTP_SERVER_H
#define _HTTP_SERVER_H
#include "../../include/esp3d_config.h"
//class WebSocketsServer;
#if defined (ARDUINO_ARCH_ESP32)
class WebServer;
#define WEBSERVER WebServer
#endif //ARDUINO_ARCH_ESP32
#if defined (ARDUINO_ARCH_ESP8266)
class ESP8266WebServer;
#define WEBSERVER ESP8266WebServer
#endif //ARDUINO_ARCH_ESP8266


//Upload status
typedef enum {
    UPLOAD_STATUS_NONE = 0,
    UPLOAD_STATUS_FAILED = 1,
    UPLOAD_STATUS_CANCELLED = 2,
    UPLOAD_STATUS_SUCCESSFUL = 3,
    UPLOAD_STATUS_ONGOING  = 4
} upload_status_type;

class HTTP_Server
{
public:
    HTTP_Server();
    ~HTTP_Server();
    static bool begin();
    static void end();
    static void handle();
    static bool started()
    {
        return _started;
    }
    static uint16_t port()
    {
        return _port;
    }
private:
    static bool _started;
    static WEBSERVER * _webserver;
    static uint16_t _port;
    static uint8_t _upload_status;
    static const char * getContentType (const char * filename);
    static const char * get_Splited_Value(String data, char separator, int index);
#ifdef SSDP_FEATURE
    static void handle_SSDP ();
#endif //SSDP_FEATURE
    static void init_handlers();
    static bool StreamFSFile(const char* filename, const char * contentType);
    static void handle_root();
    static void handle_login();
    static void handle_not_found ();
    static void handle_web_command ();
    // static void handle_Websocket_Event(uint8_t num, uint8_t type, uint8_t * payload, size_t length);
#ifdef FILESYSTEM_FEATURE
    static void FSFileupload ();
    static void handleFSFileList ();
#endif //FILESYSTEM_FEATURE
#ifdef WEB_UPDATE_FEATURE
    static void handleUpdate ();
    static void WebUpdateUpload ();
#endif //WEB_UPDATE_FEATURE
    //static bool is_realtime_cmd(char c);
#ifdef ENABLE_SD_CARD
    //static void handle_direct_SDFileList();
    //static void SDFile_direct_upload();
    //static bool deleteRecursive(String path);
#endif
};

#endif //_HTTP_SERVER_H

