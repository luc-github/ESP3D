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
#define MAX_GPIO 37
int ChannelAttached2Pin[16]= {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
#else
#define MAX_GPIO 16
#endif
#ifdef TIMESTAMP_FEATURE
#include <time.h>
#endif
#ifdef ESP_OLED_FEATURE
#include "esp_oled.h"
#endif

#ifdef DHT_FEATURE
#include "DHTesp.h"
extern DHTesp dht;
#endif

#ifdef NOTIFICATION_FEATURE
#include "notifications_service.h"
#endif

String COMMAND::buffer_serial;
String COMMAND::buffer_tcp;

#define ERROR_CMD_MSG (output == WEB_PIPE)?F("Error: Wrong Command"):F("Cmd Error")
#define INCORRECT_CMD_MSG (output == WEB_PIPE)?F("Error: Incorrect Command"):F("Incorrect Cmd")
#define OK_CMD_MSG (output == WEB_PIPE)?F("ok"):F("Cmd Ok")

extern uint8_t Checksum(const char * line, uint16_t lineSize);
extern bool sendLine2Serial (String &  line, int32_t linenb, int32_t* newlinenb);

const char * encodeString(const char * s){
    static String tmp;
    tmp = s;
    while(tmp.indexOf("'")!=-1)tmp.replace("'", "&#39;");
    while(tmp.indexOf("\"")!=-1)tmp.replace("\"", "&#34;");
    if (tmp =="") tmp=" ";
    return tmp.c_str();
}
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
bool COMMAND::execute_command (int cmd, String cmd_params, tpipe output, level_authenticate_type auth_level, ESPResponseStream  *espresponse)
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
    if ( auth_type == LEVEL_ADMIN) {
        LOG("admin identified\r\n");
    } else  {
        if( auth_type == LEVEL_USER) {
            LOG("user identified\r\n");
        } else {
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
            ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse);
        }
#ifdef AUTHENTICATION_FEATURE
        if (auth_type != LEVEL_ADMIN) {
            ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse);
        } else
#endif
            if (!CONFIG::write_string (EP_STA_SSID, parameter.c_str() ) ) {
                ESPCOM::println (ERROR_CMD_MSG, output, espresponse);
                response = false;
            } else {
                ESPCOM::println (OK_CMD_MSG, output, espresponse);
            }
        break;
    //STA Password
    //[ESP101]<Password>[pwd=<admin password>]
    case 101:
        parameter = get_param (cmd_params, "", true);
        if (!CONFIG::isPasswordValid (parameter.c_str() ) ) {
            ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse);
            response = false;
        }
#ifdef AUTHENTICATION_FEATURE
        if (auth_type != LEVEL_ADMIN) {
            ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse);
            response = false;
        } else
#endif
            if (!CONFIG::write_string (EP_STA_PASSWORD, parameter.c_str() ) ) {
                ESPCOM::println (ERROR_CMD_MSG, output, espresponse);
                response = false;
            } else {
                ESPCOM::println (OK_CMD_MSG, output, espresponse);
            }
        break;
    //Hostname
    //[ESP102]<hostname>[pwd=<admin password>]
    case 102:
        parameter = get_param (cmd_params, "", true);
        if (!CONFIG::isHostnameValid (parameter.c_str() ) ) {
            ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse);
            response = false;
        }
#ifdef AUTHENTICATION_FEATURE
        if (auth_type != LEVEL_ADMIN) {
            ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse);
            response = false;
        } else
#endif
            if (!CONFIG::write_string (EP_HOSTNAME, parameter.c_str() ) ) {
                ESPCOM::println (ERROR_CMD_MSG, output, espresponse);
                response = false;
            } else {
                ESPCOM::println (OK_CMD_MSG, output, espresponse);
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
            ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse);
            response = false;
        }
        if ( (mode == CLIENT_MODE) || (mode == AP_MODE) ) {
#ifdef AUTHENTICATION_FEATURE
            if (auth_type != LEVEL_ADMIN) {
                ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse);
                response = false;
            } else
#endif
                if (!CONFIG::write_byte (EP_WIFI_MODE, mode) ) {
                    ESPCOM::println (ERROR_CMD_MSG, output, espresponse);
                    response = false;
                } else {
                    ESPCOM::println (OK_CMD_MSG, output, espresponse);
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
            ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse);
            response = false;
        }
        if ( (mode == STATIC_IP_MODE) || (mode == DHCP_MODE) ) {
#ifdef AUTHENTICATION_FEATURE
            if (auth_type != LEVEL_ADMIN) {
                ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse);
                response = false;
            } else
#endif
                if (!CONFIG::write_byte (EP_STA_IP_MODE, mode) ) {
                    ESPCOM::println (ERROR_CMD_MSG, output, espresponse);
                    response = false;
                } else {
                    ESPCOM::println (OK_CMD_MSG, output, espresponse);
                }
        }
        break;
#ifndef USE_AS_UPDATER_ONLY
    //AP SSID
    //[ESP105]<SSID>[pwd=<admin password>]
    case 105:
        parameter = get_param (cmd_params, "", true);
        if (!CONFIG::isSSIDValid (parameter.c_str() ) ) {
            ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse);
            response = false;
        }
#ifdef AUTHENTICATION_FEATURE
        if (auth_type != LEVEL_ADMIN) {
            ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse);
            response = false;
        } else
#endif
            if (!CONFIG::write_string (EP_AP_SSID, parameter.c_str() ) ) {
                ESPCOM::println (ERROR_CMD_MSG, output, espresponse);
                response = false;
            } else {
                ESPCOM::println (OK_CMD_MSG, output, espresponse);
            }
        break;
    //AP Password
    //[ESP106]<Password>[pwd=<admin password>]
    case 106:
        parameter = get_param (cmd_params, "", true);
        if (!CONFIG::isPasswordValid (parameter.c_str() ) ) {
            ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse);
            response = false;
        }
#ifdef AUTHENTICATION_FEATURE
        if (auth_type != LEVEL_ADMIN) {
            ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse);
            response = false;
        } else
#endif
            if (!CONFIG::write_string (EP_AP_PASSWORD, parameter.c_str() ) ) {
                ESPCOM::println (ERROR_CMD_MSG, output, espresponse);
                response = false;
            } else {
                ESPCOM::println (OK_CMD_MSG, output, espresponse);
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
            ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse);
            response = false;
        }
        if ( (mode == STATIC_IP_MODE) || (mode == DHCP_MODE) ) {
#ifdef AUTHENTICATION_FEATURE
            if (auth_type != LEVEL_ADMIN) {
                ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse);
                response = false;
            } else
#endif
                if (!CONFIG::write_byte (EP_AP_IP_MODE, mode) ) {
                    ESPCOM::println (ERROR_CMD_MSG, output, espresponse);
                    response = false;
                } else {
                    ESPCOM::println (OK_CMD_MSG, output, espresponse);
                }
        }
        break;
    // Set wifi on/off
    //[ESP110]<state>[pwd=<admin password>]
    case 110:
        parameter = get_param (cmd_params, "", true);
        if (parameter == "ON") {
            mode = 1;
        } else if (parameter == "OFF") {
            mode = 0;
        } else if (parameter == "RESTART") {
            mode = 2;
        } else {
            ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse);
            response = false;
        }
        if (response) {
#ifdef AUTHENTICATION_FEATURE
            if (auth_type != LEVEL_ADMIN) {
                ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse);
                response = false;
            } else
