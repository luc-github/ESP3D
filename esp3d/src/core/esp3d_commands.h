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
#if defined(WIFI_FEATURE)
  void ESP100(int cmd_params_pos, ESP3DMessage* msg);
  void ESP101(int cmd_params_pos, ESP3DMessage* msg);
#endif  // WIFI_FEATURE
#if defined(WIFI_FEATURE)
  void ESP102(int cmd_params_pos, ESP3DMessage* msg);
  void ESP103(int cmd_params_pos, ESP3DMessage* msg);
  void ESP104(int cmd_params_pos, ESP3DMessage* msg);
  void ESP105(int cmd_params_pos, ESP3DMessage* msg);
  void ESP106(int cmd_params_pos, ESP3DMessage* msg);
  void ESP107(int cmd_params_pos, ESP3DMessage* msg);
  void ESP108(int cmd_params_pos, ESP3DMessage* msg);
#endif  // WIFI_FEATURE
#if defined(WIFI_FEATURE) || defined(BLUETOOTH_FEATURE) || defined(ETH_FEATURE)
  void ESP110(int cmd_params_pos, ESP3DMessage* msg);
#endif  // WIFI_FEATURE || BLUETOOTH_FEATURE || ETH_FEATURE
#if defined(WIFI_FEATURE) || defined(ETH_FEATURE)
  void ESP111(int cmd_params_pos, ESP3DMessage* msg);
#endif  // WIFI_FEATURE || ETH_FEATURE
#if defined(WIFI_FEATURE) || defined(ETH_FEATURE) || defined(BT_FEATURE)
  void ESP112(int cmd_params_pos, ESP3DMessage* msg);
  void ESP114(int cmd_params_pos, ESP3DMessage* msg);
  void ESP115(int cmd_params_pos, ESP3DMessage* msg);
#endif  // WIFI_FEATURE || BLUETOOTH_FEATURE || ETH_FEATURE
#if defined(ETH_FEATURE)
  void ESP116(int cmd_params_pos, ESP3DMessage* msg);
  void ESP117(int cmd_params_pos, ESP3DMessage* msg);
  void ESP118(int cmd_params_pos, ESP3DMessage* msg);
#endif  // ETH_FEATURE
#if defined(HTTP_FEATURE)
  void ESP120(int cmd_params_pos, ESP3DMessage* msg);
  void ESP121(int cmd_params_pos, ESP3DMessage* msg);
#endif  // HTTP_FEATURE
#if defined(TELNET_FEATURE)
  void ESP130(int cmd_params_pos, ESP3DMessage* msg);
  void ESP131(int cmd_params_pos, ESP3DMessage* msg);
#endif  // TELNET_FEATURE
#if defined(TIMESTAMP_FEATURE)
  void ESP140(int cmd_params_pos, ESP3DMessage* msg);
#endif  // TIMESTAMP_FEATURE
  void ESP150(int cmd_params_pos, ESP3DMessage* msg);
#if defined(WS_DATA_FEATURE)
  void ESP160(int cmd_params_pos, ESP3DMessage* msg);
  void ESP161(int cmd_params_pos, ESP3DMessage* msg);
#endif  // WS_DATA_FEATURE
#if defined(CAMERA_DEVICE)
  void ESP170(int cmd_params_pos, ESP3DMessage* msg);
  void ESP171(int cmd_params_pos, ESP3DMessage* msg);
#endif  // CAMERA_DEVICE
#if defined(FTP_FEATURE)
  void ESP180(int cmd_params_pos, ESP3DMessage* msg);
  void ESP181(int cmd_params_pos, ESP3DMessage* msg);
#endif  // FTP_FEATURE
#if defined(WEBDAV_FEATURE)
  void ESP190(int cmd_params_pos, ESP3DMessage* msg);
  void ESP191(int cmd_params_pos, ESP3DMessage* msg);
#endif  // WEBDAV_FEATURE
#if defined(SD_DEVICE)
  void ESP200(int cmd_params_pos, ESP3DMessage* msg);
