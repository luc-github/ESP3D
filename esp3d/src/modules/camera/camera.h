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

class Camera
{
public:
    Camera();
    ~Camera();
    bool initHardware(bool forceinit = false);
    bool begin(bool forceinit = false);
    void end();
    void handle();
    int command(const char * param, const char * value);
    uint8_t GetModel();
    const char *GetModelString();
    bool started()
    {
        return _started;
    }
    bool initialised()
    {
        return _initialised;
    }
    bool stopHardware();
    void connect(bool status)
    {
        _connected = status;
    }
    bool isconnected()
    {
        return _connected;
    }
    uint16_t port()
    {
        return _port;
    }
private:
    bool _initialised;
    bool _started;
    bool _connected;
    uint16_t _port;
};

extern Camera esp3d_camera;

#endif //_CAMERA_H

