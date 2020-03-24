/*
  telnet_server.h -  telnet service functions class

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

#ifndef _TELNET_SERVER_H
#define _TELNET_SERVER_H

class WiFiServer;
class WiFiClient;
#include "Print.h"

#define ESP3D_TELNET_BUFFER_SIZE 1200

class Telnet_Server : public Print
{
public:
    Telnet_Server();
    ~Telnet_Server();
    bool begin(uint16_t port = 0, bool debug=false);
    void end();
    void handle();
    bool reset();
    bool started();
    bool isConnected();
    const char* clientIPAddress();
    size_t write(uint8_t c);
    size_t write(const uint8_t *buffer, size_t size);
    inline size_t write(const char * s)
    {
        return write((uint8_t*) s, strlen(s));
    }
    inline size_t write(unsigned long n)
    {
        return write((uint8_t) n);
    }
    inline size_t write(long n)
    {
        return write((uint8_t) n);
    }
    inline size_t write(unsigned int n)
    {
        return write((uint8_t) n);
    }
    inline size_t write(int n)
    {
        return write((uint8_t) n);
    }
    int available();
    uint availableForWrite();
    void flush();
    int read(void);
    size_t readBytes (uint8_t * sbuf, size_t len);
    uint16_t port()
    {
        return _port;
    }
    void closeClient();
private:
    bool _started;
    WiFiServer * _telnetserver;
    WiFiClient  _telnetClients;
    uint16_t _port;
    bool _isdebug;
    uint32_t _lastflush;
    uint8_t *_buffer;
    size_t _buffer_size;
    void push2buffer(uint8_t * sbuf, size_t len);
    void flushbuffer();
};

extern Telnet_Server telnet_server;

#endif

