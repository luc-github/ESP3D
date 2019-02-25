/*
  websocket_server.cpp -  websocket functions class

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

#if defined (HTTP_FEATURE) || defined(WS_DATA_FEATURE)


#include "websocket_server.h"
#include <WebSocketsServer.h>
#include "../../core/settings_esp3d.h"

WebSocket_Server websocket_terminal_server;

void handle_Websocket_Terminal_Event(uint8_t num, uint8_t type, uint8_t * payload, size_t length)
{

    switch(type) {
    case WStype_DISCONNECTED:
        //Serial.printf("[%u] Disconnected!\n", num);
        break;
    case WStype_CONNECTED: {
        String s = "currentID:" + String(num);
        // send message to client
        websocket_terminal_server.set_currentID(num);
        websocket_terminal_server.Socket_Server()->sendTXT(num, s);
        s = "activeID:" + String(num);
        websocket_terminal_server.Socket_Server()->broadcastTXT(s);
    }
    break;
    case WStype_TEXT:
        //USE_SERIAL.printf("[%u] get Text: %s\n", num, payload);

        // send message to client
        // webSocket.sendTXT(num, "message here");

        // send data to all connected clients
        // webSocket.broadcastTXT("message here");
        break;
    case WStype_BIN:
        //USE_SERIAL.printf("[%u] get binary length: %u\n", num, length);
        //hexdump(payload, length);

        // send message to client
        // webSocket.sendBIN(num, payload, length);
        break;
    default:
        break;
    }

}

WebSocket_Server::WebSocket_Server()
{
    _websocket_server = nullptr;
    _started = false;
    _port = 0;
    _current_id = 0;

}
WebSocket_Server::~WebSocket_Server()
{
    end();
}
bool WebSocket_Server::begin(uint16_t port)
{
    end();
    if(port == 0) {
        _port = Settings_ESP3D::read_uint32(ESP_HTTP_PORT) +1;
    } else {
        _port  = port;
    }
    _websocket_server = new WebSocketsServer(_port);
    if (_websocket_server) {
        _websocket_server->begin();
#if defined (HTTP_FEATURE) //terminal websocket for HTTP
        if(port == 0) {
            _websocket_server->onEvent(handle_Websocket_Terminal_Event);
        }
#endif //HTTP_FEATURE
        _started = true;
    } else {
        end();
    }
    return _started;
}

void WebSocket_Server::end()
{
    _current_id = 0;
    _TXbufferSize = 0;
    //_RXbufferSize = 0;
    //_RXbufferpos = 0;
    if (_websocket_server) {
        _websocket_server->close();
        delete _websocket_server;
        _websocket_server = nullptr;
        _port = 0;
    }
    _started = false;
}


WebSocket_Server::operator bool() const
{
    return _started;
}

void WebSocket_Server::set_currentID(uint8_t current_id)
{
    _current_id = current_id;
}
uint8_t WebSocket_Server::get_currentID()
{
    return _current_id;
}
WebSocketsServer * WebSocket_Server::Socket_Server()
{
    return _websocket_server;
}


/*int WebSocket_Server::available(){
    return _RXbufferSize;
}*/


size_t WebSocket_Server::write(uint8_t c)
{
    return write(&c,1);;
}

size_t WebSocket_Server::write(const uint8_t *buffer, size_t size)
{
    if((buffer == NULL) ||(!_websocket_server) || (size == 0)) {
        log_esp3d("%s %d",_websocket_server?"[SOCKET]No socket":"[SOCKET]No buffer", size);
        return 0;
    }
    if (_TXbufferSize==0) {
        _lastflush = millis();
    }
    //send full line
    if (_TXbufferSize + size > TXBUFFERSIZE) {
        flush();
    }
    //need periodic check to force to flush in case of no end
    for (int i = 0; i < size; i++) {
        _TXbuffer[_TXbufferSize] = buffer[i];
        _TXbufferSize++;
    }
    log_esp3d("[SOCKET]buffer size %d",_TXbufferSize);
    return size;
}

/*int WebSocket_Server::peek(void){
    if (_RXbufferSize > 0)return _RXbuffer[_RXbufferpos];
    else return -1;
}*/

/*bool WebSocket_Server::push (const char * data){
    int data_size = strlen(data);
    if ((data_size + _RXbufferSize) <= RXBUFFERSIZE){
        int current = _RXbufferpos + _RXbufferSize;
        if (current > RXBUFFERSIZE) current = current - RXBUFFERSIZE;
        for (int i = 0; i < data_size; i++){
        if (current > (RXBUFFERSIZE-1)) current = 0;
        _RXbuffer[current] = data[i];
        current ++;
        }
        _RXbufferSize+=strlen(data);
        return true;
    }
    return false;
}*/

/*int WebSocket_Server::read(void){
    if (_RXbufferSize > 0) {
        int v = _RXbuffer[_RXbufferpos];
        _RXbufferpos++;
        if (_RXbufferpos > (RXBUFFERSIZE-1))_RXbufferpos = 0;
        _RXbufferSize--;
        return v;
    } else return -1;
}*/

void WebSocket_Server::handle()
{
    if (_TXbufferSize > 0) {
        if ((_TXbufferSize>=TXBUFFERSIZE) || ((millis()- _lastflush) > FLUSHTIMEOUT)) {
            log_esp3d("[SOCKET]need flush, buffer size %d",_TXbufferSize);
            flush();
        }
    }
    if (_websocket_server) {
        _websocket_server->loop();
    }
}


void WebSocket_Server::flush(void)
{
    if (_TXbufferSize > 0) {
        log_esp3d("[SOCKET]flush data, buffer size %d",_TXbufferSize);
        if (_websocket_server) {
            _websocket_server->broadcastBIN
            (_TXbuffer,_TXbufferSize);
        }
        //refresh timout
        _lastflush = millis();
        //reset buffer
        _TXbufferSize = 0;
    }
}



#endif // HTTP_FEATURE || WS_DATA_FEATURE

