/*
  esp3d_usb_serial_service.cpp -  serial services functions class

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
#if defined(USB_SERIAL_FEATURE)
#include "../../core/esp3d_string.h"
#include "../authentication/authentication_service.h"
#include "../../core/esp3d_settings.h"
#include "usb_serial_service.h"

ESP3DUsbSerialService esp3d_usb_serial_service;

#define SERIAL_COMMUNICATION_TIMEOUT 500


// Serial Parameters
#define ESP_USBSERIAL_PARAM SERIAL_8N1


#define TIMEOUT_USB_SERIAL_FLUSH 1500
// Constructor
ESP3DUsbSerialService::ESP3DUsbSerialService() {
  _buffer_size = 0;
  _mutex = NULL;
  _started = false;
#if defined(AUTHENTICATION_FEATURE)
  _needauthentication = true;
#else
  _needauthentication = false;
#endif  // AUTHENTICATION_FEATURE
  _origin = ESP3DClientType::usb_serial;
  _messagesInFIFO.setId("in");
  _messagesInFIFO.setMaxSize(0);  // no limit
  _baudRate = 0;
}

// Destructor
ESP3DUsbSerialService::~ESP3DUsbSerialService() { end(); }

// extra parameters that do not need a begin
void ESP3DUsbSerialService::setParameters() {
#if defined(AUTHENTICATION_FEATURE)
  _needauthentication =
      (ESP3DSettings::readByte(ESP_SECURE_SERIAL) == 0) ? false : true;
#else
  _needauthentication = false;
#endif  // AUTHENTICATION_FEATURE
}

void ESP3DUsbSerialService::initAuthentication() {
#if defined(AUTHENTICATION_FEATURE)
  _auth = ESP3DAuthenticationLevel::guest;
#else
  _auth = ESP3DAuthenticationLevel::admin;
#endif  // AUTHENTICATION_FEATURE
}
ESP3DAuthenticationLevel ESP3DUsbSerialService::getAuthentication() {
  if (_needauthentication) {
    return _auth;
  }
  return ESP3DAuthenticationLevel::admin;
}

void ESP3DUsbSerialService::receiveSerialCb() { esp3d_usb_serial_service.receiveCb(); }

#if defined(ESP_SERIAL_BRIDGE_OUTPUT)
void ESP3DUsbSerialService::receiveBridgeSerialCb() {
  serial_bridge_service.receiveCb();
}
#endif  // ESP_SERIAL_BRIDGE_OUTPUT

void ESP3DUsbSerialService::receiveCb() {
  if (!started()) {
    return;
  }
 /* if (xSemaphoreTake(_mutex, portMAX_DELAY)) {
    uint32_t now = millis();
    while ((millis() - now) < SERIAL_COMMUNICATION_TIMEOUT) {
      if (Serials[_serialIndex]->available()) {
        _buffer[_buffer_size] = Serials[_serialIndex]->read();
        _buffer_size++;
        now = millis();
        if (_buffer_size > ESP3D_SERIAL_BUFFER_SIZE ||
            _buffer[_buffer_size - 1] == '\n' ||
            _buffer[_buffer_size - 1] == '\r') {
          if (_buffer[_buffer_size - 1] == '\r') {
            _buffer[_buffer_size - 1] = '\n';
          }
          flushbuffer();
        }
      }
    }

    xSemaphoreGive(_mutex);
  } else {
    esp3d_log_e("Mutex not taken");
  }*/
}

// Setup Serial
bool ESP3DUsbSerialService::begin() {
  _mutex = xSemaphoreCreateMutex();
  if (_mutex == NULL) {
    esp3d_log_e("Mutex creation failed");
    return false;
  }
  _lastflush = millis();
  // read from settings
  uint32_t br = 0;
  uint32_t defaultBr = 0;

      br = ESP3DSettings::readUint32(ESP_USB_SERIAL_BAUD_RATE);
      defaultBr = ESP3DSettings::getDefaultIntegerSetting(ESP_USB_SERIAL_BAUD_RATE);

  setParameters();
  esp3d_log("Baud rate is %d , default is %d", br, defaultBr);
  _buffer_size = 0;
  // change only if different from current
  if (br != baudRate()) {
    if (!ESP3DSettings::isValidIntegerSetting(br, ESP_USB_SERIAL_BAUD_RATE)) {
      br = defaultBr;
    }

    /*Serials[_serialIndex]->setRxBufferSize(SERIAL_RX_BUFFER_SIZE);
    Serials[_serialIndex]->begin(br, ESP_SERIAL_PARAM, _rxPin, _txPin);
    _baudRate = br;
    if (_id == MAIN_SERIAL) {
      Serials[_serialIndex]->onReceive(receiveSerialCb);
    }
*/
  }
  _started = true;
  esp3d_log("Serial %d for %d is started", _serialIndex, _id);
  return true;
}

