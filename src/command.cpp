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
#include "config.h"
#include "command.h"
#include "wificonf.h"
#include "webinterface.h"
#ifndef FS_NO_GLOBALS
#define FS_NO_GLOBALS
#endif
#include <FS.h>
#if defined(ARDUINO_ARCH_ESP32)
#include "SPIFFS.h"
#define MAX_GPIO 16
#else
#define MAX_GPIO 37
#endif

String COMMAND::buffer_serial;
String COMMAND::buffer_tcp;

#define ERROR_CMD_MSG (output == WEB_PIPE)?F("Error: Wrong Command"):F("M117 Cmd Error")
#define INCORRECT_CMD_MSG (output == WEB_PIPE)?F("Error: Incorrect Command"):F("M117 Incorrect Cmd")
#define OK_CMD_MSG (output == WEB_PIPE)?F("ok"):F("M117 Cmd Ok")

String COMMAND::get_param (String & cmd_params, const char * id, bool withspace)
{
    static String parameter;
    String sid = id;
    int start;
    int end = -1;
    parameter = "";
    //if no id it means it is first part of cmd
    if (strlen (id) == 0) {
        start = 0;
    }
    //else find id position
    else {
        start = cmd_params.indexOf (id);
    }
    //if no id found and not first part leave
    if (start == -1 ) {
        return parameter;
    }
    //password and SSID can have space so handle it
    //if no space expected use space as delimiter
    if (!withspace) {
        end = cmd_params.indexOf (" ", start);
    }
#ifdef AUTHENTICATION_FEATURE
    //if space expected only one parameter but additional password may be present
    else if (sid != " pwd=") {
        end = cmd_params.indexOf (" pwd=", start);
    }
#endif
    //if no end found - take all
    if (end == -1) {
        end = cmd_params.length();
    }
    //extract parameter
    parameter = cmd_params.substring (start + strlen (id), end);
    //be sure no extra space
    parameter.trim();
    return parameter;
}
#ifdef AUTHENTICATION_FEATURE
//check admin password
bool COMMAND::isadmin (String & cmd_params)
{
    String adminpassword;
    String sadminPassword;
    if (!CONFIG::read_string (EP_ADMIN_PWD, sadminPassword, MAX_LOCAL_PASSWORD_LENGTH) ) {
        LOG ("ERROR getting admin\r\n")
        sadminPassword = FPSTR (DEFAULT_ADMIN_PWD);
    }
    adminpassword = get_param (cmd_params, "pwd=", true);
    if (!sadminPassword.equals (adminpassword) ) {
        LOG("Not identified from command line\r\n")
        return false;
    } else {
        return true;
    }
}
//check user password - admin password is also valid
bool COMMAND::isuser (String & cmd_params)
{
    String userpassword;
    String suserPassword;
    if (!CONFIG::read_string (EP_USER_PWD, suserPassword, MAX_LOCAL_PASSWORD_LENGTH) ) {
        LOG ("ERROR getting user\r\n")
        suserPassword = FPSTR (DEFAULT_USER_PWD);
    }
    userpassword = get_param (cmd_params, "pwd=", true);
    //it is not user password
    if (!suserPassword.equals (userpassword) ) {
        //check admin password
        return COMMAND::isadmin (cmd_params);
    } else {
        return true;
    }
}
#endif
bool COMMAND::execute_command (int cmd, String cmd_params, tpipe output, level_authenticate_type auth_level, AsyncResponseStream  *asyncresponse)
{
    bool response = true;
    level_authenticate_type auth_type = auth_level;
#ifdef AUTHENTICATION_FEATURE
    
    if (isadmin(cmd_params)) {
        auth_type = LEVEL_ADMIN;
        LOG("you are Admin\r\n");
    }
    if (isuser (cmd_params) && (auth_type != LEVEL_ADMIN) ) {
        auth_type = LEVEL_USER;
        LOG("you are User\r\n");
    }
#ifdef DEBUG_ESP3D
    if ( auth_type == LEVEL_ADMIN)  
        {
            LOG("admin identified\r\n");
        }
    else  {
        if( auth_type == LEVEL_USER)  
            {
                LOG("user identified\r\n");
            }
        else  
            {
                LOG("guest identified\r\n");
            }
        }
#endif
#endif
    //manage parameters
    byte mode = 254;
    String parameter;
    LOG ("Execute Command\r\n")
    switch (cmd) {
    //STA SSID
    //[ESP100]<SSID>[pwd=<admin password>]
    case 100:
        parameter = get_param (cmd_params, "", true);
        if (!CONFIG::isSSIDValid (parameter.c_str() ) ) {
            BRIDGE::println (INCORRECT_CMD_MSG, output, asyncresponse);
        }
#ifdef AUTHENTICATION_FEATURE
        if (auth_type != LEVEL_ADMIN) {
            BRIDGE::println (INCORRECT_CMD_MSG, output, asyncresponse);
        } else
#endif
            if (!CONFIG::write_string (EP_STA_SSID, parameter.c_str() ) ) {
                BRIDGE::println (ERROR_CMD_MSG, output, asyncresponse);
                response = false;
            } else {
                BRIDGE::println (OK_CMD_MSG, output, asyncresponse);
            }
        break;
    //STA Password
    //[ESP101]<Password>[pwd=<admin password>]
    case 101:
        parameter = get_param (cmd_params, "", true);
        if (!CONFIG::isPasswordValid (parameter.c_str() ) ) {
            BRIDGE::println (INCORRECT_CMD_MSG, output, asyncresponse);
            response = false;
        }
#ifdef AUTHENTICATION_FEATURE
        if (auth_type != LEVEL_ADMIN) {
            BRIDGE::println (INCORRECT_CMD_MSG, output, asyncresponse);
            response = false;
        } else
#endif
            if (!CONFIG::write_string (EP_STA_PASSWORD, parameter.c_str() ) ) {
                BRIDGE::println (ERROR_CMD_MSG, output, asyncresponse);
                response = false;
            } else {
                BRIDGE::println (OK_CMD_MSG, output, asyncresponse);
            }
        break;
    //Hostname
    //[ESP102]<hostname>[pwd=<admin password>]
    case 102:
        parameter = get_param (cmd_params, "", true);
        if (!CONFIG::isHostnameValid (parameter.c_str() ) ) {
            BRIDGE::println (INCORRECT_CMD_MSG, output, asyncresponse);
            response = false;
        }
#ifdef AUTHENTICATION_FEATURE
        if (auth_type != LEVEL_ADMIN) {
            BRIDGE::println (INCORRECT_CMD_MSG, output, asyncresponse);
            response = false;
        } else
#endif
            if (!CONFIG::write_string (EP_HOSTNAME, parameter.c_str() ) ) {
                BRIDGE::println (ERROR_CMD_MSG, output, asyncresponse);
                response = false;
            } else {
                BRIDGE::println (OK_CMD_MSG, output, asyncresponse);
            }
        break;
    //Wifi mode (STA/AP)
    //[ESP103]<mode>[pwd=<admin password>]
    case 103:
        parameter = get_param (cmd_params, "", true);
        if (parameter == "STA") {
            mode = CLIENT_MODE;
        } else if (parameter == "AP") {
            mode = AP_MODE;
        } else {
            BRIDGE::println (INCORRECT_CMD_MSG, output, asyncresponse);
            response = false;
        }
        if ( (mode == CLIENT_MODE) || (mode == AP_MODE) ) {
#ifdef AUTHENTICATION_FEATURE
            if (auth_type != LEVEL_ADMIN) {
                BRIDGE::println (INCORRECT_CMD_MSG, output, asyncresponse);
                response = false;
            } else
#endif
                if (!CONFIG::write_byte (EP_WIFI_MODE, mode) ) {
                    BRIDGE::println (ERROR_CMD_MSG, output, asyncresponse);
                    response = false;
                } else {
                    BRIDGE::println (OK_CMD_MSG, output, asyncresponse);
                }
        }
        break;
    //STA IP mode (DHCP/STATIC)
    //[ESP104]<mode>[pwd=<admin password>]
    case 104:
        parameter = get_param (cmd_params, "", true);
        if (parameter == "STATIC") {
            mode = STATIC_IP_MODE;
        } else if (parameter == "DHCP") {
            mode = DHCP_MODE;
        } else {
            BRIDGE::println (INCORRECT_CMD_MSG, output, asyncresponse);
            response = false;
        }
        if ( (mode == STATIC_IP_MODE) || (mode == DHCP_MODE) ) {
#ifdef AUTHENTICATION_FEATURE
            if (auth_type != LEVEL_ADMIN) {
                BRIDGE::println (INCORRECT_CMD_MSG, output, asyncresponse);
                response = false;
            } else
#endif
                if (!CONFIG::write_byte (EP_STA_IP_MODE, mode) ) {
                    BRIDGE::println (ERROR_CMD_MSG, output, asyncresponse);
                    response = false;
                } else {
                    BRIDGE::println (OK_CMD_MSG, output, asyncresponse);
                }
        }
        break;
    //AP SSID
    //[ESP105]<SSID>[pwd=<admin password>]
    case 105:
        parameter = get_param (cmd_params, "", true);
        if (!CONFIG::isSSIDValid (parameter.c_str() ) ) {
            BRIDGE::println (INCORRECT_CMD_MSG, output, asyncresponse);
            response = false;
        }
#ifdef AUTHENTICATION_FEATURE
        if (auth_type != LEVEL_ADMIN) {
            BRIDGE::println (INCORRECT_CMD_MSG, output, asyncresponse);
            response = false;
        } else
#endif
            if (!CONFIG::write_string (EP_AP_SSID, parameter.c_str() ) ) {
                BRIDGE::println (ERROR_CMD_MSG, output, asyncresponse);
                response = false;
            } else {
                BRIDGE::println (OK_CMD_MSG, output, asyncresponse);
            }
        break;
    //AP Password
    //[ESP106]<Password>[pwd=<admin password>]
    case 106:
        parameter = get_param (cmd_params, "", true);
        if (!CONFIG::isPasswordValid (parameter.c_str() ) ) {
            BRIDGE::println (INCORRECT_CMD_MSG, output, asyncresponse);
            response = false;
        }
#ifdef AUTHENTICATION_FEATURE
        if (auth_type != LEVEL_ADMIN) {
            BRIDGE::println (INCORRECT_CMD_MSG, output, asyncresponse);
            response = false;
        } else
#endif
            if (!CONFIG::write_string (EP_AP_PASSWORD, parameter.c_str() ) ) {
                BRIDGE::println (ERROR_CMD_MSG, output, asyncresponse);
                response = false;
            } else {
                BRIDGE::println (OK_CMD_MSG, output, asyncresponse);
            }
        break;
    //AP IP mode (DHCP/STATIC)
    //[ESP107]<mode>[pwd=<admin password>]
    case 107:
        parameter = get_param (cmd_params, "", true);
        if (parameter == "STATIC") {
            mode = STATIC_IP_MODE;
        } else if (parameter == "DHCP") {
            mode = DHCP_MODE;
        } else {
            BRIDGE::println (INCORRECT_CMD_MSG, output, asyncresponse);
            response = false;
        }
        if ( (mode == STATIC_IP_MODE) || (mode == DHCP_MODE) ) {
#ifdef AUTHENTICATION_FEATURE
            if (auth_type != LEVEL_ADMIN) {
                BRIDGE::println (INCORRECT_CMD_MSG, output, asyncresponse);
                response = false;
            } else
#endif
                if (!CONFIG::write_byte (EP_AP_IP_MODE, mode) ) {
                    BRIDGE::println (ERROR_CMD_MSG, output, asyncresponse);
                    response = false;
                } else {
                    BRIDGE::println (OK_CMD_MSG, output, asyncresponse);
                }
        }
        break;
    // Set wifi on/off
    //[ESP110]<state>[pwd=<admin password>]
    case 110:
        parameter = get_param (cmd_params, "", true);
        if (parameter == "on") {
            mode = 1;
        } else if (parameter == "off") {
            mode = 0;
        } else if (parameter == "restart") {
            mode = 2;
        } else {
            BRIDGE::println (INCORRECT_CMD_MSG, output, asyncresponse);
            response = false;
        }
        if (response) {
#ifdef AUTHENTICATION_FEATURE
            if (auth_type != LEVEL_ADMIN) {
                BRIDGE::println (INCORRECT_CMD_MSG, output, asyncresponse);
                response = false;
            } else
#endif
                if (mode == 0) {
                    if (WiFi.getMode() != WIFI_OFF) {
                        //disable wifi
                        ESP_SERIAL_OUT.println ("M117 Disabling Wifi");
                        WiFi.mode (WIFI_OFF);
                        wifi_config.Disable_servers();
                        return response;
                    } else {
                        BRIDGE::println ("M117 Wifi already off", output, asyncresponse);
                    }
                } else if (mode == 1) { //restart device is the best way to start everything clean
                    if (WiFi.getMode() == WIFI_OFF) {
                        ESP_SERIAL_OUT.println ("M117 Enabling Wifi");
                        web_interface->restartmodule = true;
                    } else {
                        BRIDGE::println ("M117 Wifi already on", output, asyncresponse);
                    }
                } else  { //restart wifi and restart is the best way to start everything clean
                    ESP_SERIAL_OUT.println ("M117 Enabling Wifi");
                    web_interface->restartmodule = true;
                }
        }
        break;
    //Get current IP
    //[ESP111]<header answer>
    case 111: {
        String currentIP ;
        if (WiFi.getMode() == WIFI_STA) {
            currentIP = WiFi.localIP().toString();
        } else {
            currentIP = WiFi.softAPIP().toString();
        }
        BRIDGE::print (cmd_params, output, asyncresponse);
        BRIDGE::println (currentIP, output, asyncresponse);
        LOG (cmd_params)
        LOG (currentIP)
        LOG ("\r\n")
    }
    break;
    //Get hostname
    //[ESP112]<header answer>
    case 112: {
        String shost ;
        if (!CONFIG::read_string (EP_HOSTNAME, shost, MAX_HOSTNAME_LENGTH) ) {
            shost = wifi_config.get_default_hostname();
        }
        BRIDGE::print (cmd_params, output, asyncresponse);
        BRIDGE::println (shost, output, asyncresponse);
        LOG (cmd_params)
        LOG (shost)
        LOG ("\r\n")
    }
    break;

#ifdef DIRECT_PIN_FEATURE
    //Get/Set pin value
    //[ESP201]P<pin> V<value> [PULLUP=YES RAW=YES]pwd=<admin password>
    case 201:
        parameter = get_param (cmd_params, "", true);
#ifdef AUTHENTICATION_FEATURE
        if (auth_type == LEVEL_GUEST) {
            BRIDGE::println (INCORRECT_CMD_MSG, output, asyncresponse);
            response = false;
        } else
#endif
        {
            //check if have pin
            parameter = get_param (cmd_params, "P", false);
            LOG ("Pin:")
            LOG (parameter)
            LOG ("\r\n")
            if (parameter == "") {
                BRIDGE::println (INCORRECT_CMD_MSG, output, asyncresponse);
                response = false;
            } else {
                int pin = parameter.toInt();
                //check pin is valid and not serial used pins
                if ( (pin >= 0) && (pin <= 16) && ! ( (pin == 1) || (pin == 3) ) ) {
                    //check if is set or get
                    parameter = get_param (cmd_params, "V", false);
                    //it is a get
                    if (parameter == "") {
                        //this is to not set pin mode
                        parameter = get_param (cmd_params, "RAW=", false);
                        if (parameter != "YES") {
                            parameter = get_param (cmd_params, "PULLUP=", false);
                            if (parameter == "YES") {
                                //GPIO16 is different than others
                                if (pin < MAX_GPIO) {
                                    LOG ("Set as input pull up\r\n")
                                    pinMode (pin, INPUT_PULLUP);
                                }
#ifdef ARDUINO_ARCH_ESP8266
                                else {
                                    LOG ("Set as input pull down 16\r\n")
                                    pinMode (pin, INPUT_PULLDOWN_16);
                                }
#endif
                            } else {
                                LOG ("Set as input\r\n")
                                pinMode (pin, INPUT);
                            }
                        }
                        int value = digitalRead (pin);
                        LOG ("Read:");
                        LOG (String (value).c_str() )
                        LOG ("\n");
                        BRIDGE::println (String (value).c_str(), output, asyncresponse);
                    } else {
                        //it is a set
                        int value = parameter.toInt();
                        //verify it is a 0 or a 1
                        if ( (value == 0) || (value == 1) ) {
                            pinMode (pin, OUTPUT);
                            LOG ("Set:")
                            LOG (String ( (value == 0) ? LOW : HIGH) )
                            LOG ("\r\n")
                            digitalWrite (pin, (value == 0) ? LOW : HIGH);
                            BRIDGE::println (OK_CMD_MSG, output, asyncresponse);
                        } else {
                            BRIDGE::println (INCORRECT_CMD_MSG, output, asyncresponse);
                            response = false;
                        }
                    }
                } else {
                    BRIDGE::println (INCORRECT_CMD_MSG, output, asyncresponse);
                    response = false;
                }
            }
        }
        break;
#endif

	//Get full EEPROM settings content
    //[ESP400]
    case 400: {
        char sbuf[MAX_DATA_LENGTH + 1];
        uint8_t ipbuf[4];
        byte bbuf = 0;
        int ibuf = 0;
        parameter = get_param (cmd_params, "", true);
        //Start JSON
        BRIDGE::println (F ("{\"EEPROM\":["), output, asyncresponse);
        if (cmd_params == "network" || cmd_params == "") {

            //1- Baud Rate
            BRIDGE::print (F ("{\"F\":\"network\",\"P\":\""), output, asyncresponse);
            BRIDGE::print ( (const char *) CONFIG::intTostr (EP_BAUD_RATE), output, asyncresponse);
            BRIDGE::print (F ("\",\"T\":\"I\",\"V\":\""), output, asyncresponse);
            if (!CONFIG::read_buffer (EP_BAUD_RATE,  (byte *) &ibuf, INTEGER_LENGTH) ) {
                BRIDGE::print ("???", output, asyncresponse);
            } else {
                BRIDGE::print ( (const char *) CONFIG::intTostr (ibuf), output, asyncresponse);
            }
            BRIDGE::print (F ("\",\"H\":\"Baud Rate\",\"O\":[{\"9600\":\"9600\"},{\"19200\":\"19200\"},{\"38400\":\"38400\"},{\"57600\":\"57600\"},{\"115200\":\"115200\"},{\"230400\":\"230400\"},{\"250000\":\"250000\"}]}"), output, asyncresponse);
            BRIDGE::println (F (","), output, asyncresponse);

            //2-Sleep Mode
            BRIDGE::print (F ("{\"F\":\"network\",\"P\":\""), output, asyncresponse);
            BRIDGE::print ( (const char *) CONFIG::intTostr (EP_SLEEP_MODE), output, asyncresponse);
            BRIDGE::print (F ("\",\"T\":\"B\",\"V\":\""), output, asyncresponse);
            if (!CONFIG::read_byte (EP_SLEEP_MODE, &bbuf ) ) {
                BRIDGE::print ("???", output, asyncresponse);
            } else {
                BRIDGE::print ( (const char *) CONFIG::intTostr (bbuf), output, asyncresponse);
            }
            BRIDGE::print (F ("\",\"H\":\"Sleep Mode\",\"O\":[{\"None\":\""), output, asyncresponse);
            BRIDGE::print ( (const char *) CONFIG::intTostr (WIFI_NONE_SLEEP), output, asyncresponse);
            BRIDGE::print (F ("\"},{\"Light\":\""), output, asyncresponse);
            BRIDGE::print ( (const char *) CONFIG::intTostr (WIFI_LIGHT_SLEEP), output, asyncresponse);
            BRIDGE::print (F ("\"},{\"Modem\":\""), output, asyncresponse);
            BRIDGE::print ( (const char *) CONFIG::intTostr (WIFI_MODEM_SLEEP), output, asyncresponse);
            BRIDGE::print (F ("\"}]}"), output, asyncresponse);
            BRIDGE::println (F (","), output, asyncresponse);

            //3-Web Port
            BRIDGE::print (F ("{\"F\":\"network\",\"P\":\""), output, asyncresponse);
            BRIDGE::print ( (const char *) CONFIG::intTostr (EP_WEB_PORT), output, asyncresponse);
            BRIDGE::print (F ("\",\"T\":\"I\",\"V\":\""), output, asyncresponse);
            if (!CONFIG::read_buffer (EP_WEB_PORT,  (byte *) &ibuf, INTEGER_LENGTH) ) {
                BRIDGE::print ("???", output, asyncresponse);
            } else {
                BRIDGE::print ( (const char *) CONFIG::intTostr (ibuf), output, asyncresponse);
            }
            BRIDGE::print (F ("\",\"H\":\"Web Port\",\"S\":\""), output, asyncresponse);
            BRIDGE::print ( (const char *) CONFIG::intTostr (DEFAULT_MAX_WEB_PORT), output, asyncresponse);
            BRIDGE::print (F ("\",\"M\":\""), output, asyncresponse);
            BRIDGE::print ( (const char *) CONFIG::intTostr (DEFAULT_MIN_WEB_PORT), output, asyncresponse);
            BRIDGE::print (F ("\"}"), output, asyncresponse);
            BRIDGE::println (F (","), output, asyncresponse);

            //4-Data Port
            BRIDGE::print (F ("{\"F\":\"network\",\"P\":\""), output, asyncresponse);
            BRIDGE::print ( (const char *) CONFIG::intTostr (EP_DATA_PORT), output, asyncresponse);
            BRIDGE::print (F ("\",\"T\":\"I\",\"V\":\""), output, asyncresponse);
            if (!CONFIG::read_buffer (EP_DATA_PORT,  (byte *) &ibuf, INTEGER_LENGTH) ) {
                BRIDGE::print ("???", output, asyncresponse);
            } else {
                BRIDGE::print ( (const char *) CONFIG::intTostr (ibuf), output, asyncresponse);
            }
            BRIDGE::print (F ("\",\"H\":\"Data Port\",\"S\":\""), output, asyncresponse);
            BRIDGE::print ( (const char *) CONFIG::intTostr (DEFAULT_MAX_DATA_PORT), output, asyncresponse);
            BRIDGE::print (F ("\",\"M\":\""), output, asyncresponse);
            BRIDGE::print ( (const char *) CONFIG::intTostr (DEFAULT_MIN_DATA_PORT), output, asyncresponse);
            BRIDGE::print (F ("\"}"), output, asyncresponse);
            BRIDGE::println (F (","), output, asyncresponse);
#ifdef AUTHENTICATION_FEATURE
            //5-Admin password
            BRIDGE::print (F ("{\"F\":\"network\",\"P\":\""), output, asyncresponse);
            BRIDGE::print ( (const char *) CONFIG::intTostr (EP_ADMIN_PWD), output, asyncresponse);
            BRIDGE::print (F ("\",\"T\":\"S\",\"V\":\""), output, asyncresponse);
            if (!CONFIG::read_string (EP_ADMIN_PWD, sbuf, MAX_LOCAL_PASSWORD_LENGTH) ) {
                BRIDGE::print ("???", output, asyncresponse);
            } else {
                BRIDGE::print ("********", output, asyncresponse);
            }
            BRIDGE::print (F ("\",\"S\":\""), output, asyncresponse);
            BRIDGE::print ( (const char *) CONFIG::intTostr (MAX_LOCAL_PASSWORD_LENGTH), output, asyncresponse);
            BRIDGE::print (F ("\",\"H\":\"Admin Password\",\"M\":\""), output, asyncresponse);
            BRIDGE::print ( (const char *) CONFIG::intTostr (MIN_LOCAL_PASSWORD_LENGTH), output, asyncresponse);
            BRIDGE::print (F ("\"}"), output, asyncresponse);
            BRIDGE::println (F (","), output, asyncresponse);

            //6-User password
            BRIDGE::print (F ("{\"F\":\"network\",\"P\":\""), output, asyncresponse);
            BRIDGE::print ( (const char *) CONFIG::intTostr (EP_USER_PWD), output, asyncresponse);
            BRIDGE::print (F ("\",\"T\":\"S\",\"V\":\""), output, asyncresponse);
            if (!CONFIG::read_string (EP_USER_PWD, sbuf, MAX_LOCAL_PASSWORD_LENGTH) ) {
                BRIDGE::print ("???", output, asyncresponse);
            } else {
                BRIDGE::print ("********", output, asyncresponse);
            }
            BRIDGE::print (F ("\",\"S\":\""), output, asyncresponse);
            BRIDGE::print ( (const char *) CONFIG::intTostr (MAX_LOCAL_PASSWORD_LENGTH), output, asyncresponse);
            BRIDGE::print (F ("\",\"H\":\"User Password\",\"M\":\""), output, asyncresponse);
            BRIDGE::print ( (const char *) CONFIG::intTostr (MIN_LOCAL_PASSWORD_LENGTH), output, asyncresponse);
            BRIDGE::print (F ("\"}"), output, asyncresponse);
            BRIDGE::println (F (","), output, asyncresponse);
#endif
            //7-Hostname
            BRIDGE::print (F ("{\"F\":\"network\",\"P\":\""), output, asyncresponse);
            BRIDGE::print ( (const char *) CONFIG::intTostr (EP_HOSTNAME), output, asyncresponse);
            BRIDGE::print (F ("\",\"T\":\"S\",\"V\":\""), output, asyncresponse);
            if (!CONFIG::read_string (EP_HOSTNAME, sbuf, MAX_HOSTNAME_LENGTH) ) {
                BRIDGE::print ("???", output, asyncresponse);
            } else {
                BRIDGE::print (sbuf, output, asyncresponse);
            }
            BRIDGE::print (F ("\",\"H\":\"Hostname\" ,\"S\":\""), output, asyncresponse);
            BRIDGE::print ( (const char *) CONFIG::intTostr (MAX_HOSTNAME_LENGTH), output, asyncresponse);
            BRIDGE::print (F ("\", \"M\":\""), output, asyncresponse);
            BRIDGE::print ( (const char *) CONFIG::intTostr (MIN_HOSTNAME_LENGTH), output, asyncresponse);
            BRIDGE::print (F ("\"}"), output, asyncresponse);
            BRIDGE::println (F (","), output, asyncresponse);

            //8-wifi mode
            BRIDGE::print (F ("{\"F\":\"network\",\"P\":\""), output, asyncresponse);
            BRIDGE::print ( (const char *) CONFIG::intTostr (EP_WIFI_MODE), output, asyncresponse);
            BRIDGE::print (F ("\",\"T\":\"B\",\"V\":\""), output, asyncresponse);
            if (!CONFIG::read_byte (EP_WIFI_MODE, &bbuf ) ) {
                BRIDGE::print ("???", output, asyncresponse);
            } else {
                BRIDGE::print ( (const char *) CONFIG::intTostr (bbuf), output, asyncresponse);
            }
            BRIDGE::print (F ("\",\"H\":\"Wifi mode\",\"O\":[{\"AP\":\"1\"},{\"STA\":\"2\"}]}"), output, asyncresponse);
            BRIDGE::println (F (","), output, asyncresponse);

            //9-STA SSID
            BRIDGE::print (F ("{\"F\":\"network\",\"P\":\""), output, asyncresponse);
            BRIDGE::print ( (const char *) CONFIG::intTostr (EP_STA_SSID), output, asyncresponse);
            BRIDGE::print (F ("\",\"T\":\"S\",\"V\":\""), output, asyncresponse);
            if (!CONFIG::read_string (EP_STA_SSID, sbuf, MAX_SSID_LENGTH) ) {
                BRIDGE::print ("???", output, asyncresponse);
            } else {
                BRIDGE::print (sbuf, output, asyncresponse);
            }
            BRIDGE::print (F ("\",\"S\":\""), output, asyncresponse);
            BRIDGE::print ( (const char *) CONFIG::intTostr (MAX_SSID_LENGTH), output, asyncresponse);
            BRIDGE::print (F ("\",\"H\":\"Station SSID\",\"M\":\""), output, asyncresponse);
            BRIDGE::print ( (const char *) CONFIG::intTostr (MIN_SSID_LENGTH), output, asyncresponse);
            BRIDGE::print (F ("\"}"), output, asyncresponse);
            BRIDGE::println (F (","), output, asyncresponse);

            //10-STA password
            BRIDGE::print (F ("{\"F\":\"network\",\"P\":\""), output, asyncresponse);
            BRIDGE::print ( (const char *) CONFIG::intTostr (EP_STA_PASSWORD), output, asyncresponse);
            BRIDGE::print (F ("\",\"T\":\"S\",\"V\":\""), output, asyncresponse);
            if (!CONFIG::read_string (EP_STA_PASSWORD, sbuf, MAX_PASSWORD_LENGTH) ) {
                BRIDGE::print ("???", output, asyncresponse);
            } else {
                BRIDGE::print ("********", output, asyncresponse);
            }
            BRIDGE::print (F ("\",\"S\":\""), output, asyncresponse);
            BRIDGE::print ( (const char *) CONFIG::intTostr (MAX_PASSWORD_LENGTH), output, asyncresponse);
            BRIDGE::print (F ("\",\"H\":\"Station Password\",\"M\":\""), output, asyncresponse);
            BRIDGE::print ( (const char *) CONFIG::intTostr (MIN_PASSWORD_LENGTH), output, asyncresponse);
            BRIDGE::print (F ("\"}"), output, asyncresponse);
            BRIDGE::println (F (","), output, asyncresponse);

            //11-Station Network Mode
            BRIDGE::print (F ("{\"F\":\"network\",\"P\":\""), output, asyncresponse);
            BRIDGE::print ( (const char *) CONFIG::intTostr (EP_STA_PHY_MODE), output, asyncresponse);
            BRIDGE::print (F ("\",\"T\":\"B\",\"V\":\""), output, asyncresponse);
            if (!CONFIG::read_byte (EP_STA_PHY_MODE, &bbuf ) ) {
                BRIDGE::print ("???", output, asyncresponse);
            } else {
                BRIDGE::print ( (const char *) CONFIG::intTostr (bbuf), output, asyncresponse);
            }
            BRIDGE::print (F ("\",\"H\":\"Station Network Mode\",\"O\":[{\"11b\":\""), output, asyncresponse);
            BRIDGE::print ( (const char *) CONFIG::intTostr (WIFI_PHY_MODE_11B), output, asyncresponse);
            BRIDGE::print (F ("\"},{\"11g\":\""), output, asyncresponse);
            BRIDGE::print ( (const char *) CONFIG::intTostr (WIFI_PHY_MODE_11G), output, asyncresponse);
            BRIDGE::print (F ("\"},{\"11n\":\""), output, asyncresponse);
            BRIDGE::print ( (const char *) CONFIG::intTostr (WIFI_PHY_MODE_11N), output, asyncresponse);
            BRIDGE::print (F ("\"}]}"), output, asyncresponse);
            BRIDGE::println (F (","), output, asyncresponse);

            //12-STA IP mode
            BRIDGE::print (F ("{\"F\":\"network\",\"P\":\""), output, asyncresponse);
            BRIDGE::print ( (const char *) CONFIG::intTostr (EP_STA_IP_MODE), output, asyncresponse);
            BRIDGE::print (F ("\",\"T\":\"B\",\"V\":\""), output, asyncresponse);
            if (!CONFIG::read_byte (EP_STA_IP_MODE, &bbuf ) ) {
                BRIDGE::print ("???", output, asyncresponse);
            } else {
                BRIDGE::print ( (const char *) CONFIG::intTostr (bbuf), output, asyncresponse);
            }
            BRIDGE::print (F ("\",\"H\":\"Station IP Mode\",\"O\":[{\"DHCP\":\"1\"},{\"Static\":\"2\"}]}"), output, asyncresponse);
            BRIDGE::println (F (","), output, asyncresponse);

            //13-STA static IP
            BRIDGE::print (F ("{\"F\":\"network\",\"P\":\""), output, asyncresponse);
            BRIDGE::print ( (const char *) CONFIG::intTostr (EP_STA_IP_VALUE), output, asyncresponse);
            BRIDGE::print (F ("\",\"T\":\"A\",\"V\":\""), output, asyncresponse);
            if (!CONFIG::read_buffer (EP_STA_IP_VALUE, (byte *) ipbuf, IP_LENGTH) ) {
                BRIDGE::print ("???", output, asyncresponse);
            } else {
                BRIDGE::print (IPAddress (ipbuf).toString().c_str(), output, asyncresponse);
            }
            BRIDGE::print (F ("\",\"H\":\"Station Static IP\"}"), output, asyncresponse);
            BRIDGE::println (F (","), output, asyncresponse);

            //14-STA static Mask
            BRIDGE::print (F ("{\"F\":\"network\",\"P\":\""), output, asyncresponse);
            BRIDGE::print ( (const char *) CONFIG::intTostr (EP_STA_MASK_VALUE), output, asyncresponse);
            BRIDGE::print (F ("\",\"T\":\"A\",\"V\":\""), output, asyncresponse);
            if (!CONFIG::read_buffer (EP_STA_MASK_VALUE, (byte *) ipbuf, IP_LENGTH) ) {
                BRIDGE::print ("???", output, asyncresponse);
            } else {
                BRIDGE::print (IPAddress (ipbuf).toString().c_str(), output, asyncresponse);
            }
            BRIDGE::print (F ("\",\"H\":\"Station Static Mask\"}"), output, asyncresponse);
            BRIDGE::println (F (","), output, asyncresponse);

            //15-STA static Gateway
            BRIDGE::print (F ("{\"F\":\"network\",\"P\":\""), output, asyncresponse);
            BRIDGE::print ( (const char *) CONFIG::intTostr (EP_STA_GATEWAY_VALUE), output, asyncresponse);
            BRIDGE::print (F ("\",\"T\":\"A\",\"V\":\""), output, asyncresponse);
            if (!CONFIG::read_buffer (EP_STA_GATEWAY_VALUE, (byte *) ipbuf, IP_LENGTH) ) {
                BRIDGE::print ("???", output, asyncresponse);
            } else {
                BRIDGE::print (IPAddress (ipbuf).toString().c_str(), output, asyncresponse);
            }
            BRIDGE::print (F ("\",\"H\":\"Station Static Gateway\"}"), output, asyncresponse);
            BRIDGE::println (F (","), output, asyncresponse);

            //16-AP SSID
            BRIDGE::print (F ("{\"F\":\"network\",\"P\":\""), output, asyncresponse);
            BRIDGE::print ( (const char *) CONFIG::intTostr (EP_AP_SSID), output, asyncresponse);
            BRIDGE::print (F ("\",\"T\":\"S\",\"V\":\""), output, asyncresponse);
            if (!CONFIG::read_string (EP_AP_SSID, sbuf, MAX_SSID_LENGTH) ) {
                BRIDGE::print ("???", output, asyncresponse);
            } else {
                BRIDGE::print (sbuf, output, asyncresponse);
            }
            BRIDGE::print (F ("\",\"S\":\""), output, asyncresponse);
            BRIDGE::print ( (const char *) CONFIG::intTostr (MAX_SSID_LENGTH), output, asyncresponse);
            BRIDGE::print (F ("\",\"H\":\"AP SSID\",\"M\":\""), output, asyncresponse);
            BRIDGE::print ( (const char *) CONFIG::intTostr (MIN_SSID_LENGTH), output, asyncresponse);
            BRIDGE::print (F ("\"}"), output, asyncresponse);
            BRIDGE::println (F (","), output, asyncresponse);

            //17-AP password
            BRIDGE::print (F ("{\"F\":\"network\",\"P\":\""), output, asyncresponse);
            BRIDGE::print ( (const char *) CONFIG::intTostr (EP_AP_PASSWORD), output, asyncresponse);
            BRIDGE::print (F ("\",\"T\":\"S\",\"V\":\""), output, asyncresponse);
            if (!CONFIG::read_string (EP_AP_PASSWORD, sbuf, MAX_PASSWORD_LENGTH) ) {
                BRIDGE::print ("???", output, asyncresponse);
            } else {
                BRIDGE::print ("********", output, asyncresponse);
            }
            BRIDGE::print (F ("\",\"S\":\""), output, asyncresponse);
            BRIDGE::print ( (const char *) CONFIG::intTostr (MAX_PASSWORD_LENGTH), output, asyncresponse);
            BRIDGE::print (F ("\",\"H\":\"AP Password\",\"M\":\""), output, asyncresponse);
            BRIDGE::print ( (const char *) CONFIG::intTostr (MIN_PASSWORD_LENGTH), output, asyncresponse);
            BRIDGE::print (F ("\"}"), output, asyncresponse);
            BRIDGE::println (F (","), output, asyncresponse);

            //18 - AP Network Mode
            BRIDGE::print (F ("{\"F\":\"network\",\"P\":\""), output, asyncresponse);
            BRIDGE::print ( (const char *) CONFIG::intTostr (EP_AP_PHY_MODE), output, asyncresponse);
            BRIDGE::print (F ("\",\"T\":\"B\",\"V\":\""), output, asyncresponse);
            if (!CONFIG::read_byte (EP_AP_PHY_MODE, &bbuf ) ) {
                BRIDGE::print ("???", output, asyncresponse);
            } else {
                BRIDGE::print ( (const char *) CONFIG::intTostr (bbuf), output, asyncresponse);
            }
            BRIDGE::print (F ("\",\"H\":\"AP Network Mode\",\"O\":[{\"11b\":\""), output, asyncresponse);
            BRIDGE::print ( (const char *) CONFIG::intTostr (WIFI_PHY_MODE_11B), output, asyncresponse);
            BRIDGE::print (F ("\"},{\"11g\":\""), output, asyncresponse);
            BRIDGE::print ( (const char *) CONFIG::intTostr (WIFI_PHY_MODE_11G), output, asyncresponse);
            BRIDGE::print (F ("\"}]}"), output, asyncresponse);
            BRIDGE::println (F (","), output, asyncresponse);

            //19-AP SSID visibility
            BRIDGE::print (F ("{\"F\":\"network\",\"P\":\""), output, asyncresponse);
            BRIDGE::print ( (const char *) CONFIG::intTostr (EP_SSID_VISIBLE), output, asyncresponse);
            BRIDGE::print (F ("\",\"T\":\"B\",\"V\":\""), output, asyncresponse);
            if (!CONFIG::read_byte (EP_SSID_VISIBLE, &bbuf ) ) {
                BRIDGE::print ("???", output, asyncresponse);
            } else {
                BRIDGE::print ( (const char *) CONFIG::intTostr (bbuf), output, asyncresponse);
            }
            BRIDGE::print (F ("\",\"H\":\"SSID Visible\",\"O\":[{\"No\":\"0\"},{\"Yes\":\"1\"}]}"), output, asyncresponse);
            BRIDGE::println (F (","), output, asyncresponse);

            //20-AP Channel
            BRIDGE::print (F ("{\"F\":\"network\",\"P\":\""), output, asyncresponse);
            BRIDGE::print ( (const char *) CONFIG::intTostr (EP_CHANNEL), output, asyncresponse);
            BRIDGE::print (F ("\",\"T\":\"B\",\"V\":\""), output, asyncresponse);
            if (!CONFIG::read_byte (EP_CHANNEL, &bbuf ) ) {
                BRIDGE::print ("???", output, asyncresponse);
            } else {
                BRIDGE::print ( (const char *) CONFIG::intTostr (bbuf), output, asyncresponse);
            }
            BRIDGE::print (F ("\",\"H\":\"AP Channel\",\"O\":["), output, asyncresponse);
            for (int i = 1; i < 12 ; i++) {
                BRIDGE::print (F ("{\""), output, asyncresponse);
                BRIDGE::print ( (const char *) CONFIG::intTostr (i), output, asyncresponse);
                BRIDGE::print (F ("\":\""), output, asyncresponse);
                BRIDGE::print ( (const char *) CONFIG::intTostr (i), output, asyncresponse);
                BRIDGE::print (F ("\"}"), output, asyncresponse);
                if (i < 11) {
                    BRIDGE::print (F (","), output, asyncresponse);
                }
            }
            BRIDGE::print (F ("]}"), output, asyncresponse);
            BRIDGE::println (F (","), output, asyncresponse);

            //21-AP Authentication
            BRIDGE::print (F ("{\"F\":\"network\",\"P\":\""), output, asyncresponse);
            BRIDGE::print ( (const char *) CONFIG::intTostr (EP_AUTH_TYPE), output, asyncresponse);
            BRIDGE::print (F ("\",\"T\":\"B\",\"V\":\""), output, asyncresponse);
            if (!CONFIG::read_byte (EP_AUTH_TYPE, &bbuf ) ) {
                BRIDGE::print ("???", output, asyncresponse);
            } else {
                BRIDGE::print ( (const char *) CONFIG::intTostr (bbuf), output, asyncresponse);
            }
            BRIDGE::print (F ("\",\"H\":\"Authentication\",\"O\":[{\"Open\":\""), output, asyncresponse);
            BRIDGE::print ( (const char *) CONFIG::intTostr (AUTH_OPEN), output, asyncresponse);
            BRIDGE::print (F ("\"},{\"WPA\":\""), output, asyncresponse);
            BRIDGE::print ( (const char *) CONFIG::intTostr (AUTH_WPA_PSK), output, asyncresponse);
            BRIDGE::print (F ("\"},{\"WPA2\":\""), output, asyncresponse);
            BRIDGE::print ( (const char *) CONFIG::intTostr (AUTH_WPA2_PSK), output, asyncresponse);
            BRIDGE::print (F ("\"},{\"WPA/WPA2\":\""), output, asyncresponse);
            BRIDGE::print ( (const char *) CONFIG::intTostr (AUTH_WPA_WPA2_PSK), output, asyncresponse);
            BRIDGE::print (F ("\"}]}"), output, asyncresponse);
            BRIDGE::println (F (","), output, asyncresponse);

            //22-AP IP mode
            BRIDGE::print (F ("{\"F\":\"network\",\"P\":\""), output, asyncresponse);
            BRIDGE::print ( (const char *) CONFIG::intTostr (EP_AP_IP_MODE), output, asyncresponse);
            BRIDGE::print (F ("\",\"T\":\"B\",\"V\":\""), output, asyncresponse);
            if (!CONFIG::read_byte (EP_AP_IP_MODE, &bbuf ) ) {
                BRIDGE::print ("???", output, asyncresponse);
            } else {
                BRIDGE::print ( (const char *) CONFIG::intTostr (bbuf), output, asyncresponse);
            }
            BRIDGE::print (F ("\",\"H\":\"AP IP Mode\",\"O\":[{\"DHCP\":\"1\"},{\"Static\":\"2\"}]}"), output, asyncresponse);
            BRIDGE::println (F (","), output, asyncresponse);

            //23-AP static IP
            BRIDGE::print (F ("{\"F\":\"network\",\"P\":\""), output, asyncresponse);
            BRIDGE::print ( (const char *) CONFIG::intTostr (EP_AP_IP_VALUE), output, asyncresponse);
            BRIDGE::print (F ("\",\"T\":\"A\",\"V\":\""), output, asyncresponse);
            if (!CONFIG::read_buffer (EP_AP_IP_VALUE, (byte *) ipbuf, IP_LENGTH) ) {
                BRIDGE::print ("???", output, asyncresponse);
            } else {
                BRIDGE::print (IPAddress (ipbuf).toString().c_str(), output, asyncresponse);
            }
            BRIDGE::print (F ("\",\"H\":\"AP Static IP\"}"), output, asyncresponse);
            BRIDGE::println (F (","), output, asyncresponse);

            //24-AP static Mask
            BRIDGE::print (F ("{\"F\":\"network\",\"P\":\""), output, asyncresponse);
            BRIDGE::print ( (const char *) CONFIG::intTostr (EP_AP_MASK_VALUE), output, asyncresponse);
            BRIDGE::print (F ("\",\"T\":\"A\",\"V\":\""), output, asyncresponse);
            if (!CONFIG::read_buffer (EP_AP_MASK_VALUE, (byte *) ipbuf, IP_LENGTH) ) {
                BRIDGE::print ("???", output, asyncresponse);
            } else {
                BRIDGE::print (IPAddress (ipbuf).toString().c_str(), output, asyncresponse);
            }
            BRIDGE::print (F ("\",\"H\":\"AP Static Mask\"}"), output, asyncresponse);
            BRIDGE::println (F (","), output, asyncresponse);

            //25-AP static Gateway
            BRIDGE::print (F ("{\"F\":\"network\",\"P\":\""), output, asyncresponse);
            BRIDGE::print ( (const char *) CONFIG::intTostr (EP_AP_GATEWAY_VALUE), output, asyncresponse);
            BRIDGE::print (F ("\",\"T\":\"A\",\"V\":\""), output, asyncresponse);
            if (!CONFIG::read_buffer (EP_AP_GATEWAY_VALUE, (byte *) ipbuf, IP_LENGTH) ) {
                BRIDGE::print ("???", output, asyncresponse);
            } else {
                BRIDGE::print (IPAddress (ipbuf).toString().c_str(), output, asyncresponse);
            }
            BRIDGE::print (F ("\",\"H\":\"AP Static Gateway\"}"), output, asyncresponse);

        }

        if (cmd_params == "printer" || cmd_params == "") {
            if (cmd_params == "") {
                BRIDGE::println (F (","), output, asyncresponse);
            }
            //Target FW
            BRIDGE::print (F ("{\"F\":\"printer\",\"P\":\""), output, asyncresponse);
            BRIDGE::print ( (const char *) CONFIG::intTostr (EP_TARGET_FW), output, asyncresponse);
            BRIDGE::print (F ("\",\"T\":\"B\",\"V\":\""), output, asyncresponse);
            if (!CONFIG::read_byte (EP_TARGET_FW, &bbuf ) ) {
                BRIDGE::print ("Unknown", output, asyncresponse);
            } else {
                BRIDGE::print ( (const char *) CONFIG::intTostr (bbuf), output, asyncresponse);
            }
            BRIDGE::print (F ("\",\"H\":\"Target FW\",\"O\":[{\"Repetier\":\""), output, asyncresponse);
            BRIDGE::print ( (const char *) CONFIG::intTostr (REPETIER), output, asyncresponse);
            BRIDGE::print (F ("\"},{\"Repetier for Davinci\":\""), output, asyncresponse);
            BRIDGE::print ( (const char *) CONFIG::intTostr (REPETIER4DV), output, asyncresponse);
            BRIDGE::print (F ("\"},{\"Marlin\":\""), output, asyncresponse);
            BRIDGE::print ( (const char *) CONFIG::intTostr (MARLIN), output, asyncresponse);
            BRIDGE::print (F ("\"},{\"Marlin Kimbra\":\""), output, asyncresponse);
            BRIDGE::print ( (const char *) CONFIG::intTostr (MARLINKIMBRA), output, asyncresponse);
            BRIDGE::print (F ("\"},{\"Smoothieware\":\""), output, asyncresponse);
            BRIDGE::print ( (const char *) CONFIG::intTostr (SMOOTHIEWARE), output, asyncresponse);
            BRIDGE::print (F ("\"},{\"Grbl\":\""), output, asyncresponse);
            BRIDGE::print ( (const char *) CONFIG::intTostr (GRBL), output, asyncresponse);
            BRIDGE::print (F ("\"},{\"Unknown\":\""), output, asyncresponse);
            BRIDGE::print ( (const char *) CONFIG::intTostr (UNKNOWN_FW), output, asyncresponse);
            BRIDGE::print (F ("\"}]}"), output, asyncresponse);

        }

        //end JSON
        BRIDGE::println (F ("\n]}"), output, asyncresponse);
    }
    break;

    //Set EEPROM setting
    //[ESP401]P=<position> T=<type> V=<value> pwd=<user/admin password>
    case 401: {
        //check validity of parameters
        String spos = get_param (cmd_params, "P=", false);
        String styp = get_param (cmd_params, "T=", false);
        String sval = get_param (cmd_params, "V=", true);
        sval.trim();
        int pos = spos.toInt();
        if ( (pos == 0 && spos != "0") || (pos > LAST_EEPROM_ADDRESS || pos < 0) ) {
            response = false;
        }
        if (! (styp == "B" || styp == "S" || styp == "A" || styp == "I") ) {
            response = false;
        }
        if (sval.length() == 0) {
            response = false;
        }


#ifdef AUTHENTICATION_FEATURE
        if (response) {
            //check authentication
            level_authenticate_type auth_need = LEVEL_ADMIN;
            for (int i = 0; i < AUTH_ENTRY_NB; i++) {
                if (Setting[i][0] == pos ) {
                    auth_need = (level_authenticate_type) (Setting[i][1]);
                    i = AUTH_ENTRY_NB;
                }
            }
            if ( (auth_need == LEVEL_ADMIN && auth_type == LEVEL_USER) || (auth_type == LEVEL_GUEST) ) {
                response = false;
            }
        }
#endif
        if (response) {
            if (styp == "B") {
                byte bbuf = sval.toInt();
                if (!CONFIG::write_byte (pos, bbuf) ) {
                    response = false;
                } else {
                    //dynamique refresh is better than restart the board
                    if (pos == EP_TARGET_FW) {
                        CONFIG::InitFirmwareTarget();
                    }
                    if (pos == EP_IS_DIRECT_SD) {
                        CONFIG::InitDirectSD();
                        if (CONFIG::is_direct_sd) {
                            CONFIG::InitPins();
                        }
                    }
                }
            }
            if (styp == "I") {
                int ibuf = sval.toInt();
                if (!CONFIG::write_buffer (pos, (const byte *) &ibuf, INTEGER_LENGTH) ) {
                    response = false;
                }
            }
            if (styp == "S") {
                if (!CONFIG::write_string (pos, sval.c_str() ) ) {
                    response = false;
                }
            }
            if (styp == "A") {
                byte ipbuf[4];
                if (CONFIG::split_ip (sval.c_str(), ipbuf) < 4) {
                    response = false;
                } else if (!CONFIG::write_buffer (pos, ipbuf, IP_LENGTH) ) {
                    response = false;
                }
            }
        }
        if (!response) {
            BRIDGE::println (INCORRECT_CMD_MSG, output, asyncresponse);
        } else {
            BRIDGE::println (OK_CMD_MSG, output, asyncresponse);
        }

    }
    break;

    //Get available AP list (limited to 30)
    //output is JSON or plain text according parameter
    //[ESP410]<plain>
    case 410: {
        parameter = get_param (cmd_params, "", true);
        bool plain = (parameter == "plain");
        if (!plain) {
            BRIDGE::print (F ("{\"AP_LIST\":["), output, asyncresponse);
        }
        int n = WiFi.scanComplete();
        if (n == -2) {
            WiFi.scanNetworks (true);
        } else if (n) {
            for (int i = 0; i < n; ++i) {
                if (i > 0) {
                    if (!plain) {
                        BRIDGE::print (F (","), output, asyncresponse);
                    } else {
                        BRIDGE::print (F ("\n"), output, asyncresponse);
                    }
                }
                if (!plain) {
                    BRIDGE::print (F ("{\"SSID\":\""), output, asyncresponse);
                }
                BRIDGE::print (WiFi.SSID (i).c_str(), output, asyncresponse);
                if (!plain) {
                    BRIDGE::print (F ("\",\"SIGNAL\":\""), output, asyncresponse);
                } else {
                    BRIDGE::print (F ("\t"), output, asyncresponse);
                }
                BRIDGE::print (CONFIG::intTostr (wifi_config.getSignal (WiFi.RSSI (i) ) ), output, asyncresponse);;
                //BRIDGE::print(F("%"), output, asyncresponse);
                if (!plain) {
                    BRIDGE::print (F ("\",\"IS_PROTECTED\":\""), output, asyncresponse);
                }
                if (WiFi.encryptionType (i) == ENC_TYPE_NONE) {
                    if (!plain) {
                        BRIDGE::print (F ("0"), output, asyncresponse);
                    } else {
                        BRIDGE::print (F ("\tOpen"), output, asyncresponse);
                    }
                } else {
                    if (!plain) {
                        BRIDGE::print (F ("1"), output, asyncresponse);
                    } else {
                        BRIDGE::print (F ("\tSecure"), output, asyncresponse);
                    }
                }
                if (!plain) {
                    BRIDGE::print (F ("\"}"), output, asyncresponse);
                }
            }
            WiFi.scanDelete();
            if (WiFi.scanComplete() == -2) {
                WiFi.scanNetworks (true);
            }
        }
        if (!plain) {
            BRIDGE::print (F ("]}"), output, asyncresponse);
        } else {
            BRIDGE::print (F ("\n"), output, asyncresponse);
        }
    }
    break;
    //Get ESP current status in plain or JSON
    //[ESP420]<plain>
    case 420: {
        parameter = get_param (cmd_params, "", true);
        CONFIG::print_config (output, (parameter == "plain"), asyncresponse);
    }
    break;
    //Set ESP mode
    //cmd is RESET, SAFEMODE, RESTART
    //[ESP444]<cmd>pwd=<admin password>
    case 444:
        parameter = get_param (cmd_params, "", true);
#ifdef AUTHENTICATION_FEATURE
        if (auth_type != LEVEL_ADMIN) {
            BRIDGE::println (INCORRECT_CMD_MSG, output, asyncresponse);
            response = false;
        } else
#endif
        {
            if (parameter == "RESET") {
                CONFIG::reset_config();
                BRIDGE::println (F ("Reset done - restart needed"), output, asyncresponse);
            } else if (parameter == "SAFEMODE") {
                wifi_config.Safe_Setup();
                BRIDGE::println (F ("Set Safe Mode  - restart needed"), output, asyncresponse);
            } else  if (parameter == "RESTART") {
                BRIDGE::println (F ("Restart started"), output, asyncresponse);
                web_interface->restartmodule = true;
            } else {
                BRIDGE::println (INCORRECT_CMD_MSG, output, asyncresponse);
                response = false;
            }
        }
        break;
#ifdef AUTHENTICATION_FEATURE
    //Change / Reset user password
    //[ESP555]<password>pwd=<admin password>
    case 555: {
        if (auth_type == LEVEL_ADMIN) {
            parameter = get_param (cmd_params, "", true);
            if (parameter.length() == 0) {
                if (CONFIG::write_string (EP_USER_PWD, FPSTR (DEFAULT_USER_PWD) ) ) {
                    BRIDGE::println (OK_CMD_MSG, output, asyncresponse);
                } else {
                    BRIDGE::println (ERROR_CMD_MSG, output, asyncresponse);
                    response = false;
                }
            } else {
                if (CONFIG::isLocalPasswordValid (parameter.c_str() ) ) {
                    if (CONFIG::write_string (EP_USER_PWD, parameter.c_str() ) ) {
                        BRIDGE::println (OK_CMD_MSG, output, asyncresponse);
                    } else {
                        BRIDGE::println (ERROR_CMD_MSG, output, asyncresponse);
                        response = false;
                    }
                } else {
                    BRIDGE::println (INCORRECT_CMD_MSG, output, asyncresponse);
                    response = false;
                }
            }
        } else {
            BRIDGE::println (INCORRECT_CMD_MSG, output, asyncresponse);
            response = false;
        }
        break;
    }
#endif
    //[ESP700]<filename>
    case 700: { //read local file
        //be sure serial is locked
        if ( (web_interface->blockserial) ) {
            break;
        }
        cmd_params.trim() ;
        if ( (cmd_params.length() > 0) && (cmd_params[0] != '/') ) {
            cmd_params = "/" + cmd_params;
        }
        FS_FILE currentfile = SPIFFS.open (cmd_params, SPIFFS_FILE_READ);
        if (currentfile) {//if file open success
            //flush to be sure send buffer is empty
            ESP_SERIAL_OUT.flush();
            //until no line in file
            while (currentfile.available()) {
                String currentline = currentfile.readStringUntil('\n');
                currentline.replace("\n","");
                currentline.replace("\r","");
                if (currentline.length() > 0) {
                    int ESPpos = currentline.indexOf ("[ESP");
                    if (ESPpos > -1) {
                        //is there the second part?
                        int ESPpos2 = currentline.indexOf ("]", ESPpos);
                        if (ESPpos2 > -1) {
                            //Split in command and parameters
                            String cmd_part1 = currentline.substring (ESPpos + 4, ESPpos2);
                            String cmd_part2 = "";
                            //is there space for parameters?
                            if (ESPpos2 < currentline.length() ) {
                                cmd_part2 = currentline.substring (ESPpos2 + 1);
                            }
                            //if command is a valid number then execute command
                            if(cmd_part1.toInt()!=0) {
                                execute_command(cmd_part1.toInt(),cmd_part2,NO_PIPE, auth_type, asyncresponse);
                            }
                            //if not is not a valid [ESPXXX] command ignore it
                        }
                    } else {
                        //send line to serial
                        ESP_SERIAL_OUT.println (currentline);
                        CONFIG::wait (1);
                        //flush to be sure send buffer is empty
                        ESP_SERIAL_OUT.flush();
                    }
                CONFIG::wait (1);
                }
            }
            currentfile.close();
            BRIDGE::println (OK_CMD_MSG, output, asyncresponse);
        } else {
            BRIDGE::println (ERROR_CMD_MSG, output, asyncresponse);
            response = false;
        }

        break;
    }
    //Format SPIFFS
    //[ESP710]FORMAT pwd=<admin password>
    case 710:
        parameter = get_param (cmd_params, "", true);
#ifdef AUTHENTICATION_FEATURE
        if (auth_type != LEVEL_ADMIN) {
            BRIDGE::println (INCORRECT_CMD_MSG, output, asyncresponse);
            response = false;
        } else
#endif
        {
            if (parameter == "FORMAT") {
                BRIDGE::print (F ("Formating"), output, asyncresponse);
                SPIFFS.format();
                BRIDGE::println (F ("...Done"), output, asyncresponse);
            } else {
                BRIDGE::println (INCORRECT_CMD_MSG, output, asyncresponse);
                response = false;
            }
        }
        break;
    //SPIFFS total size and used size
    //[ESP720]<header answer>
    case 720:
        BRIDGE::print (cmd_params, output, asyncresponse);
#ifdef ARDUINO_ARCH_ESP8266
        fs::FSInfo info;
        SPIFFS.info (info);
        BRIDGE::print ("SPIFFS Total:", output, asyncresponse);
        BRIDGE::print (CONFIG::formatBytes (info.totalBytes).c_str(), output, asyncresponse);
        BRIDGE::print (" Used:", output, asyncresponse);
        BRIDGE::println (CONFIG::formatBytes (info.usedBytes).c_str(), output, asyncresponse);
#else
        BRIDGE::print ("SPIFFS  Total:", output, asyncresponse);
        BRIDGE::print (CONFIG::formatBytes (SPIFFS.totalBytes() ).c_str(), output, asyncresponse);
        BRIDGE::print (" Used:", output, asyncresponse);
        BRIDGE::println (CONFIG::formatBytes (SPIFFS.usedBytes() ).c_str(), output, asyncresponse);
#endif
        break;
    //get fw version firmare target and fw version
    //[ESP800]<header answer>
    case 800: {
        byte sd_dir = 0;
        BRIDGE::print (cmd_params, output, asyncresponse);
        BRIDGE::print (F ("FW version:"), output, asyncresponse);
        BRIDGE::print (FW_VERSION, output, asyncresponse);
        BRIDGE::print (F (" # FW target:"), output, asyncresponse);
        BRIDGE::print (CONFIG::GetFirmwareTargetShortName(), output, asyncresponse);
        BRIDGE::print (F (" # FW HW:"), output, asyncresponse);
        BRIDGE::print (F ("Direct SD"), output, asyncresponse);
        BRIDGE::print (F (" # primary sd:"), output, asyncresponse);
        BRIDGE::print (F ("none"), output, asyncresponse);
        BRIDGE::print (F (" # secondary sd:"), output, asyncresponse);
        BRIDGE::print (F ("none"), output, asyncresponse);
        BRIDGE::print (F (" # authentication:"), output, asyncresponse);
#ifdef AUTHENTICATION_FEATURE
        BRIDGE::print (F ("yes"), output, asyncresponse);
#else
        BRIDGE::print (F ("no"), output, asyncresponse);
#endif
        BRIDGE::println ("", output, asyncresponse);
    }
    break;
    //get fw target
    //[ESP801]<header answer>
    case 801:
        BRIDGE::print (cmd_params, output, asyncresponse);
        BRIDGE::println (CONFIG::GetFirmwareTargetShortName(), output, asyncresponse);
        break;
    case 810:
        web_interface->blockserial = false;
        break;
    //[ESP999]<cmd>
    case 999:
        cmd_params.trim();
#ifdef ERROR_MSG_FEATURE
        if (cmd_params == "ERROR") {
            web_interface->error_msg.clear();
            BRIDGE::println (OK_CMD_MSG, output, asyncresponse);
            break;
        }
#endif
#ifdef INFO_MSG_FEATURE
        if (cmd_params == "INFO") {
            web_interface->info_msg.clear();
            BRIDGE::println (OK_CMD_MSG, output, asyncresponse);
            break;
        }
#endif
#ifdef STATUS_MSG_FEATURE
        if (cmd_params == "STATUS") {
            web_interface->status_msg.clear();
            BRIDGE::println (OK_CMD_MSG, output, asyncresponse);
            break;
        }
#endif
        if (cmd_params == "ALL") {
#ifdef ERROR_MSG_FEATURE
            web_interface->error_msg.clear();
#endif
#ifdef STATUS_MSG_FEATURE
            web_interface->status_msg.clear();
#endif
#ifdef INFO_MSG_FEATURE
            web_interface->info_msg.clear();
#endif
            BRIDGE::println (OK_CMD_MSG, output, asyncresponse);
            break;
        }
    default:
        BRIDGE::println (INCORRECT_CMD_MSG, output, asyncresponse);
        response = false;
    }
    return response;
}

