/*
  command.cpp - ESP3D configuration class

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

#include "command.h"
#include "config.h"
#include "wifi.h"
#include "webinterface.h"
#ifdef SDCARD_FEATURE
#ifndef FS_NO_GLOBALS
#define FS_NO_GLOBALS
#endif
#endif
#include <FS.h>

String COMMAND::buffer_serial;
String COMMAND::buffer_tcp;

#define ERROR_CMD_MSG F("\nM117 Cmd Error")
#define INCORRECT_CMD_MSG F("\nM117 Incorrect Cmd")
#define OK_CMD_MSG F("\nM117 Cmd Ok")

String COMMAND::get_param(String & cmd_params, const char * id, bool withspace)
{
    static String parameter;
    String sid=id;
    int start;
    int end = -1;
    parameter = "";
    //if no id it means it is first part of cmd
    if (strlen(id) == 0) start = 0;
    //else find id position
    else start = cmd_params.indexOf(id);
    //if no id found and not first part leave
    if (start == -1 ) return parameter;
    //password and SSID can have space so handle it
    //if no space expected use space as delimiter
    if (!withspace)end = cmd_params.indexOf(" ",start);
    //if space expected only one parameter but additional password may be present
    else if (sid!="pwd=")end = cmd_params.indexOf("pwd=",start);
    //if no end found - take all
    if (end == -1) end = cmd_params.length();
    //extract parameter
    parameter = cmd_params.substring(start+strlen(id),end);
    //be sure no extra space
    parameter.trim();
    return parameter;
}
#ifdef AUTHENTICATION_FEATURE
bool COMMAND::isadmin(String & cmd_params)
{
    String adminpassword;
    String sadminPassword;
    if (!CONFIG::read_string(EP_ADMIN_PWD, sadminPassword , MAX_LOCAL_PASSWORD_LENGTH)) {
        LOG("ERROR getting admin\n")
        sadminPassword=FPSTR(DEFAULT_ADMIN_PWD);
        }
    adminpassword = get_param(cmd_params,"pwd=", true);
    if (!sadminPassword.equals(adminpassword)) {
            LOG("Not allowed \n")
            return false;
            }
    else return true;
}
#endif
void COMMAND::execute_command(int cmd,String cmd_params)
{
    //manage parameters
    byte mode = 254;
    String parameter;
    switch(cmd) {
    //STA SSID
    //[ESP100]<SSID>[pwd=<admin password>]
    case 100:
        parameter = get_param(cmd_params,"", true);
        if (!CONFIG::isSSIDValid(parameter.c_str()))Serial.println(INCORRECT_CMD_MSG);
#ifdef AUTHENTICATION_FEATURE
        if (!isadmin(cmd_params)) {
            Serial.println(INCORRECT_CMD_MSG);
            }
        else
#endif
        if(!CONFIG::write_string(EP_STA_SSID,parameter.c_str())) {
            Serial.println(ERROR_CMD_MSG);
        } else {
            Serial.println(OK_CMD_MSG);
        }
        break;
    //STA Password
    //[ESP101]<Password>[pwd=<admin password>]
    case 101:
        parameter = get_param(cmd_params,"", true);
        if (!CONFIG::isPasswordValid(parameter.c_str()))Serial.println(INCORRECT_CMD_MSG);
#ifdef AUTHENTICATION_FEATURE
        if (!isadmin(cmd_params)) {
            Serial.println(INCORRECT_CMD_MSG);
            }
        else
#endif
        if(!CONFIG::write_string(EP_STA_PASSWORD,parameter.c_str())) {
            Serial.println(ERROR_CMD_MSG);
        } else {
            Serial.println(OK_CMD_MSG);
        }
        break;
     //Hostname
     //[ESP102]<hostname>[pwd=<admin password>]
     case 102:
        parameter = get_param(cmd_params,"", true);
        if (!CONFIG::isHostnameValid(parameter.c_str()))Serial.println(INCORRECT_CMD_MSG);
#ifdef AUTHENTICATION_FEATURE
        if (!isadmin(cmd_params)) {
            Serial.println(INCORRECT_CMD_MSG);
            }
        else
#endif
        if(!CONFIG::write_string(EP_HOSTNAME,parameter.c_str())) {
            Serial.println(ERROR_CMD_MSG);
        } else {
            Serial.println(OK_CMD_MSG);
        }
        break;
    //Wifi mode (STA/AP)
    //[ESP103]<mode>[pwd=<admin password>]
    case 103:
        parameter = get_param(cmd_params,"", true);
        if (parameter == "STA") {
            mode = CLIENT_MODE;
        } else if (parameter == "AP") {
            mode = AP_MODE;
        } else {
            Serial.println(INCORRECT_CMD_MSG);
        }
        if ((mode == CLIENT_MODE) || (mode == AP_MODE)){
#ifdef AUTHENTICATION_FEATURE
            if (!isadmin(cmd_params)) {
                Serial.println(INCORRECT_CMD_MSG);
            }
            else
#endif
            if(!CONFIG::write_byte(EP_WIFI_MODE,mode)) {
                Serial.println(ERROR_CMD_MSG);
            } else {
                Serial.println(OK_CMD_MSG);
            }
        }
        break;
    //STA IP mode (DHCP/STATIC)
    //[ESP104]<mode>[pwd=<admin password>]
    case 104:
        parameter = get_param(cmd_params,"", true);
        if (parameter == "STATIC") {
            mode = STATIC_IP_MODE;
        } else if (parameter == "DHCP") {
            mode = DHCP_MODE;
        } else{
            Serial.println(INCORRECT_CMD_MSG);
        }
        if ((mode == STATIC_IP_MODE) || (mode == DHCP_MODE)){
#ifdef AUTHENTICATION_FEATURE
            if (!isadmin(cmd_params)) {
                Serial.println(INCORRECT_CMD_MSG);
            }
            else
#endif
            if(!CONFIG::write_byte(EP_STA_IP_MODE,mode)) {
                Serial.println(ERROR_CMD_MSG);
            } else {
                Serial.println(OK_CMD_MSG);
            }
        }
        break;
    //AP SSID
    //[ESP105]<SSID>[pwd=<admin password>]
    case 105:
        parameter = get_param(cmd_params,"", true);
        if (!CONFIG::isSSIDValid(parameter.c_str()))Serial.println(INCORRECT_CMD_MSG);
#ifdef AUTHENTICATION_FEATURE
        if (!isadmin(cmd_params)) {
            Serial.println(INCORRECT_CMD_MSG);
            }
        else
#endif
        if(!CONFIG::write_string(EP_AP_SSID,parameter.c_str())) {
            Serial.println(ERROR_CMD_MSG);
        } else {
            Serial.println(OK_CMD_MSG);
        }
        break;
    //AP Password
    //[ESP106]<Password>[pwd=<admin password>]
    case 106:
        parameter = get_param(cmd_params,"", true);
        if (!CONFIG::isPasswordValid(parameter.c_str()))Serial.println(INCORRECT_CMD_MSG);
#ifdef AUTHENTICATION_FEATURE
        if (!isadmin(cmd_params)) {
            Serial.println(INCORRECT_CMD_MSG);
            }
        else
#endif
        if(!CONFIG::write_string(EP_AP_PASSWORD,parameter.c_str())) {
            Serial.println(ERROR_CMD_MSG);
        } else {
            Serial.println(OK_CMD_MSG);
        }
        break;
    //AP IP mode (DHCP/STATIC)
    //[ESP107]<mode>[pwd=<admin password>]
    case 107:
        parameter = get_param(cmd_params,"", true);
        if (parameter == "STATIC") {
            mode = STATIC_IP_MODE;
        } else if (parameter == "DHCP") {
            mode = DHCP_MODE;
        } else{
            Serial.println(INCORRECT_CMD_MSG);
        }
        if ((mode == STATIC_IP_MODE) || (mode == DHCP_MODE)){
#ifdef AUTHENTICATION_FEATURE
            if (!isadmin(cmd_params)) {
                Serial.println(INCORRECT_CMD_MSG);
            }
            else
#endif
            if(!CONFIG::write_byte(EP_AP_IP_MODE,mode)) {
                Serial.println(ERROR_CMD_MSG);
            } else {
                Serial.println(OK_CMD_MSG);
            }
        }
        break;
    //Get current IP
    //[ESP111]<header answer>
    case 111: {
        String currentIP ;
        if (WiFi.getMode()==WIFI_STA) {
            currentIP=WiFi.localIP().toString();
        } else {
            currentIP=WiFi.softAPIP().toString();
        }
        Serial.print("\n\r");
        Serial.print(cmd_params);
        Serial.println(currentIP);
        Serial.print("\r\n");
    }
    break;
    //Get hostname
    //[ESP112]<header answer>
    case 112: {
        String shost ;
         if (!CONFIG::read_string(EP_HOSTNAME, shost , MAX_HOSTNAME_LENGTH)) {
            shost=wifi_config.get_default_hostname();
            }
        Serial.print("\n\r");
        Serial.print(cmd_params);
        Serial.println(shost);
        Serial.print("\r\n");
    }
    break;

#ifdef DIRECT_PIN_FEATURE
	//Get/Set pin value
	//[ESP201]P<pin> V<value>
    case 201: {
		//check if have pin
        parameter = get_param(cmd_params,"P", true);
        LOG(parameter)
        LOG("\n")
        if (parameter == "")
			{
				Serial.println(INCORRECT_CMD_MSG);
			}
		else{
			int pin = parameter.toInt();
			//check pin is valid and not serial used pins
			if ((pin >= 0) && (pin <= 16) && !((pin == 1) || (pin == 3)))
				{
					//check if is set or get
					parameter = get_param(cmd_params,"V", true);
					//it is a get
					if (parameter == "")
						{   //GPIO16 is different than
							if (pin <16) pinMode(pin, INPUT_PULLUP);
							else pinMode(pin, INPUT_PULLDOWN_16);
							delay(10);
							int value =	 digitalRead(pin);
							Serial.println(String(value));
						}
					else{
						//it is a set
						int value = parameter.toInt();
						//verify it is a 0 or a 1
						if ((value == 0) || (value == 1))
							{
								 pinMode(pin, OUTPUT);
								 delay(10);
								 digitalWrite(pin, (value == 0)?LOW:HIGH);
							}
						else Serial.println(INCORRECT_CMD_MSG);
						}
				}
			else Serial.println(INCORRECT_CMD_MSG);
			}
		}
		break;
#endif

    //Get/Set ESP mode
    //cmd is RESET, SAFEMODE, CONFIG, RESTART
    //[ESP444]<cmd>pwd=<admin password>
    case 444:
		parameter = get_param(cmd_params,"", true);
#ifdef AUTHENTICATION_FEATURE
        if (!isadmin(cmd_params)) {
            Serial.println(INCORRECT_CMD_MSG);
            }
        else
#endif
		{
			if (parameter=="RESET") {
				CONFIG::reset_config();
			}
			if (parameter=="SAFEMODE") {
				wifi_config.Safe_Setup();
			}
			if (parameter=="RESTART") {
				CONFIG::esp_restart();
			}
		}
        if (parameter=="CONFIG") {
            CONFIG::print_config();
        }
        break;
#ifdef AUTHENTICATION_FEATURE
     //Change / Reset user password
     //[ESP555]<password>pwd=<admin password>
     case 555:
        {
        if (isadmin(cmd_params)) {
            parameter = get_param(cmd_params,"", true);
            if (parameter.length() == 0){
                if(CONFIG::write_string(EP_USER_PWD,FPSTR(DEFAULT_USER_PWD))) {
                    Serial.println(OK_CMD_MSG);
                }
                else {
                    Serial.println(ERROR_CMD_MSG);
                }
            } else {
                if (CONFIG::isLocalPasswordValid(parameter.c_str())){
                    if(CONFIG::write_string(EP_USER_PWD,parameter.c_str())) {
                    Serial.println(OK_CMD_MSG);
                }
                else {
                    Serial.println(ERROR_CMD_MSG);
                }
                }
                else Serial.println(INCORRECT_CMD_MSG);
            }
        }
        else Serial.println(INCORRECT_CMD_MSG);
        break;
        }
#endif
    //[ESP700]<filename>
    case 700: //read local file
        {//be sure serial is locked
        if ((web_interface->blockserial)) break;
        cmd_params.trim() ;
        if ((cmd_params.length() > 0) && (cmd_params[0] != '/')) cmd_params = "/" + cmd_params;
        FSFILE currentfile = SPIFFS.open(cmd_params, "r");
        if (currentfile) {//if file open success
            //flush to be sure send buffer is empty
            Serial.flush();
            //read content
            String currentline = currentfile.readString();
            //until no line in file
            while (currentline.length() >0)
                {   //send line to serial
                    Serial.println(currentline);
                    //flush to be sure send buffer is empty
                    delay(0);
                    Serial.flush();
                    currentline="";
                    //read next line if any
                    currentline = currentfile.readString();
                }
            currentfile.close();
            }
        break;
        }
    //get fw version
    //[ESP800]<header answer>
    case 800:
        Serial.print(cmd_params);
        Serial.print("\nFW version:");
        Serial.println(FW_VERSION);
        break;
    //clear status/error/info list
    //[ESP999]<cmd>
    case 999:
        cmd_params.trim();
#ifdef ERROR_MSG_FEATURE
        if (cmd_params=="ERROR") {
            web_interface->error_msg.clear();
        }
#endif
#ifdef INFO_MSG_FEATURE
        if (cmd_params=="INFO") {
            web_interface->info_msg.clear();
        }
#endif
#ifdef STATUS_MSG_FEATURE
        if (cmd_params=="STATUS") {
            web_interface->status_msg.clear();
        }
#endif
        if (cmd_params=="ALL") {
#ifdef ERROR_MSG_FEATURE
            web_interface->error_msg.clear();
#endif
#ifdef STATUS_MSG_FEATURE
            web_interface->status_msg.clear();
#endif
#ifdef INFO_MSG_FEATURE
            web_interface->info_msg.clear();
#endif
        }
        break;
        //default:

    }
}

void COMMAND::check_command(String buffer)
{
    String buffer2;
//if direct access to SDCard no need to handle the M20 command answer
#ifndef DIRECT_SDCARD_FEATURE
    static bool bfileslist=false;
    static uint32_t start_list=0;
    //if SD list is not on going
    if (!bfileslist) {
        //check if command is a start of SD File list
        int filesstart = buffer.indexOf("Begin file list");
        //yes it is file list starting to be displayed
        if (filesstart>-1) {
            //init time out
            start_list = millis();
            //set file list started
            bfileslist=true;
            //clear current list
            web_interface->fileslist.clear();
            //block any new output to serial from ESP to avoid pollution
            (web_interface->blockserial) = true;
            return;
        }
#endif
#ifdef TEMP_MONITORING_FEATURE
        int Tpos = buffer.indexOf("T:");
#endif
#ifdef POS_MONITORING_FEATURE
        int Xpos = buffer.indexOf("X:");
        int Ypos = buffer.indexOf("Y:");
        int Zpos = buffer.indexOf("Z:");
#endif
#if FIRMWARE_TARGET == SMOOTHIEWARE
#ifdef SPEED_MONITORING_FEATURE
        int Speedpos = buffer.indexOf("Speed factor at ");
#endif
#ifdef FLOW_MONITORING_FEATURE
        int Flowpos = buffer.indexOf("Flow rate at ");
#endif
#ifdef ERROR_MSG_FEATURE
        int Errorpos= buffer.indexOf("error:");
#endif
#ifdef INFO_MSG_FEATURE
        int Infopos= buffer.indexOf("info:");
#endif
#ifdef STATUS_MSG_FEATURE
        int Statuspos= buffer.indexOf("warning:");
#endif
#else
#ifdef SPEED_MONITORING_FEATURE
        int Speedpos = buffer.indexOf("SpeedMultiply:");
#endif
#ifdef FLOW_MONITORING_FEATURE
        int Flowpos = buffer.indexOf("FlowMultiply:");
#endif
#ifdef ERROR_MSG_FEATURE
        int Errorpos= buffer.indexOf("Error:");
#endif
#ifdef INFO_MSG_FEATURE
        int Infopos= buffer.indexOf("Info:");
#endif
#ifdef STATUS_MSG_FEATURE
        int Statuspos= buffer.indexOf("Status:");
#endif
#endif

#ifdef SERIAL_COMMAND_FEATURE
        String ESP_Command;
        int ESPpos = buffer.indexOf("[ESP");
        if (ESPpos>-1) {
            //is there the second part?
            int ESPpos2 = buffer.indexOf("]",ESPpos);
            if (ESPpos2>-1) {
                //Split in command and parameters
                String cmd_part1=buffer.substring(ESPpos+4,ESPpos2);
                String cmd_part2="";
                //is there space for parameters?
                if (ESPpos2<buffer.length()) {
                    cmd_part2=buffer.substring(ESPpos2+1);
                }
                //if command is a valid number then execute command
                if(cmd_part1.toInt()!=0) {
                    execute_command(cmd_part1.toInt(),cmd_part2);
                }
                //if not is not a valid [ESPXXX] command
            }
        }
#endif
#ifdef TEMP_MONITORING_FEATURE
        //check for temperature
        if (Tpos>-1) {
            //look for valid temperature answer
            int slashpos = buffer.indexOf(" /",Tpos);
            int spacepos = buffer.indexOf(" ",slashpos+1);
            //if match mask T:xxx.xx /xxx.xx
            if(spacepos-Tpos < 17) {
                web_interface->answer4M105=buffer; //do not interprete just need when requested so store it
                web_interface->last_temp=millis();
            }
        }
#endif
#ifdef POS_MONITORING_FEATURE
        //Position of axis
        if (Xpos>-1 && Ypos>-1 && Zpos>-1) {
            web_interface->answer4M114=buffer;
        }
#endif
#ifdef SPEED_MONITORING_FEATURE
        //Speed
        if (Speedpos>-1) {
            //get just the value
#if FIRMWARE_TARGET == SMOOTHIEWARE
            buffer2 =buffer.substring(Speedpos+16);
            int p2 = buffer2.indexOf(".");
            web_interface->answer4M220=buffer2.substring(0,p2);
#else
            web_interface->answer4M220=buffer.substring(Speedpos+14);
#endif
        }
#endif
#ifdef FLOW_MONITORING_FEATURE
        //Flow
        if (Flowpos>-1) {
            //get just the value
#if FIRMWARE_TARGET == SMOOTHIEWARE
            buffer2 =buffer.substring(Flowpos+13);
            int p2 = buffer2.indexOf(".");
            web_interface->answer4M221=buffer2.substring(0,p2);
#else
            web_interface->answer4M221=buffer.substring(Flowpos+13);
#endif
        }
#endif
#ifdef ERROR_MSG_FEATURE
        //Error
        if (Errorpos>-1 && !(buffer.indexOf("Format error")!=-1 || buffer.indexOf("wait")==Errorpos+6) ) {
            (web_interface->error_msg).add(buffer.substring(Errorpos+6).c_str());
        }
#endif
#ifdef INFO_MSG_FEATURE
        //Info
        if (Infopos>-1) {
            (web_interface->info_msg).add(buffer.substring(Infopos+5).c_str());
        }
#endif
#ifdef STATUS_MSG_FEATURE
        //Status
        if (Statuspos>-1) {
#if FIRMWARE_TARGET == SMOOTHIEWARE
            (web_interface->status_msg).add(buffer.substring(Statuspos+8).c_str());
#else
            (web_interface->status_msg).add(buffer.substring(Statuspos+7).c_str());
#endif
        }
#endif
#ifndef DIRECT_SDCARD_FEATURE
    } else { //listing file is on going
        //check if we are too long
        if ((millis()-start_list)>30000) { //timeout in case of problem
            bfileslist=false;
            (web_interface->blockserial) = false; //release serial
            LOG("Time out\n");
        } else {
            //check if this is the end
            if (buffer.indexOf("End file list")>-1) {
                bfileslist=false;
                (web_interface->blockserial) = false;
                LOG("End list\n");
            } else {
                //Serial.print(buffer);
                //add list to buffer
                web_interface->fileslist.add(buffer);
                LOG(String(web_interface->fileslist.size()));
                LOG(":");
                LOG(buffer);
                LOG('\n');
            }
        }
    }
#endif
}

//read a buffer in an array
void COMMAND::read_buffer_serial(uint8_t *b, size_t len)
{
    for (long i = 0; i< len; i++) {
        read_buffer_serial(*b);
        *b++;
    }
}

//read buffer as char
void COMMAND::read_buffer_tcp(uint8_t b)
{
    static bool previous_was_char=false;
    static bool iscomment=false;
//to ensure it is continuous string, no char separated by binaries
    if (!previous_was_char) {
        buffer_tcp="";
        iscomment = false;
    }
//is comment ?
if (char(b) == ';') iscomment = true;
//it is a char so add it to buffer
    if (isPrintable(b)) {
        previous_was_char=true;
        //add char if not a comment
        if (!iscomment)buffer_tcp+=char(b);
    } else {
        previous_was_char=false; //next call will reset the buffer
    }
//this is not printable but end of command check if need to handle it
    if (b==13 ||b==10) {
		//reset comment flag
		iscomment = false;
        //Minimum is something like M10 so 3 char
        if (buffer_tcp.length()>3) {
            check_command(buffer_tcp);
        }
    }
}

//read buffer as char
void COMMAND::read_buffer_serial(uint8_t b)
{
    static bool previous_was_char=false;
    static bool iscomment=false;
//to ensure it is continuous string, no char separated by binaries
    if (!previous_was_char) {
        buffer_serial="";
        iscomment = false;
    }
//is comment ?
if (char(b) == ';') iscomment = true;
//it is a char so add it to buffer
    if (isPrintable(b)) {
        previous_was_char=true;
        if (!iscomment)buffer_serial+=char(b);
    } else {
        previous_was_char=false; //next call will reset the buffer
    }
//this is not printable but end of command check if need to handle it
    if (b==13) {
		//reset comment flag
		iscomment = false;
        //Minimum is something like M10 so 3 char
        if (buffer_serial.length()>3) {
            check_command(buffer_serial);
        }
    }
}
