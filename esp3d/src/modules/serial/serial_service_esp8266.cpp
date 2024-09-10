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
#if defined(ARDUINO_ARCH_ESP8266)
#include "../../include/esp3d_config.h"
#if COMMUNICATION_PROTOCOL == MKS_SERIAL || COMMUNICATION_PROTOCOL == RAW_SERIAL
#include "../../core/esp3d_commands.h"
#include "../../core/esp3d_settings.h"
#include "../../core/esp3d_string.h"
#include "serial_service.h"

#if COMMUNICATION_PROTOCOL == MKS_SERIAL
#include "../mks/mks_service.h"
#endif  // COMMUNICATION_PROTOCOL == MKS_SERIAL
#include "../authentication/authentication_service.h"
#define MAX_SERIAL 2
HardwareSerial *Serials[MAX_SERIAL] = {&Serial, &Serial1};

// Serial Parameters
#define ESP_SERIAL_PARAM SERIAL_8N1

ESP3DSerialService esp3d_serial_service = ESP3DSerialService(MAIN_SERIAL);

const uint32_t SupportedBaudList[] = {9600,    19200,   38400,  57600,  74880,
                                      115200,  230400,  250000, 500000, 921600,
                                      1000000, 1958400, 2000000};
const size_t SupportedBaudListSize = sizeof(SupportedBaudList) / sizeof(uint32_t);

#define TIMEOUT_SERIAL_FLUSH 1500
// Constructor
ESP3DSerialService::ESP3DSerialService(uint8_t id) {
  _buffer_size = 0;
  _started = false;
#if defined(AUTHENTICATION_FEATURE)
  _needauthentication = true;
#else
  _needauthentication = false;
#endif  // AUTHENTICATION_FEATURE
  _id = id;

  _rxPin = ESP_RX_PIN;
  _txPin = ESP_TX_PIN;
  _origin = ESP3DClientType::serial;
}

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

// Setup Serial
bool ESP3DSerialService::begin(uint8_t serialIndex) {
  _serialIndex = serialIndex - 1;
  esp3d_log("Serial %d begin for %d", _serialIndex, _id);
  if (_serialIndex >= MAX_SERIAL) {
    esp3d_log_e("Serial %d begin for %d failed, index out of range",
                _serialIndex, _id);
    return false;
  }
  _lastflush = millis();
  // read from settings
  uint32_t br = 0;
  uint32_t defaultBr = 0;

  br = ESP3DSettings::readUint32(ESP_BAUD_RATE);
  defaultBr = ESP3DSettings::getDefaultIntegerSetting(ESP_BAUD_RATE);
  setParameters();
  esp3d_log("Baud rate is %d , default is %d", br, defaultBr);
  _buffer_size = 0;
  // change only if different from current
  if (br != baudRate() || (_rxPin != -1) || (_txPin != -1)) {
    if (!ESP3DSettings::isValidIntegerSetting(br, ESP_BAUD_RATE)) {
      br = defaultBr;
    }
    Serials[_serialIndex]->setRxBufferSize(SERIAL_RX_BUFFER_SIZE);
    Serials[_serialIndex]->begin(br, ESP_SERIAL_PARAM, SERIAL_FULL,
                                 (_txPin == -1) ? 1 : _txPin);
    if (_rxPin != -1) {
      Serials[_serialIndex]->pins((_txPin == -1) ? 1 : _txPin, _rxPin);
    }
  }
  _started = true;
  esp3d_log("Serial %d for %d is started", _serialIndex, _id);
  return true;
}
// End serial
bool ESP3DSerialService::end() {
  flush();
  delay(100);
  swap();
  Serials[_serialIndex]->end();
  _buffer_size = 0;
  _started = false;
  initAuthentication();
  return true;
}

// return the array of uint32_t and array size
const uint32_t *ESP3DSerialService::get_baudratelist(uint8_t *count) {
  if (count) {
    *count = sizeof(SupportedBaudList) / sizeof(uint32_t);
  }
  return SupportedBaudList;
}

// Function which could be called in other loop
void ESP3DSerialService::handle() {
  if (!_started) {
    return;
  }
  // Do we have some data waiting
  size_t len = Serials[_serialIndex]->available();
  if (len > 0) {
    // if yes read them
    esp3d_log("Got %d chars in serial", len);
    uint8_t *sbuf = (uint8_t *)malloc(len);
    if (sbuf) {
      size_t count = readBytes(sbuf, len);
      // push to buffer
      if (count > 0) {
        push2buffer(sbuf, count);
      }
      // freen buffer
      free(sbuf);
    }
  }
  // we cannot left data in buffer too long
  // in case some commands "forget" to add \n
  if (((millis() - _lastflush) > TIMEOUT_SERIAL_FLUSH) && (_buffer_size > 0)) {
    flushbuffer();
  }
}


void ESP3DSerialService::flushbuffer() {
  _buffer[_buffer_size] = 0x0;

  // dispatch command
  if (_started) {
    ESP3DMessage *message = esp3d_message_manager.newMsg(
        _origin,
        _id == MAIN_SERIAL ? ESP3DClientType::all_clients
                           : esp3d_commands.getOutputClient(),
        (uint8_t *)_buffer, _buffer_size, getAuthentication());
    if (message) {
      // process command
      message->type = ESP3DMessageType::unique;
      esp3d_commands.process(message);
    } else {
      esp3d_log_e("Cannot create message");
    }
  }
  _lastflush = millis();
  _buffer_size = 0;
}

