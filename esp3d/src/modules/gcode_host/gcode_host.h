/*
  gcode_host.h -  gcode host functions class

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



#ifndef _GCODE_HOST_H
#define _GCODE_HOST_H

#include <Arduino.h>
#include "../authentication/authentication_service.h"
class ESP3DOutput;

#define DEFAULT_TIMOUT 2000
#define MAX_TRY_2_SEND 5
#define ERROR_NO_ERROR          0
#define ERROR_TIME_OUT          1
#define ERROR_CANNOT_SEND_DATA  2
#define ERROR_LINE_NUMBER       3
#define ERROR_ACK_NUMBER        4
#define ERROR_MEMORY_PROBLEM    5
#define ERROR_RESEND            6
#define ERROR_NUMBER_MISMATCH   7
#define ERROR_LINE_IGNORED      8

class GcodeHost
{
public:
    GcodeHost();
    ~GcodeHost();
    bool begin(bool waitwhenidle = false);
    void end();
    void handle();
    bool sendCommand(const char* command, bool checksum = false, bool wait4ack = true, const char * ack=nullptr);
    uint32_t currentCommandNumber()
    {
        return _commandnumber;
    }
    void setCommandNumber(uint32_t n)
    {
        _commandnumber = n;
    }
    bool resetCommandNumbering();
    uint8_t Checksum(const char * command, uint32_t commandSize);
    String CheckSumCommand(const char* command, uint32_t commandnb);
    size_t wait_for_data(uint32_t timeout = DEFAULT_TIMOUT);
    bool wait_for_ack(uint32_t timeout = DEFAULT_TIMOUT, bool checksum=false, const char * ack=nullptr);
    bool purge(uint32_t timeout = DEFAULT_TIMOUT);
    uint32_t Get_commandNumber(String & response);
    bool waitWhenIdle()
    {
        return _waitwhenidle;
    }
    uint8_t getErrorNum()
    {
        return _error;
    }
    bool processFile(const char * filename, level_authenticate_type auth_type, ESP3DOutput * output);
    bool processFSFile(const char * filename, level_authenticate_type auth_type, ESP3DOutput * output);
    bool processLine(const char * line, level_authenticate_type auth_type, ESP3DOutput * output);
    bool processscript(const char * line);
private:
    uint32_t _commandnumber;
    uint32_t _needcommandnumber;
    bool _waitwhenidle;
    uint8_t _error;
};

extern GcodeHost esp3d_gcode_host;

#endif //_GCODE_HOST_H

