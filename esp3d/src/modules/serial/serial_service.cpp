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
#if COMMUNICATION_PROTOCOL == MKS_SERIAL || \
    COMMUNICATION_PROTOCOL == RAW_SERIAL || defined(ESP_SERIAL_BRIDGE_OUTPUT)
#include "../../core/esp3d_commands.h"
#include "../../core/esp3d_settings.h"
#include "serial_service.h"

#if COMMUNICATION_PROTOCOL == MKS_SERIAL
#include "../mks/mks_service.h"
#endif  // COMMUNICATION_PROTOCOL == MKS_SERIAL
#include "../authentication/authentication_service.h"
#if defined(ARDUINO_ARCH_ESP8266)
#define MAX_SERIAL 2
HardwareSerial *Serials[MAX_SERIAL] = {&Serial, &Serial1};
#endif  // ARDUINO_ARCH_ESP8266

#if defined(ARDUINO_ARCH_ESP32)

#if defined(CONFIG_IDF_TARGET_ESP32C3)
#define MAX_SERIAL 2
HardwareSerial *Serials[MAX_SERIAL] = {&Serial, &Serial1};
#else
#define MAX_SERIAL 3
HardwareSerial *Serials[MAX_SERIAL] = {&Serial, &Serial1, &Serial2};
#endif

#endif  // ARDUINO_ARCH_ESP32

// Serial Parameters
#define ESP_SERIAL_PARAM SERIAL_8N1

#define ESP3DSERIAL_RUNNING_PRIORITY 1
#define ESP3DSERIAL_RUNNING_CORE 1
#define SERIAL_YIELD 10

ESP3DSerialService esp3d_serial_service = ESP3DSerialService(MAIN_SERIAL);
#if defined(ESP_SERIAL_BRIDGE_OUTPUT)
ESP3DSerialService serial_bridge_service = ESP3DSerialService(BRIDGE_SERIAL);
#endif  // ESP_SERIAL_BRIDGE_OUTPUT

#if defined(ARDUINO_ARCH_ESP32) && defined(SERIAL_INDEPENDANT_TASK)
TaskHandle_t _hserialtask = nullptr;
#endif  // ARDUINO_ARCH_ESP32

const uint32_t SupportedBaudList[] = {9600,   19200,  38400,  57600,
                                      74880,  115200, 230400, 250000,
                                      500000, 921600, 1958400};
const size_t SupportedBaudListSize = sizeof(SupportedBaudList) / sizeof(long);

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
  switch (_id) {
    case MAIN_SERIAL:
      _rxPin = ESP_RX_PIN;
      _txPin = ESP_TX_PIN;
      _origin = ESP3DClientType::serial;
      break;
#if defined(ESP_SERIAL_BRIDGE_OUTPUT)
    case BRIDGE_SERIAL:
      _rxPin = ESP_BRIDGE_RX_PIN;
      _txPin = ESP_BRIDGE_TX_PIN;
      _origin = ESP3DClientType::serial_bridge;
      break;
#endif  // ESP_SERIAL_BRIDGE_OUTPUT
    default:
      _rxPin = ESP_RX_PIN;
      _txPin = ESP_TX_PIN;
      _origin = ESP3DClientType::serial;
      break;
  }
}

// Destructor
ESP3DSerialService::~ESP3DSerialService() { end(); }