bool COMMAND::check_command (String buffer, tpipe output, bool handlelockserial, bool executecmd)
{
    String buffer2;
    LOG ("Check Command:")
    LOG (buffer)
    LOG ("\r\n")
    bool is_temp = false;
    if ( (buffer.indexOf ("T:") > -1 ) || (buffer.indexOf ("B:") > -1 ) ) {
        is_temp = true;
    }
    if ( ( CONFIG::GetFirmwareTarget()  == REPETIER4DV) || (CONFIG::GetFirmwareTarget() == REPETIER) ) {
        //save time no need to continue
        if ( (buffer.indexOf ("busy:") > -1) || (buffer.startsWith ("wait") ) ) {
            return false;
        }
        if (buffer.startsWith ("ok") ) {
            return false;
        }
    } else  if (buffer.startsWith ("ok") && buffer.length() < 4) {
        return false;
    }
#ifdef ERROR_MSG_FEATURE
    int Errorpos = -1;
#endif
#ifdef INFO_MSG_FEATURE
    int Infopos = -1;
#endif
#ifdef STATUS_MSG_FEATURE
    int Statuspos = -1;
#endif
    if (CONFIG::GetFirmwareTarget()  == SMOOTHIEWARE) {
#ifdef ERROR_MSG_FEATURE
        Errorpos = buffer.indexOf ("error:");
#endif
#ifdef INFO_MSG_FEATURE
        Infopos = buffer.indexOf ("info:");
#endif
#ifdef STATUS_MSG_FEATURE
        Statuspos = buffer.indexOf ("warning:");
#endif
    } else {
#ifdef ERROR_MSG_FEATURE
        Errorpos = buffer.indexOf ("Error:");
#endif
#ifdef INFO_MSG_FEATURE
        Infopos = buffer.indexOf ("Info:");
#endif
#ifdef STATUS_MSG_FEATURE
        Statuspos = -1;
        if (CONFIG::GetFirmwareTarget()  == MARLIN) {
            Statuspos = buffer.indexOf ("echo:");
        } else {
            Statuspos = buffer.indexOf ("Status:");
        }

#endif
    }

#ifdef SERIAL_COMMAND_FEATURE
    if (executecmd) {
        String ESP_Command;
        int ESPpos = buffer.indexOf ("[ESP");
        if (ESPpos > -1) {
            //is there the second part?
            int ESPpos2 = buffer.indexOf ("]", ESPpos);
            if (ESPpos2 > -1) {
                //Split in command and parameters
                String cmd_part1 = buffer.substring (ESPpos + 4, ESPpos2);
                String cmd_part2 = "";
                //is there space for parameters?
                if (ESPpos2 < buffer.length() ) {
                    cmd_part2 = buffer.substring (ESPpos2 + 1);
                }
                //if command is a valid number then execute command
                if (cmd_part1.toInt() != 0) {
                    execute_command (cmd_part1.toInt(), cmd_part2, output);
                }
                //if not is not a valid [ESPXXX] command
            }
        }
    }
#endif
#ifdef ERROR_MSG_FEATURE
    //Error
    if (Errorpos > -1 && ! (buffer.indexOf ("Format error") != -1 || buffer.indexOf ("wait") == Errorpos + 6) ) {
        String ss = buffer.substring (Errorpos + 6);
        ss.replace ("\"", "");
        ss.replace ("'", "");
        (web_interface->error_msg).add (ss.c_str() );
    }
#endif
#ifdef INFO_MSG_FEATURE
    //Info
    if (Infopos > -1) {
        String ss = buffer.substring (Errorpos + 5);
        ss.replace ("\"", "");
        ss.replace ("'", "");
        (web_interface->info_msg).add (ss.c_str() );
    }
#endif
#ifdef STATUS_MSG_FEATURE
    //Status
    if (Statuspos > -1) {
        String ss ;
        if (CONFIG::GetFirmwareTarget() == SMOOTHIEWARE) {
            ss = buffer.substring (Errorpos + 8);
        } else if (CONFIG::GetFirmwareTarget() == MARLIN) {
            ss = buffer.substring (Errorpos + 5);
        } else {
            ss = buffer.substring (Errorpos + 7);
        }
        ss.replace ("\"", "");
        ss.replace ("'", "");
        (web_interface->info_msg).add (ss.c_str() );
    }
#endif

    return is_temp;
}