#endif
                if (mode == 0) {
                    if ((WiFi.getMode() != WIFI_OFF)  || wifi_config.WiFi_on) {
                        //disable wifi
                        ESPCOM::println ("Disabling Wifi", output, espresponse);
#ifdef ESP_OLED_FEATURE
                        OLED_DISPLAY::display_signal(-1);
                        OLED_DISPLAY::setCursor(0, 0);
                        ESPCOM::print("", OLED_PIPE);
                        OLED_DISPLAY::setCursor(0, 16);
                        ESPCOM::print("", OLED_PIPE);
                        OLED_DISPLAY::setCursor(0, 48);
                        ESPCOM::print("Wifi disabled", OLED_PIPE);
#endif
                        WiFi.disconnect(true);
                        WiFi.enableSTA (false);
                        WiFi.enableAP (false);
                        WiFi.mode (WIFI_OFF);
                        wifi_config.WiFi_on = false;
                        wifi_config.Disable_servers();
                        return response;
                    } else {
                        ESPCOM::println ("Wifi already off", output, espresponse);
                    }
                } else if (mode == 1) { //restart device is the best way to start everything clean
                    if ((WiFi.getMode() == WIFI_OFF) || !wifi_config.WiFi_on) {
                        ESPCOM::println ("Enabling Wifi", output, espresponse);
                        web_interface->restartmodule = true;
                    } else {
                        ESPCOM::println ("Wifi already on", output, espresponse);
                    }
                } else  { //restart wifi and restart is the best way to start everything clean
                    ESPCOM::println ("Enabling Wifi", output, espresponse);
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
        ESPCOM::print (cmd_params, output, espresponse);
        ESPCOM::println (currentIP, output, espresponse);
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
        ESPCOM::print (cmd_params, output, espresponse);
        ESPCOM::println (shost, output, espresponse);
        LOG (cmd_params)
        LOG (shost)
        LOG ("\r\n")
    }
    break;
#if defined(TIMESTAMP_FEATURE)
    //restart time client
    case 114: {
        CONFIG::init_time_client();
        ESPCOM::println (OK_CMD_MSG, output, espresponse);
        LOG ("restart time client\r\n")
    }
    break;
    //get time client
    case 115: {
        struct tm  tmstruct;
        time_t now;
        String stmp = "";
        time(&now);
        localtime_r(&now, &tmstruct);
        stmp = String((tmstruct.tm_year)+1900) + "-";
        if (((tmstruct.tm_mon)+1) < 10) {
            stmp +="0";
        }
        stmp += String(( tmstruct.tm_mon)+1) + "-";
        if (tmstruct.tm_mday < 10) {
            stmp +="0";
        }
        stmp += String(tmstruct.tm_mday) + " ";
        if (tmstruct.tm_hour < 10) {
            stmp +="0";
        }
        stmp += String(tmstruct.tm_hour) + ":";
        if (tmstruct.tm_min < 10) {
            stmp +="0";
        }
        stmp += String(tmstruct.tm_min) + ":";
        if (tmstruct.tm_sec < 10) {
            stmp +="0";
        }
        stmp += String(tmstruct.tm_sec);
        ESPCOM::println(stmp.c_str(), output, espresponse);
    }
    break;
#endif

#ifdef DIRECT_PIN_FEATURE
    //Get/Set pin value
    //[ESP201]P<pin> V<value> [PULLUP=YES RAW=YES ANALOG=NO ANALOG_RANGE=255 CLEARCHANNELS=NO]pwd=<admin password>
    case 201:
        parameter = get_param (cmd_params, "", true);
#ifdef AUTHENTICATION_FEATURE
        if (auth_type == LEVEL_GUEST) {
            ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse);
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
                ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse);
                response = false;
            } else {
                int pin = parameter.toInt();
                //check pin is valid
                if ((pin >= 0) && (pin <= MAX_GPIO)) {
                    //check if analog or digital
                    bool isdigital = true;

                    parameter = get_param (cmd_params, "ANALOG=", false);
                    if (parameter == "YES") {
                        LOG ("Set as analog\r\n")
                        isdigital=false;
#ifdef ARDUINO_ARCH_ESP32
                        parameter = get_param (cmd_params, "CLEARCHANNELS=", false);
                        if (parameter == "YES") {
                            for (uint8_t p = 0; p < 16; p++) {
                                if(ChannelAttached2Pin[p] != -1) {
                                    ledcDetachPin(ChannelAttached2Pin[p]);
                                    ChannelAttached2Pin[p] = -1;
                                }
                            }
                        }
#endif
                    }
                    //check if is set or get
                    parameter = get_param (cmd_params, "V", false);
                    //it is a get
                    if (parameter == "") {
                        int value = 0;
                        if(isdigital) {
                            //this is to not set pin mode
                            parameter = get_param (cmd_params, "RAW=", false);
                            if (parameter == "NO") {
                                parameter = get_param (cmd_params, "PULLUP=", false);
                                if (parameter == "NO") {
                                    LOG ("Set as input\r\n")
                                    pinMode (pin, INPUT);
                                } else {
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
                                }
                            }
                            value = digitalRead (pin);
                        } else {
#ifdef ARDUINO_ARCH_ESP8266 //only one ADC on ESP8266 A0
                            value = analogRead (A0);
#else
                            value = analogRead (pin);
#endif
                        }
                        LOG ("Read:");
                        LOG (String (value).c_str() )
                        LOG ("\n");
                        ESPCOM::println (String (value).c_str(), output, espresponse);
                    } else {
                        //it is a set
                        int value = parameter.toInt();
                        if (isdigital) {
                            //verify it is a 0 or a 1
                            if ( (value == 0) || (value == 1) ) {
                                pinMode (pin, OUTPUT);
                                LOG ("Set:")
                                LOG (String ( (value == 0) ? LOW : HIGH) )
                                LOG ("\r\n")
                                digitalWrite (pin, (value == 0) ? LOW : HIGH);
                                ESPCOM::println (OK_CMD_MSG, output, espresponse);
                            } else {
                                ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse);
                                response = false;
                            }
                        } else {
                            int analog_range= 255;
                            parameter = get_param (cmd_params, "ANALOG_RANGE=", false);
                            if (parameter.length() > 0) {
                                analog_range = parameter.toInt();
                            }
                            LOG ("Range ")
                            LOG(String (analog_range).c_str() )
                            LOG ("\r\n")
                            if ( (value >= 0) || (value <= analog_range+1) ) {
                                LOG ("Set:")
                                LOG (String ( value) )
                                LOG ("\r\n")
#ifdef ARDUINO_ARCH_ESP8266

                                analogWriteRange(analog_range);
                                pinMode(pin, OUTPUT);
                                analogWrite(pin, value);
#else
                                int channel  = -1;
                                for (uint8_t p = 0; p < 16; p++) {
                                    if(ChannelAttached2Pin[p] == pin) {
                                        channel = p;
                                    }
                                }
                                if (channel==-1) {
                                    for (uint8_t p = 0; p < 16; p++) {
                                        if(ChannelAttached2Pin[p] == -1) {
                                            channel = p;
                                            ChannelAttached2Pin[p] = pin;
                                            p  = 16;
                                        }
                                    }
                                }
                                uint8_t resolution = 0;
                                analog_range++;
                                switch(analog_range) {
                                case 8191:
                                    resolution=13;
                                    break;
                                case 1024:
                                    resolution=10;
                                    break;
                                case 2047:
                                    resolution=11;
                                    break;
                                case 4095:
                                    resolution=12;
                                    break;
                                default:
                                    resolution=8;
                                    analog_range = 255;
                                    break;
                                }
                                if ((channel==-1) || (value > (analog_range-1))) {
                                    ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse);
                                    return false;
                                }
                                ledcSetup(channel, 1000, resolution);
                                ledcAttachPin(pin, channel);
                                ledcWrite(channel, value);
#endif
                            } else {
                                ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse);
                                response = false;
                            }
                        }
                    }
                } else {
                    ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse);
                    response = false;
                }
            }
        }
        break;
#endif