// End serial
bool ESP3DUsbSerialService::end() {
  flush();
  delay(100);
  if (_mutex != NULL) {
    vSemaphoreDelete(_mutex);
    _mutex = NULL;
  }
  Serials[_serialIndex]->end();
  _buffer_size = 0;
  _started = false;
  initAuthentication();
  return true;
}

// return the array of uint32_t and array size
const uint32_t *ESP3DUsbSerialService::get_baudratelist(uint8_t *count) {
  if (count) {
    *count = sizeof(SupportedBaudList) / sizeof(uint32_t);
  }
  return SupportedBaudList;
}

// Function which could be called in other loop
void ESP3DUsbSerialService::handle() {
  // Do we have some data waiting
  size_t len = _messagesInFIFO.size();
  if (len > 0) {
    if (len > 10) {
      len = 10;
    }
    while (len > 0) {
      esp3d_log_d("Serial in fifo size %d", _messagesInFIFO.size());
      ESP3DMessage *message = _messagesInFIFO.pop();
      if (message) {
        esp3d_commands.process(message);
      } else {
        esp3d_log_e("Cannot create message");
      }
      len--;
    }
  }
}

void ESP3DUsbSerialService::flushbuffer() {
  _buffer[_buffer_size] = 0x0;
  if (_buffer_size == 1 && _buffer[0] == '\n') {
    _buffer_size = 0;
    return;
  }

  // dispatch command
  ESP3DMessage *message = esp3d_message_manager.newMsg(
      _origin,
      _id == MAIN_SERIAL ? ESP3DClientType::all_clients
                         : esp3d_commands.getOutputClient(),
      (uint8_t *)_buffer, _buffer_size, getAuthentication());
  if (message) {
    // process command
    message->type = ESP3DMessageType::unique;
    esp3d_log_d("Message sent to fifo list");
    _messagesInFIFO.push(message);
  } else {
    esp3d_log_e("Cannot create message");
  }
  _lastflush = millis();
  _buffer_size = 0;
}

// push collected data to buffer and proceed accordingly
void ESP3DUsbSerialService::push2buffer(uint8_t *sbuf, size_t len) {
if (!_started) {
    return;
  }
  esp3d_log("buffer get %d data ", len);
 /* for (size_t i = 0; i < len; i++) {
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
  }*/
}



 void ESP3DUsbSerialService::updateBaudRate(uint32_t br) {
   if (br != _baudRate) {
 /*    Serials[_serialIndex]->flush();
     Serials[_serialIndex]->updateBaudRate(br);*/
     _baudRate = br;
   }
 }

 // Get current baud rate
 uint32_t ESP3DUsbSerialService::baudRate() {
   
   return _baudRate;
 }

 size_t ESP3DUsbSerialService::writeBytes(const uint8_t *buffer, size_t size) {
   if (!_started) {
     return 0;
   }/*
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
   }*/
   return 0;
 }

 size_t ESP3DUsbSerialService::readBytes(uint8_t *sbuf, size_t len) {
   if (!_started) {
     return -1;
   }
  // return Serials[_serialIndex]->readBytes(sbuf, len);
   return 0;
 }

 void ESP3DUsbSerialService::flush() {
   if (!_started) {
     return;
   }
  // Serials[_serialIndex]->flush();
 }

 void ESP3DUsbSerialService::swap() {
   // Nothing to do
 }

 bool ESP3DUsbSerialService::dispatch(ESP3DMessage *message) {
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
 

#endif  // USB_SERIAL_FEATURE