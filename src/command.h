/*
  command.h - ESP3D configuration class

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

#ifndef COMMAND_h
#define COMMAND_h
#include <Arduino.h>
#include "espcom.h"


class COMMAND
{
public:
    static String buffer_serial;
    static String buffer_tcp;
    static void read_buffer_serial (uint8_t *b, size_t len);
    static void read_buffer_serial (uint8_t b);
#ifdef TCP_IP_DATA_FEATURE
    static void read_buffer_tcp (uint8_t b);
#endif
    static bool check_command (String buffer, tpipe output, bool handlelockserial = true, bool executecmd = true);
    static bool execute_command (int cmd, String cmd_params, tpipe output, level_authenticate_type auth_level = LEVEL_GUEST, ESPResponseStream  *espresponse = NULL);
    static String get_param (String & cmd_params, const char * id, bool withspace = false);
    static bool isadmin (String & cmd_params);
    static bool isuser (String & cmd_params);
};

#endif