#ifdef ESP_OLED_FEATURE
    //Output to oled
    //[ESP210]<Text>
    case 210: {
        parameter = get_param (cmd_params, "C=", false);
        int c = parameter.toInt();
        parameter = get_param (cmd_params, "L=", false);
        int l = parameter.toInt();
        parameter = get_param (cmd_params, "T=", true);
        OLED_DISPLAY::setCursor(c, l);
        ESPCOM::print(parameter.c_str(), OLED_PIPE);
        ESPCOM::println (OK_CMD_MSG, output, espresponse);
    }
    break;
    //Output to oled line 1
    //[ESP211]<Text>
    case 211: {
        parameter = get_param (cmd_params, "", true);
        OLED_DISPLAY::setCursor(0, 0);
        ESPCOM::print(parameter.c_str(), OLED_PIPE);
        ESPCOM::println (OK_CMD_MSG, output, espresponse);
    }
    break;
    //Output to oled line 2
    //[ESP212]<Text>
    case 212: {
        parameter = get_param (cmd_params, "", true);
        OLED_DISPLAY::setCursor(0, 16);
        ESPCOM::print(parameter.c_str(), OLED_PIPE);
        ESPCOM::println (OK_CMD_MSG, output, espresponse);
    }
    break;
    //Output to oled line 3
    //[ESP213]<Text>
    case 213: {
        parameter = get_param (cmd_params, "", true);
        OLED_DISPLAY::setCursor(0, 32);
        ESPCOM::print(parameter.c_str(), OLED_PIPE);
        ESPCOM::println (OK_CMD_MSG, output, espresponse);
    }
    break;
    //Output to oled line 4
    //[ESP214]<Text>
    case 214: {
        parameter = get_param (cmd_params, "", true);
        OLED_DISPLAY::setCursor(0, 48);
        ESPCOM::print(parameter.c_str(), OLED_PIPE);
        ESPCOM::println (OK_CMD_MSG, output, espresponse);
    }
    break;
#endif
    //Command delay
    case 290: {
        parameter = get_param (cmd_params, "", true);
#ifdef AUTHENTICATION_FEATURE
        if (auth_type == LEVEL_GUEST) {
            ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse);
            response = false;
        } else