//read a buffer in an array
void COMMAND::read_buffer_serial (uint8_t *b, size_t len)
{
    for (long i = 0; i < len; i++) {
        read_buffer_serial (b[i]);
        //*b++;
    }
}

#ifdef TCP_IP_DATA_FEATURE
//read buffer as char
void COMMAND::read_buffer_tcp (uint8_t b)
{
    static bool previous_was_char = false;
    static bool iscomment = false;
//to ensure it is continuous string, no char separated by binaries
    if (!previous_was_char) {
        buffer_tcp = "";
        iscomment = false;
    }
//is comment ?
    if (char (b) == ';') {
        iscomment = true;
    }
//it is a char so add it to buffer
    if (isPrintable (b) ) {
        previous_was_char = true;
        //add char if not a comment
        if (!iscomment) {
            buffer_tcp += char (b);
        }
    } else {
        previous_was_char = false; //next call will reset the buffer
    }
//this is not printable but end of command check if need to handle it
    if (b == 13 || b == 10) {
        //reset comment flag
        iscomment = false;
        //Minimum is something like M10 so 3 char
        if (buffer_tcp.length() > 3) {
            check_command (buffer_tcp, TCP_PIPE);
        }
    }
}
#endif
//read buffer as char
void COMMAND::read_buffer_serial (uint8_t b)
{
    static bool previous_was_char = false;
    static bool iscomment = false;
//to ensure it is continuous string, no char separated by binaries
    if (!previous_was_char) {
        buffer_serial = "";
        iscomment = false;
    }
//is comment ?
    if (char (b) == ';') {
        iscomment = true;
    }
//it is a char so add it to buffer
    if (isPrintable (b) ) {
        previous_was_char = true;
        if (!iscomment) {
            buffer_serial += char (b);
        }
    } else {
        previous_was_char = false; //next call will reset the buffer
    }
//this is not printable but end of command check if need to handle it
    if (b == 13 || b == 10) {
        //reset comment flag
        iscomment = false;
        //Minimum is something like M10 so 3 char
        if (buffer_serial.length() > 3) {
            check_command (buffer_serial, SERIAL_PIPE);
        }
    }
}
