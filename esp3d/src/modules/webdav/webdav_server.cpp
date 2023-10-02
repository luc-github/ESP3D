/*
  webdav_server.cpp -  webdav server functions class

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

#include "../../include/esp3d_config.h"

#if defined (WEBDAV_FEATURE)
#include <WiFiServer.h>
#include <WiFiClient.h>
#include "webdav_server.h"
#include "../../core/settings_esp3d.h"
#include "../../core/esp3doutput.h"
#include "../../core/commands.h"

Webdav_Server webdav_server;

void Webdav_Server::closeClient()
{
    if(_dav.Client()) {
        _dav.Client().stop();
    }
}

void Webdav_Server::dir()
{
    _dav.dir("/", &Serial);
};

bool Webdav_Server::isConnected()
{
    if ( !_started) {
        return false;
    }
    if (_dav.Client()) {
        return (_dav.Client().connected());
    }
    return false;
}

const char* Webdav_Server::clientIPAddress()
{
    static String res;
    res = "0.0.0.0";
    if (_dav.Client() && _dav.Client().connected()) {
        res = _dav.Client().remoteIP().toString();
    }
    return res.c_str();
}


Webdav_Server::Webdav_Server():_tcpserver(0)
{
    _started = false;
    _port = 0;
}

Webdav_Server::~Webdav_Server()
{
    end();
}

/**
 * begin WebDav setup
 */
bool Webdav_Server::begin()
{
    end();
    if (Settings_ESP3D::read_byte(ESP_WEBDAV_ON) !=1) {
        return true;
    }
    _port = Settings_ESP3D::read_uint32(ESP_WEBDAV_PORT);

    _tcpserver.begin(_port);
    _dav.begin(&_tcpserver);
    _started = true;
    return _started;
}
/**
 * End WebDav
 */
void Webdav_Server::end()
{
    _started = false;
    _port = 0;
    closeClient();
    _tcpserver.stop();
    _dav.end();
}

bool Webdav_Server::started()
{
    return _started;
}

void Webdav_Server::handle()
{
    _dav.handleClient();
}

#endif //WEBDAV_FEATURE