#endif
        {
            if (parameter.length() != 0) {
                ESPCOM::println ("Pause", output, espresponse);
                CONFIG::wait(parameter.toInt());
                }
            ESPCOM::println (OK_CMD_MSG, output, espresponse);
        }
    }
    break;
    //display ESP3D EEPROM version detected
    case 300: {
        uint8_t v = CONFIG::get_EEPROM_version();
        ESPCOM::println (String(v).c_str(), output, espresponse);
    }
    break;
    
    //Get full EEPROM settings content
    //[ESP400]
    case 400: {
        char sbuf[MAX_DATA_LENGTH + 1];
        uint8_t ipbuf[4];
        byte bbuf = 0;
        int ibuf = 0;
        parameter = get_param (cmd_params, "", true);
        //Start JSON
        ESPCOM::println (F ("{\"EEPROM\":["), output, espresponse);
        if (cmd_params == "network" || cmd_params == "") {

            //1- Baud Rate
            ESPCOM::print (F ("{\"F\":\"network\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_BAUD_RATE), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"I\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_buffer (EP_BAUD_RATE,  (byte *) &ibuf, INTEGER_LENGTH) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (ibuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"Baud Rate\",\"O\":[{\"9600\":\"9600\"},{\"19200\":\"19200\"},{\"38400\":\"38400\"},{\"57600\":\"57600\"},{\"115200\":\"115200\"},{\"230400\":\"230400\"},{\"250000\":\"250000\"},{\"500000\":\"500000\"},{\"921600\":\"921600\"}]}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);

            //2-Sleep Mode
            ESPCOM::print (F ("{\"F\":\"network\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_SLEEP_MODE), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_byte (EP_SLEEP_MODE, &bbuf ) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"Sleep Mode\",\"O\":[{\"None\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (WIFI_NONE_SLEEP), output, espresponse);
            ESPCOM::print (F ("\"},{\"Light\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (WIFI_LIGHT_SLEEP), output, espresponse);
            ESPCOM::print (F ("\"},{\"Modem\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (WIFI_MODEM_SLEEP), output, espresponse);
            ESPCOM::print (F ("\"}]}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);

            //3-Web Port
            ESPCOM::print (F ("{\"F\":\"network\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_WEB_PORT), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"I\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_buffer (EP_WEB_PORT,  (byte *) &ibuf, INTEGER_LENGTH) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (ibuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"Web Port\",\"S\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (DEFAULT_MAX_WEB_PORT), output, espresponse);
            ESPCOM::print (F ("\",\"M\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (DEFAULT_MIN_WEB_PORT), output, espresponse);
            ESPCOM::print (F ("\"}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);

            //4-Data Port
            ESPCOM::print (F ("{\"F\":\"network\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_DATA_PORT), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"I\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_buffer (EP_DATA_PORT,  (byte *) &ibuf, INTEGER_LENGTH) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (ibuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"Data Port\",\"S\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (DEFAULT_MAX_DATA_PORT), output, espresponse);
            ESPCOM::print (F ("\",\"M\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (DEFAULT_MIN_DATA_PORT), output, espresponse);
            ESPCOM::print (F ("\"}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);
#ifdef AUTHENTICATION_FEATURE
            //5-Admin password
            ESPCOM::print (F ("{\"F\":\"network\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_ADMIN_PWD), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"S\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_string (EP_ADMIN_PWD, sbuf, MAX_LOCAL_PASSWORD_LENGTH) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ("********", output, espresponse);
            }
            ESPCOM::print (F ("\",\"S\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MAX_LOCAL_PASSWORD_LENGTH), output, espresponse);
            ESPCOM::print (F ("\",\"H\":\"Admin Password\",\"M\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MIN_LOCAL_PASSWORD_LENGTH), output, espresponse);
            ESPCOM::print (F ("\"}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);

            //6-User password
            ESPCOM::print (F ("{\"F\":\"network\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_USER_PWD), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"S\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_string (EP_USER_PWD, sbuf, MAX_LOCAL_PASSWORD_LENGTH) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ("********", output, espresponse);
            }
            ESPCOM::print (F ("\",\"S\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MAX_LOCAL_PASSWORD_LENGTH), output, espresponse);
            ESPCOM::print (F ("\",\"H\":\"User Password\",\"M\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MIN_LOCAL_PASSWORD_LENGTH), output, espresponse);
            ESPCOM::print (F ("\"}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);
#endif
            //7-Hostname
            ESPCOM::print (F ("{\"F\":\"network\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_HOSTNAME), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"S\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_string (EP_HOSTNAME, sbuf, MAX_HOSTNAME_LENGTH) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print (encodeString(sbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"Hostname\" ,\"S\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MAX_HOSTNAME_LENGTH), output, espresponse);
            ESPCOM::print (F ("\", \"M\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MIN_HOSTNAME_LENGTH), output, espresponse);
            ESPCOM::print (F ("\"}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);

            //8-wifi mode
            ESPCOM::print (F ("{\"F\":\"network\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_WIFI_MODE), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_byte (EP_WIFI_MODE, &bbuf ) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"Wifi mode\",\"O\":[{\"AP\":\"1\"},{\"STA\":\"2\"}]}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);

            //9-STA SSID
            ESPCOM::print (F ("{\"F\":\"network\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_STA_SSID), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"S\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_string (EP_STA_SSID, sbuf, MAX_SSID_LENGTH) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print (encodeString(sbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"S\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MAX_SSID_LENGTH), output, espresponse);
            ESPCOM::print (F ("\",\"H\":\"Station SSID\",\"M\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MIN_SSID_LENGTH), output, espresponse);
            ESPCOM::print (F ("\"}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);

            //10-STA password
            ESPCOM::print (F ("{\"F\":\"network\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_STA_PASSWORD), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"S\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_string (EP_STA_PASSWORD, sbuf, MAX_PASSWORD_LENGTH) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ("********", output, espresponse);
            }
            ESPCOM::print (F ("\",\"S\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MAX_PASSWORD_LENGTH), output, espresponse);
            ESPCOM::print (F ("\",\"H\":\"Station Password\",\"M\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MIN_PASSWORD_LENGTH), output, espresponse);
            ESPCOM::print (F ("\"}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);

            //11-Station Network Mode
            ESPCOM::print (F ("{\"F\":\"network\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_STA_PHY_MODE), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_byte (EP_STA_PHY_MODE, &bbuf ) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"Station Network Mode\",\"O\":[{\"11b\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (WIFI_PHY_MODE_11B), output, espresponse);
            ESPCOM::print (F ("\"},{\"11g\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (WIFI_PHY_MODE_11G), output, espresponse);
            ESPCOM::print (F ("\"},{\"11n\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (WIFI_PHY_MODE_11N), output, espresponse);
            ESPCOM::print (F ("\"}]}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);

            //12-STA IP mode
            ESPCOM::print (F ("{\"F\":\"network\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_STA_IP_MODE), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_byte (EP_STA_IP_MODE, &bbuf ) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"Station IP Mode\",\"O\":[{\"DHCP\":\"1\"},{\"Static\":\"2\"}]}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);

            //13-STA static IP
            ESPCOM::print (F ("{\"F\":\"network\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_STA_IP_VALUE), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"A\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_buffer (EP_STA_IP_VALUE, (byte *) ipbuf, IP_LENGTH) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print (IPAddress (ipbuf).toString().c_str(), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"Station Static IP\"}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);

            //14-STA static Mask
            ESPCOM::print (F ("{\"F\":\"network\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_STA_MASK_VALUE), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"A\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_buffer (EP_STA_MASK_VALUE, (byte *) ipbuf, IP_LENGTH) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print (IPAddress (ipbuf).toString().c_str(), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"Station Static Mask\"}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);

            //15-STA static Gateway
            ESPCOM::print (F ("{\"F\":\"network\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_STA_GATEWAY_VALUE), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"A\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_buffer (EP_STA_GATEWAY_VALUE, (byte *) ipbuf, IP_LENGTH) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print (IPAddress (ipbuf).toString().c_str(), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"Station Static Gateway\"}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);

            //16-AP SSID
            ESPCOM::print (F ("{\"F\":\"network\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_AP_SSID), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"S\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_string (EP_AP_SSID, sbuf, MAX_SSID_LENGTH) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print (encodeString(sbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"S\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MAX_SSID_LENGTH), output, espresponse);
            ESPCOM::print (F ("\",\"H\":\"AP SSID\",\"M\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MIN_SSID_LENGTH), output, espresponse);
            ESPCOM::print (F ("\"}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);

            //17-AP password
            ESPCOM::print (F ("{\"F\":\"network\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_AP_PASSWORD), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"S\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_string (EP_AP_PASSWORD, sbuf, MAX_PASSWORD_LENGTH) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ("********", output, espresponse);
            }
            ESPCOM::print (F ("\",\"S\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MAX_PASSWORD_LENGTH), output, espresponse);
            ESPCOM::print (F ("\",\"H\":\"AP Password\",\"M\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MIN_PASSWORD_LENGTH), output, espresponse);
            ESPCOM::print (F ("\"}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);

            //18 - AP Network Mode
            ESPCOM::print (F ("{\"F\":\"network\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_AP_PHY_MODE), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_byte (EP_AP_PHY_MODE, &bbuf ) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"AP Network Mode\",\"O\":[{\"11b\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (WIFI_PHY_MODE_11B), output, espresponse);
            ESPCOM::print (F ("\"},{\"11g\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (WIFI_PHY_MODE_11G), output, espresponse);
            ESPCOM::print (F ("\"}]}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);

            //19-AP SSID visibility
            ESPCOM::print (F ("{\"F\":\"network\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_SSID_VISIBLE), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_byte (EP_SSID_VISIBLE, &bbuf ) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"SSID Visible\",\"O\":[{\"No\":\"0\"},{\"Yes\":\"1\"}]}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);

            //20-AP Channel
            ESPCOM::print (F ("{\"F\":\"network\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_CHANNEL), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_byte (EP_CHANNEL, &bbuf ) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"AP Channel\",\"O\":["), output, espresponse);
            for (int i = 1; i < 12 ; i++) {
                ESPCOM::print (F ("{\""), output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (i), output, espresponse);
                ESPCOM::print (F ("\":\""), output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (i), output, espresponse);
                ESPCOM::print (F ("\"}"), output, espresponse);
                if (i < 11) {
                    ESPCOM::print (F (","), output, espresponse);
                }
            }
            ESPCOM::print (F ("]}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);

            //21-AP Authentication
            ESPCOM::print (F ("{\"F\":\"network\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_AUTH_TYPE), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_byte (EP_AUTH_TYPE, &bbuf ) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"Authentication\",\"O\":[{\"Open\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (AUTH_OPEN), output, espresponse);
            ESPCOM::print (F ("\"},{\"WPA\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (AUTH_WPA_PSK), output, espresponse);
            ESPCOM::print (F ("\"},{\"WPA2\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (AUTH_WPA2_PSK), output, espresponse);
            ESPCOM::print (F ("\"},{\"WPA/WPA2\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (AUTH_WPA_WPA2_PSK), output, espresponse);
            ESPCOM::print (F ("\"}]}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);

            //22-AP IP mode
            ESPCOM::print (F ("{\"F\":\"network\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_AP_IP_MODE), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_byte (EP_AP_IP_MODE, &bbuf ) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"AP IP Mode\",\"O\":[{\"DHCP\":\"1\"},{\"Static\":\"2\"}]}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);

            //23-AP static IP
            ESPCOM::print (F ("{\"F\":\"network\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_AP_IP_VALUE), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"A\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_buffer (EP_AP_IP_VALUE, (byte *) ipbuf, IP_LENGTH) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print (IPAddress (ipbuf).toString().c_str(), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"AP Static IP\"}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);

            //24-AP static Mask
            ESPCOM::print (F ("{\"F\":\"network\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_AP_MASK_VALUE), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"A\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_buffer (EP_AP_MASK_VALUE, (byte *) ipbuf, IP_LENGTH) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print (IPAddress (ipbuf).toString().c_str(), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"AP Static Mask\"}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);

            //25-AP static Gateway
            ESPCOM::print (F ("{\"F\":\"network\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_AP_GATEWAY_VALUE), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"A\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_buffer (EP_AP_GATEWAY_VALUE, (byte *) ipbuf, IP_LENGTH) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print (IPAddress (ipbuf).toString().c_str(), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"AP Static Gateway\"}"), output, espresponse);
#if defined(TIMESTAMP_FEATURE)
            ESPCOM::println (F (","), output, espresponse);
            //26-Time zone
            ESPCOM::print (F ("{\"F\":\"network\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_TIMEZONE), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_byte (EP_TIMEZONE, &bbuf ) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr ( (int8_t) bbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"Time Zone\",\"O\":["), output, espresponse);
            for (int i = -12; i <= 12 ; i++) {
                ESPCOM::print (F ("{\""), output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (i), output, espresponse);
                ESPCOM::print (F ("\":\""), output, espresponse);
                ESPCOM::print ( (const char *) CONFIG::intTostr (i), output, espresponse);
                ESPCOM::print (F ("\"}"), output, espresponse);
                if (i < 12) {
                    ESPCOM::print (F (","), output, espresponse);
                }
            }
            ESPCOM::print (F ("]}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);

            //27- DST
            ESPCOM::print (F ("{\"F\":\"network\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_TIME_ISDST), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_byte (EP_TIME_ISDST, &bbuf ) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"Day Saving Time\",\"O\":[{\"No\":\"0\"},{\"Yes\":\"1\"}]}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);

            //28- Time Server1
            ESPCOM::print (F ("{\"F\":\"network\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_TIME_SERVER1), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"S\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_string (EP_TIME_SERVER1, sbuf, MAX_DATA_LENGTH) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print (encodeString(sbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"S\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MAX_DATA_LENGTH), output, espresponse);
            ESPCOM::print (F ("\",\"H\":\"Time Server 1\",\"M\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MIN_DATA_LENGTH), output, espresponse);
            ESPCOM::print (F ("\"}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);

            //29- Time Server2
            ESPCOM::print (F ("{\"F\":\"network\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_TIME_SERVER2), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"S\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_string (EP_TIME_SERVER2, sbuf, MAX_DATA_LENGTH) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print (encodeString(sbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"S\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MAX_DATA_LENGTH), output, espresponse);
            ESPCOM::print (F ("\",\"H\":\"Time Server 2\",\"M\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MIN_DATA_LENGTH), output, espresponse);
            ESPCOM::print (F ("\"}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);

            //30- Time Server3
            ESPCOM::print (F ("{\"F\":\"network\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_TIME_SERVER3), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"S\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_string (EP_TIME_SERVER3, sbuf, MAX_DATA_LENGTH) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print (encodeString(sbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"S\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MAX_DATA_LENGTH), output, espresponse);
            ESPCOM::print (F ("\",\"H\":\"Time Server 3\",\"M\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MIN_DATA_LENGTH), output, espresponse);
            ESPCOM::print (F ("\"}"), output, espresponse);
#endif

#ifdef NOTIFICATION_FEATURE
            ESPCOM::println (F (","), output, espresponse);
            //Notification type
            ESPCOM::print (F ("{\"F\":\"network\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (ESP_NOTIFICATION_TYPE), output, espresponse);
            ESPCOM::print (F("\",\"T\":\"B\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_byte (ESP_NOTIFICATION_TYPE, &bbuf ) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
            }
            ESPCOM::print (F("\",\"H\":\"Notification\",\"O\":[{\"None\":\"0\"},{\"Pushover\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (ESP_PUSHOVER_NOTIFICATION), output, espresponse);
            ESPCOM::print (F("\"},{\"Email\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (ESP_EMAIL_NOTIFICATION), output, espresponse);
            ESPCOM::print (F("\"},{\"Line\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (ESP_LINE_NOTIFICATION), output, espresponse);
            ESPCOM::print (F("\"}]}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);
            //Token 1
            ESPCOM::print (F("{\"F\":\"network\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (ESP_NOTIFICATION_TOKEN1), output, espresponse);
            ESPCOM::print ( F("\",\"T\":\"S\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_string (ESP_NOTIFICATION_TOKEN1, sbuf, MAX_NOTIFICATION_TOKEN_LENGTH) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ("********", output, espresponse);
            }
            ESPCOM::print ( F("\",\"S\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MAX_NOTIFICATION_TOKEN_LENGTH), output, espresponse);
            ESPCOM::print ( F ("\",\"H\":\"Token 1\",\"M\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MIN_NOTIFICATION_TOKEN_LENGTH), output, espresponse);
            ESPCOM::print ( F("\"}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);
            //Token 2
            ESPCOM::print (F("{\"F\":\"network\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (ESP_NOTIFICATION_TOKEN2), output, espresponse);
            ESPCOM::print ( F("\",\"T\":\"S\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_string (ESP_NOTIFICATION_TOKEN2, sbuf, MAX_NOTIFICATION_TOKEN_LENGTH) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ("********", output, espresponse);
            }
            ESPCOM::print ( F("\",\"S\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MAX_NOTIFICATION_TOKEN_LENGTH), output, espresponse);
            ESPCOM::print ( F ("\",\"H\":\"Token 2\",\"M\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MIN_NOTIFICATION_TOKEN_LENGTH), output, espresponse);
            ESPCOM::print ( F("\"}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);
            //Notifications Settings
            ESPCOM::print (F("{\"F\":\"network\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (ESP_NOTIFICATION_SETTINGS), output, espresponse);
            ESPCOM::print ( F("\",\"T\":\"S\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_string (ESP_NOTIFICATION_SETTINGS, sbuf, MAX_NOTIFICATION_TOKEN_LENGTH) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print (encodeString(sbuf), output, espresponse);
            }
            ESPCOM::print ( F("\",\"S\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MAX_NOTIFICATION_SETTINGS_LENGTH), output, espresponse);
            ESPCOM::print ( F ("\",\"H\":\"Notifications Settings\",\"M\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MIN_NOTIFICATION_SETTINGS_LENGTH), output, espresponse);
            ESPCOM::print ( F("\"}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);
            //Auto Notification
            ESPCOM::print (F ("{\"F\":\"network\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (ESP_AUTO_NOTIFICATION), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_byte (ESP_AUTO_NOTIFICATION, &bbuf ) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"Auto notification\",\"O\":[{\"No\":\"0\"},{\"Yes\":\"1\"}]}"), output, espresponse);
            
            
#endif //NOTIFICATION_FEATURE
        }

        if (cmd_params == "printer" || cmd_params == "") {
            if (cmd_params == "") {
                ESPCOM::println (F (","), output, espresponse);
            }
            //Target FW
            ESPCOM::print (F ("{\"F\":\"printer\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_TARGET_FW), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_byte (EP_TARGET_FW, &bbuf ) ) {
                ESPCOM::print ("Unknown", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
            }
            ESPCOM::print (F ("\",\"H\":\"Target FW\",\"O\":[{\"Repetier\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (REPETIER), output, espresponse);
            ESPCOM::print (F ("\"},{\"Repetier for Davinci\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (REPETIER4DV), output, espresponse);
            ESPCOM::print (F ("\"},{\"Marlin\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MARLIN), output, espresponse);
            ESPCOM::print (F ("\"},{\"Marlin Kimbra\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (MARLINKIMBRA), output, espresponse);
            ESPCOM::print (F ("\"},{\"Smoothieware\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (SMOOTHIEWARE), output, espresponse);
            ESPCOM::print (F ("\"},{\"Grbl\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (GRBL), output, espresponse);
            ESPCOM::print (F ("\"},{\"Unknown\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (UNKNOWN_FW), output, espresponse);
            ESPCOM::print (F ("\"}]}"), output, espresponse);
            ESPCOM::println (F (","), output, espresponse);

            //Output flag
            ESPCOM::print (F ("{\"F\":\"printer\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_OUTPUT_FLAG), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"F\",\"V\":\""), output, espresponse);
            if (!CONFIG::read_byte (EP_OUTPUT_FLAG, &bbuf ) ) {
                ESPCOM::print ("???", output, espresponse);
            } else {
                ESPCOM::print ( (const char *) CONFIG::intTostr (bbuf), output, espresponse);
            }
            String s = "\",\"H\":\"Output msg\",\"O\":[{\"M117\":\"";
            s+= CONFIG::intTostr(FLAG_BLOCK_M117);
            s+= "\"}";
#ifdef ESP_OLED_FEATURE
            s+=",{\"Oled\":\"";
            s+= CONFIG::intTostr(FLAG_BLOCK_OLED);
            s+="\"}";
#endif
            s+=",{\"Serial\":\"";
            s+= CONFIG::intTostr(FLAG_BLOCK_SERIAL);
            s+="\"}";
#ifdef WS_DATA_FEATURE
            s+=",{\"Web Socket\":\"";
            s+= CONFIG::intTostr(FLAG_BLOCK_WSOCKET);
            s+="\"}";
#endif
#ifdef TCP_IP_DATA_FEATURE
            s+=",{\"TCP\":\"";
            s+= CONFIG::intTostr(FLAG_BLOCK_TCP);
            s+="\"}";
#endif
            s+= "]}";
            ESPCOM::print (s, output, espresponse);

#ifdef DHT_FEATURE

            //DHT type
            ESPCOM::println (F (","), output, espresponse);
            ESPCOM::print (F ("{\"F\":\"printer\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_DHT_TYPE), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"B\",\"V\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (CONFIG::DHT_type), output, espresponse);
            ESPCOM::print (F ("\",\"H\":\"DHT Type\",\"O\":[{\"None\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (255), output, espresponse);
            ESPCOM::print (F ("\"},{\"DHT11\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (DHTesp::DHT11), output, espresponse);
            ESPCOM::print (F ("\"},{\"DHT22\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (DHTesp::DHT22), output, espresponse);
            ESPCOM::print (F ("\"},{\"AM2302\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (DHTesp::RHT03), output, espresponse);
            ESPCOM::print (F ("\"},{\"RHT03\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (DHTesp::AM2302), output, espresponse);
            ESPCOM::print (F ("\"}]}"), output, espresponse);

            //DHT interval
            ESPCOM::println (F (","), output, espresponse);
            ESPCOM::print (F ("{\"F\":\"printer\",\"P\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (EP_DHT_INTERVAL), output, espresponse);
            ESPCOM::print (F ("\",\"T\":\"I\",\"V\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (CONFIG::DHT_interval), output, espresponse);
            ESPCOM::print (F ("\",\"H\":\"DHT check (seconds)\",\"S\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (DEFAULT_MAX_WEB_PORT), output, espresponse);
            ESPCOM::print (F ("\",\"M\":\""), output, espresponse);
            ESPCOM::print ( (const char *) CONFIG::intTostr (0), output, espresponse);
            ESPCOM::print (F ("\"}"), output, espresponse);
#endif

        }

        //end JSON
        ESPCOM::println (F ("\n]}"), output, espresponse);
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
        if (! (styp == "B" || styp == "S" || styp == "A" || styp == "I" || styp == "F") ) {
            response = false;
        }
        if ((sval.length() == 0) && !((pos==EP_AP_PASSWORD) || (pos==EP_STA_PASSWORD))) {
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
            if ((styp == "B")  ||  (styp == "F")) {
                byte bbuf = sval.toInt();
                if (!CONFIG::write_byte (pos, bbuf) ) {
                    response = false;
                } else {
                    //dynamique refresh is better than restart the board
                    if (pos == EP_OUTPUT_FLAG) {
                        CONFIG::output_flag = bbuf;
                    }
                    if (pos == EP_TARGET_FW) {
                        CONFIG::InitFirmwareTarget();
                    }
#ifdef DHT_FEATURE
                    if (pos == EP_DHT_TYPE) {
                        CONFIG::DHT_type = bbuf;
                        CONFIG::InitDHT(true);
                    }
#endif
#ifdef NOTIFICATION_FEATURE
                    if (pos == ESP_AUTO_NOTIFICATION) {
                        notificationsservice.setAutonotification ((bbuf == 0)? false: true);
                    }
#endif
#if defined(TIMESTAMP_FEATURE)
                    if ( (pos == EP_TIMEZONE) || (pos == EP_TIME_ISDST) || (pos == EP_TIME_SERVER1) || (pos == EP_TIME_SERVER2) || (pos == EP_TIME_SERVER3) ) {
                        CONFIG::init_time_client();
                    }
#endif
                }
            }
            if (styp == "I") {
                int ibuf = sval.toInt();
                if (!CONFIG::write_buffer (pos, (const byte *) &ibuf, INTEGER_LENGTH) ) {
                    response = false;
                } else {
#ifdef DHT_FEATURE
                    if (pos == EP_DHT_INTERVAL) {
                        CONFIG::DHT_interval = ibuf;
                    }
#endif
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
            ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse);
        } else {
            ESPCOM::println (OK_CMD_MSG, output, espresponse);
        }

    }
    break;

    //Get available AP list (limited to 30)
    //output is JSON or plain text according parameter
    //[ESP410]<plain>
    case 410: {
        parameter = get_param (cmd_params, "", true);
        bool plain = (parameter == "plain");

#if defined(ASYNCWEBSERVER)
        if (!plain) {
            ESPCOM::print (F ("{\"AP_LIST\":["), output, espresponse);
        }
        int n = WiFi.scanComplete();
        if (n == -2) {
            WiFi.scanNetworks (ESP_USE_ASYNC);
        } else if (n) {
#else
        int n =  WiFi.scanNetworks ();
        if (!plain) {
            ESPCOM::print (F ("{\"AP_LIST\":["), output, espresponse);
        }
#endif

            for (int i = 0; i < n; ++i) {
                if (i > 0) {
                    if (!plain) {
                        ESPCOM::print (F (","), output, espresponse);
                    } else {
                        ESPCOM::print (F ("\n"), output, espresponse);
                    }
                }
                if (!plain) {
                    ESPCOM::print (F ("{\"SSID\":\""), output, espresponse);
                    ESPCOM::print (encodeString(WiFi.SSID (i).c_str()), output, espresponse);
                } else ESPCOM::print (WiFi.SSID (i).c_str(), output, espresponse);
                if (!plain) {
                    ESPCOM::print (F ("\",\"SIGNAL\":\""), output, espresponse);
                } else {
                    ESPCOM::print (F ("\t"), output, espresponse);
                }
                ESPCOM::print (CONFIG::intTostr (wifi_config.getSignal (WiFi.RSSI (i) ) ), output, espresponse);;
                //ESPCOM::print(F("%"), output, espresponse);
                if (!plain) {
                    ESPCOM::print (F ("\",\"IS_PROTECTED\":\""), output, espresponse);
                }
                if (WiFi.encryptionType (i) == ENC_TYPE_NONE) {
                    if (!plain) {
                        ESPCOM::print (F ("0"), output, espresponse);
                    } else {
                        ESPCOM::print (F ("\tOpen"), output, espresponse);
                    }
                } else {
                    if (!plain) {
                        ESPCOM::print (F ("1"), output, espresponse);
                    } else {
                        ESPCOM::print (F ("\tSecure"), output, espresponse);
                    }
                }
                if (!plain) {
                    ESPCOM::print (F ("\"}"), output, espresponse);
                }
            }
            WiFi.scanDelete();
#if defined(ASYNCWEBSERVER)
            if (WiFi.scanComplete() == -2) {
                WiFi.scanNetworks (ESP_USE_ASYNC);
            }
        }
#endif
        if (!plain) {
            ESPCOM::print (F ("]}"), output, espresponse);
        } else {
            ESPCOM::print (F ("\n"), output, espresponse);
        }
    }
    break;
#endif //USE_AS_UPDATER_ONLY
    //Get ESP current status in plain or JSON
    //[ESP420]<plain>
    case 420: {
        parameter = get_param (cmd_params, "", true);
        CONFIG::print_config (output, (parameter == "plain"), espresponse);
    }
    break;
    //Set ESP mode
    //cmd is RESET, SAFEMODE, RESTART
    //[ESP444]<cmd>pwd=<admin password>
    case 444:
        parameter = get_param (cmd_params, "", true);
#ifdef AUTHENTICATION_FEATURE
        if (auth_type != LEVEL_ADMIN) {
            ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse);
            response = false;
        } else
#endif
        {
            if (parameter == "RESET") {
                CONFIG::reset_config();
                ESPCOM::println (F ("Reset done - restart needed"), output, espresponse);
            } else if (parameter == "SAFEMODE") {
                wifi_config.Safe_Setup();
                ESPCOM::println (F ("Set Safe Mode  - restart needed"), output, espresponse);
            } else  if (parameter == "RESTART") {
                ESPCOM::println (F ("Restart started"), output, espresponse);
                web_interface->restartmodule = true;
            } else {
                ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse);
                response = false;
            }
        }
        break;
#ifndef USE_AS_UPDATER_ONLY
    //[ESP500]<gcode>
    case 500: { //send GCode with check sum caching right line numbering
        //be sure serial is locked
        if ( (web_interface->blockserial) ) {
            break;
        }
        int32_t linenb = 1;
        cmd_params.trim() ;
        if (sendLine2Serial (cmd_params, linenb,  &linenb)) {
            ESPCOM::println (OK_CMD_MSG, output, espresponse);
        } else { //it may failed because of skip if repetier so let's reset numbering first
            if ( ( CONFIG::GetFirmwareTarget() == REPETIER4DV) || (CONFIG::GetFirmwareTarget() == REPETIER) ) {
                //reset numbering
                String cmd = "M110 N0";
                if (sendLine2Serial (cmd, -1,  NULL)) {
                    linenb = 1;
                    //if success let's try again to send the command
                    if (sendLine2Serial (cmd_params, linenb,  &linenb)) {
                        ESPCOM::println (OK_CMD_MSG, output, espresponse);
                    } else {
                        ESPCOM::println (ERROR_CMD_MSG, output, espresponse);
                        response = false;
                    }
                } else {
                    ESPCOM::println (ERROR_CMD_MSG, output, espresponse);
                    response = false;
                }
            } else {

                ESPCOM::println (ERROR_CMD_MSG, output, espresponse);
                response = false;
            }
        }
    }
    break;
    //[ESP501]<line>
    case 501: { //send line checksum
        cmd_params.trim();
        int8_t chk = Checksum(cmd_params.c_str(),cmd_params.length());
        String schecksum = "Checksum: " + String(chk);
        ESPCOM::println (schecksum, output, espresponse);
    }
#ifdef AUTHENTICATION_FEATURE
    //Change / Reset user password
    //[ESP555]<password>pwd=<admin password>
    case 555: {
        if (auth_type == LEVEL_ADMIN) {
            parameter = get_param (cmd_params, "", true);
            if (parameter.length() == 0) {
                if (CONFIG::write_string (EP_USER_PWD, FPSTR (DEFAULT_USER_PWD) ) ) {
                    ESPCOM::println (OK_CMD_MSG, output, espresponse);
                } else {
                    ESPCOM::println (ERROR_CMD_MSG, output, espresponse);
                    response = false;
                }
            } else {
                if (CONFIG::isLocalPasswordValid (parameter.c_str() ) ) {
                    if (CONFIG::write_string (EP_USER_PWD, parameter.c_str() ) ) {
                        ESPCOM::println (OK_CMD_MSG, output, espresponse);
                    } else {
                        ESPCOM::println (ERROR_CMD_MSG, output, espresponse);
                        response = false;
                    }
                } else {
                    ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse);
                    response = false;
                }
            }
        } else {
            ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse);
            response = false;
        }
        break;
    }
#endif
#ifdef NOTIFICATION_FEATURE
    //Send Notification
    //[ESP600]msg [pwd=<admin password>]
    case 600:
#ifdef AUTHENTICATION_FEATURE
        if (auth_type == LEVEL_GUEST) {
            ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse);
            return false;
        }
#endif
        parameter = get_param (cmd_params, "", true);
        if (parameter.length() == 0) {
            ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse);
            return false;
        }
        if (notificationsservice.sendMSG("ESP3D Notification", parameter.c_str())) {
            ESPCOM::println (OK_CMD_MSG, output, espresponse);
        } else {
            ESPCOM::println (ERROR_CMD_MSG, output, espresponse);
            response = false;
        }
        break;
    //Set/Get Notification settings
    //[ESP610]type=<NONE/PUSHOVER/EMAIL/LINE> T1=<token1> T2=<token2> TS=<Settings> [pwd=<admin password>]
    //Get will give type and settings only not the protected T1/T2
    case 610:
#ifdef AUTHENTICATION_FEATURE
        if (auth_type == LEVEL_GUEST) {
            ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse);
            return false;
        }
#endif
        parameter = get_param (cmd_params, "", false);
        //get
        if (parameter.length() == 0) {
            uint8_t Ntype =  0;
            if (!CONFIG::read_byte (ESP_NOTIFICATION_TYPE, &Ntype ) ) {
                Ntype =0;
            }
            char sbuf[MAX_DATA_LENGTH + 1];
            static String tmp;
            tmp = (Ntype == ESP_PUSHOVER_NOTIFICATION)?"PUSHOVER":(Ntype == ESP_EMAIL_NOTIFICATION)?"EMAIL":(Ntype == ESP_LINE_NOTIFICATION)?"LINE":"NONE";
            if (CONFIG::read_string (ESP_NOTIFICATION_SETTINGS, sbuf, MAX_NOTIFICATION_SETTINGS_LENGTH) ) {
                tmp+= " ";
                tmp += sbuf;
            }
            ESPCOM::println (tmp.c_str(), output, espresponse);
        } else {
            response = false;
            //type
            parameter = get_param (cmd_params, "type=");
            if (parameter.length() > 0) {
                uint8_t Ntype;
                parameter.toUpperCase();
                if (parameter == "NONE") {
                    Ntype = 0;
                } else if (parameter == "PUSHOVER") {
                    Ntype = ESP_PUSHOVER_NOTIFICATION;
                } else if (parameter == "EMAIL") {
                    Ntype = ESP_EMAIL_NOTIFICATION;
                } else if (parameter == "LINE") {
                    Ntype = ESP_LINE_NOTIFICATION;
                } else {
                    ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse);
                    return false;
                }
                if (!CONFIG::write_byte (ESP_NOTIFICATION_TYPE, Ntype) ) {
                    ESPCOM::println (ERROR_CMD_MSG, output, espresponse);
                    return false;
                } else {
                    response = true;
                }
            }
            //Settings
            parameter = get_param (cmd_params, "TS=");
            if (parameter.length() > 0) {
                if (!CONFIG::write_string (ESP_NOTIFICATION_SETTINGS, parameter.c_str() ) ) {
                    ESPCOM::println (ERROR_CMD_MSG, output, espresponse);
                    return false;
                } else {
                    response = true;
                }
            }
            //Token1
            parameter = get_param (cmd_params, "T1=");
            if (parameter.length() > 0) {
                if (!CONFIG::write_string (ESP_NOTIFICATION_TOKEN1, parameter.c_str() ) ) {
                    ESPCOM::println (ERROR_CMD_MSG, output, espresponse);
                    return false;
                } else {
                    response = true;
                }
            }
            //Token2
            parameter = get_param (cmd_params, "T2=");
            if (parameter.length() > 0) {
                if (!CONFIG::write_string (ESP_NOTIFICATION_TOKEN2, parameter.c_str() ) ) {
                    ESPCOM::println (ERROR_CMD_MSG, output, espresponse);
                    return false;
                } else {
                    response = true;
                }
            }
            if (response) {
                //Restart service
                notificationsservice.begin();
                ESPCOM::println (OK_CMD_MSG, output, espresponse);
            } else {
                ESPCOM::println (ERROR_CMD_MSG, output, espresponse);
            }
        }
        break;
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
            ESPCOM::flush (DEFAULT_PRINTER_PIPE);
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
                                execute_command(cmd_part1.toInt(),cmd_part2,NO_PIPE, auth_type, espresponse);
                            }
                            //if not is not a valid [ESPXXX] command ignore it
                        }
                    } else {
                        //send line to serial
                        ESPCOM::println (currentline, DEFAULT_PRINTER_PIPE);
                        CONFIG::wait (1);
                        //flush to be sure send buffer is empty
                        ESPCOM::flush (DEFAULT_PRINTER_PIPE);
                    }
                    CONFIG::wait (1);
                }
            }
            currentfile.close();
            ESPCOM::println (OK_CMD_MSG, output, espresponse);
        } else {
            ESPCOM::println (ERROR_CMD_MSG, output, espresponse);
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
            ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse);
            response = false;
        } else
#endif
        {
            if (parameter == "FORMAT") {
                ESPCOM::print (F ("Formating"), output, espresponse);
                SPIFFS.format();
                ESPCOM::println (F ("...Done"), output, espresponse);
            } else {
                ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse);
                response = false;
            }
        }
        break;
    //SPIFFS total size and used size
    //[ESP720]<header answer>
    case 720:
        ESPCOM::print (cmd_params, output, espresponse);
#ifdef ARDUINO_ARCH_ESP8266
        fs::FSInfo info;
        SPIFFS.info (info);
        ESPCOM::print ("SPIFFS Total:", output, espresponse);
        ESPCOM::print (CONFIG::formatBytes (info.totalBytes).c_str(), output, espresponse);
        ESPCOM::print (" Used:", output, espresponse);
        ESPCOM::println (CONFIG::formatBytes (info.usedBytes).c_str(), output, espresponse);
#else
        ESPCOM::print ("SPIFFS  Total:", output, espresponse);
        ESPCOM::print (CONFIG::formatBytes (SPIFFS.totalBytes() ).c_str(), output, espresponse);
        ESPCOM::print (" Used:", output, espresponse);
        ESPCOM::println (CONFIG::formatBytes (SPIFFS.usedBytes() ).c_str(), output, espresponse);
#endif
        break;
#endif //USE_AS_UPDATER_ONLY
    //get fw version firmare target and fw version
    //[ESP800]<header answer>
    case 800: {
        byte sd_dir = 0;
        String shost ;
        if (!CONFIG::read_string (EP_HOSTNAME, shost, MAX_HOSTNAME_LENGTH) ) {
            shost = wifi_config.get_default_hostname();
        }
        ESPCOM::print (cmd_params, output, espresponse);
        ESPCOM::print (F ("FW version:"), output, espresponse);
        ESPCOM::print (FW_VERSION, output, espresponse);
        ESPCOM::print (F (" # FW target:"), output, espresponse);
        ESPCOM::print (CONFIG::GetFirmwareTargetShortName(), output, espresponse);
        ESPCOM::print (F (" # FW HW:"), output, espresponse);
        if (CONFIG::is_direct_sd) {
            ESPCOM::print (F ("Direct SD"), output, espresponse);
        } else {
            ESPCOM::print (F ("Serial SD"), output, espresponse);
        }
        ESPCOM::print (F (" # primary sd:"), output, espresponse);
        if (!CONFIG::read_byte (EP_PRIMARY_SD, &sd_dir ) ) {
            sd_dir = DEFAULT_PRIMARY_SD;
        }
        if (sd_dir == SD_DIRECTORY) {
            ESPCOM::print (F ("/sd/"), output, espresponse);
        } else if (sd_dir == EXT_DIRECTORY) {
            ESPCOM::print (F ("/ext/"), output, espresponse);
        } else {
            ESPCOM::print (F ("none"), output, espresponse);
        }
        ESPCOM::print (F (" # secondary sd:"), output, espresponse);
        if (!CONFIG::read_byte (EP_SECONDARY_SD, &sd_dir ) ) {
            sd_dir = DEFAULT_SECONDARY_SD;
        }
        if (sd_dir == SD_DIRECTORY) {
            ESPCOM::print (F ("/sd/"), output, espresponse);
        } else if (sd_dir == EXT_DIRECTORY) {
            ESPCOM::print (F ("/ext/"), output, espresponse);
        } else {
            ESPCOM::print (F ("none"), output, espresponse);
        }
        ESPCOM::print (F (" # authentication:"), output, espresponse);
#ifdef AUTHENTICATION_FEATURE
        ESPCOM::print (F ("yes"), output, espresponse);
#else
        ESPCOM::print (F ("no"), output, espresponse);
#endif
        ESPCOM::print (F (" # webcommunication:"), output, espresponse);
#if defined (ASYNCWEBSERVER)
        ESPCOM::print (F ("Async"), output, espresponse);
#else
        ESPCOM::print (F ("Sync:"), output, espresponse);
        String sp = String(wifi_config.iweb_port+1);
        sp += ":";
        if (WiFi.getMode() == WIFI_STA) {
             sp += WiFi.localIP().toString();
        } else if ((WiFi.getMode() == WIFI_AP) || (WiFi.getMode() == WIFI_AP_STA)) {
             sp += WiFi.softAPIP().toString();
        } else {
             sp += "0.0.0.0";
        }
        ESPCOM::print (sp.c_str(), output, espresponse);
#endif

        ESPCOM::print (F (" # hostname:"), output, espresponse);
        ESPCOM::print (shost, output, espresponse);
        if (WiFi.getMode() == WIFI_AP) {
            ESPCOM::print (F("(AP mode)"), output, espresponse);
        }

        ESPCOM::println ("", output, espresponse);
    }
    break;
#ifndef USE_AS_UPDATER_ONLY
    //get fw target
    //[ESP801]<header answer>
    case 801:
        ESPCOM::print (cmd_params, output, espresponse);
        ESPCOM::println (CONFIG::GetFirmwareTargetShortName(), output, espresponse);
        break;
    case 810:
        web_interface->blockserial = false;
        break;
    case 900:
        parameter = get_param (cmd_params, "", true);
#ifdef AUTHENTICATION_FEATURE
        if (auth_type == LEVEL_GUEST) {
            ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse);
            response = false;
        }
#endif
        if (parameter.length() == 0) {
            if (CONFIG::is_com_enabled) {
                ESPCOM::print (F ("ENABLED"), output, espresponse);
            } else {
                ESPCOM::print (F ("DISABLED"), output, espresponse);
            }
        } else {
            if (parameter == "ENABLE") {
                CONFIG::DisableSerial();
                 if (!CONFIG::InitBaudrate()){
                     ESPCOM::print (F ("Cannot enable serial communication"), output, espresponse);
                 } else {
                     ESPCOM::print (F ("Enable serial communication"), output, espresponse);
                 }
            } else if (parameter == "DISABLE") {
                ESPCOM::print (F ("Disable serial communication"), output, espresponse);
                CONFIG::DisableSerial();
            } else {
                ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse);
                response = false;
            }
        }
        break;
#endif //USE_AS_UPDATER_ONLY
    default:
        ESPCOM::println (INCORRECT_CMD_MSG, output, espresponse);
        response = false;
        break;
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

#ifdef SERIAL_COMMAND_FEATURE
    if (executecmd) {
        String ESP_Command;
        int ESPpos = -1;
#ifdef MKS_TFT_FEATURE
        if (buffer.startsWith("at+")) {
            //echo
            ESPCOM::print (buffer, output);
            ESPCOM::print ("\r\r\n", output);
            if (buffer.startsWith("at+net_wanip=?")) {
                String ipstr;
                if (WiFi.getMode() == WIFI_STA) {
                    ipstr = WiFi.localIP().toString() + "," + WiFi.subnetMask().toString()+ "," + WiFi.gatewayIP().toString()+"\r\n";
                } else {
                    ipstr = WiFi.softAPIP().toString() + ",255.255.255.0," + WiFi.softAPIP().toString()+"\r\n";
                }
                ESPCOM::print (ipstr, output);
            } else if (buffer.startsWith("at+wifi_ConState=?")) {
                ESPCOM::print ("Connected\r\n", output);
            } else {
                ESPCOM::print ("ok\r\n", output);
            }
            return false;
        }
#endif
        ESPpos = buffer.indexOf ("[ESP");
        if (ESPpos == -1 && (CONFIG::GetFirmwareTarget() == SMOOTHIEWARE)) {
            ESPpos = buffer.indexOf ("[esp");
        }
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
            check_command (buffer_serial, DEFAULT_PRINTER_PIPE);
        }
    }
}
