/*
  camera.h -  camera functions class

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



#ifndef _CAMERA_H
#define _CAMERA_H
//class WebSocketsServer;
#if defined (ARDUINO_ARCH_ESP32)
class WebServer;
#define STREAMSERVER WebServer
#endif //ARDUINO_ARCH_ESP32
#if defined (ARDUINO_ARCH_ESP8266)
#include <ESP8266WebServer.h>
#define STREAMSERVER ESP8266WebServer
#endif //ARDUINO_ARCH_ESP8266

class Camera
{
public:
    Camera();
    ~Camera();
    bool begin();
    void end();
    bool initHardware();
    bool stopHardware();
    bool startStreamServer();
    bool stopStreamServer();
    static void handle_stream();
    static void handle_snap();
    void process();
    void handle();
    static int command(const char * param, const char * value);
    uint8_t GetModel();
    const char *GetModelString();
    bool started()
    {
        return _started;
    }
    bool serverstarted()
    {
        return _server_started;
    }
    void connect(bool status)
    {
        _connected = status;
    }
    bool isconnected()
    {
        return _connected;
    }
    bool isinitialised()
    {
        return _initialised;
    }
    uint16_t port()
    {
        return _port;
    }
private:
    static bool _initialised;
    static STREAMSERVER * _streamserver;
    bool _server_started;
    bool _started;
    static bool _connected;
    uint16_t _port;
};

extern Camera esp3d_camera;

#endif //_CAMERA_H

