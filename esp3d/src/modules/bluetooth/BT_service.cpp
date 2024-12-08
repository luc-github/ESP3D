/*
  BT_service.cpp -  Bluetooth service functions class

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

#ifdef ARDUINO_ARCH_ESP32

#include "../../include/esp3d_config.h"

#ifdef BLUETOOTH_FEATURE
#include "../../core/esp3d_commands.h"
#include "../../core/esp3d_settings.h"
#include "../../core/esp3d_string.h"
#include "../network/netconfig.h"
#include "BT_service.h"
#include "BluetoothSerial.h"

BluetoothSerial SerialBT;
#ifdef __cplusplus
extern "C" {
#endif
const uint8_t *esp_bt_dev_get_address(void);
#ifdef __cplusplus
}
#endif

#define TIMEOUT_BT_FLUSH 1500

BTService bt_service;

String BTService::_btname = "";
String BTService::_btclient = "";

BTService::BTService() {
  _buffer_size = 0;
  _started = false;
}

BTService::~BTService() { end(); }

bool BTService::isConnected() {
  return ((_btclient.length() > 0) ? true : false);
}

void BTService::setClientAddress(const char *saddress) { _btclient = saddress; }

static void my_spp_cb(esp_spp_cb_event_t event, esp_spp_cb_param_t *param) {
  switch (event) {
    case ESP_SPP_SRV_OPEN_EVT: {
      // Server connection open
      char str[18];
      str[17] = '\0';
      uint8_t *addr = param->srv_open.rem_bda;
      sprintf(str, "%02X:%02X:%02X:%02X:%02X:%02X", addr[0], addr[1], addr[2],
              addr[3], addr[4], addr[5]);
      BTService::setClientAddress(str);
      String stmp = "BT Connected with ";
      stmp += str;
      esp3d_commands.dispatch(stmp.c_str(), ESP3DClientType::all_clients, no_id,
                              ESP3DMessageType::unique, ESP3DClientType::system,
                              ESP3DAuthenticationLevel::admin);
    } break;

    case ESP_SPP_CLOSE_EVT: {
      // Client connection closed
      esp3d_commands.dispatch("BT Disconnected", ESP3DClientType::all_clients,
                              no_id, ESP3DMessageType::unique,
                              ESP3DClientType::system,
                              ESP3DAuthenticationLevel::admin);
      BTService::setClientAddress("");
    } break;
    default:
      break;
  }
}

const char *BTService::macAddress() {
  const uint8_t *point = esp_bt_dev_get_address();
  static char str[18];
  str[17] = '\0';
  sprintf(str, "%02X:%02X:%02X:%02X:%02X:%02X", (int)point[0], (int)point[1],
          (int)point[2], (int)point[3], (int)point[4], (int)point[5]);
  return str;
}

const char *BTService::clientmacAddress() { return _btclient.c_str(); }

/**
 * begin BT setup
 */
bool BTService::begin() {
  bool res = true;
  _buffer_size = 0;
  _lastflush = millis();
  // stop BT Serial if active
  end();
  // Get hostname
  // this allow to adjust if necessary
  _btname = ESP3DSettings::readString(ESP_HOSTNAME);
  String stmp;
  if (!SerialBT.begin(_btname)) {
    stmp = "BT failed start";
    res = false;
  } else {
    SerialBT.register_callback(&my_spp_cb);
    stmp = "Bluetooth Started with: '" + _btname + "'";
  }
  esp3d_commands.dispatch(stmp.c_str(), ESP3DClientType::all_clients, no_id,
                          ESP3DMessageType::unique, ESP3DClientType::bluetooth,
                          ESP3DAuthenticationLevel::admin);
  initAuthentication();

  return res;
}

/**
 * End BT
 */
void BTService::end() {
  flush();
  SerialBT.end();
  _buffer_size = 0;
  _started = false;
}

/**
 * Reset BT
 */
bool BTService::reset() {
  // nothing to reset
  return true;
}

/**
 * Check if BT is on and working
 */
bool BTService::started() {
  _started = btStarted();
  return _started;
}

/**
 * Handle not critical actions that must be done in sync environement
 */
void BTService::handle() {
  // Do we have some data waiting
  size_t len = SerialBT.available();
  if (len > 0) {
    // if yes read them
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
  if (((millis() - _lastflush) > TIMEOUT_BT_FLUSH) && (_buffer_size > 0)) {
    flushBuffer();
  }
}

void BTService::initAuthentication() {
#if defined(AUTHENTICATION_FEATURE)
  _auth = ESP3DAuthenticationLevel::guest;
#else
  _auth = ESP3DAuthenticationLevel::admin;
#endif  // AUTHENTICATION_FEATURE
}
ESP3DAuthenticationLevel BTService::getAuthentication() { return _auth; }

void BTService::flushData(const uint8_t *data, size_t size,
                          ESP3DMessageType type) {
  ESP3DMessage *message = esp3d_message_manager.newMsg(
      ESP3DClientType::bluetooth, esp3d_commands.getOutputClient(), data,
      size, _auth);

  if (message) {
    message->type = type;
    esp3d_log("Process Message");
    esp3d_commands.process(message);
  } else {
    esp3d_log_e("Cannot create message");
  }
  _lastflush = millis();
}

void BTService::flushChar(char c) {
  flushData((uint8_t *)&c, 1, ESP3DMessageType::realtimecmd);
}

void BTService::flushBuffer() {
  _buffer[_buffer_size] = 0x0;
  flushData((uint8_t *)_buffer, _buffer_size, ESP3DMessageType::unique);
  _buffer_size = 0;
}

// push collected data to buffer and proceed accordingly
void BTService::push2buffer(uint8_t *sbuf, size_t len) {
  if (!_buffer || !_started) {
    return;
  }
  for (size_t i = 0; i < len; i++) {
    _lastflush = millis();
    if (esp3d_string::isRealTimeCommand(sbuf[i])) {
      flushChar(sbuf[i]);
    } else {
      _buffer[_buffer_size] = sbuf[i];
      _buffer_size++;
      if (_buffer_size > ESP3D_BT_BUFFER_SIZE ||
          _buffer[_buffer_size - 1] == '\n') {
        flushBuffer();
      }
    }
  }
}

size_t BTService::writeBytes(const uint8_t *buffer, size_t size) {
  if (availableForWrite() >= size) {
    return SerialBT.write(buffer, size);
  } else {
    size_t sizetosend = size;
    size_t sizesent = 0;
    uint8_t *buffertmp = (uint8_t *)buffer;
    uint32_t starttime = millis();
    // loop until all is sent or timeout
    while (sizetosend > 0 && ((millis() - starttime) < 100)) {
      size_t available = availableForWrite();
      if (available > 0) {
        // in case less is sent
        available =
            SerialBT.write(&buffertmp[sizesent],
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

int BTService::availableForWrite() {
  return 128;  // SerialBT.availableForWrite();
}

int BTService::available() { return SerialBT.available(); }

size_t BTService::readBytes(uint8_t *sbuf, size_t len) {
  return SerialBT.readBytes(sbuf, len);
}

void BTService::flush() { SerialBT.flush(); }

const char *BTService::hostname() { return _btname.c_str(); }

bool BTService::dispatch(ESP3DMessage *message) {
  if (!message || !_started) {
    return false;
  }
  if (message->size > 0 && message->data) {
    size_t sentcnt = writeBytes(message->data, message->size);
    if (sentcnt != message->size) {
      return false;
    }
    esp3d_message_manager.deleteMsg(message);
    return true;
  }

  return false;
}

#endif  // BLUETOOTH_FEATURE

#endif  // ARDUINO_ARCH_ESP32
