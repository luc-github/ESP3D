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

#include "../../include/esp3d_config.h"
#ifdef ESP_GCODE_HOST_FEATURE
#include "gcode_host.h"
#include "../../core/settings_esp3d.h"
#include "../../core/commands.h"
#include "../../core/esp3doutput.h"
#include "../serial/serial_service.h"
#include "../filesystem/esp_filesystem.h"

GcodeHost esp3d_gcode_host;

GcodeHost::GcodeHost()
{
    _commandnumber = 0;
    _waitwhenidle = false;
    _error = ERROR_NO_ERROR;
}

GcodeHost::~GcodeHost()
{
    end();
}

bool GcodeHost::begin(bool waitwhenidle)
{
    end();
    _waitwhenidle = waitwhenidle;
    return true;
}

void GcodeHost::end()
{
    _commandnumber = 0;
    _error = ERROR_NO_ERROR;
}
void GcodeHost::handle()
{
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

size_t GcodeHost::wait_for_data(uint32_t timeout)
{
    uint32_t start = millis();
    while ((serial_service.available() < 2) && ((millis()-start) < timeout)) {
        Hal::wait (0);  //minimum delay is 10 actually
    }
    return serial_service.available();
}

bool GcodeHost::resetCommandNumbering()
{
    String resetcmd = "M110 N0";
    if (Settings_ESP3D::GetFirmwareTarget() == SMOOTHIEWARE) {
        resetcmd = "N0 M110";
    } else {
        resetcmd = "M110 N0";
    }
    _commandnumber = 1;
    return sendCommand(resetcmd.c_str());
}

/*bool GcodeHost::endUpload(){

    return true;
}*/

bool GcodeHost::wait_for_ack(uint32_t timeout, bool checksum, const char * ack)
{
    _needcommandnumber = _commandnumber;
    uint32_t start = millis();
    String answer = "";
    while ((millis()-start) < timeout) {
        size_t len = serial_service.available();
        if (len > 0) {
            uint8_t * sbuf = (uint8_t *)malloc(len+1);
            if(!sbuf) {
                _error = ERROR_MEMORY_PROBLEM;
                return false;
            }
            sbuf[len] = '\0';
            answer+= (const char *)sbuf;
            free(sbuf);
            log_esp3d("Answer:  %s",answer.c_str());
            //check for ack
            if (ack!=nullptr) {
                if (answer.indexOf(ack) != -1) {
                    _error = ERROR_NO_ERROR;
                    return true;
                }
            } else {
                //wait is not an ack as it can appear any time
                if (answer.indexOf("ok") != -1) {
                    if (!checksum) {
                        _error = ERROR_NO_ERROR;
                        return true;
                    } else {
                        //check number
                        String ackstring = "ok " + String(_commandnumber);
                        if (answer.indexOf(ackstring) != -1) {
                            _error = ERROR_NO_ERROR;
                            return true;
                        }
                    }
                }
            }
            //check for error
            if ((answer.indexOf("Resend:") != -1) || (answer.indexOf("rs N") != -1)) {
                _needcommandnumber = Get_commandNumber(answer);
                if (_needcommandnumber == _commandnumber) {
                    _error = ERROR_RESEND;
                }  else {
                    _error = ERROR_NUMBER_MISMATCH;
                    log_esp3d("Error provived %d but need  %d", _commandnumber, _needcommandnumber);
                }

                return false;
            }
            if (answer.indexOf("skip") != -1) {
                _error = ERROR_LINE_IGNORED;
                return false;
            }
        }

        Hal::wait (0);  //minimum delay is 10 actually
    }
    _error = ERROR_ACK_NUMBER;
    return false;
}

//the command MUST NOT have '\n' at the end
//the max command try to send the same command if resend is asked no more than MAX_TRY_2_SEND times
//others error cancel the sending
//number mismatch / skip / timeout error must be managed out of this function
//line number incrementation is not done in this function neither
bool GcodeHost::sendCommand(const char* command, bool checksum, bool wait4ack, const char * ack)
{
    log_esp3d("Send command: %s", command);
    String s;
    if(checksum) {
        s = CheckSumCommand(command, _commandnumber);
    } else {
        s = command;
    }
    for(uint8_t try_nb = 0; try_nb < MAX_TRY_2_SEND; try_nb ++) {
        _error = ERROR_NO_ERROR;
        purge();
        if ((_error != ERROR_NO_ERROR) && wait4ack) {
            return false;
        } else {
            //if no need to wait for ack the purge has no real impact but clear buffer
            _error = ERROR_NO_ERROR;
        }
        uint32_t start = millis();
        //to give a chance to not overload buffer
        bool done = false;
        while (((millis() - start) < DEFAULT_TIMOUT) && !done) {
            if (serial_service.availableForWrite() > s.length()) {
                if (strlen(command) == serial_service.write((const uint8_t*)s.c_str(), s.length())) {
                    if (serial_service.write('\n')==1) {
                        if(!wait4ack) {
                            log_esp3d("No need ack");
                            return true;
                        }
                        //process answer
                        if (wait_for_ack(DEFAULT_TIMOUT, ack)) {
                            log_esp3d("Command got ack");
                            return true;
                        } else {
                            //what is the error ?
                            log_esp3d("Error: %d", _error);
                            //no need to retry for this one
                            if (_error == ERROR_MEMORY_PROBLEM) {
                                return false;
                            }
                            //need to resend command
                            if (_error == ERROR_RESEND) {
                                done = true;
                            }
                            //the printer ask for another command line so exit
                            if ((_error == ERROR_NUMBER_MISMATCH) || (_error == ERROR_LINE_IGNORED)) {
                                return false;
                            }
                        }
                    }
                }
            }
        }
    }
    if (_error == ERROR_NO_ERROR) {
        _error = ERROR_CANNOT_SEND_DATA;
        log_esp3d("Error: %d", _error);
    }
    return false;
}

bool GcodeHost::purge(uint32_t timeout)
{
    uint32_t start = millis();
    uint8_t buf [51];
    _error = 0;
    log_esp3d("Purge started");
    while (serial_service.available() > 0) {
        if ((millis() - start ) > timeout) {
            log_esp3d("Purge timeout\r\n");
            _error = ERROR_TIME_OUT;
            return false;
        }
        size_t len = serial_service.readBytes (buf, 50);
        buf[len] = '\0';
        log_esp3d("**\n%s\n", (const char *)buf);
        if ( (Settings_ESP3D::GetFirmwareTarget() == REPETIER4DV) || (Settings_ESP3D::GetFirmwareTarget() == REPETIER) || _waitwhenidle) {
            String s = (const char *)buf;
            //repetier never stop sending data so no need to wait if have 'wait' or 'busy'
            if((s.indexOf ("wait") > -1) || (s.indexOf ("busy") > -1)) {
                return true;
            }
            log_esp3d("Impossible to purge\r\n");
        }
        Hal::wait (0);
    }
    log_esp3d("Purge done");
    return true;
}

uint32_t GcodeHost::Get_commandNumber(String & response)
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

bool GcodeHost::processFSFile(const char * filename, level_authenticate_type auth_type, ESP3DOutput * output)
{
    bool res = true;
    log_esp3d("Processing FS : %s", filename);
    if (!ESP_FileSystem::exists(filename)) {
        log_esp3d("Cannot find file");
        return false;
    }
    ESP_File f = ESP_FileSystem::open(filename);
    if (!f.isOpen()) {
        log_esp3d("Cannot open file");
        return false;
    }
    size_t filesize  = f.size();
    int8_t ch;
    String cmd = "";
    for (size_t c = 0; c< filesize ; c++) {
        ch = f.read();
        if (ch == -1) {
            log_esp3d("Error reading file");
            f.close();
            return false;
        }
        if ((ch == 13)||(ch == 10) || (c==(filesize-1))) {
            //for end of file without \n neither \r
            if (!((ch == 13)||(ch == 10)) && (c==(filesize-1))) {
                cmd+=(char)ch;
            }
            cmd.trim();
            if(cmd.length() > 0) {
                //ignore  comments
                if (cmd[0]!=';') {
                    //it is internal or not ?
                    if(esp3d_commands.is_esp_command((uint8_t *)cmd.c_str(), cmd.length())) {
                        esp3d_commands.process((uint8_t *)cmd.c_str(), cmd.length(), output, auth_type);
                    } else {
                        if (!sendCommand(cmd.c_str(),false, true)) {
                            log_esp3d("Error sending command");
                            //To stop instead of continue may need some trigger
                            res = false;
                        }
                    }
                }
                cmd="";
            }

        } else {
            cmd+=(char)ch;
        }
    }
    f.close();
    return res;
}

bool GcodeHost::processscript(const char * line)
{
    bool res = true;
    String s = line;
    s.trim();
    ESP3DOutput output(ESP_ALL_CLIENTS);
    if (s.startsWith(ESP_FLASH_FS_HEADER)) {
        res = processFile(line, LEVEL_ADMIN, &output);
    } else {
        res = processLine(line, LEVEL_ADMIN, &output);
    }
    return res;
}

//split line of command separated by '\n'
bool GcodeHost::processLine(const char * line, level_authenticate_type auth_type, ESP3DOutput * output)
{
    bool res = true;
    String s = "";
    for (uint p = 0; p < strlen(line); p++) {
        if ((line[p]==10) || (line[p]==13) || (p == (strlen(line)-1))) {
            if (!((line[p]==10) || (line[p]==13)) && (p == (strlen(line)-1))) {
                s+=line[p];
            }
            s.trim();
            if (s.length()>0) {
                //ignore  comments
                if (s[0]!=';') {
                    //it is internal or not ?
                    if(esp3d_commands.is_esp_command((uint8_t *)s.c_str(), s.length())) {
                        esp3d_commands.process((uint8_t *)s.c_str(), s.length(), output, auth_type);
                    } else {
                        //no check sum no ack
                        if (!sendCommand(s.c_str(),false, false)) {
                            log_esp3d("Error sending command");
                            //To stop instead of continue may need some trigger
                            res = false;
                        }
                    }
                }
            }
            s = "";
        } else {
            s+=line[p];
        }
    }
    return res;
}

bool GcodeHost::processFile(const char * filename, level_authenticate_type auth_type, ESP3DOutput * output)
{
    String FileName = filename;
    FileName.trim();
    log_esp3d("Processing: %s", FileName.c_str());
    if (FileName.startsWith(ESP_FLASH_FS_HEADER)) {
        String f = FileName.substring(strlen(ESP_FLASH_FS_HEADER),FileName.length());
        return processFSFile(f.c_str(), auth_type, output);
    }
    //TODO SD = SDCard
    //TODO UD = USB DISK
    log_esp3d("Invalid filename");
    return false;
}

#endif //ESP_GCODE_HOST_FEATURE
