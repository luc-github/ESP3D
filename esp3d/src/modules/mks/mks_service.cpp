/*
  mks_service.cpp -  mks communication service functions class

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
#if COMMUNICATION_PROTOCOL == MKS_SERIAL
#include "../../core/esp3d_message.h"
#include "../../core/esp3d_settings.h"
#include "../http/http_server.h"
#include "../network/netconfig.h"
#include "../serial/serial_service.h"
#include "../telnet/telnet_server.h"
#include "../wifi/wificonfig.h"
#include "mks_service.h"

// Flag Pins
#define ESP_FLAG_PIN 0
#define BOARD_FLAG_PIN 4
// Flag pins values
#define BOARD_READY_FLAG_VALUE LOW

// Frame offsets
#define MKS_FRAME_HEAD_OFFSET 0
#define MKS_FRAME_TYPE_OFFSET 1
#define MKS_FRAME_DATALEN_OFFSET 2
#define MKS_FRAME_DATA_OFFSET 4

// Frame flags
#define MKS_FRAME_HEAD_FLAG (char)0xa5
#define MKS_FRAME_TAIL_FLAG (char)0xfc

// Network states
#define MKS_FRAME_NETWORK_OK_STATE (char)0x0a
#define MKS_FRAME_NETWORK_FAIL_STATE (char)0x05
#define MKS_FRAME_NETWORK_ERROR_STATE (char)0x0e

// Network modes
#define MKS_FRAME_NETWORK_AP_MODE (char)0x01
#define MKS_FRAME_NETWORK_STA_MODE (char)0x02
#define MKS_FRAME_NETWORK_APSTA_MODE (char)0x03

// Cloud states
#define MKS_FRAME_CLOUD_BINDED_STATE (char)0x12
#define MKS_FRAME_CLOUD_NOT_BINDED_STATE (char)0x13
#define MKS_FRAME_CLOUD_DISCONNECTED_STATE (char)0x10
#define MKS_FRAME_CLOUD_DISABLED_STATE (char)0x00

// Data types
#define MKS_FRAME_DATA_NETWORK_TYPE (char)0x0
#define MKS_FRAME_DATA_COMMAND_TYPE (char)0x1
#define MKS_FRAME_DATA_FIRST_FRAGMENT_TYPE (char)0x2
#define MKS_FRAME_DATA_FRAGMENT_TYPE (char)0x3
#define MKS_FRAME_DATA_HOTSPOTS_LIST_TYPE (char)0x4
#define MKS_FRAME_DATA_STATIC_IP_TYPE (char)0x5

#define MKS_TYPE_NET (char)0x0
#define MKS_TYPE_PRINTER (char)0x1
#define MKS_TYPE_TRANSFER (char)0x2
#define MKS_TYPE_EXCEPTION (char)0x3
#define MKS_TYPE_CLOUD (char)0x4
#define MKS_TYPE_UNBIND (char)0x5
#define MKS_TYPE_WID (char)0x6
#define MKS_TYPE_SCAN_WIFI (char)0x7
#define MKS_TYPE_MANUAL_IP (char)0x8
#define MKS_TYPE_WIFI_CTRL (char)0x9

#define CONNECT_STA 0x1
#define DISCONNECT_STA 0x2
#define REMOVE_STA_INFO 0x3

#define UNKNOW_STATE 0x0
#define ERROR_STATE 0x1
#define SUCCESS_STATE 0x2

#define NB_HOTSPOT_MAX 15

// Timeouts
#define FRAME_WAIT_TO_SEND_TIMEOUT 2000
#define ACK_TIMEOUT 5000
#define NET_FRAME_REFRESH_TIME 10000

#define UPLOAD_BAUD_RATE 1958400

bool MKSService::_started = false;
uint8_t MKSService::_frame[MKS_FRAME_SIZE] = {0};
char MKSService::_moduleId[22] = {0};
uint8_t MKSService::_uploadStatus = UNKNOW_STATE;
long MKSService::_commandBaudRate = 115200;
bool MKSService::_uploadMode = false;

bool MKSService::isHead(const char c) { return (c == MKS_FRAME_HEAD_FLAG); }
bool MKSService::isTail(const char c) { return (c == MKS_FRAME_TAIL_FLAG); }
bool MKSService::isCommand(const char c) { return (c == MKS_TYPE_TRANSFER); }
bool MKSService::isFrame(const char c) {
  if ((c >= MKS_TYPE_NET) && (c <= MKS_TYPE_WIFI_CTRL)) {
    return true;
  }
  return false;
}

bool MKSService::dispatch(ESP3DMessage *message) {
  if (!message || !_started) {
    return false;
  }
  if (message->size > 0 && message->data) {
    if (sendGcodeFrame((const char *)message->data)) {
      esp3d_message_manager.deleteMsg(message);
      return true;
    }
  }
  return false;
}

bool MKSService::begin() {
  // setup the pins
  pinMode(BOARD_FLAG_PIN, INPUT);
  pinMode(ESP_FLAG_PIN, OUTPUT);
  _started = true;
  // max size is 21
  sprintf(_moduleId, "HJNLM000%02X%02X%02X%02X%02X%02X", WiFi.macAddress()[0],
          WiFi.macAddress()[1], WiFi.macAddress()[2], WiFi.macAddress()[3],
          WiFi.macAddress()[4], WiFi.macAddress()[5]);
  commandMode(true);
  return true;
}

void MKSService::commandMode(bool fromSettings) {
  if (fromSettings) {
    _commandBaudRate = ESP3DSettings::readUint32(ESP_BAUD_RATE);
  }
  esp3d_log("Cmd Mode");
  _uploadMode = false;
  esp3d_serial_service.updateBaudRate(_commandBaudRate);
}
void MKSService::uploadMode() {
  esp3d_log("Upload Mode");
  _uploadMode = true;
  esp3d_serial_service.updateBaudRate(UPLOAD_BAUD_RATE);
}

uint MKSService::getFragmentID(uint32_t fragmentNumber, bool isLast) {
  esp3d_log("Fragment: %d %s", fragmentNumber, isLast ? " is last" : "");
  if (isLast) {
    fragmentNumber |= (1 << 31);
  } else {
    fragmentNumber &= ~(1 << 31);
  }
  esp3d_log("Fragment is now: %d", fragmentNumber);
  return fragmentNumber;
}

bool MKSService::sendFirstFragment(const char *filename, size_t filesize) {
  uint fileNameLen = strlen(filename);
  uint dataLen = fileNameLen + 5;
  clearFrame();
  // Head Flag
  _frame[MKS_FRAME_HEAD_OFFSET] = MKS_FRAME_HEAD_FLAG;
  // Type Flag
  _frame[MKS_FRAME_TYPE_OFFSET] = MKS_FRAME_DATA_FIRST_FRAGMENT_TYPE;
  // Fragment size
  _frame[MKS_FRAME_DATALEN_OFFSET] = dataLen & 0xff;
  _frame[MKS_FRAME_DATALEN_OFFSET + 1] = dataLen >> 8;
  // FileName size
  _frame[MKS_FRAME_DATA_OFFSET] = strlen(filename);
  // File Size
  _frame[MKS_FRAME_DATA_OFFSET + 1] = filesize & 0xff;
  _frame[MKS_FRAME_DATA_OFFSET + 2] = (filesize >> 8) & 0xff;
  _frame[MKS_FRAME_DATA_OFFSET + 3] = (filesize >> 16) & 0xff;
  _frame[MKS_FRAME_DATA_OFFSET + 4] = (filesize >> 24) & 0xff;
  // Filename
  strncpy((char *)&_frame[MKS_FRAME_DATA_OFFSET + 5], filename, fileNameLen);
  // Tail Flag
  _frame[dataLen + 4] = MKS_FRAME_TAIL_FLAG;
  esp3d_log("Filename: %s  Filesize: %d", filename, filesize);
  for (uint i = 0; i < dataLen + 5; i++) {
    esp3d_log("%c %x", _frame[i], _frame[i]);
  }
  _uploadStatus = UNKNOW_STATE;
  if (canSendFrame()) {
    _uploadStatus = UNKNOW_STATE;
    if (esp3d_serial_service.writeBytes(_frame, dataLen + 5) == (dataLen + 5)) {
      esp3d_log("First fragment Ok");
      sendFrameDone();
      return true;
    }
  }
  esp3d_log("Failed");
  sendFrameDone();
  return false;
}

bool MKSService::sendFragment(const uint8_t *dataFrame, const size_t dataSize,
                              uint fragmentID) {
  uint dataLen = dataSize + 4;
  esp3d_log("Fragment datalen:%d", dataSize);
  // Head Flag
  _frame[MKS_FRAME_HEAD_OFFSET] = MKS_FRAME_HEAD_FLAG;
  // Type Flag
  _frame[MKS_FRAME_TYPE_OFFSET] = MKS_FRAME_DATA_FRAGMENT_TYPE;
  // Fragment size
  _frame[MKS_FRAME_DATALEN_OFFSET] = dataLen & 0xff;
  _frame[MKS_FRAME_DATALEN_OFFSET + 1] = dataLen >> 8;
  // Fragment ID
  _frame[MKS_FRAME_DATA_OFFSET] = fragmentID & 0xff;
  _frame[MKS_FRAME_DATA_OFFSET + 1] = (fragmentID >> 8) & 0xff;
  _frame[MKS_FRAME_DATA_OFFSET + 2] = (fragmentID >> 16) & 0xff;
  _frame[MKS_FRAME_DATA_OFFSET + 3] = (fragmentID >> 24) & 0xff;
  // data
  if ((dataSize > 0) && (dataFrame != nullptr)) {
    memcpy(&_frame[MKS_FRAME_DATA_OFFSET + 4], dataFrame, dataSize);
  }
  if (dataSize < MKS_FRAME_DATA_MAX_SIZE) {
    clearFrame(dataLen + 4);
  }
  // Tail Flag
  _frame[dataLen + 4] = MKS_FRAME_TAIL_FLAG;
  /* for (uint i =0; i< dataLen + 5 ; i++) {
           esp3d_log("%c %x",_frame[i],_frame[i]);
       }*/
  if (canSendFrame()) {
    _uploadStatus = UNKNOW_STATE;
    if (esp3d_serial_service.writeBytes(_frame, MKS_FRAME_SIZE) ==
        MKS_FRAME_SIZE) {
      esp3d_log("Ok");
      sendFrameDone();
      return true;
    }
    esp3d_log_e("Error with size sent");
  }
  esp3d_log_e("Failed");
  sendFrameDone();
  return false;
}