// push collected data to buffer and proceed accordingly
void ESP3DSerialService::push2buffer(uint8_t *sbuf, size_t len) {
  if (!_started) {
    return;
  }
  esp3d_log("buffer get %d data ", len);
#if COMMUNICATION_PROTOCOL == MKS_SERIAL
  static bool isFrameStarted = false;
  static bool isCommandFrame = false;
  static uint8_t type;
  // expected size
  static int16_t framePos = -1;
  // currently received
  static uint datalen = 0;
  for (size_t i = 0; i < len; i++) {
    esp3d_log("Data : %c %x", sbuf[i], sbuf[i]);
    framePos++;
    _lastflush = millis();
    // so frame head was detected
    if (isFrameStarted) {
      // checking it is a valid Frame header
      if (framePos == 1) {
        esp3d_log("type = %x", sbuf[i]);
        if (MKSService::isFrame(char(sbuf[i]))) {
          if (MKSService::isCommand(char(sbuf[i]))) {
            isCommandFrame = true;
            esp3d_log("type: Command");
          } else {
            esp3d_log("type: other");
            type = sbuf[i];
            isCommandFrame = false;
          }
        } else {
          esp3d_log_e("wrong frame type");
          isFrameStarted = false;
          _buffer_size = 0;
        }
      } else if ((framePos == 2) || (framePos == 3)) {
        // add size to int
        if (framePos == 2) {
          datalen = sbuf[i];
        } else {
          datalen += (sbuf[i] << 8);
          esp3d_log("Data len: %d", datalen);
          if (datalen > (ESP3D_SERIAL_BUFFER_SIZE - 5)) {
            esp3d_log_e("Overflow in data len");
            isFrameStarted = false;
            _buffer_size = 0;
          }
        }
      } else if (MKSService::isTail(char(sbuf[i]))) {
        esp3d_log("got tail");
        _buffer[_buffer_size] = '\0';
        esp3d_log("size is %d", _buffer_size);
        // let check integrity
        if (_buffer_size == datalen) {
          esp3d_log("Flushing buffer");
          if (isCommandFrame) {
            flushbuffer();
          } else {
            MKSService::handleFrame(type, (const uint8_t *)_buffer,
                                    _buffer_size);
          }
        } else {
          esp3d_log_e("Error in data len");
        }
        // clear frame infos
        _buffer_size = 0;
        isFrameStarted = false;

      } else {
        // it is data
        if (_buffer_size < ESP3D_SERIAL_BUFFER_SIZE - 5) {
          _buffer[_buffer_size] = sbuf[i];
          _buffer_size++;
        } else {
          esp3d_log_e("Overflow in data len");
          isFrameStarted = false;
          _buffer_size = 0;
        }
      }
    } else {
      // frame is not started let see if it is a head
      if (MKSService::isHead(char(sbuf[i]))) {
        esp3d_log("got head");
        // yes it is
        isFrameStarted = true;
        framePos = 0;
        _buffer_size = 0;
      } else {
        // no so let reset all and just ignore it
        // TODO should we handle these data ?
        esp3d_log_e("Unidentified data : %c %x", sbuf[i], sbuf[i]);
        isCommandFrame = false;
        framePos = -1;
        datalen = 0;
      }
    }
  }
#else
  for (size_t i = 0; i < len; i++) {
    _lastflush = millis();
    // command is defined
    if ((char(sbuf[i]) == '\n') || (char(sbuf[i]) == '\r')) {
      if (_buffer_size < ESP3D_SERIAL_BUFFER_SIZE) {
        _buffer[_buffer_size] = sbuf[i];
        _buffer_size++;
      }
      flushbuffer();
    } else if (esp3d_string::isPrintableChar(char(sbuf[i]))) {
      if (_buffer_size < ESP3D_SERIAL_BUFFER_SIZE) {
        _buffer[_buffer_size] = sbuf[i];
        _buffer_size++;
      } else {
        flushbuffer();
        _buffer[_buffer_size] = sbuf[i];
        _buffer_size++;
      }
    } else {  // it is not printable char
      // clean buffer first
      if (_buffer_size > 0) {
        flushbuffer();
      }
      // process char
      _buffer[_buffer_size] = sbuf[i];
      _buffer_size++;
      flushbuffer();
    }
  }
#endif
}

// Reset Serial Setting (baud rate)
bool ESP3DSerialService::reset() {
  esp3d_log("Reset serial");
  return ESP3DSettings::writeUint32(
      ESP_BAUD_RATE, ESP3DSettings::getDefaultIntegerSetting(ESP_BAUD_RATE));
}

void ESP3DSerialService::updateBaudRate(uint32_t br) {
  if (br != baudRate()) {
    Serials[_serialIndex]->flush();
    Serials[_serialIndex]->updateBaudRate(br);
  }
}

// Get current baud rate
uint32_t ESP3DSerialService::baudRate() {
  return Serials[_serialIndex]->baudRate();
}

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

void ESP3DSerialService::swap() {
#ifdef ARDUINO_ARCH_ESP8266
  Serials[_serialIndex]->swap();
#endif  // ARDUINO_ARCH_ESP8266
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

#endif  // COMMUNICATION_PROTOCOL == MKS_SERIAL || COMMUNICATION_PROTOCOL ==
        // RAW_SERIAL
#endif  // ARDUINO_ARCH_ESP8266