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

#define ERROR_NO_ERROR          0
#define ERROR_TIME_OUT          1
#define ERROR_CANNOT_SEND_DATA  2
#define ERROR_LINE_NUMBER       3
#define ERROR_ACK_NUMBER        4
#define ERROR_MEMORY_PROBLEM    5
#define ERROR_RESEND            6
#define ERROR_NUMBER_MISMATCH   7
#define ERROR_LINE_IGNORED      8
#define ERROR_FILE_SYSTEM       9
#define ERROR_CHECKSUM          10
#define ERROR_UNKNOW            11
#define ERROR_FILE_NOT_FOUND    12
#define ERROR_STREAM_ABORTED    13

#define HOST_NO_STREAM     0
#define HOST_START_STREAM  1
#define HOST_READ_LINE     2
#define HOST_PROCESS_LINE  3
#define HOST_WAIT4_ACK     4
#define HOST_PAUSE_STREAM  5
#define HOST_RESUME_STREAM 6
#define HOST_STOP_STREAM   7
#define HOST_ERROR_STREAM  8
#define HOST_ABORT_STREAM  9

#define TYPE_SCRIPT_STREAM 0
#define TYPE_FS_STREAM     1
#define TYPE_SD_STREAM     2

#define  ESP_HOST_BUFFER_SIZE 255

class GcodeHost
{
public:
    GcodeHost();
    ~GcodeHost();
    bool begin();
    void end();
    void handle();
    bool push(const uint8_t * sbuf, size_t len);
    void flush();
    /*bool sendCommand(const char* command, bool checksum = false, bool wait4ack = true, const char * ack=nullptr);*/
    uint32_t currentCommandNumber()
    {
        return _commandNumber;
    }
    void setCommandNumber(uint32_t n)
    {
        _commandNumber = n;
    }
    bool resetCommandNumbering();
    uint8_t Checksum(const char * command, uint32_t commandSize);
    String CheckSumCommand(const char* command, uint32_t commandnb);

    /*bool wait_for_ack(uint32_t timeout = DEFAULT_TIMOUT, bool checksum=false, const char * ack=nullptr);*/

    uint32_t getCommandNumber(String & response);

    uint8_t getErrorNum()
    {
        return _error;
    }

    void  setErrorNum(uint8_t error)
    {
        _error = error;
    }

    uint8_t getStatus()
    {
        return _step;
    }

    size_t totalSize()
    {
        return _totalSize;
    }
    size_t processedSize()
    {
        return _processedSize;
    }
    uint8_t getFSType()
    {
        return _fsType;
    }
    const char * fileName()
    {
        if (_fileName.length() == 0) {
            return nullptr;
        }
        return _fileName.c_str();
    }
    bool processScript(const char * line, level_authenticate_type auth_type = LEVEL_ADMIN, ESP3DOutput * output=nullptr);
    bool processFile(const char * filename, level_authenticate_type auth_type= LEVEL_ADMIN, ESP3DOutput * output=nullptr);
    bool abort();
    bool pause();
    bool resume();
    void startStream();
    void readNextCommand();
    void endStream();
    void processCommand();
    bool isCommand();
    bool isAckNeeded();
    bool isAck(String & line);

private:
    uint8_t _buffer [ESP_HOST_BUFFER_SIZE+1];
    size_t _bufferSize;
    size_t _totalSize;
    size_t _processedSize;
    uint32_t _commandNumber;
    uint32_t _needCommandNumber;
    uint8_t _error;
    uint8_t _step;
    uint8_t _nextStep;
    uint32_t _currentPosition;
    String _fileName;
    String _script;
    uint8_t _fsType;
    String _currentCommand;
    String _response;
    ESP3DOutput _outputStream;
    level_authenticate_type _auth_type;
    uint64_t _startTimeOut;
    bool _needRelease ;
};

extern GcodeHost esp3d_gcode_host;

#endif //_GCODE_HOST_H