void MKSService::sendWifiHotspots() {
  uint8_t ssid_name_length;
  uint dataOffset = 1;
  uint8_t total_hotspots = 0;
  uint8_t currentmode = WiFi.getMode();
  if (currentmode == WIFI_AP) {
    WiFi.mode(WIFI_AP_STA);
  }
  clearFrame();
  // clean memory
  WiFi.scanDelete();
  int n = WiFi.scanNetworks();
  esp3d_log("scan done");
  if (n == 0) {
    esp3d_log("no networks found");
  } else {
    esp3d_log("%d networks found", n);
    clearFrame();
    _frame[MKS_FRAME_HEAD_OFFSET] = MKS_FRAME_HEAD_FLAG;
    _frame[MKS_FRAME_TYPE_OFFSET] = MKS_FRAME_DATA_HOTSPOTS_LIST_TYPE;
    for (uint8_t i = 0; i < n; ++i) {
      int8_t signal_rssi = 0;
      if (total_hotspots > NB_HOTSPOT_MAX) {
        break;
      }
      signal_rssi = WiFi.RSSI(i);
      // Print SSID and RSSI for each network found
      esp3d_log("%d: %s (%d) %s", i + 1, WiFi.SSID(i).c_str(), signal_rssi,
                (WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");
      ssid_name_length = WiFi.SSID(i).length();
      if (ssid_name_length > MAX_SSID_LENGTH) {
        esp3d_log_e("Name too long, ignored");
        continue;
      }
      if (signal_rssi < MIN_RSSI) {
        esp3d_log("Signal too low, ignored");
        continue;
      }
      _frame[MKS_FRAME_DATA_OFFSET + dataOffset] = ssid_name_length;
      for (uint8_t p = 0; p < ssid_name_length; p++) {
        _frame[MKS_FRAME_DATA_OFFSET + dataOffset + 1 + p] = WiFi.SSID(i)[p];
      }
      _frame[MKS_FRAME_DATA_OFFSET + dataOffset + ssid_name_length + 1] =
          WiFi.RSSI(i);
      dataOffset += ssid_name_length + 2;
      total_hotspots++;
    }
    _frame[MKS_FRAME_DATA_OFFSET] = total_hotspots;
    _frame[MKS_FRAME_DATA_OFFSET + dataOffset] = MKS_FRAME_TAIL_FLAG;
    _frame[MKS_FRAME_DATALEN_OFFSET] = dataOffset & 0xff;
    _frame[MKS_FRAME_DATALEN_OFFSET + 1] = dataOffset >> 8;
    esp3d_log("Size of data in frame %d ", dataOffset);
    for (uint i = 0; i < dataOffset + 5; i++) {
      esp3d_log("%c %x", _frame[i], _frame[i]);
    }
    if (canSendFrame()) {
      if (esp3d_serial_service.writeBytes(_frame, dataOffset + 5) ==
          (dataOffset + 5)) {
        esp3d_log("Ok");
        sendFrameDone();
      } else {
        esp3d_log_e("Send scan failed");
      }
    } else {
      esp3d_log_e("Cannot send scan");
    }
    // clean memory
    WiFi.scanDelete();
  }
  // Restore mode
  WiFi.mode((WiFiMode_t)currentmode);
  sendFrameDone();
}

void MKSService::handleFrame(const uint8_t type, const uint8_t *dataFrame,
                             const size_t dataSize) {
  esp3d_log("Command is %d", type);
  switch (type) {
    // wifi setup
    case MKS_TYPE_NET:
      esp3d_log("************MKS_TYPE_NET*************");
      messageWiFiConfig(dataFrame, dataSize);
      break;
    // not supported in Marlin
    // Confirmed as private source
    case MKS_TYPE_PRINTER:
      // ignored
      esp3d_log("************MKS_TYPE_PRINTER*************");
      break;
    // File transfer if not command
    case MKS_TYPE_TRANSFER:
      // todo
      esp3d_log("************MKS_TYPE_TRANSFER*************");
      break;
    // Error when doing transfer
    case MKS_TYPE_EXCEPTION:
      esp3d_log("************MKS_TYPE_EXCEPTION*************");
      messageException(dataFrame, dataSize);
      break;
    // not supported (cloud)
    case MKS_TYPE_CLOUD:
      // ignored
      esp3d_log("************MKS_TYPE_CLOUD*************");
      break;
    // not supported (cloud)
    case MKS_TYPE_WID:
      // ignored
      esp3d_log("************MKS_TYPE_WID*************");
      break;
    // hot spot list
    case MKS_TYPE_SCAN_WIFI:
      esp3d_log("************MKS_TYPE_SCAN_WIFI*************");
      sendWifiHotspots();
      break;
    // setup Manual IP
    // not supported in Marlin, so do same for the moment
    case MKS_TYPE_MANUAL_IP:
      // ignored
      esp3d_log("************MKS_TYPE_MANUAL_IP*************");
      break;
    // On/Off Wifi
    case MKS_TYPE_WIFI_CTRL:
      esp3d_log("************MKS_TYPE_WIFI_CTRL*************");
      messageWiFiControl(dataFrame, dataSize);
      break;
    default:
      esp3d_log_e("Unknow type");
  }
}

void MKSService::messageWiFiControl(const uint8_t *dataFrame,
                                    const size_t dataSize) {
  if (dataSize != 1) {
    return;
  }
  switch (dataFrame[0]) {
    case CONNECT_STA:
      esp3d_log("CONNECT_STA");
      if (!NetConfig::started()) {
        NetConfig::begin();
      }
      break;
    case DISCONNECT_STA:
      esp3d_log("CONNECT_STA");
      if (NetConfig::started()) {
        NetConfig::end();
      }
      break;
    case REMOVE_STA_INFO:
      esp3d_log("REMOVE_STA_INFO");
      if (NetConfig::started()) {
        NetConfig::end();
      }
      ESP3DSettings::reset(true);
      break;
    default:
      esp3d_log_e("WiFi control flag not supported");
  }
}
// Exception handle - but actually not used
void MKSService::messageException(const uint8_t *dataFrame,
                                  const size_t dataSize) {
  if (dataSize != 1) {
    return;
  }
  if ((dataFrame[0] == ERROR_STATE) || (dataFrame[0] == SUCCESS_STATE)) {
    _uploadStatus = dataFrame[0];
    esp3d_log("Tranfer: %s", dataFrame[0] == ERROR_STATE ? "Error" : "Success");
  } else {
    _uploadStatus = UNKNOW_STATE;
    esp3d_log_e("Tranfer state unknown");
  }
}

void MKSService::messageWiFiConfig(const uint8_t *dataFrame,
                                   const size_t dataSize) {
  String ssid;
  String password;
  String savedSsid;
  String savedPassword;
  bool needrestart = false;
  // Sanity check
  if (dataSize < 2) {
    esp3d_log_e("Invalid data");
    return;
  }
  if ((dataFrame[0] != MKS_FRAME_NETWORK_AP_MODE) &&
      (dataFrame[0] != MKS_FRAME_NETWORK_STA_MODE)) {
    esp3d_log_e("Invalid mode");
    return;
  }
  if ((dataFrame[1] > dataSize - 3) || (dataFrame[1] == 0) ||
      (dataFrame[1] > MAX_SSID_LENGTH)) {
    esp3d_log_e("Invalid ssid size");
    return;
  }
  if ((uint)(dataFrame[1] + 3) > dataSize) {
    esp3d_log_e("Overflow password size");
    return;
  }
  if ((dataFrame[dataFrame[1] + 2]) > MAX_PASSWORD_LENGTH) {
    esp3d_log_e("Invalid password size");
    return;
  }
  // get SSID and password
  for (uint8_t i = 0; i < dataFrame[1]; i++) {
    ssid += (char)dataFrame[2 + i];
  }
  for (uint8_t j = 0; j < dataFrame[2 + dataFrame[1]]; j++) {
    password += (char)dataFrame[3 + j + dataFrame[1]];
  }
  if (dataFrame[0] == MKS_FRAME_NETWORK_AP_MODE) {
    if (ESP3DSettings::readByte(ESP_RADIO_MODE) != ESP_WIFI_AP) {
      ESP3DSettings::writeByte(ESP_RADIO_MODE, ESP_WIFI_AP);
      needrestart = true;
    }
    savedSsid = ESP3DSettings::readString(ESP_AP_SSID);
    savedPassword = ESP3DSettings::readString(ESP_AP_PASSWORD);
    if (savedSsid != ssid) {
      ESP3DSettings::writeString(ESP_AP_SSID, ssid.c_str());
      needrestart = true;
    }
    if (savedPassword != password) {
      ESP3DSettings::writeString(ESP_AP_PASSWORD, password.c_str());
      needrestart = true;
    }
  } else {
    if (ESP3DSettings::readByte(ESP_RADIO_MODE) != ESP_WIFI_STA) {
      ESP3DSettings::writeByte(ESP_RADIO_MODE, ESP_WIFI_STA);
      needrestart = true;
    }
    savedSsid = ESP3DSettings::readString(ESP_STA_SSID);
    savedPassword = ESP3DSettings::readString(ESP_STA_PASSWORD);
    if (savedSsid != ssid) {
      ESP3DSettings::writeString(ESP_STA_SSID, ssid.c_str());
      needrestart = true;
    }
    if (savedPassword != password) {
      ESP3DSettings::writeString(ESP_STA_PASSWORD, password.c_str());
      needrestart = true;
    }
    if (needrestart) {
      // change also to DHCP for new value
      ESP3DSettings::writeByte(ESP_STA_IP_MODE, DHCP_MODE);
    }
  }
  if (needrestart) {
    esp3d_log("Modifications done - restarting network");
    NetConfig::begin();
  }
}

bool MKSService::canSendFrame() {
  esp3d_log("Is board ready for frame?");
  digitalWrite(ESP_FLAG_PIN, BOARD_READY_FLAG_VALUE);
  uint32_t startTime = millis();
  while ((millis() - startTime) < FRAME_WAIT_TO_SEND_TIMEOUT) {
    if (digitalRead(BOARD_FLAG_PIN) == BOARD_READY_FLAG_VALUE) {
      esp3d_log("Yes");
      return true;
    }
    ESP3DHal::wait(0);
  }
  esp3d_log("Time out no board answer");
  return false;
}

void MKSService::sendFrameDone() {
  digitalWrite(ESP_FLAG_PIN, !BOARD_READY_FLAG_VALUE);
}

bool MKSService::sendGcodeFrame(const char *cmd) {
  if (_uploadMode) {
    return false;
  }
  String tmp = cmd;
  if (tmp.endsWith("\n")) {
    tmp[tmp.length() - 1] = '\0';
  }
  esp3d_log("Packing: *%s*, size=%d", tmp.c_str(), strlen(tmp.c_str()));
  clearFrame();
  _frame[MKS_FRAME_HEAD_OFFSET] = MKS_FRAME_HEAD_FLAG;
  _frame[MKS_FRAME_TYPE_OFFSET] = MKS_FRAME_DATA_COMMAND_TYPE;
  for (uint i = 0; i < strlen(tmp.c_str()); i++) {
    _frame[MKS_FRAME_DATA_OFFSET + i] = tmp[i];
  }
  _frame[MKS_FRAME_DATA_OFFSET + strlen(tmp.c_str())] = '\r';
  _frame[MKS_FRAME_DATA_OFFSET + strlen(tmp.c_str()) + 1] = '\n';
  _frame[MKS_FRAME_DATA_OFFSET + strlen(tmp.c_str()) + 2] = MKS_FRAME_TAIL_FLAG;
  _frame[MKS_FRAME_DATALEN_OFFSET] = (strlen(tmp.c_str()) + 2) & 0xff;
  _frame[MKS_FRAME_DATALEN_OFFSET + 1] =
      ((strlen(tmp.c_str()) + 2) >> 8) & 0xff;

  esp3d_log("Size of data in frame %d ", strlen(tmp.c_str()) + 2);
  // for (uint i =0; i< strlen(tmp.c_str())+7;i++){
  // esp3d_log("%c %x",_frame[i],_frame[i]);
  // }

  if (canSendFrame()) {
    if (esp3d_serial_service.writeBytes(_frame, strlen(tmp.c_str()) + 7) ==
        (strlen(tmp.c_str()) + 7)) {
      esp3d_log("Ok");
      sendFrameDone();
      return true;
    }
  }
  esp3d_log_e("Failed");
  sendFrameDone();
  return false;
}

bool MKSService::sendNetworkFrame() {
  size_t dataOffset = 0;
  String s;
  static uint32_t lastsend = 0;
  if (_uploadMode) {
    return false;
  }
  if ((millis() - lastsend) > NET_FRAME_REFRESH_TIME) {
    lastsend = millis();
    esp3d_log("Network frame preparation");
    // Prepare
    clearFrame();
    _frame[MKS_FRAME_HEAD_OFFSET] = MKS_FRAME_HEAD_FLAG;
    _frame[MKS_FRAME_TYPE_OFFSET] = MKS_FRAME_DATA_NETWORK_TYPE;
    if (NetConfig::getMode() == ESP_WIFI_STA) {
      esp3d_log("STA Mode");
      if (WiFi.status() == WL_CONNECTED) {
        ///////////////////////////////////
        // IP Segment
        // IP value
        IPAddress ip = NetConfig::localIPAddress();
        _frame[MKS_FRAME_DATA_OFFSET] = ip[0];
        _frame[MKS_FRAME_DATA_OFFSET + 1] = ip[1];
        _frame[MKS_FRAME_DATA_OFFSET + 2] = ip[2];
        _frame[MKS_FRAME_DATA_OFFSET + 3] = ip[3];
        esp3d_log("IP %d.%d.%d.%d", _frame[MKS_FRAME_DATA_OFFSET],
                  _frame[MKS_FRAME_DATA_OFFSET + 1],
                  _frame[MKS_FRAME_DATA_OFFSET + 2],
                  _frame[MKS_FRAME_DATA_OFFSET + 3]);
        //////////////////////////////////
        // State Segment
        // Connected state (OK)
        _frame[MKS_FRAME_DATA_OFFSET + 6] = MKS_FRAME_NETWORK_OK_STATE;
      } else {
        ///////////////////////////////////
        // IP Segment
        // No need - bytes are already cleared
        //////////////////////////////////
        // State Segment
        // Connected state (Disconnected)
        _frame[MKS_FRAME_DATA_OFFSET + 6] = MKS_FRAME_NETWORK_FAIL_STATE;
      }
      //////////////////////////////////
      // Mode Segment
      _frame[MKS_FRAME_DATA_OFFSET + 7] = MKS_FRAME_NETWORK_STA_MODE;
      //////////////////////////////////
      // Wifi_name_len Segment
      s = ESP3DSettings::readString(ESP_STA_SSID);
      _frame[MKS_FRAME_DATA_OFFSET + 8] = s.length();
      dataOffset = MKS_FRAME_DATA_OFFSET + 9;
      //////////////////////////////////
      // Wifi_name Segment
      strcpy((char *)&_frame[dataOffset], s.c_str());
      dataOffset += s.length();
      //////////////////////////////////
      // Wifi_key_len Segment
      s = ESP3DSettings::readString(ESP_STA_PASSWORD);
      _frame[dataOffset] = s.length();
      dataOffset++;
      //////////////////////////////////
      // Wifi_key Segment
      strcpy((char *)&_frame[dataOffset], s.c_str());
      dataOffset += s.length();
    } else if (NetConfig::getMode() == ESP_WIFI_AP ||
               (NetConfig::getMode() == ESP_AP_SETUP)) {
      esp3d_log("AP Mode");
      ///////////////////////////////////
      // IP Segment
      // IP value
      IPAddress ip = NetConfig::localIPAddress();
      _frame[MKS_FRAME_DATA_OFFSET] = ip[0];
      _frame[MKS_FRAME_DATA_OFFSET + 1] = ip[1];
      _frame[MKS_FRAME_DATA_OFFSET + 2] = ip[2];
      _frame[MKS_FRAME_DATA_OFFSET + 3] = ip[3];
      //////////////////////////////////
      // State Segment
      // Connected state (OK)
      _frame[MKS_FRAME_DATA_OFFSET + 6] = MKS_FRAME_NETWORK_OK_STATE;
      //////////////////////////////////
      // Mode Segment
      _frame[MKS_FRAME_DATA_OFFSET + 7] = MKS_FRAME_NETWORK_AP_MODE;
      //////////////////////////////////
      // Wifi_name_len Segment
      String s = ESP3DSettings::readString(ESP_AP_SSID);
      _frame[MKS_FRAME_DATA_OFFSET + 8] = s.length();
      dataOffset = MKS_FRAME_DATA_OFFSET + 9;
      //////////////////////////////////
      // Wifi_name Segment
      strcpy((char *)&_frame[dataOffset], s.c_str());
      dataOffset += s.length();
      //////////////////////////////////
      // Wifi_key_len Segment
      s = ESP3DSettings::readString(ESP_AP_PASSWORD);
      _frame[dataOffset] = s.length();
      dataOffset++;
      //////////////////////////////////
      // Wifi_key Segment
      strcpy((char *)&_frame[dataOffset], s.c_str());
      dataOffset += s.length();
    } else {
      // not supported
      esp3d_log_e("Mode not supported : %d ", NetConfig::getMode());
      return false;
    }
    //////////////////////////////////
    // Cloud Services port Segment
    // hard coded
    _frame[MKS_FRAME_DATA_OFFSET + 4] = (telnet_server.port()) & 0xff;
    _frame[MKS_FRAME_DATA_OFFSET + 5] = ((telnet_server.port()) >> 8) & 0xff;
    esp3d_log("Cloud port: %d", (telnet_server.port()));

    //////////////////////////////////
    // Cloud State Segment
    // hard coded as disabled in upstream FW
    _frame[dataOffset] = MKS_FRAME_CLOUD_DISABLED_STATE;
    dataOffset++;
    //////////////////////////////////
    // Cloud host len Segment
    // Use ESP3D IP instead
    s = NetConfig::localIPAddress().toString();
    _frame[dataOffset] = s.length();
    dataOffset++;
    //////////////////////////////////
    // Cloud host Segment
    // Use ESP3D IP instead
    strcpy((char *)&_frame[dataOffset], s.c_str());
    dataOffset += s.length();
    //////////////////////////////////
    // Cloud host port Segment
    // use webserver port instead
    _frame[dataOffset] = (HTTP_Server::port()) & 0xff;
    dataOffset++;
    _frame[dataOffset] = ((HTTP_Server::port()) >> 8) & 0xff;
    dataOffset++;
    //////////////////////////////////
    // Module id len Segment
    // Use hostname instead
    _frame[dataOffset] = strlen(_moduleId);
    dataOffset++;
    //////////////////////////////////
    // Module id  Segment
    strcpy((char *)&_frame[dataOffset], _moduleId);
    dataOffset += strlen(_moduleId);
    //////////////////////////////////
    // FW version len Segment
    _frame[dataOffset] = strlen(FW_VERSION) + 6;
    dataOffset++;
    //////////////////////////////////
    // FW version  Segment
    strcpy((char *)&_frame[dataOffset], "ESP3D_" FW_VERSION);
    dataOffset += strlen(FW_VERSION) + 6;
    //////////////////////////////////
    // Tail Segment
    _frame[dataOffset] = MKS_FRAME_TAIL_FLAG;

    //////////////////////////////////
    // Data len Segment
    // Calculated from above
    _frame[MKS_FRAME_DATALEN_OFFSET] = (dataOffset - 4) & 0xff;
    _frame[MKS_FRAME_DATALEN_OFFSET + 1] = ((dataOffset - 4) >> 8) & 0xff;
    esp3d_log("Size of data in frame %d ", dataOffset - 4);
    if (canSendFrame()) {
      if (esp3d_serial_service.writeBytes(_frame, dataOffset + 1) ==
          (dataOffset + 1)) {
        esp3d_log("Ok");
        sendFrameDone();
        return true;
      }
    }
    sendFrameDone();
    esp3d_log_e("Failed");
  }

  return false;
}

void MKSService::clearFrame(uint start) {
  memset(&_frame[start], 0, sizeof(_frame) - start);
}
void MKSService::handle() {
  if (_started) {
    sendNetworkFrame();
  }
  // network frame every 10s
}
void MKSService::end() { _started = false; }

#endif  // COMMUNICATION_PROTOCOL == MKS_SERIAL
