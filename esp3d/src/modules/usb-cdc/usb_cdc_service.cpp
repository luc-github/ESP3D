/*
  usb_cdc_service.cpp -  native USB CDC (device) client functions class

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
#if defined(ARDUINO_ARCH_ESP32)
#include "../../include/esp3d_config.h"
#if defined(USB_CDC_CLIENT_FEATURE)
#include "../../core/esp3d_commands.h"
#include "../../core/esp3d_settings.h"
#include "../../core/esp3d_string.h"
#include "usb_cdc_port.h"
#include "usb_cdc_service.h"

#define TIMEOUT_USB_CDC_FLUSH 1500

ESP3DUsbCdcService esp3d_usb_cdc_service;

ESP3DUsbCdcService::ESP3DUsbCdcService() {
  _buffer_size = 0;
  _started = false;
  _lastflush = 0;
#if defined(AUTHENTICATION_FEATURE)
  _needauthentication = true;
#else
  _needauthentication = false;
#endif  // AUTHENTICATION_FEATURE
  initAuthentication();
}

ESP3DUsbCdcService::~ESP3DUsbCdcService() { end(); }

// extra parameters that do not need a begin
void ESP3DUsbCdcService::setParameters() {
#if defined(AUTHENTICATION_FEATURE)
  // USB CDC is a local serial link, it follows the serial security setting
  _needauthentication =
      (ESP3DSettings::readByte(ESP_SECURE_SERIAL) == 0) ? false : true;
#else
  _needauthentication = false;
#endif  // AUTHENTICATION_FEATURE
}

void ESP3DUsbCdcService::initAuthentication() {
#if defined(AUTHENTICATION_FEATURE)
  _auth = ESP3DAuthenticationLevel::guest;
#else
  _auth = ESP3DAuthenticationLevel::admin;
#endif  // AUTHENTICATION_FEATURE
}

ESP3DAuthenticationLevel ESP3DUsbCdcService::getAuthentication() {
  if (_needauthentication) {
    return _auth;
  }
  return ESP3DAuthenticationLevel::admin;
}

bool ESP3DUsbCdcService::begin() {
  end();
  if (ESP3DSettings::readByte(ESP_USB_CDC_ON) == 0) {
    esp3d_log("USB CDC is disabled");
    return true;
  }
  esp3d_usb_cdc_port_begin();
  setParameters();
  initAuthentication();
  _buffer_size = 0;
  _lastflush = millis();
  _started = true;
  esp3d_log("USB CDC is started");
  return true;
}

// the port itself is never stopped: it is also used by log and to reflash
bool ESP3DUsbCdcService::end() {
  _buffer_size = 0;
  _started = false;
  initAuthentication();
  return true;
}

bool ESP3DUsbCdcService::reset() {
  return ESP3DSettings::writeByte(
      ESP_USB_CDC_ON, ESP3DSettings::getDefaultByteSetting(ESP_USB_CDC_ON));
}

bool ESP3DUsbCdcService::isConnected() {
  return _started && ESP3D_USB_CDC_PORT;
}

void ESP3DUsbCdcService::handle() {
  static bool was_connected = false;
  if (!_started) {
    return;
  }
  bool connected = isConnected();
  if (connected != was_connected) {
    was_connected = connected;
    esp3d_log("USB CDC %s", connected ? "connected" : "disconnected");
    if (!connected) {
      // next host must authenticate again
      initAuthentication();
      _buffer_size = 0;
    }
  }
  int len = ESP3D_USB_CDC_PORT.available();
  if (len > 0) {
    uint8_t sbuf[64];
    while (len > 0) {
      size_t count = ESP3D_USB_CDC_PORT.read(
          sbuf, ((size_t)len > sizeof(sbuf)) ? sizeof(sbuf) : len);
      if (count == 0) {
        break;
      }
      push2buffer(sbuf, count);
      len -= count;
    }
  }
  // we cannot left data in buffer too long
  // in case some commands "forget" to add \n
  if (((millis() - _lastflush) > TIMEOUT_USB_CDC_FLUSH) && (_buffer_size > 0)) {
    flushBuffer();
  }
}

void ESP3DUsbCdcService::flushData(const uint8_t *data, size_t size,
                                   ESP3DMessageType type) {
  ESP3DMessage *message = esp3d_message_manager.newMsg(
      ESP3DClientType::usb_cdc, esp3d_commands.getOutputClient(), data, size,
      getAuthentication());

  if (message) {
    message->type = type;
    esp3d_log("Process Message");
    esp3d_commands.process(message);
  } else {
    esp3d_log_e("Cannot create message");
  }
  _lastflush = millis();
}

void ESP3DUsbCdcService::flushChar(char c) {
  flushData((uint8_t *)&c, 1, ESP3DMessageType::realtimecmd);
}

void ESP3DUsbCdcService::flushBuffer() {
  _buffer[_buffer_size] = 0x0;
  flushData((uint8_t *)_buffer, _buffer_size, ESP3DMessageType::unique);
  _buffer_size = 0;
}

// push collected data to buffer and proceed accordingly
void ESP3DUsbCdcService::push2buffer(uint8_t *sbuf, size_t len) {
  for (size_t i = 0; i < len; i++) {
    _lastflush = millis();
    if (esp3d_string::isRealTimeCommand(sbuf[i])) {
      flushChar(sbuf[i]);
    } else {
      _buffer[_buffer_size] = sbuf[i];
      _buffer_size++;
      if (_buffer_size >= ESP3D_USB_CDC_BUFFER_SIZE ||
          _buffer[_buffer_size - 1] == '\n') {
        flushBuffer();
      }
    }
  }
}

size_t ESP3DUsbCdcService::writeBytes(const uint8_t *buffer, size_t size) {
  if (!isConnected()) {
    return 0;
  }
  size_t sizetosend = size;
  size_t sizesent = 0;
  uint32_t starttime = millis();
  // loop until all is sent or timeout or host is gone
  while (sizetosend > 0 && ((millis() - starttime) < 100) && isConnected()) {
    int available = ESP3D_USB_CDC_PORT.availableForWrite();
    size_t sent = 0;
    if (available > 0) {
      sent = ESP3D_USB_CDC_PORT.write(
          &buffer[sizesent],
          ((size_t)available >= sizetosend) ? sizetosend : available);
    }
    if (sent > 0) {
      sizetosend -= sent;
      sizesent += sent;
      starttime = millis();
    } else {
      ESP3DHal::wait(5);
    }
  }
  return sizesent;
}

bool ESP3DUsbCdcService::dispatch(ESP3DMessage *message) {
  if (!message || !_started) {
    if (!message) {
      esp3d_log_e("No message");
    } else {
      esp3d_log_e("USB CDC not started");
    }
    return false;
  }
  if (message->size > 0 && message->data) {
    size_t sentcnt = writeBytes(message->data, message->size);
    if (sentcnt != message->size) {
      esp3d_log_e("USB CDC sent %d bytes, expected %d bytes", sentcnt,
                  message->size);
      return false;
    }
    esp3d_message_manager.deleteMsg(message);
    return true;
  }
  esp3d_log_e("Error null data");
  return false;
}

#endif  // USB_CDC_CLIENT_FEATURE
#endif  // ARDUINO_ARCH_ESP32