#if SD_DEVICE != ESP_SDIO
  void ESP202(int cmd_params_pos, ESP3DMessage* msg);
#endif  // SD_DEVICE != ESP_SDIO
#ifdef SD_UPDATE_FEATURE
  void ESP402(int cmd_params_pos, ESP3DMessage* msg);
#endif  // SD_UPDATE_FEATURE
#endif  // SD_DEVICE
#ifdef DIRECT_PIN_FEATURE
  void ESP201(int cmd_params_pos, ESP3DMessage* msg);
#endif  // DIRECT_PIN_FEATURE
#if defined(PRINTER_HAS_DISPLAY)
  void ESP212(int cmd_params_pos, ESP3DMessage* msg);
#endif  // PRINTER_HAS_DISPLAY
#if defined(DISPLAY_DEVICE)
  void ESP214(int cmd_params_pos, ESP3DMessage* msg);
#if defined(DISPLAY_TOUCH_DRIVER)
  void ESP215(int cmd_params_pos, ESP3DMessage* msg);
#endif  // DISPLAY_TOUCH_DRIVER
#endif  // DISPLAY_DEVICE
#ifdef SENSOR_DEVICE
  void ESP210(int cmd_params_pos, ESP3DMessage* msg);
#endif  // SENSOR_DEVICE
  void ESP220(int cmd_params_pos, ESP3DMessage* msg);
  void ESP290(int cmd_params_pos, ESP3DMessage* msg);
#if defined(ESP_LUA_INTERPRETER_FEATURE)
  void ESP300(int cmd_params_pos, ESP3DMessage* msg);
  void ESP301(int cmd_params_pos, ESP3DMessage* msg);
#endif  // ESP_LUA_INTERPRETER_FEATURE
  void ESP400(int cmd_params_pos, ESP3DMessage* msg);
  void ESP401(int cmd_params_pos, ESP3DMessage* msg);
#if defined(WIFI_FEATURE)
  void ESP410(int cmd_params_pos, ESP3DMessage* msg);
#endif  // WIFI_FEATURE
  void ESP420(int cmd_params_pos, ESP3DMessage* msg);
  void ESP444(int cmd_params_pos, ESP3DMessage* msg);
#ifdef MDNS_FEATURE
  void ESP450(int cmd_params_pos, ESP3DMessage* msg);
#endif  // MDNS_FEATURE
#if defined(AUTHENTICATION_FEATURE)
  void ESP500(int cmd_params_pos, ESP3DMessage* msg);
  void ESP510(int cmd_params_pos, ESP3DMessage* msg);
  void ESP550(int cmd_params_pos, ESP3DMessage* msg);
  void ESP555(int cmd_params_pos, ESP3DMessage* msg);
#endif  // AUTHENTICATION_FEATURE
#if defined(NOTIFICATION_FEATURE)
  void ESP600(int cmd_params_pos, ESP3DMessage* msg);
  void ESP610(int cmd_params_pos, ESP3DMessage* msg);
  void ESP620(int cmd_params_pos, ESP3DMessage* msg);
#endif  // NOTIFICATION_FEATURE
#if defined(GCODE_HOST_FEATURE)
  void ESP700(int cmd_params_pos, ESP3DMessage* msg);
  void ESP701(int cmd_params_pos, ESP3DMessage* msg);
#endif  // GCODE_HOST_FEATURE
#if defined(FILESYSTEM_FEATURE)
  void ESP710(int cmd_params_pos, ESP3DMessage* msg);
  void ESP720(int cmd_params_pos, ESP3DMessage* msg);
  void ESP730(int cmd_params_pos, ESP3DMessage* msg);
#endif  // FILESYSTEM_FEATURE
#if defined(SD_DEVICE)
  void ESP715(int cmd_params_pos, ESP3DMessage* msg);
  void ESP750(int cmd_params_pos, ESP3DMessage* msg);
  void ESP740(int cmd_params_pos, ESP3DMessage* msg);
