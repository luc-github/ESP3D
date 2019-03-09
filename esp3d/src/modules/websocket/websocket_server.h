/*
  websocket_server.h -  websocket functions class

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


#ifndef _WEBSOCKET_SERVER_H_
#define _WEBSOCKET_SERVER_H_

#include "Print.h"
#define TXBUFFERSIZE 1200
//#define RXBUFFERSIZE 128
#define FLUSHTIMEOUT 500
class WebSocketsServer;
class WebSocket_Server: public Print
{
public:
    WebSocket_Server();
    ~WebSocket_Server();
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
    bool begin(uint16_t port=0);
    uint16_t port()
    {
        return _port;
    }
    void end();
    //int available();
    //int peek(void);
    //int read(void);
    void pushMSG (const char * data);
    void pushMSG (uint num, const char * data);
    void flush(void);
    void handle();
    operator bool() const;
    void set_currentID(uint8_t current_id);
    uint8_t get_currentID();
private:
    bool _started;
    uint16_t _port;
    uint32_t _lastflush;
    WebSocketsServer * _websocket_server;
    uint8_t _TXbuffer[TXBUFFERSIZE];
    uint16_t _TXbufferSize;
    uint8_t _current_id;
    //uint8_t _RXbuffer[RXBUFFERSIZE];
    //uint16_t _RXbufferSize;
    //uint16_t _RXbufferpos;
};

extern WebSocket_Server websocket_terminal_server;
#endif //_WEBSOCKET_SERVER_H_
