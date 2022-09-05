/*
  websocket_server.cpp -  websocket functions class

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

#if defined (HTTP_FEATURE) || defined(WS_DATA_FEATURE)


#include "websocket_server.h"
#include <WebSocketsServer.h>
#include "../../core/settings_esp3d.h"
#include "../../core/esp3doutput.h"
#include "../../core/commands.h"
#include "../authentication/authentication_service.h"
WebSocket_Server websocket_terminal_server("webui-v3");
#if defined(WS_DATA_FEATURE)
WebSocket_Server websocket_data_server("arduino");
#endif //WS_DATA_FEATURE
void WebSocket_Server::pushMSG (const char * data)
{
    if (_websocket_server) {
        _websocket_server->broadcastTXT(data);
        log_esp3d("[%u]Broadcast %s", _current_id,data);
    }
}

void WebSocket_Server::enableOnly (uint num)
{
    //some sanity check
    if (_websocket_server) {
        for (uint8_t i=0; i<WEBSOCKETS_SERVER_CLIENT_MAX; i++)
            if(i!=num && _websocket_server->clientIsConnected(i)) {
                _websocket_server->disconnect(i);
            }
    }
}


void WebSocket_Server::pushMSG (uint num, const char * data)
{
    if (_websocket_server) {
        _websocket_server->sendTXT(num, data);
        log_esp3d("[%u]Send %s", num,data);
    }
}

void WebSocket_Server::closeClients()
{
    if (_websocket_server) {
        _websocket_server->disconnect();
    }
}
#if defined(WS_DATA_FEATURE)
//Events for Websocket bridge
void handle_Websocket_Server_Event(uint8_t num, uint8_t type, uint8_t * payload, size_t length)
{
    (void)num;
    switch(type) {
    case WStype_DISCONNECTED:
        log_esp3d("[%u] Disconnected! port %d", num,websocket_data_server.port());
        break;
    case WStype_CONNECTED: {
        log_esp3d("[%u] Connected! port %d, %s", num,websocket_data_server.port(), payload);
    }
    break;
    case WStype_TEXT:
        log_esp3d("[%u] get Text: %s port %d", num, payload,websocket_data_server.port());
        websocket_data_server.push2RXbuffer(payload, length);
        break;
    case WStype_BIN:
        log_esp3d("[%u] get binary length: %u port %d", num, length,websocket_data_server.port());
        websocket_data_server.push2RXbuffer(payload, length);
        break;
    default:
        break;
    }

}
#endif //WS_DATA_FEATURE
#if defined (HTTP_FEATURE)
//Events for Websocket used in WebUI for events and serial bridge
void handle_Websocket_Terminal_Event(uint8_t num, uint8_t type, uint8_t * payload, size_t length)
{
    (void)payload;
    (void)length;
    String msg;
    switch(type) {
    case WStype_DISCONNECTED:
        log_esp3d("[%u] Socket Disconnected port %d!", num,websocket_terminal_server.port());
        break;
    case WStype_CONNECTED: {
        log_esp3d("[%u] Connected! port %d, %s", num,websocket_terminal_server.port(), (const char *)payload);
        msg = "currentID:" + String(num);
        // send message to client
        websocket_terminal_server.set_currentID(num);
        websocket_terminal_server.pushMSG(num, msg.c_str());
        msg = "activeID:" + String(num);
        websocket_terminal_server.pushMSG(msg.c_str());
        websocket_terminal_server.enableOnly(num);
        log_esp3d("[%u] Socket connected port %d", num,websocket_terminal_server.port());
    }
    break;
    case WStype_TEXT:
#if defined (AUTHENTICATION_FEATURE)
        //we do not expect any input but ping to get session timeout if any
        if (AuthenticationService::getSessionTimeout() != 0) {
            msg = (const char*)payload;
            if (msg.startsWith("PING:")) {
                String session = msg.substring(5);
                String response = "PING:"+String(AuthenticationService::getSessionRemaining(session.c_str()));
                response += ":"+String(AuthenticationService::getSessionTimeout());
                websocket_terminal_server.pushMSG(num, response.c_str());
            }
        }
#endif //AUTHENTICATION_FEATURE 
        //log_esp3d("[IGNORED][%u] get Text: %s  port %d", num, payload, websocket_terminal_server.port());
        break;
    case WStype_BIN:
        //we do not expect any input
        //log_esp3d("[IGNORED][%u] get binary length: %u  port %d", num, length, websocket_terminal_server.port());
        break;
    default:
        break;
    }

}
#endif //HTTP_FEATURE 

int WebSocket_Server::available()
{
    return _RXbufferSize;
}
int WebSocket_Server::availableForWrite()
{
    return TXBUFFERSIZE -_TXbufferSize;
}
WebSocket_Server::WebSocket_Server(const char * protocol )
{
    _websocket_server = nullptr;
    _started = false;
    _port = 0;
    _current_id = 0;
    _RXbuffer = nullptr;
    _RXbufferSize = 0;
    _protocol = protocol;

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
        if (Settings_ESP3D::read_byte(ESP_WEBSOCKET_ON) == 0) {
            return true;
        }
    }
    _websocket_server = new WebSocketsServer(_port,"",_protocol.c_str());
    if (_websocket_server) {
        _websocket_server->begin();
#if defined (HTTP_FEATURE) //terminal websocket for HTTP
        if(port == 0) {
            _websocket_server->onEvent(handle_Websocket_Terminal_Event);
        }
#endif //HTTP_FEATURE
#if defined (WS_DATA_FEATURE) //terminal websocket for HTTP
        if((port != 0) && _protocol!="debug") {
            _websocket_server->onEvent(handle_Websocket_Server_Event);
            _RXbuffer= (uint8_t *)malloc(RXBUFFERSIZE +1);
            if (!_RXbuffer) {
                return false;
            }
        }
#endif //WS_DATA_FEATURE
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
    if(_RXbuffer) {
        free(_RXbuffer);
        _RXbuffer = nullptr;
    }
    _RXbufferSize = 0;
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

size_t WebSocket_Server::write(uint8_t c)
{
    return write(&c,1);
}

size_t WebSocket_Server::write(const uint8_t *buffer, size_t size)
{
    if (_started) {
        if((buffer == nullptr) ||(!_websocket_server) || (size == 0)) {
            return 0;
        }
        if (_TXbufferSize==0) {
            _lastTXflush = millis();
        }
        //send full line
        if (_TXbufferSize + size > TXBUFFERSIZE) {
            flushTXbuffer();
        }
        if(_websocket_server->connectedClients() == 0) {
            return 0;
        }
        //need periodic check to force to flush in case of no end
        for (uint i = 0; i < size; i++) {
            _TXbuffer[_TXbufferSize] = buffer[i];
            _TXbufferSize++;
        }
        return size;
    }
    return 0;
}

void WebSocket_Server::push2RXbuffer(uint8_t * sbuf, size_t len)
{
    if (!_RXbuffer || !_started) {
        return;
    }
    for (size_t i = 0; i < len; i++) {
        _lastRXflush = millis();
        //command is defined
        if ((char(sbuf[i]) == '\n')|| (char(sbuf[i]) == '\r')) {
            if (_RXbufferSize < RXBUFFERSIZE) {
                _RXbuffer[_RXbufferSize] = sbuf[i];
                _RXbufferSize++;
            }
            flushRXbuffer();
        } else if (isPrintable (char(sbuf[i]) ) ) {
            if (_RXbufferSize < RXBUFFERSIZE) {
                _RXbuffer[_RXbufferSize] = sbuf[i];
                _RXbufferSize++;
            } else {
                flushRXbuffer();
                _RXbuffer[_RXbufferSize] = sbuf[i];
                _RXbufferSize++;
            }
        } else { //it is not printable char
            //clean buffer first
            if (_RXbufferSize > 0) {
                flushRXbuffer();
            }
            //process char
            _RXbuffer[_RXbufferSize] = sbuf[i];
            _RXbufferSize++;
            flushRXbuffer();
        }
    }
}

void WebSocket_Server::flushRXbuffer()
{
    if (!_RXbuffer || !_started) {
        _RXbufferSize = 0;
        return;
    }
    ESP3DOutput output(ESP_WEBSOCKET_CLIENT);
    _RXbuffer[_RXbufferSize] = 0x0;
    //dispatch command
    esp3d_commands.process(_RXbuffer, _RXbufferSize, &output);
    _lastRXflush = millis();
    _RXbufferSize = 0;
}


void WebSocket_Server::handle()
{
    Hal::wait(0);
    if (_started) {
        if (_TXbufferSize > 0) {
            if ((_TXbufferSize>=TXBUFFERSIZE) || ((millis()- _lastTXflush) > FLUSHTIMEOUT)) {
                flushTXbuffer();
            }
        }
        if (_RXbufferSize > 0) {
            if ((_RXbufferSize>=RXBUFFERSIZE) || ((millis()- _lastRXflush) > FLUSHTIMEOUT)) {
                flushRXbuffer();
            }
        }
        if (_websocket_server) {
            _websocket_server->loop();
        }
    }
}

void WebSocket_Server::flush(void)
{
    flushTXbuffer();
    flushRXbuffer();
}

void WebSocket_Server::flushTXbuffer(void)
{
    if (_started) {
        if ((_TXbufferSize > 0) && (_websocket_server->connectedClients() > 0 )) {

            if (_websocket_server) {
                _websocket_server->broadcastBIN(_TXbuffer,_TXbufferSize);
                log_esp3d("WS Broadcast bin port %d: %d bytes", port(), _TXbufferSize);
            }
            //refresh timout
            _lastTXflush = millis();

        }
    }
    //reset buffer
    _TXbufferSize = 0;
}



#endif // HTTP_FEATURE || WS_DATA_FEATURE

