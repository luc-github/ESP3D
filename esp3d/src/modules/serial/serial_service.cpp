/*
  esp3d_serial_service.cpp -  serial services functions class

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
#if COMMUNICATION_PROTOCOL == RAW_SERIAL || \
    defined(ESP_SERIAL_BRIDGE_OUTPUT) || COMMUNICATION_PROTOCOL == MKS_SERIAL
#include "../../core/esp3d_commands.h"
#include "../../core/esp3d_settings.h"
#include "../../core/esp3d_string.h"
#include "../authentication/authentication_service.h"
#include "serial_service.h"

extern HardwareSerial *Serials[];

// Serial Parameters

ESP3DSerialService esp3d_serial_service = ESP3DSerialService(MAIN_SERIAL);
#if defined(ESP_SERIAL_BRIDGE_OUTPUT)
ESP3DSerialService serial_bridge_service = ESP3DSerialService(BRIDGE_SERIAL);
#endif  // ESP_SERIAL_BRIDGE_OUTPUT

const uint32_t SupportedBaudList[] = {9600,    19200,   38400,  57600,  74880,
                                      115200,  230400,  250000, 500000, 921600,
                                      1000000, 1958400, 2000000};
const size_t SupportedBaudListSize =
    sizeof(SupportedBaudList) / sizeof(uint32_t);



// Destructor
ESP3DSerialService::~ESP3DSerialService() { end(); }

// extra parameters that do not need a begin
void ESP3DSerialService::setParameters() {
#if defined(AUTHENTICATION_FEATURE)
  _needauthentication =
      (ESP3DSettings::readByte(ESP_SECURE_SERIAL) == 0) ? false : true;
#else
  _needauthentication = false;
#endif  // AUTHENTICATION_FEATURE
}

void ESP3DSerialService::initAuthentication() {
#if defined(AUTHENTICATION_FEATURE)
  _auth = ESP3DAuthenticationLevel::guest;
#else
  _auth = ESP3DAuthenticationLevel::admin;
#endif  // AUTHENTICATION_FEATURE
}
ESP3DAuthenticationLevel ESP3DSerialService::getAuthentication() {
  if (_needauthentication) {
    return _auth;
  }
  return ESP3DAuthenticationLevel::admin;
}

// return the array of uint32_t and array size
const uint32_t *ESP3DSerialService::get_baudratelist(uint8_t *count) {
  if (count) {
    *count = sizeof(SupportedBaudList) / sizeof(uint32_t);
  }
  return SupportedBaudList;
}




void ESP3DSerialService::flushChar(char c) { flushData((uint8_t *)&c, 1, ESP3DMessageType::realtimecmd); }

void ESP3DSerialService::flushBuffer() {
  _buffer[_buffer_size] = 0x0;
  flushData((uint8_t *)_buffer, _buffer_size, ESP3DMessageType::unique);
  _buffer_size = 0;
}


void ESP3DSerialService::updateBaudRate(uint32_t br) {
  if (br != _baudRate) {
    Serials[_serialIndex]->flush();
    Serials[_serialIndex]->updateBaudRate(br);
    _baudRate = br;
  }
}

// Get current baud rate
uint32_t ESP3DSerialService::baudRate() { return _baudRate; }

size_t ESP3DSerialService::writeBytes(const uint8_t *buffer, size_t size) {
  if (!_started) {
    return 0;
  }
  if ((uint)Serials[_serialIndex]->availableForWrite() >= size) {
    return Serials[_serialIndex]->write(buffer, size);
  } else {
    size_t sizetosend = size;
    size_t sizesent = 0;
    uint8_t *buffertmp = (uint8_t *)buffer;
    uint32_t starttime = millis();
    // loop until all is sent or timeout
    while (sizetosend > 0 && ((millis() - starttime) < 100)) {
      size_t available = Serials[_serialIndex]->availableForWrite();
      if (available > 0) {
        // in case less is sent
        available = Serials[_serialIndex]->write(
            &buffertmp[sizesent],
            (available >= sizetosend) ? sizetosend : available);
        sizetosend -= available;
        sizesent += available;
        starttime = millis();
      } else {
        ESP3DHal::wait(5);
      }
    }
    return sizesent;
  }
}

size_t ESP3DSerialService::readBytes(uint8_t *sbuf, size_t len) {
  if (!_started) {
    return -1;
  }
  return Serials[_serialIndex]->readBytes(sbuf, len);
}

void ESP3DSerialService::flush() {
  if (!_started) {
    return;
  }
  Serials[_serialIndex]->flush();
}


bool ESP3DSerialService::dispatch(ESP3DMessage *message) {
  bool done = false;
  // Only is serial service is started
  if (_started) {
    // Only if message is not null
    if (message) {
      // if message is not null
      if (message->data && message->size != 0) {
        if (writeBytes(message->data, message->size) == message->size) {
          flush();
          // Delete message now
          esp3d_message_manager.deleteMsg(message);
          done = true;
        } else {
          esp3d_log_e("Error while sending data");
        }
      } else {
        esp3d_log_e("Error null data");
      }
    } else {
      esp3d_log_e("Error null message");
    }
  }
  return done;
}

#endif  // COMMUNICATION_PROTOCOL == RAW_SERIAL ||
        // defined(ESP_SERIAL_BRIDGE_OUTPUT)