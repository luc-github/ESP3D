
/*
  ftp_server.h -  ftp service functions class

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

#ifndef _FTP_SERVER_H
#define _FTP_SERVER_H

class Ftp_Server
{
public:
    Ftp_Server();
    ~Ftp_Server();
    bool begin();
    void end();
    void handle();
    bool reset();
    bool started();
    bool isConnected();
    const char* clientIPAddress();
    uint16_t ctrlport()
    {
        return _ctrlport;
    }
    uint16_t datapassiveport()
    {
        return _datapassiveport;
    }
    uint16_t dataactiveport()
    {
        return _dataactiveport;
    }
    void closeClient();
private:
    bool _started;
    uint16_t _ctrlport;
    uint16_t _dataactiveport;
    uint16_t _datapassiveport;
};

extern Ftp_Server ftp_server;

#endif