#endif  // SD_DEVICE
#if defined(GLOBAL_FILESYSTEM_FEATURE)
  void ESP780(int cmd_params_pos, ESP3DMessage* msg);
  void ESP790(int cmd_params_pos, ESP3DMessage* msg);
#endif  // GLOBAL_FILESYSTEM_FEATURE
  void ESP800(int cmd_params_pos, ESP3DMessage* msg);
#if COMMUNICATION_PROTOCOL != SOCKET_SERIAL
  void ESP900(int cmd_params_pos, ESP3DMessage* msg);
  void ESP901(int cmd_params_pos, ESP3DMessage* msg);
#endif  // COMMUNICATION_PROTOCOL != SOCKET_SERIAL
#if defined(USB_SERIAL_FEATURE)
  void ESP902(int cmd_params_pos, ESP3DMessage* msg);
  void ESP950(int cmd_params_pos, ESP3DMessage* msg);
#endif  // defined(USB_SERIAL_FEATURE)
#if defined(ESP_SERIAL_BRIDGE_OUTPUT)
  void ESP930(int cmd_params_pos, ESP3DMessage* msg);
  void ESP931(int cmd_params_pos, ESP3DMessage* msg);
#endif  // defined (ESP_SERIAL_BRIDGE_OUTPUT)
#ifdef BUZZER_DEVICE
  void ESP910(int cmd_params_pos, ESP3DMessage* msg);
  void ESP250(int cmd_params_pos, ESP3DMessage* msg);
#endif  // BUZZER_DEVICE
#if defined(ARDUINO_ARCH_ESP32) &&                             \
    (CONFIG_IDF_TARGET_ESP32S3 || CONFIG_IDF_TARGET_ESP32S2 || \
     CONFIG_IDF_TARGET_ESP32C3)
  void ESP999(int cmd_params_pos, ESP3DMessage* msg);
#endif  // ARDUINO_ARCH_ESP32
  bool dispatch(const char* sbuf, ESP3DClientType target,
                ESP3DRequest requestId,
                ESP3DMessageType type = ESP3DMessageType::head,
                ESP3DClientType origin = ESP3DClientType::command,
                ESP3DAuthenticationLevel authentication_level =
                    ESP3DAuthenticationLevel::guest);
  bool dispatch(ESP3DMessage* msg, const char* sbuf);
  bool dispatch(ESP3DMessage* msg, uint8_t* sbuf, size_t len);
  bool dispatch(ESP3DMessage* msg);
  bool dispatchAnswer(ESP3DMessage* msg, uint cmdid, bool json, bool hasError,
                      const char* answerMsg);
  bool dispatchIdValue(bool json, const char* Id, const char* value,
                       ESP3DClientType target, ESP3DRequest requestId,
                       bool isFirst = false);
  bool dispatchKeyValue(bool json, const char* key, const char* value,
                        ESP3DClientType target, ESP3DRequest requestId,
                        bool nested = false, bool isFirst = false);
  bool dispatch(uint8_t* sbuf, size_t size, ESP3DClientType target,
                ESP3DRequest requestId, ESP3DMessageType type,
                ESP3DClientType origin,
                ESP3DAuthenticationLevel authentication_level);
  bool dispatchSetting(bool json, const char* filter, ESP3DSettingIndex index,
                       const char* help, const char** optionValues,
                       const char** optionLabels, uint32_t maxsize,
                       uint32_t minsize, uint32_t minsize2, uint8_t precision,
                       const char* unit, bool needRestart,
                       ESP3DClientType target, ESP3DRequest requestId,
                       bool isFirst = false);
  bool dispatchAuthenticationError(ESP3DMessage* msg, uint cmdid, bool json);
  bool formatCommand(char* cmd, size_t len);
  ESP3DClientType getOutputClient(bool fromSettings = false);
  void setOutputClient(ESP3DClientType output_client) {
    _output_client = output_client;
  }

 private:
  ESP3DClientType _output_client;
};

extern ESP3DCommands esp3d_commands;

#endif  // COMMANDS_H
