/*
  esp3d_commands.h - ESP3D commands class

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

#ifndef COMMANDS_H
#define COMMANDS_H
#include <Arduino.h>

#include "../modules/authentication/authentication_service.h"
#include "esp3d_message.h"

class ESP3DCommands {
 public:
  ESP3DCommands();
  ~ESP3DCommands();
  void process(ESP3DMessage* msg);

  bool is_esp_command(uint8_t* sbuf, size_t len);

  void execute_internal_command(int cmd, int cmd_params_pos, ESP3DMessage* msg);

  const char* get_param(ESP3DMessage* msg, uint start, const char* label,
                        bool* found = nullptr);

  const char* get_param(const char* data, uint size, uint start,
                        const char* label, bool* found = nullptr);

  const char* format_response(uint cmdID, bool isjson = false, bool isok = true,
                              const char* message = "");

  bool hasTag(ESP3DMessage* msg, uint start, const char* label);
  const char* get_clean_param(ESP3DMessage* msg, uint start);
  bool has_param(ESP3DMessage* msg, uint start);

  void ESP0(int cmd_params_pos, ESP3DMessage* msg);

  /*
#if defined(WIFI_FEATURE)
  bool ESP100(const char* cmd_params, ESP3DAuthenticationLevel auth_level,
              ESP3D_Message* esp3dmsg);
  bool ESP101(const char* cmd_params, ESP3DAuthenticationLevel auth_level,
              ESP3D_Message* esp3dmsg);
#endif  // WIFI_FEATURE
#if defined(WIFI_FEATURE) || defined(ETH_FEATURE)
  bool ESP102(const char* cmd_params, ESP3DAuthenticationLevel auth_level,
              ESP3D_Message* esp3dmsg);
  bool ESP103(const char* cmd_params, ESP3DAuthenticationLevel auth_level,
              ESP3D_Message* esp3dmsg);
#endif  // WIFI_FEATURE ||ETH_FEATURE
#if defined(WIFI_FEATURE) || defined(BLUETOOTH_FEATURE) || defined(ETH_FEATURE)
  bool ESP104(const char* cmd_params, ESP3DAuthenticationLevel auth_level,
              ESP3D_Message* esp3dmsg);
#endif  // WIFI_FEATURE || BLUETOOTH_FEATURE || ETH_FEATURE
#if defined(WIFI_FEATURE)
  bool ESP105(const char* cmd_params, ESP3DAuthenticationLevel auth_level,
              ESP3D_Message* esp3dmsg);
  bool ESP106(const char* cmd_params, ESP3DAuthenticationLevel auth_level,
              ESP3D_Message* esp3dmsg);
  bool ESP107(const char* cmd_params, ESP3DAuthenticationLevel auth_level,
              ESP3D_Message* esp3dmsg);
  bool ESP108(const char* cmd_params, ESP3DAuthenticationLevel auth_level,
              ESP3D_Message* esp3dmsg);
#endif  // WIFI_FEATURE
#if defined(WIFI_FEATURE) || defined(BLUETOOTH_FEATURE) || defined(ETH_FEATURE)
  bool ESP110(const char* cmd_params, ESP3DAuthenticationLevel auth_level,
              ESP3D_Message* esp3dmsg);
#endif  // WIFI_FEATURE || BLUETOOTH_FEATURE || ETH_FEATURE
#if defined(WIFI_FEATURE) || defined(ETH_FEATURE)
  bool ESP111(const char* cmd_params, ESP3DAuthenticationLevel auth_level,
              ESP3D_Message* esp3dmsg);
#endif  // WIFI_FEATURE || ETH_FEATURE
#if defined(WIFI_FEATURE) || defined(ETH_FEATURE) || defined(BT_FEATURE)
  bool ESP112(const char* cmd_params, ESP3DAuthenticationLevel auth_level,
              ESP3D_Message* esp3dmsg);
  bool ESP114(const char* cmd_params, ESP3DAuthenticationLevel auth_level,
              ESP3D_Message* esp3dmsg);
  bool ESP115(const char* cmd_params, ESP3DAuthenticationLevel auth_level,
              ESP3D_Message* esp3dmsg);
#endif  // WIFI_FEATURE || BLUETOOTH_FEATURE || ETH_FEATURE
#if defined(HTTP_FEATURE)
  bool ESP120(const char* cmd_params, ESP3DAuthenticationLevel auth_level,
              ESP3D_Message* esp3dmsg);
  bool ESP121(const char* cmd_params, ESP3DAuthenticationLevel auth_level,
              ESP3D_Message* esp3dmsg);
#endif  // HTTP_FEATURE
#if defined(TELNET_FEATURE)
  bool ESP130(const char* cmd_params, ESP3DAuthenticationLevel auth_level,
              ESP3D_Message* esp3dmsg);
  bool ESP131(const char* cmd_params, ESP3DAuthenticationLevel auth_level,
              ESP3D_Message* esp3dmsg);
#endif  // TELNET_FEATURE
#if defined(TIMESTAMP_FEATURE)
  bool ESP140(const char* cmd_params, ESP3DAuthenticationLevel auth_level,
              ESP3D_Message* esp3dmsg);
#endif  // TIMESTAMP_FEATURE
  bool ESP150(const char* cmd_params, ESP3DAuthenticationLevel auth_level,
              ESP3D_Message* esp3dmsg);
#if defined(WS_DATA_FEATURE)
  bool ESP160(const char* cmd_params, ESP3DAuthenticationLevel auth_level,
              ESP3D_Message* esp3dmsg);
  bool ESP161(const char* cmd_params, ESP3DAuthenticationLevel auth_level,
              ESP3D_Message* esp3dmsg);
#endif  // WS_DATA_FEATURE
#if defined(CAMERA_DEVICE)
  bool ESP170(const char* cmd_params, ESP3DAuthenticationLevel auth_level,
              ESP3D_Message* esp3dmsg);
  bool ESP171(const char* cmd_params, ESP3DAuthenticationLevel auth_level,
              ESP3D_Message* esp3dmsg);
#endif  // CAMERA_DEVICE
#if defined(FTP_FEATURE)
  bool ESP180(const char* cmd_params, ESP3DAuthenticationLevel auth_level,
              ESP3D_Message* esp3dmsg);
  bool ESP181(const char* cmd_params, ESP3DAuthenticationLevel auth_level,
              ESP3D_Message* esp3dmsg);
#endif  // FTP_FEATURE
#if defined(WEBDAV_FEATURE)
  bool ESP190(const char* cmd_params, ESP3DAuthenticationLevel auth_level,
              ESP3D_Message* esp3dmsg);
  bool ESP191(const char* cmd_params, ESP3DAuthenticationLevel auth_level,
              ESP3D_Message* esp3dmsg);
#endif  // WEBDAV_FEATURE
#if defined(SD_DEVICE)
  bool ESP200(const char* cmd_params, ESP3DAuthenticationLevel auth_level,
              ESP3D_Message* esp3dmsg);
#if SD_DEVICE != ESP_SDIO
  bool ESP202(const char* cmd_params, ESP3DAuthenticationLevel auth_level,
              ESP3D_Message* esp3dmsg);
#endif  // SD_DEVICE != ESP_SDIO
#ifdef SD_UPDATE_FEATURE
  bool ESP402(const char* cmd_params, ESP3DAuthenticationLevel auth_level,
              ESP3D_Message* esp3dmsg);
#endif  // SD_UPDATE_FEATURE
#endif  // SD_DEVICE
#ifdef DIRECT_PIN_FEATURE
  bool ESP201(const char* cmd_params, ESP3DAuthenticationLevel auth_level,
              ESP3D_Message* esp3dmsg);
#endif  // DIRECT_PIN_FEATURE
#if defined(DISPLAY_DEVICE)
  bool ESP214(const char* cmd_params, ESP3DAuthenticationLevel auth_level,
              ESP3D_Message* esp3dmsg);
#if defined(DISPLAY_TOUCH_DRIVER)
  bool ESP215(const char* cmd_params, ESP3DAuthenticationLevel auth_level,
              ESP3D_Message* esp3dmsg);
#endif  // DISPLAY_TOUCH_DRIVER
#endif  // DISPLAY_DEVICE
#ifdef SENSOR_DEVICE
  bool ESP210(const char* cmd_params, ESP3DAuthenticationLevel auth_level,
              ESP3D_Message* esp3dmsg);
#endif  // SENSOR_DEVICE
  bool ESP220(const char* cmd_params, ESP3DAuthenticationLevel auth_level,
              ESP3D_Message* esp3dmsg);
  bool ESP290(const char* cmd_params, ESP3DAuthenticationLevel auth_level,
              ESP3D_Message* esp3dmsg);
  bool ESP400(const char* cmd_params, ESP3DAuthenticationLevel auth_level,
              ESP3D_Message* esp3dmsg);
  bool ESP401(const char* cmd_params, ESP3DAuthenticationLevel auth_level,
              ESP3D_Message* esp3dmsg);
#if defined(WIFI_FEATURE)
  bool ESP410(const char* cmd_params, ESP3DAuthenticationLevel auth_level,
              ESP3D_Message* esp3dmsg);
#endif  // WIFI_FEATURE
  bool ESP420(const char* cmd_params, ESP3DAuthenticationLevel auth_level,
              ESP3D_Message* esp3dmsg);
  bool ESP444(const char* cmd_params, ESP3DAuthenticationLevel auth_level,
              ESP3D_Message* esp3dmsg);
#ifdef MDNS_FEATURE
  bool ESP450(const char* cmd_params, ESP3DAuthenticationLevel auth_level,
              ESP3D_Message* esp3dmsg);
#endif  // MDNS_FEATURE
#if defined(AUTHENTICATION_FEATURE)
  bool ESP550(const char* cmd_params, ESP3DAuthenticationLevel auth_level,
              ESP3D_Message* esp3dmsg);
  bool ESP555(const char* cmd_params, ESP3DAuthenticationLevel auth_level,
              ESP3D_Message* esp3dmsg);
#endif  // AUTHENTICATION_FEATURE
#if defined(NOTIFICATION_FEATURE)
  bool ESP600(const char* cmd_params, ESP3DAuthenticationLevel auth_level,
              ESP3D_Message* esp3dmsg);
  bool ESP610(const char* cmd_params, ESP3DAuthenticationLevel auth_level,
              ESP3D_Message* esp3dmsg);
  bool ESP620(const char* cmd_params, ESP3DAuthenticationLevel auth_level,
              ESP3D_Message* esp3dmsg);
#endif  // NOTIFICATION_FEATURE
#if defined(GCODE_HOST_FEATURE)
  bool ESP700(const char* cmd_params, ESP3DAuthenticationLevel auth_level,
              ESP3D_Message* esp3dmsg);
  bool ESP701(const char* cmd_params, ESP3DAuthenticationLevel auth_level,
              ESP3D_Message* esp3dmsg);
#endif  // GCODE_HOST_FEATURE
#if defined(FILESYSTEM_FEATURE)
  bool ESP710(const char* cmd_params, ESP3DAuthenticationLevel auth_level,
              ESP3D_Message* esp3dmsg);
  bool ESP720(const char* cmd_params, ESP3DAuthenticationLevel auth_level,
              ESP3D_Message* esp3dmsg);
  bool ESP730(const char* cmd_params, ESP3DAuthenticationLevel auth_level,
              ESP3D_Message* esp3dmsg);
#endif  // FILESYSTEM_FEATURE
#if defined(SD_DEVICE)
  bool ESP715(const char* cmd_params, ESP3DAuthenticationLevel auth_level,
              ESP3D_Message* esp3dmsg);
  bool ESP750(const char* cmd_params, ESP3DAuthenticationLevel auth_level,
              ESP3D_Message* esp3dmsg);
  bool ESP740(const char* cmd_params, ESP3DAuthenticationLevel auth_level,
              ESP3D_Message* esp3dmsg);
#endif  // SD_DEVICE
#if defined(GLOBAL_FILESYSTEM_FEATURE)
  bool ESP780(const char* cmd_params, ESP3DAuthenticationLevel auth_level,
              ESP3D_Message* esp3dmsg);
  bool ESP790(const char* cmd_params, ESP3DAuthenticationLevel auth_level,
              ESP3D_Message* esp3dmsg);
#endif  // GLOBAL_FILESYSTEM_FEATURE
  bool ESP800(const char* cmd_params, ESP3DAuthenticationLevel auth_level,
              ESP3D_Message* esp3dmsg);
#if COMMUNICATION_PROTOCOL != SOCKET_SERIAL
  bool ESP900(const char* cmd_params, ESP3DAuthenticationLevel auth_level,
              ESP3D_Message* esp3dmsg);
  bool ESP901(const char* cmd_params, ESP3DAuthenticationLevel auth_level,
              ESP3D_Message* esp3dmsg);
#endif  // COMMUNICATION_PROTOCOL != SOCKET_SERIAL
#if defined(ESP_SERIAL_BRIDGE_OUTPUT)
  bool ESP930(const char* cmd_params, ESP3DAuthenticationLevel auth_level,
              ESP3D_Message* esp3dmsg);
  bool ESP931(const char* cmd_params, ESP3DAuthenticationLevel auth_level,
              ESP3D_Message* esp3dmsg);
#endif  // defined (ESP_SERIAL_BRIDGE_OUTPUT)
#ifdef BUZZER_DEVICE
  bool ESP910(const char* cmd_params, ESP3DAuthenticationLevel auth_level,
              ESP3D_Message* esp3dmsg);
  bool ESP250(const char* cmd_params, ESP3DAuthenticationLevel auth_level,
              ESP3D_Message* esp3dmsg);
#endif  // BUZZER_DEVICE
#if defined(ARDUINO_ARCH_ESP32) &&                             \
    (CONFIG_IDF_TARGET_ESP32S3 || CONFIG_IDF_TARGET_ESP32S2 || \
     CONFIG_IDF_TARGET_ESP32C3)
  bool ESP999(const char* cmd_params, ESP3DAuthenticationLevel auth_level,
              ESP3D_Message* esp3dmsg);
#endif  // ARDUINO_ARCH_ESP32
*/
  bool dispatch(const char* sbuf, ESP3DClientType target,
                ESP3DRequest requestId,
                ESP3DMessageType type = ESP3DMessageType::head,
                ESP3DClientType origin = ESP3DClientType::command,
                ESP3DAuthenticationLevel authentication_level =
                    ESP3DAuthenticationLevel::guest);
  bool dispatch(ESP3DMessage* msg, const char* sbuf);
  bool dispatch(ESP3DMessage* msg, uint8_t* sbuf, size_t len);
  bool dispatch(ESP3DMessage* msg);
  ESP3DClientType getOutputClient(bool fromSettings = false);
  void setOutputClient(ESP3DClientType output_client) {
    _output_client = output_client;
  }

 private:
  ESP3DClientType _output_client;
  /*bool _dispatchSetting(bool json, const char* filter, ESP3DSettingIndex
index, const char* help, const char** optionValues, const char** optionLabels,
uint32_t maxsize, uint32_t minsize, uint32_t minsize2, uint8_t precision, const
char* unit, bool needRestart, ESP3D_Message* esp3dmsg, bool isFirst = false);*/
};

extern ESP3DCommands esp3d_commands;

#endif  // COMMANDS_H
