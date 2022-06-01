/*
  serial2socket.cpp -  serial 2 socket functions class

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

//#define ESP_DEBUG_FEATURE DEBUG_OUTPUT_SERIAL0
#include "../../include/esp3d_config.h"

#if defined(ESP3DLIB_ENV) && COMMUNICATION_PROTOCOL == SOCKET_SERIAL
#include <Arduino.h>
#include "serial2socket.h"
#include "../../core/esp3doutput.h"
#include "../../core/commands.h"

Serial_2_Socket Serial2Socket;


Serial_2_Socket::Serial_2_Socket()
{
    end();
}
Serial_2_Socket::~Serial_2_Socket()
{
    end();
}
void Serial_2_Socket::begin(long speed)
{
    end();
}

void Serial_2_Socket::enable(bool enable)
{
    _started = enable;
}

void Serial_2_Socket::pause(bool state)
{
    _paused = state;
    if (_paused) {
        _TXbufferSize = 0;
        _RXbufferSize = 0;
        _RXbufferpos = 0;
    } else {
        _lastflush = millis();
    }
}

bool Serial_2_Socket::isPaused()
{
    return _paused;
}

void Serial_2_Socket::end()
{
    _TXbufferSize = 0;
    _RXbufferSize = 0;
    _RXbufferpos = 0;
    _started = false;
    _paused = false;
    _lastflush = millis();
}

long Serial_2_Socket::baudRate()
{
    return 0;
}

bool Serial_2_Socket::started()
{
    return _started;
}

Serial_2_Socket::operator bool() const
{
    return true;
}

int Serial_2_Socket::available()
{
    if (_paused) {
        return 0;
    }
    return _RXbufferSize;
}


size_t Serial_2_Socket::write(uint8_t c)
{
    if (!_started || _paused) {
        return 1;
    }
    return write(&c,1);
}

size_t Serial_2_Socket::write(const uint8_t *buffer, size_t size)
{
    if(buffer == NULL || size == 0 || !_started || _paused) {
        log_esp3d("Serial2Socket: no data, not started or paused");
        return size;
    }
    if (_TXbufferSize==0) {
        _lastflush = millis();
    }
    //send full line
    if (_TXbufferSize + size > S2S_TXBUFFERSIZE) {
        flush();
    }
    //need periodic check to force to flush in case of no end
    for (int i = 0; i < size; i++) {
        _TXbuffer[_TXbufferSize] = buffer[i];
        _TXbufferSize++;
        if (buffer[i] == (const uint8_t)'\n'|| buffer[i] == (const uint8_t)'\r') {
            log_esp3d("S2S: %s TXSize: %d", (const char *)_TXbuffer, _TXbufferSize);
            flush();
        }
    }
    handle_flush();
    return size;
}

int Serial_2_Socket::peek(void)
{
    if (_RXbufferSize > 0 && _started) {
        return _RXbuffer[_RXbufferpos];
    } else {
        return -1;
    }
}

bool Serial_2_Socket::push (const uint8_t *buffer, size_t size)
{
    if (buffer == NULL || size == 0 || !_started || _paused) {
        return false;
    }
    int data_size = size;
    if ((data_size + _RXbufferSize) <= S2S_RXBUFFERSIZE) {
        int current = _RXbufferpos + _RXbufferSize;
        if (current > S2S_RXBUFFERSIZE) {
            current = current - S2S_RXBUFFERSIZE;
        }
        for (int i = 0; i < data_size; i++) {
            if (current > (S2S_RXBUFFERSIZE-1)) {
                current = 0;
            }
            _RXbuffer[current] = buffer[i];
            current ++;
        }
        _RXbufferSize+=size;
        _RXbuffer[current] = 0;
        return true;
    }
    return false;
}

int Serial_2_Socket::read(void)
{
    if (_RXbufferSize > 0 && _started && !_paused) {
        int v = _RXbuffer[_RXbufferpos];
        _RXbufferpos++;
        if (_RXbufferpos > (S2S_RXBUFFERSIZE-1)) {
            _RXbufferpos = 0;
        }
        _RXbufferSize--;
        return v;
    } else {
        return -1;
    }

}

void Serial_2_Socket::handle()
{
    handle_flush();
}

void Serial_2_Socket::handle_flush()
{
    if (_TXbufferSize > 0 && _started   && !_paused) {
        if ((_TXbufferSize>=S2S_TXBUFFERSIZE) || ((millis()- _lastflush) > S2S_FLUSHTIMEOUT)) {
            log_esp3d("force socket flush");
            flush();
        }
    }
}
void Serial_2_Socket::flush(void)
{
    if (_TXbufferSize > 0 && _started && !_paused) {
        ESP3DOutput output(ESP_SOCKET_SERIAL_CLIENT);
        //dispatch command
        esp3d_commands.process(_TXbuffer,_TXbufferSize, &output);
        //refresh timout
        _lastflush = millis();
        //reset buffer
        _TXbufferSize = 0;
    }
}

#endif // ESP3DLIB_ENV
