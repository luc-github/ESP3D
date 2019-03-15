/*
  commands.h - ESP3D commands class

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

#ifndef COMMANDS_H
#define COMMANDS_H
#include <Arduino.h>
#include "../modules/authentication/authentication_service.h"
class ESP3DOutput;

class Commands
{
public:
    Commands();
    ~Commands();
    void process(uint8_t * sbuf, size_t len, ESP3DOutput * output, level_authenticate_type auth = LEVEL_GUEST, ESP3DOutput * outputonly = nullptr);
    bool is_esp_command(uint8_t * sbuf, size_t len);
    bool execute_internal_command(int cmd, const char* cmd_params, level_authenticate_type auth_level, ESP3DOutput * output);
    int get_space_pos(const char * string, uint from = 0);
    const char* get_param (const char * cmd_params, const char * label);
    bool hastag (const char * cmd_params, const char * tag);
#if defined (WIFI_FEATURE)
    bool ESP100(const char* cmd_params, level_authenticate_type auth_level, ESP3DOutput * output);
    bool ESP101(const char* cmd_params, level_authenticate_type auth_level, ESP3DOutput * output);
#endif //WIFI_FEATURE
#if defined (WIFI_FEATURE) || defined (ETH_FEATURE)
    bool ESP102(const char* cmd_params, level_authenticate_type auth_level, ESP3DOutput * output);
    bool ESP103(const char* cmd_params, level_authenticate_type auth_level, ESP3DOutput * output);
#endif //WIFI_FEATURE ||ETH_FEATURE
#if defined (WIFI_FEATURE)
    bool ESP105(const char* cmd_params, level_authenticate_type auth_level, ESP3DOutput * output);
    bool ESP106(const char* cmd_params, level_authenticate_type auth_level, ESP3DOutput * output);
    bool ESP107(const char* cmd_params, level_authenticate_type auth_level, ESP3DOutput * output);
    bool ESP108(const char* cmd_params, level_authenticate_type auth_level, ESP3DOutput * output);
#endif //WIFI_FEATURE
#if defined( WIFI_FEATURE) ||  defined( BLUETOOTH_FEATURE) || defined (ETH_FEATURE)
    bool ESP110(const char* cmd_params, level_authenticate_type auth_level, ESP3DOutput * output);
#endif //WIFI_FEATURE || BLUETOOTH_FEATURE || ETH_FEATURE
#if defined( WIFI_FEATURE) || defined (ETH_FEATURE)
    bool ESP111(const char* cmd_params, level_authenticate_type auth_level, ESP3DOutput * output);
#endif //WIFI_FEATURE || ETH_FEATURE
#if defined(WIFI_FEATURE) || defined(ETH_FEATURE) || defined(BT_FEATURE)
    bool ESP112(const char* cmd_params, level_authenticate_type auth_level, ESP3DOutput * output);
    bool ESP115(const char* cmd_params, level_authenticate_type auth_level, ESP3DOutput * output);
#endif //WIFI_FEATURE || BLUETOOTH_FEATURE || ETH_FEATURE
#if defined(HTTP_FEATURE)
    bool ESP120(const char* cmd_params, level_authenticate_type auth_level, ESP3DOutput * output);
    bool ESP121(const char* cmd_params, level_authenticate_type auth_level, ESP3DOutput * output);
#endif //HTTP_FEATURE
#if defined(TELNET_FEATURE)
    bool ESP130(const char* cmd_params, level_authenticate_type auth_level, ESP3DOutput * output);
    bool ESP131(const char* cmd_params, level_authenticate_type auth_level, ESP3DOutput * output);
#endif //TELNET_FEATURE
#if defined(TIMESTAMP_FEATURE)
    bool ESP140(const char* cmd_params, level_authenticate_type auth_level, ESP3DOutput * output);
#endif //TIMESTAMP_FEATURE
    bool ESP150(const char* cmd_params, level_authenticate_type auth_level, ESP3DOutput * output);
#ifdef DIRECT_PIN_FEATURE
    bool ESP201(const char* cmd_params, level_authenticate_type auth_level, ESP3DOutput * output);
#endif //DIRECT_PIN_FEATURE
#ifdef DHT_DEVICE
    bool ESP210(const char* cmd_params, level_authenticate_type auth_level, ESP3DOutput * output);
#endif //DHT_DEVICE
    bool ESP400(const char* cmd_params, level_authenticate_type auth_level, ESP3DOutput * output);
    bool ESP401(const char* cmd_params, level_authenticate_type auth_level, ESP3DOutput * output);
#if defined (WIFI_FEATURE)
    bool ESP410(const char* cmd_params, level_authenticate_type auth_level, ESP3DOutput * output);
#endif //WIFI_FEATURE
    bool ESP420(const char* cmd_params, level_authenticate_type auth_level, ESP3DOutput * output);
    bool ESP444(const char* cmd_params, level_authenticate_type auth_level, ESP3DOutput * output);
#if defined (AUTHENTICATION_FEATURE)
    bool ESP550(const char* cmd_params, level_authenticate_type auth_level, ESP3DOutput * output);
    bool ESP555(const char* cmd_params, level_authenticate_type auth_level, ESP3DOutput * output);
#endif //AUTHENTICATION_FEATURE
#if defined(NOTIFICATION_FEATURE)
    bool ESP600(const char* cmd_params, level_authenticate_type auth_level, ESP3DOutput * output);
    bool ESP610(const char* cmd_params, level_authenticate_type auth_level, ESP3DOutput * output);
#endif //NOTIFICATION_FEATURE
#if defined(FILESYSTEM_FEATURE)
    bool ESP710(const char* cmd_params, level_authenticate_type auth_level, ESP3DOutput * output);
    bool ESP720(const char* cmd_params, level_authenticate_type auth_level, ESP3DOutput * output);
#endif //FILESYSTEM_FEATURE
    bool ESP800(const char* cmd_params, level_authenticate_type auth_level, ESP3DOutput * output);
};

extern Commands esp3d_commands;

#endif //COMMANDS_H
