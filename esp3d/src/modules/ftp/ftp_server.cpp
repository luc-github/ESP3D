/*
  ftp_server.cpp -  ftp server functions class

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

#include "../../include/esp3d_config.h"

#if defined (FTP_FEATURE)
#include "ftp_server.h"
#include "../../core/settings_esp3d.h"
#include "../../core/esp3doutput.h"

Ftp_Server ftp_server;


void Ftp_Server::closeClient()
{
    /*if(_telnetClients) {
        _telnetClients.stop();
    }*/
}

bool Ftp_Server::isConnected()
{
    return false;
    /*
    if ( !_started || _telnetserver == NULL) {
        return false;
    }
    //check if there are any new clients
    if (_telnetserver->hasClient()) {
        //find free/disconnected spot
        if (!_telnetClients || !_telnetClients.connected()) {
            if(_telnetClients) {
                _telnetClients.stop();
            }
            _telnetClients = _telnetserver->available();
            //new client
        }
    }
    if (_telnetserver->hasClient()) {
        //no free/disconnected spot so reject
        _telnetserver->available().stop();
    }
    return _telnetClients.connected();*/
}

const char* Ftp_Server::clientIPAddress()
{
    static String res;
    /*res = "0.0.0.0";
    if (_telnetClients && _telnetClients.connected()) {
        res = _telnetClients.remoteIP().toString();
    }*/
    return res.c_str();
}


Ftp_Server::Ftp_Server()
{
    _started = false;
    _ctrlport = 0;
    _dataactiveport = 0;
    _datapassiveport = 0;
}
Ftp_Server::~Ftp_Server()
{
    end();
}

/**
 * begin Telnet setup
 */
bool Ftp_Server::begin()
{
    end();
    if (Settings_ESP3D::read_byte(ESP_TELNET_ON) !=1) {
        return true;
    }
    _ctrlport = Settings_ESP3D::read_uint32(ESP_FTP_CTRL_PORT);
    _dataactiveport = Settings_ESP3D::read_uint32(ESP_FTP_DATA_ACTIVE_PORT);
    _datapassiveport = Settings_ESP3D::read_uint32(ESP_FTP_DATA_PASSIVE_PORT);
    _started = true;
    //create instance
    /*  _telnetserver= new WiFiServer(_port);
      if (!_telnetserver) {
          return false;
      }
      _telnetserver->setNoDelay(true);
      //start telnet server
      _telnetserver->begin();
      _started = true;
      _lastflush = millis();*/
    return _started;
}
/**
 * End Telnet
 */
void Ftp_Server::end()
{
    _started = false;
    _ctrlport = 0;
    _dataactiveport = 0;
    _datapassiveport = 0;
}

/**
 * Reset Telnet
 */
bool Ftp_Server::reset()
{
    //nothing to reset
    return true;
}

bool Ftp_Server::started()
{
    return _started;
}

void Ftp_Server::handle()
{
    Hal::wait(0);
    //TODO
}

#endif //FTP_FEATURE