// dedicated serial task
#if defined(ARDUINO_ARCH_ESP32) && defined(SERIAL_INDEPENDANT_TASK)
void ESP3DSerialTaskfn(void *parameter) {
  for (;;) {
    esp3d_serial_service.process();
    ESP3DHal::wait(SERIAL_YIELD);  // Yield to other tasks
  }
  vTaskDelete(NULL);
}
#endif  // ARDUINO_ARCH_ESP32

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
  if (_id == BRIDGE_SERIAL &&
      ESP3DSettings::readByte(ESP_SERIAL_BRIDGE_ON) == 0) {
    esp3d_log("Serial %d for %d is disabled", _serialIndex, _id);
    return true;
  }
  if (_serialIndex >= MAX_SERIAL) {
    esp3d_log_e("Serial %d begin for %d failed, index out of range",
                _serialIndex, _id);
    return false;
  }
  _lastflush = millis();
  // read from settings
  long br = 0;
  long defaultBr = 0;
  switch (_id) {
    case MAIN_SERIAL:
      br = ESP3DSettings::readUint32(ESP_BAUD_RATE);
      defaultBr = ESP3DSettings::getDefaultIntegerSetting(ESP_BAUD_RATE);
      break;
#if defined(ESP_SERIAL_BRIDGE_OUTPUT)
    case BRIDGE_SERIAL:
      br = ESP3DSettings::readUint32(ESP_SERIAL_BRIDGE_BAUD);
      defaultBr =
          ESP3DSettings::getDefaultIntegerSetting(ESP_SERIAL_BRIDGE_BAUD);
      break;
#endif  // ESP_SERIAL_BRIDGE_OUTPUT
    default:
      esp3d_log_e("Serial %d begin for %d failed, unknown id", _serialIndex,
                  _id);
      return false;
  }
  setParameters();
  esp3d_log("Baud rate is %d , default is %d", br, defaultBr);
  _buffer_size = 0;
  // change only if different from current
  if (br != baudRate() || (_rxPin != -1) || (_txPin != -1)) {
    if (!ESP3DSettings::isValidIntegerSetting(br, ESP_BAUD_RATE)) {
      br = defaultBr;
    }
    Serials[_serialIndex]->setRxBufferSize(SERIAL_RX_BUFFER_SIZE);
#ifdef ARDUINO_ARCH_ESP8266
    Serials[_serialIndex]->begin(br, ESP_SERIAL_PARAM, SERIAL_FULL,
                                 (_txPin == -1) ? 1 : _txPin);
    if (_rxPin != -1) {
      Serials[_serialIndex]->pins((_txPin == -1) ? 1 : _txPin, _rxPin);
    }

#endif  // ARDUINO_ARCH_ESP8266
#if defined(ARDUINO_ARCH_ESP32)
    Serials[_serialIndex]->begin(br, ESP_SERIAL_PARAM, ESP_RX_PIN, ESP_TX_PIN);
#if defined(SERIAL_INDEPENDANT_TASK)
    // create serial task once
    esp3d_log("Serial %d for %d Task creation", _serialIndex, _id);
    if (_hserialtask == nullptr && _id == MAIN_SERIAL) {
      xTaskCreatePinnedToCore(
          ESP3DSerialTaskfn,            /* Task function. */
          "ESP3D Serial Task",          /* name of task. */
          8192,                         /* Stack size of task */
          NULL,                         /* parameter of the task */
          ESP3DSERIAL_RUNNING_PRIORITY, /* priority of the task */
          &_hserialtask, /* Task handle to keep track of created task */
          ESP3DSERIAL_RUNNING_CORE /* Core to run the task */
      );
    }
    if (_hserialtask == nullptr) {
      esp3d_log_e("Serial %d for %d Task creation failed", _serialIndex, _id);
      return false;
    }
#endif  // SERIAL_INDEPENDANT_TASK
#endif  // ARDUINO_ARCH_ESP32
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
    *count = sizeof(SupportedBaudList) / sizeof(long);
  }
  return SupportedBaudList;
}

// Function which could be called in other loop
void ESP3DSerialService::process() {
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

// Function which could be called in other loop
void ESP3DSerialService::handle() {
  // the serial bridge do not use independant task
  // not sure if it is sill necessary to do it for the main serial
  // TBC..
#if defined(ARDUINO_ARCH_ESP32) && defined(SERIAL_INDEPENDANT_TASK)
  if (_id == MAIN_SERIAL) {
    return;
  }
#endif  // ARDUINO_ARCH_ESP32 && SERIAL_INDEPENDANT_TASK0
  process();
}

void ESP3DSerialService::flushbuffer() {
  _buffer[_buffer_size] = 0x0;

  // dispatch command
  if (_started) {
    ESP3DMessage *message = ESP3DMessageManager::newMsg(
        _origin, ESP3DClientType::all_clients, (uint8_t *)_buffer, _buffer_size,
        getAuthentication());
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
    } else if (isPrintable(char(sbuf[i]))) {
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
  bool res = false;
  switch (_id) {
    case MAIN_SERIAL:
      return ESP3DSettings::writeUint32(
          ESP_BAUD_RATE,
          ESP3DSettings::getDefaultIntegerSetting(ESP_BAUD_RATE));
#if defined(ESP_SERIAL_BRIDGE_OUTPUT)
    case BRIDGE_SERIAL:
      res = ESP3DSettings::writeByte(
          ESP_SERIAL_BRIDGE_ON,
          ESP3DSettings::getDefaultByteSetting(ESP_SERIAL_BRIDGE_ON));
      return res &&
             ESP3DSettings::writeUint32(ESP_SERIAL_BRIDGE_BAUD,
                                        ESP3DSettings::getDefaultIntegerSetting(
                                            ESP_SERIAL_BRIDGE_BAUD));
#endif  // ESP_SERIAL_BRIDGE_OUTPUT
    default:
      return res;
  }
}

void ESP3DSerialService::updateBaudRate(long br) {
  if (br != baudRate()) {
    Serials[_serialIndex]->flush();
    Serials[_serialIndex]->updateBaudRate(br);
  }
}

// Get current baud rate
long ESP3DSerialService::baudRate() {
  long br = 0;
  br = Serials[_serialIndex]->baudRate();
#ifdef ARDUINO_ARCH_ESP32
  // workaround for ESP32
  if (br == 115201) {
    br = 115200;
  }
  if (br == 230423) {
    br = 230400;
  }
#endif  // ARDUINO_ARCH_ESP32
  return br;
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
          ESP3DMessageManager::deleteMsg(message);
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
