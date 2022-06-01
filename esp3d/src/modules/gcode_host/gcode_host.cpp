/*
  gcode_host.cpp -  gcode host functions class

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
//#define ESP_DEBUG_FEATURE DEBUG_OUTPUT_SERIAL0
#include "../../include/esp3d_config.h"
#if defined(GCODE_HOST_FEATURE)
#include "gcode_host.h"
#include "../../core/settings_esp3d.h"
#include "../../core/commands.h"
#include "../../core/esp3doutput.h"
#if defined(FILESYSTEM_FEATURE)
#include "../filesystem/esp_filesystem.h"
ESP_File FSfileHandle;
#endif //FILESYSTEM_FEATURE
#if defined(SD_DEVICE)
#include "../filesystem/esp_sd.h"
ESP_SDFile SDfileHandle;
#endif //FILESYSTEM_FEATURE

#define ESP_HOST_TIMEOUT 16000
#define MAX_TRY_2_SEND 5


GcodeHost esp3d_gcode_host;

GcodeHost::GcodeHost()
{
    end();
}

GcodeHost::~GcodeHost()
{
    end();
}

bool GcodeHost::begin()
{
    end();
    return true;
}

void GcodeHost::end()
{
    _commandNumber = 0;
    _error = ERROR_NO_ERROR;
    _step = HOST_NO_STREAM;
    _currentPosition = 0;
    _bufferSize = 0;
    _outputStream.client(ESP_STREAM_HOST_CLIENT);
    _totalSize = 0;
    _processedSize = 0;
}

bool GcodeHost::push(const uint8_t * sbuf, size_t len)
{
    if (_step == HOST_NO_STREAM) {
        return false;
    }
    log_esp3d("Push got %d bytes", len);
    for (size_t i = 0; i < len; i++) {
        //it is a line process it
        if (sbuf[i]=='\n' || sbuf[i]=='\r') {
            flush();
        } else {
            //fill buffer until it is full
            if (_bufferSize < ESP_HOST_BUFFER_SIZE) {
                _buffer[_bufferSize++] = sbuf[i];
            } else {
                //buffer is full flush it
                flush();
                _buffer[_bufferSize++] = sbuf[i];
            }
            _buffer[_bufferSize] =0;
        }
    }
    flush();

    return true;
}

bool GcodeHost::isAck(String & line)
{
    if (line.indexOf("ok") != -1) {
        log_esp3d("got ok");
        return true;
    }
    if (Settings_ESP3D::GetFirmwareTarget()==SMOOTHIEWARE) {
        if (line.indexOf("smoothie out") != -1) {
            log_esp3d("got smoothie out");
            return true;
        }
    }
    return false;
}

void GcodeHost::flush()
{
    //analyze buffer and do action if needed
    //look for \n, ok , error, ack
    //then clean buffer accordingly
    if(_bufferSize==0) {
        return;
    }
    _response = (const char*)_buffer;
    log_esp3d("Stream got the response: %s", _response.c_str());
    _response.toLowerCase();
    if (isAck(_response)) {
        //check if we have proper ok response
        //like if numbering is enabled
        if(_step == HOST_WAIT4_ACK) {
            _step=HOST_READ_LINE;
        } else {
            log_esp3d("Got ok but out of the query");
        }
    } else {
        if (_response.indexOf("error") != -1) {
            log_esp3d("Got error");
            _step = HOST_ERROR_STREAM;
        }
    }
    //What is have processing to do
    //what if have resend
    _bufferSize = 0;
}

void GcodeHost::startStream()
{
    if (_fsType ==TYPE_SCRIPT_STREAM) {
        _totalSize = _script.length();
        log_esp3d("Script line %s opened, size is %d", _script.c_str(), _totalSize);
    }
#if defined(FILESYSTEM_FEATURE)
    if (_fsType ==TYPE_FS_STREAM) {
        if (ESP_FileSystem::exists(_fileName.c_str())) {
            FSfileHandle = ESP_FileSystem::open(_fileName.c_str());
        }
        if (FSfileHandle.isOpen()) {
            _totalSize = FSfileHandle.size();
            log_esp3d("File %s opened, size is %d", _fileName.c_str(), _totalSize);
        } else {
            _error = ERROR_FILE_NOT_FOUND;
            _step = HOST_ERROR_STREAM;
            log_esp3d("File not found: %s", _fileName.c_str());
            return;
        }
    }
#endif //FILESYSTEM_FEATURE
#if defined(SD_DEVICE)
    if (_fsType ==TYPE_SD_STREAM) {
        if (!ESP_SD::accessFS()) {
            _error = ERROR_FILE_NOT_FOUND;
            _step = HOST_ERROR_STREAM;
            _needRelease = false;
            log_esp3d("File not found: %s", _fileName.c_str());
            return;
        }
        _needRelease = true;
        if (ESP_SD::getState(true) == ESP_SDCARD_NOT_PRESENT) {
            _error = ERROR_FILE_NOT_FOUND;
            _step = HOST_ERROR_STREAM;
            log_esp3d("File not found: %s", _fileName.c_str());
            return;
        }
        ESP_SD::setState(ESP_SDCARD_BUSY );

        if (ESP_SD::exists(_fileName.c_str())) {
            SDfileHandle = ESP_SD::open(_fileName.c_str());
        }
        if (SDfileHandle.isOpen()) {
            _totalSize = SDfileHandle.size();
            log_esp3d("File %s opened, size is %d", _fileName.c_str(), _totalSize);
        } else {
            _error = ERROR_FILE_NOT_FOUND;
            _step = HOST_ERROR_STREAM;
            log_esp3d("File not found: %s", _fileName.c_str());
            return;
        }
    }
#endif //SD_DEVICE
    _currentPosition = 0;
    _response = "";
    _currentPosition = 0;
    _error = ERROR_NO_ERROR;
    _step = HOST_READ_LINE;
    _nextStep = HOST_READ_LINE;
    _processedSize = 0;
}

void GcodeHost::endStream()
{
    log_esp3d("Ending Stream");
#if defined(FILESYSTEM_FEATURE)
    if (_fsType ==TYPE_FS_STREAM) {
        if (FSfileHandle.isOpen()) {
            FSfileHandle.close();
        }
    }
#endif //FILESYSTEM_FEATURE
#if defined(SD_DEVICE)
    if (_fsType ==TYPE_SD_STREAM) {
        if (SDfileHandle.isOpen()) {
            SDfileHandle.close();
        }
        if(_needRelease) {
            ESP_SD::releaseFS();
        }
    }
#endif //SD_DEVICE
    _step = HOST_NO_STREAM;
}

void GcodeHost::readNextCommand()
{
    _currentCommand = "";
    _step = HOST_PROCESS_LINE;
    if (_fsType ==TYPE_SCRIPT_STREAM) {
        log_esp3d("Reading next command from script");
        if (_currentPosition < _script.length()) {
            if (_script.indexOf(';', _currentPosition) != -1) {
                _currentCommand = _script.substring(_currentPosition, _script.indexOf(';', _currentPosition));
                _currentPosition = _script.indexOf(';', _currentPosition) + 1;
            } else {
                _currentCommand = _script.substring(_currentPosition);
                _currentPosition = _script.length();
            }
            _processedSize = _currentPosition;
            log_esp3d("Command is %s", _currentCommand.c_str());
        } else {
            _step = HOST_STOP_STREAM;
        }
    }
#if defined(FILESYSTEM_FEATURE)
    if (_fsType ==TYPE_FS_STREAM) {
        bool processing = true;
        while (processing) {
            //to handle file without endline
            int c = FSfileHandle.read();
            if (c == -1) {
                processing = false;
            } else {
                _processedSize++;
                _currentPosition++;
                if (!(((char)c =='\n') || ((char)c =='\r'))) {
                    _currentCommand+=(char)c;
                } else {
                    processing = false;
                }
            }
        }
        if (_currentCommand.length() == 0) {
            _step = HOST_STOP_STREAM;
        }
    }
#endif //FILESYSTEM_FEATURE
#if defined(SD_DEVICE)
    if (_fsType ==TYPE_SD_STREAM) {
        bool processing = true;
        while (processing) {
            //to handle file without endline
            int c = SDfileHandle.read();
            if (c == -1) {
                processing = false;
            } else {
                _processedSize++;
                _currentPosition++;
                if (!(((char)c =='\n') || ((char)c =='\r'))) {
                    _currentCommand+=(char)c;
                } else {
                    processing = false;
                }
            }
        }
        if (_currentCommand.length() == 0) {
            _step = HOST_STOP_STREAM;
        }
    }
#endif //SD_DEVICE
}

bool GcodeHost::isCommand()
{
    //clean the command
    if (_currentCommand.indexOf(';') != -1) {
        _currentCommand = _currentCommand.substring(0, _currentCommand.indexOf(';'));
    }
    _currentCommand.trim();
    if (_currentCommand.length() == 0 || _currentCommand.startsWith(";")) {
        return false;
    }
    return true;
}
bool GcodeHost::isAckNeeded()
{
    //TODO: what command do not need for ack ?
    return true;
}
void GcodeHost::processCommand()
{
    if (!isCommand()) {
        log_esp3d("Command %s is not valid", _currentCommand.c_str());
        _step = HOST_READ_LINE;
    } else {
        log_esp3d("Command %s is valid", _currentCommand.c_str());
        String cmd = _currentCommand + "\n";
        bool isESPcmd = esp3d_commands.is_esp_command((uint8_t *)_currentCommand.c_str(), _currentCommand.length());
        //TODO need to change if ESP3D
#if COMMUNICATION_PROTOCOL == SOCKET_SERIAL
        ESP3DOutput output(ESP_SOCKET_SERIAL_CLIENT);
#endif//COMMUNICATION_PROTOCOL
#if COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL == MKS_SERIAL
        ESP3DOutput output(ESP_SERIAL_CLIENT);
#endif //COMMUNICATION_PROTOCOL == SOCKET_SERIAL
        ESP3DOutput outputhost(ESP_STREAM_HOST_CLIENT);
        if (isESPcmd) {
            esp3d_commands.process((uint8_t *)cmd.c_str(), cmd.length(),&outputhost, _auth_type) ;
            //we display error in output but it is not a blocking error
            log_esp3d("Command is ESP command: %s, client is %d", cmd.c_str(), );
            _step = HOST_READ_LINE;
        } else {
#if COMMUNICATION_PROTOCOL == SOCKET_SERIAL
            esp3d_commands.process((uint8_t *)cmd.c_str(), cmd.length(),&_outputStream, _auth_type,&output,_outputStream.client()==ESP_ECHO_SERIAL_CLIENT?ESP_SOCKET_SERIAL_CLIENT:0 ) ;
#endif //COMMUNICATION_PROTOCOL == SOCKET_SERIAL            
#if COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL == MKS_SERIAL
            log_esp3d("Command is not ESP command:%s, client is %d and only is %d",cmd.c_str(), (&_outputStream?_outputStream.client():0),(&output?output.client():0));
            esp3d_commands.process((uint8_t *)cmd.c_str(), cmd.length(),&_outputStream, _auth_type,&output) ;
#endif //COMMUNICATION_PROTOCOL == SOCKET_SERIAL           
            _startTimeOut =millis();
            log_esp3d("Command is GCODE command");
            if (isAckNeeded()) {
                _step = HOST_WAIT4_ACK;
                log_esp3d("Command wait for ack");
            } else {
                _step = HOST_READ_LINE;
            }
        }
    }
}

void GcodeHost::handle()
{
    if (_step == HOST_NO_STREAM) {
        return;
    }
    switch(_step) {
    case HOST_START_STREAM:
        startStream();
        break;
    case HOST_READ_LINE:
        if (_nextStep==HOST_PAUSE_STREAM) {
            _step = HOST_PAUSE_STREAM;
            _nextStep = HOST_READ_LINE;
        } else {
            readNextCommand();
        }
        break;
    case HOST_PROCESS_LINE:
        processCommand();
        break;
    case HOST_WAIT4_ACK:
        if (millis() - _startTimeOut > ESP_HOST_TIMEOUT) {
            log_esp3d("Timeout waiting for ack");
            _error = ERROR_TIME_OUT;
            _step = HOST_ERROR_STREAM;
        }
        break;
    case HOST_PAUSE_STREAM:
        //TODO pause stream
        break;
    case HOST_RESUME_STREAM:
        //Any extra action to resume stream?
        _step = HOST_READ_LINE;
        break;
    case HOST_STOP_STREAM:
        endStream();
        break;
    case HOST_ERROR_STREAM: {
        String Error;
        if (_error == ERROR_NO_ERROR) {
            //TODO check _response to put right error
            _error = ERROR_UNKNOW;
        }
        log_esp3d("Error %d", _error);
        Error = "error: stream failed: " + String(_error) + "\n";
#if COMMUNICATION_PROTOCOL == SOCKET_SERIAL
        ESP3DOutput output(ESP_SOCKET_SERIAL_CLIENT);
#endif//COMMUNICATION_PROTOCOL
#if COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL == MKS_SERIAL
        ESP3DOutput output(ESP_SERIAL_CLIENT);
#endif//COMMUNICATION_PROTOCOL
        output.dispatch((const uint8_t *)Error.c_str(), Error.length());
        _step = HOST_STOP_STREAM;
    }
    break;
    default: //Not handled step
        log_esp3d("Not handled step %d", _step);
        break;
    }
}

bool  GcodeHost::abort()
{
    if (_step == HOST_NO_STREAM) {
        return false;
    }
    log_esp3d("Aborting script");
    //TODO: what to do in addition ?
    _error=ERROR_STREAM_ABORTED;
    //we do not use step to do faster abort
    endStream();
    return true;
}

bool GcodeHost::pause()
{
    if (_step == HOST_NO_STREAM) {
        return false;
    }
    _nextStep = HOST_PAUSE_STREAM;
    return true;
}

bool GcodeHost::resume()
{
    if (_step != HOST_PAUSE_STREAM) {
        return false;
    }
    _step = HOST_PAUSE_STREAM;
    return true;
}

uint8_t GcodeHost::Checksum(const char * command, uint32_t commandSize)
{
    uint8_t checksum_val =0;
    if (command == NULL) {
        return 0;
    }
    for (uint32_t i=0; i < commandSize; i++) {
        checksum_val = checksum_val ^ ((uint8_t)command[i]);
    }
    return checksum_val;
}

String GcodeHost::CheckSumCommand(const char* command, uint32_t commandnb)
{
    String commandchecksum = "N" + String((uint32_t)commandnb)+ " " + command;
    uint8_t crc = Checksum(commandchecksum.c_str(), commandchecksum.length());
    commandchecksum+="*"+String(crc);
    return commandchecksum;
}

bool GcodeHost::resetCommandNumbering()
{
    String resetcmd = "M110 N0";
    if (Settings_ESP3D::GetFirmwareTarget() == SMOOTHIEWARE) {
        resetcmd = "N0 M110";
    } else {
        resetcmd = "M110 N0";
    }
    _commandNumber = 1;
    //need to use process to send command
    return _outputStream.printLN(resetcmd.c_str());
}

uint32_t GcodeHost::getCommandNumber(String & response)
{
    uint32_t l = 0;
    String sresend = "Resend:";
    if ( Settings_ESP3D::GetFirmwareTarget() == SMOOTHIEWARE) {
        sresend = "rs N";
    }
    int pos = response.indexOf(sresend);
    if (pos == -1 ) {
        log_esp3d("Cannot find label %d", _error);
        return -1;
    }
    pos+=sresend.length();
    int pos2 = response.indexOf("\n", pos);
    String snum = response.substring(pos, pos2);
    //remove potential unwished char
    snum.replace("\r", "");
    l = snum.toInt();
    log_esp3d("Command number to resend is %s", String((uint32_t)l).c_str());
    return l;
}

bool GcodeHost::processScript(const char * line, level_authenticate_type auth_type, ESP3DOutput * output)
{
    _script = line;
    _script.trim();
    log_esp3d("Processing script: %s", _script.c_str());
    if (_script.length() == 0) {
        log_esp3d("No script to process");
        return false;
    }
    if (_step != HOST_NO_STREAM) {
        log_esp3d("Streaming already in progress");
        return false;
    }
    _fsType = TYPE_SCRIPT_STREAM;
    _step = HOST_START_STREAM;
    _auth_type = auth_type;
    return true;
}

bool GcodeHost::processFile(const char * filename, level_authenticate_type auth_type, ESP3DOutput * output)
{
    bool target_found = false;
#if COMMUNICATION_PROTOCOL == SOCKET_SERIAL
    log_esp3d("Processing file client is  %d", output?output->client():ESP_SOCKET_SERIAL_CLIENT);
    _outputStream.client(output?output->client():ESP_SOCKET_SERIAL_CLIENT);
#endif//COMMUNICATION_PROTOCOL
#if COMMUNICATION_PROTOCOL == RAW_SERIAL || COMMUNICATION_PROTOCOL == MKS_SERIAL
    log_esp3d("Processing file client is  %d", output?output->client():ESP_SERIAL_CLIENT);
    _outputStream.client(output?output->client():ESP_SERIAL_CLIENT);
#endif//COMMUNICATION_PROTOCOL
    //sanity check
    _fileName = filename[0]!='/'?"/":"";
    _fileName +=filename;
    _fileName.trim();
    log_esp3d("Processing file: %s", filename);
    if (_fileName.length() == 0) {
        log_esp3d("No file to process");
        return false;
    }
    if (_step != HOST_NO_STREAM) {
        log_esp3d("Streaming already in progress");
        return false;
    }
    //TODO UD = USB DISK
#if defined(SD_DEVICE)
    if (_fileName.startsWith(ESP_SD_FS_HEADER)) {
        log_esp3d("Processing SD file");
        target_found = true;
        _fileName= _fileName.substring(strlen(ESP_SD_FS_HEADER),_fileName.length());
        _fsType = TYPE_SD_STREAM;
    }
#endif //SD_DEVICE
#if defined(FILESYSTEM_FEATURE)
    if (!target_found && _fileName.startsWith(ESP_FLASH_FS_HEADER)) {
        target_found = true;
        _fileName= _fileName.substring(strlen(ESP_FLASH_FS_HEADER),_fileName.length());
        log_esp3d("Processing /FS file %s", _fileName.c_str());
        _fsType = TYPE_FS_STREAM;
    }
    //if no header it is also an FS file
    if (!target_found) {
        log_esp3d("Processing FS file %s", _fileName.c_str());
        _fsType = TYPE_FS_STREAM;
        target_found = true;
    }
#endif //FILESYSTEM_FEATURE
    //it is not a file so it is a script
    if (!target_found ) {
        target_found = true;
        _fsType = TYPE_SCRIPT_STREAM;
        //remove the /
        _script=&_fileName[1];
        log_esp3d("Processing Script file %s", _script.c_str());
        _fileName = "";
    }
    _step = HOST_START_STREAM;
    _auth_type = auth_type;
    return true;
}

#endif //GCODE_HOST_FEATURE
