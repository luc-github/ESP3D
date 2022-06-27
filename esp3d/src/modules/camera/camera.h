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
#include <WebServer.h>

class Camera
{
public:
    Camera();
    ~Camera();
    bool begin();
    void end();
    bool initHardware();
    bool stopHardware();
    bool handle_snap(WebServer * webserver, const char *path=NULL, const char* filename=NULL);
    void handle();
    int command(const char * param, const char * value);
    uint8_t GetModel();
    const char *GetModelString();
    bool started()
    {
        return _started;
    }
    bool isinitialised()
    {
        return _initialised;
    }
private:
    bool _initialised;
    bool _started;
};

extern Camera esp3d_camera;

#endif //_CAMERA_H

