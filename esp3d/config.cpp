/*
  config.cpp- ESP3D configuration class

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
#include <EEPROM.h>
#include "wifi.h"
extern "C" {
#include "user_interface.h"
}


 void CONFIG::esp_restart(){
    LOG("Restarting\n")
    Serial.flush();
    delay(500);
    Serial.swap();
    delay(100);
    ESP.restart();
    while (1) {
        delay(1);
    };
}


bool CONFIG::isHostnameValid(const char * hostname)
{
    //limited size
    char c;
    if (strlen(hostname)>MAX_HOSTNAME_LENGTH || strlen(hostname) < 1) {
        return false;
    }
    //only letter and digit
    for (int i=0; i < strlen(hostname); i++) {
        c = hostname[i];
        if (!(isdigit(c) || isalpha(c) || c=='_')) {
            return false;
        }
        if (c==' ') {
            return false;
        }
    }
    return true;
}

bool CONFIG::isSSIDValid(const char * ssid)
{
    //limited size
    char c;
    if (strlen(ssid)>MAX_SSID_LENGTH || strlen(ssid)<MIN_SSID_LENGTH) {
        return false;
    }
    //only letter and digit
    for (int i=0; i < strlen(ssid); i++) {
        c = ssid[i];
        //if (!(isdigit(c) || isalpha(c))) return false;
        if (c==' ') {
            return false;
        }
    }
    return true;
}

bool CONFIG::isPasswordValid(const char * password)
{
    //limited size
    if ((strlen(password)>MAX_PASSWORD_LENGTH)||  (strlen(password)<MIN_PASSWORD_LENGTH)) {
        return false;
    }
    //no space allowed
    for (int i=0; i < strlen(password); i++)
        if (password[i] == ' ') {
            return false;
        }

    return true;
}

bool CONFIG::isLocalPasswordValid(const char * password)
{
    char c;
    //limited size
    if ((strlen(password)>MAX_LOCAL_PASSWORD_LENGTH)||  (strlen(password)<MIN_LOCAL_PASSWORD_LENGTH)) {
        return false;
    }
    //no space allowed
    for (int i=0; i < strlen(password); i++) {
        c= password[i];
        if (c==' ') {
            return false;
        }
    }
    return true;
}

bool CONFIG::isIPValid(const char * IP)
{
    //limited size
    int internalcount=0;
    int dotcount = 0;
    bool previouswasdot=false;
    char c;

    if (strlen(IP)>15 || strlen(IP)==0) {
        return false;
    }
    //cannot start with .
    if (IP[0]=='.') {
        return false;
    }
    //only letter and digit
    for (int i=0; i < strlen(IP); i++) {
        c = IP[i];
        if (isdigit(c)) {
            //only 3 digit at once
            internalcount++;
            previouswasdot=false;
            if (internalcount>3) {
                return false;
            }
        } else if(c=='.') {
            //cannot have 2 dots side by side
            if (previouswasdot) {
                return false;
            }
            previouswasdot=true;
            internalcount=0;
            dotcount++;
        }//if not a dot neither a digit it is wrong
        else {
            return false;
        }
    }
    //if not 3 dots then it is wrong
    if (dotcount!=3) {
        return false;
    }
    //cannot have the last dot as last char
    if (IP[strlen(IP)-1]=='.') {
        return false;
    }
    return true;
}

char * CONFIG::intTostr(int value)
{
    static char result [12];
    sprintf(result,"%d",value);
    return result;
}

String CONFIG::formatBytes(size_t bytes)
{
    if (bytes < 1024) {
        return String(bytes)+" B";
    } else if(bytes < (1024 * 1024)) {
        return String(bytes/1024.0)+" KB";
    } else if(bytes < (1024 * 1024 * 1024)) {
        return String(bytes/1024.0/1024.0)+" MB";
    } else {
        return String(bytes/1024.0/1024.0/1024.0)+" GB";
    }
}

//helper to convert string to IP
//do not use IPAddress.fromString() because lack of check point and error result
//return number of parts
byte CONFIG::split_ip (const char * ptr,byte * part)
{
    if (strlen(ptr)>15 || strlen(ptr)< 7) {
        part[0]=0;
        part[1]=0;
        part[2]=0;
        part[3]=0;
        return 0;
    }

    char pstart [16];
    char * ptr2;
    strcpy(pstart,ptr);
    ptr2 = pstart;
    byte i = strlen(pstart);
    byte pos = 0;
    for (byte j=0; j<i; j++) {
        if (pstart[j]=='.') {
            if (pos==4) {
                part[0]=0;
                part[1]=0;
                part[2]=0;
                part[3]=0;
                return 0;
            }
            pstart[j]=0x0;
            part[pos]=atoi(ptr2);
            pos++;
            ptr2 = &pstart[j+1];
        }
    }
    part[pos]=atoi(ptr2);
    return pos+1;
}

//just simple helper to convert mac address to string
char * CONFIG::mac2str(uint8_t mac [WL_MAC_ADDR_LENGTH])
{
    static char macstr [18];
    if (0>sprintf(macstr,"%02X:%02X:%02X:%02X:%02X:%02X",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5])) {
        strcpy (macstr, "00:00:00:00:00:00");
    }
    return macstr;
}


//read a string
//a string is multibyte + \0, this is won't work if 1 char is multibyte like chinese char
bool CONFIG::read_string(int pos, char byte_buffer[], int size_max)
{
    //check if parameters are acceptable
    if (size_max==0 ||  pos+size_max+1 > EEPROM_SIZE || byte_buffer== NULL) {
        LOG("Error read string\n")
        return false;
    }
    EEPROM.begin(EEPROM_SIZE);
    byte b = 13; // non zero for the while loop below
    int i=0;

    //read until max size is reached or \0 is found
    while (i < size_max && b != 0) {
        b = EEPROM.read(pos+i);
        byte_buffer[i]=b;
        i++;
    }

    // Be sure there is a 0 at the end.
    if (b!=0) {
        byte_buffer[i-1]=0x00;
    }
    EEPROM.end();

    return true;
}

bool CONFIG::read_string(int pos, String & sbuffer, int size_max)
{
    //check if parameters are acceptable
    if (size_max==0 ||  pos+size_max+1 > EEPROM_SIZE ) {
        LOG("Error read string\n")
        return false;
    }
    byte b = 13; // non zero for the while loop below
    int i=0;
    sbuffer="";

    EEPROM.begin(EEPROM_SIZE);
    //read until max size is reached or \0 is found
    while (i < size_max && b != 0) {
        b = EEPROM.read(pos+i);
        if (b!=0)sbuffer+=char(b);
        i++;
    }
    EEPROM.end();

    return true;
}

//read a buffer of size_buffer
bool CONFIG::read_buffer(int pos, byte byte_buffer[], int size_buffer)
{
    //check if parameters are acceptable
    if (size_buffer==0 ||  pos+size_buffer > EEPROM_SIZE || byte_buffer== NULL) {
        LOG("Error read buffer\n")
        return false;
    }
    int i=0;
    EEPROM.begin(EEPROM_SIZE);
    //read until max size is reached
    while (i<size_buffer ) {
        byte_buffer[i]=EEPROM.read(pos+i);
        i++;
    }
    EEPROM.end();
    return true;
}

//read a flag / byte
bool CONFIG::read_byte(int pos, byte * value)
{
    //check if parameters are acceptable
    if (pos+1 > EEPROM_SIZE) {
        LOG("Error read byte\n")
        return false;
    }
    EEPROM.begin(EEPROM_SIZE);
    value[0] = EEPROM.read(pos);
    EEPROM.end();
    return true;
}

bool CONFIG::write_string(int pos, const __FlashStringHelper *str)
{
    String stmp = str;
    return write_string(pos,stmp.c_str());
}

//write a string (array of byte with a 0x00  at the end)
bool CONFIG::write_string(int pos, const char * byte_buffer)
{
    int size_buffer;
    int maxsize = EEPROM_SIZE;
    size_buffer= strlen(byte_buffer);
    //check if parameters are acceptable
    switch (pos)
    {
        case EP_ADMIN_PWD:
        case EP_USER_PWD:
            maxsize = MAX_LOCAL_PASSWORD_LENGTH;
            break;
        case EP_AP_SSID:
        case EP_STA_SSID:
            maxsize = MAX_SSID_LENGTH;
            break;
          case EP_AP_PASSWORD:
          case EP_STA_PASSWORD:
            maxsize = MAX_PASSWORD_LENGTH;
            break;
         case EP_HOSTNAME:
            maxsize = MAX_HOSTNAME_LENGTH;
            break;
        default:
            maxsize = EEPROM_SIZE;
            break;
    }
    if (size_buffer==0 ||  pos+size_buffer+1 > EEPROM_SIZE || size_buffer > maxsize  || byte_buffer== NULL) {
        LOG("Error write string\n")
        return false;
    }
    //copy the value(s)
    EEPROM.begin(EEPROM_SIZE);
    for (int i = 0; i < size_buffer; i++) {
        EEPROM.write(pos + i, byte_buffer[i]);
    }

    //0 terminal
    EEPROM.write(pos + size_buffer, 0x00);
    EEPROM.commit();
    EEPROM.end();
    return true;
}

//write a buffer
bool CONFIG::write_buffer(int pos, const byte * byte_buffer, int size_buffer)
{
    //check if parameters are acceptable
    if (size_buffer==0 ||  pos+size_buffer > EEPROM_SIZE || byte_buffer== NULL) {
        LOG("Error write buffer\n")
        return false;
    }
    EEPROM.begin(EEPROM_SIZE);
    //copy the value(s)
    for (int i = 0; i < size_buffer; i++) {
        EEPROM.write(pos + i, byte_buffer[i]);
    }
    EEPROM.commit();
    EEPROM.end();
    return true;
}

//read a flag / byte
bool CONFIG::write_byte(int pos, const byte value)
{
    //check if parameters are acceptable
    if (pos+1 > EEPROM_SIZE) {
        LOG("Error write byte\n")
        return false;
    }
    EEPROM.begin(EEPROM_SIZE);
    EEPROM.write(pos, value);
    EEPROM.commit();
    EEPROM.end();
    return true;
}

bool CONFIG::reset_config()
{
    if(!CONFIG::write_byte(EP_WIFI_MODE,DEFAULT_WIFI_MODE)) {
        return false;
    }
    if(!CONFIG::write_buffer(EP_BAUD_RATE,(const byte *)&DEFAULT_BAUD_RATE,INTEGER_LENGTH)) {
        return false;
    }
    if(!CONFIG::write_string(EP_AP_SSID,FPSTR(DEFAULT_AP_SSID))) {
        return false;
    }
    if(!CONFIG::write_string(EP_AP_PASSWORD,FPSTR(DEFAULT_AP_PASSWORD))) {
        return false;
    }
    if(!CONFIG::write_string(EP_STA_SSID,FPSTR(DEFAULT_STA_SSID))) {
        return false;
    }
    if(!CONFIG::write_string(EP_STA_PASSWORD,FPSTR(DEFAULT_STA_PASSWORD))) {
        return false;
    }
    if(!CONFIG::write_byte(EP_AP_IP_MODE,DEFAULT_AP_IP_MODE)) {
        return false;
    }
    if(!CONFIG::write_byte(EP_STA_IP_MODE,DEFAULT_STA_IP_MODE)) {
        return false;
    }
    if(!CONFIG::write_buffer(EP_STA_IP_VALUE,DEFAULT_IP_VALUE,IP_LENGTH)) {
        return false;
    }
    if(!CONFIG::write_buffer(EP_STA_MASK_VALUE,DEFAULT_MASK_VALUE,IP_LENGTH)) {
        return false;
    }
    if(!CONFIG::write_buffer(EP_STA_GATEWAY_VALUE,DEFAULT_GATEWAY_VALUE,IP_LENGTH)) {
        return false;
    }
        if(!CONFIG::write_byte(EP_STA_PHY_MODE,DEFAULT_PHY_MODE)) {
        return false;
    }
    if(!CONFIG::write_buffer(EP_AP_IP_VALUE,DEFAULT_IP_VALUE,IP_LENGTH)) {
        return false;
    }
    if(!CONFIG::write_buffer(EP_AP_MASK_VALUE,DEFAULT_MASK_VALUE,IP_LENGTH)) {
        return false;
    }
    if(!CONFIG::write_buffer(EP_AP_GATEWAY_VALUE,DEFAULT_GATEWAY_VALUE,IP_LENGTH)) {
        return false;
    }
    if(!CONFIG::write_byte(EP_AP_PHY_MODE,DEFAULT_PHY_MODE)) {
        return false;
    }
    if(!CONFIG::write_byte(EP_SLEEP_MODE,DEFAULT_SLEEP_MODE)) {
        return false;
    }
    if(!CONFIG::write_byte(EP_CHANNEL,DEFAULT_CHANNEL)) {
        return false;
    }
    if(!CONFIG::write_byte(EP_AUTH_TYPE,DEFAULT_AUTH_TYPE)) {
        return false;
    }
    if(!CONFIG::write_byte(EP_SSID_VISIBLE,DEFAULT_SSID_VISIBLE)) {
        return false;
    }
    if(!CONFIG::write_buffer(EP_WEB_PORT,(const byte *)&DEFAULT_WEB_PORT,INTEGER_LENGTH)) {
        return false;
    }
    if(!CONFIG::write_buffer(EP_DATA_PORT,(const byte *)&DEFAULT_DATA_PORT,INTEGER_LENGTH)) {
        return false;
    }
    if(!CONFIG::write_byte(EP_REFRESH_PAGE_TIME,DEFAULT_REFRESH_PAGE_TIME)) {
        return false;
    }
    if(!CONFIG::write_string(EP_HOSTNAME,wifi_config.get_default_hostname())) {
        return false;
    }
    if(!CONFIG::write_buffer(EP_XY_FEEDRATE,(const byte *)&DEFAULT_XY_FEEDRATE,INTEGER_LENGTH)) {
        return false;
    }
    if(!CONFIG::write_buffer(EP_Z_FEEDRATE,(const byte *)&DEFAULT_Z_FEEDRATE,INTEGER_LENGTH)) {
        return false;
    }
    if(!CONFIG::write_buffer(EP_E_FEEDRATE,(const byte *)&DEFAULT_E_FEEDRATE,INTEGER_LENGTH)) {
        return false;
    }
    if(!CONFIG::write_string(EP_ADMIN_PWD,FPSTR(DEFAULT_ADMIN_PWD))) {
        return false;
    }
    if(!CONFIG::write_string(EP_USER_PWD,FPSTR(DEFAULT_USER_PWD))) {
        return false;
    }
    return true;
}

void CONFIG::print_config()
{
    //use biggest size for buffer
    char sbuf[MAX_PASSWORD_LENGTH+1];
    uint8_t ipbuf[4];
    byte bbuf=0;
    int ibuf=0;
    if (CONFIG::read_buffer(EP_BAUD_RATE,  (byte *)&ibuf , INTEGER_LENGTH)) {
        Serial.print(F("Baud rate: "));
        Serial.println(ibuf);
    } else {
        Serial.println(F("Error reading baud rate"));
    }
    if (CONFIG::read_byte(EP_SLEEP_MODE, &bbuf )) {
        Serial.print(F("Sleep mode: "));
        if (byte(bbuf)==WIFI_NONE_SLEEP) {
            Serial.println(F("None"));
        } else if (byte(bbuf)==WIFI_LIGHT_SLEEP) {
            Serial.println(F("Light"));
        } else if (byte(bbuf)==WIFI_MODEM_SLEEP) {
            Serial.println(F("Modem"));
        } else {
            Serial.println(F("???"));
        }
    } else {
        Serial.println(F("Error reading sleep mode"));
    }
    if (CONFIG::read_string(EP_HOSTNAME, sbuf , MAX_HOSTNAME_LENGTH)) {
        Serial.print(F("Hostname: "));
        Serial.println(sbuf);
    } else {
        Serial.println(F("Error reading hostname"));
    }
    if (CONFIG::read_byte(EP_WIFI_MODE, &bbuf )) {
        Serial.print(F("Mode: "));
        if (byte(bbuf) == CLIENT_MODE) {
            Serial.println(F("Station"));
            Serial.print(F("Signal: "));
            Serial.print(wifi_config.getSignal(WiFi.RSSI()));
            Serial.println(F("%"));
        } else if (byte(bbuf)==AP_MODE) {
            Serial.println(F("Access Point"));
        } else {
            Serial.println("???");
        }
    } else {
        Serial.println(F("Error reading mode"));
    }

    if (CONFIG::read_string(EP_STA_SSID, sbuf , MAX_SSID_LENGTH)) {
        Serial.print(F("Client SSID: "));
        Serial.println(sbuf);
    } else {
        Serial.println(F("Error reading SSID"));
    }

    if (CONFIG::read_byte(EP_STA_IP_MODE, &bbuf )) {
        Serial.print(F("STA IP Mode: "));
        if (byte(bbuf)==STATIC_IP_MODE) {
            Serial.println(F("Static"));
            if (CONFIG::read_buffer(EP_STA_IP_VALUE,(byte *)ipbuf , IP_LENGTH)) {
                Serial.print(F("IP: "));
                Serial.println(IPAddress(ipbuf).toString());
            } else {
                Serial.println(F("Error reading IP"));
            }

            if (CONFIG::read_buffer(EP_STA_MASK_VALUE, (byte *)ipbuf  , IP_LENGTH)) {
                Serial.print(F("Subnet: "));
                Serial.println(IPAddress(ipbuf).toString());
            } else {
                Serial.println(F("Error reading subnet"));
            }

            if (CONFIG::read_buffer(EP_STA_GATEWAY_VALUE, (byte *)ipbuf  , IP_LENGTH)) {
                Serial.print(F("Gateway: "));
                Serial.println(IPAddress(ipbuf).toString());
            } else {
                Serial.println(F("Error reading gateway"));
            }
        } else  if (byte(bbuf)==DHCP_MODE) {
            Serial.println(F("DHCP"));
        } else {
            Serial.println(F("???"));
        }
    } else {
        Serial.println(F("Error reading IP mode"));
    }

    if (CONFIG::read_byte(EP_STA_PHY_MODE, &bbuf )) {
		Serial.print(F("STA Phy mode: "));
        if (byte(bbuf)==WIFI_PHY_MODE_11B) {
            Serial.println(F("11b"));
        } else if (byte(bbuf)==WIFI_PHY_MODE_11G) {
            Serial.println(F("11g"));
        } else if (byte(bbuf)==WIFI_PHY_MODE_11N) {
            Serial.println(F("11n"));
        } else {
            Serial.println(F("???"));
        }
    } else {
        Serial.println(F("Error reading phy mode"));
    }

    if (CONFIG::read_string(EP_AP_SSID, sbuf , MAX_SSID_LENGTH)) {
        Serial.print(F("AP SSID: "));
        Serial.println(sbuf);
    } else {
        Serial.println(F("Error reading SSID"));
    }

    if (CONFIG::read_byte(EP_AP_IP_MODE, &bbuf )) {
        Serial.print(F("AP IP Mode: "));
        if (byte(bbuf)==STATIC_IP_MODE) {
            Serial.println(F("Static"));
                        if (CONFIG::read_buffer(EP_AP_IP_VALUE,(byte *)ipbuf , IP_LENGTH)) {
                Serial.print(F("IP: "));
                Serial.println(IPAddress(ipbuf).toString());
            } else {
                Serial.println(F("Error reading IP"));
            }

            if (CONFIG::read_buffer(EP_AP_MASK_VALUE, (byte *)ipbuf  , IP_LENGTH)) {
                Serial.print(F("Subnet: "));
                Serial.println(IPAddress(ipbuf).toString());
            } else {
                Serial.println(F("Error reading subnet"));
            }

            if (CONFIG::read_buffer(EP_AP_GATEWAY_VALUE, (byte *)ipbuf  , IP_LENGTH)) {
                Serial.print(F("Gateway: "));
                Serial.println(IPAddress(ipbuf).toString());
            } else {
                Serial.println(F("Error reading gateway"));
            }
        } else  if (byte(bbuf)==DHCP_MODE) {
            Serial.println(F("DHCP"));
        } else {
            Serial.println(intTostr(bbuf));
            Serial.println(F("???"));
        }
    } else {
        Serial.println(F("Error reading IP mode"));
    }

    if (CONFIG::read_byte(EP_AP_PHY_MODE, &bbuf )) {
		Serial.print(F("AP Phy mode: "));
        if (byte(bbuf)==WIFI_PHY_MODE_11B) {
            Serial.println(F("11b"));
        } else if (byte(bbuf)==WIFI_PHY_MODE_11G) {
            Serial.println(F("11g"));
        } else if (byte(bbuf)==WIFI_PHY_MODE_11N) {
            Serial.println(F("11n"));
        } else {
            Serial.println(F("???"));
        }
    } else {
        Serial.println(F("Error reading phy mode"));
    }

    if (CONFIG::read_byte(EP_CHANNEL, &bbuf )) {
        Serial.print(F("Channel: "));
        Serial.println(byte(bbuf));
    } else {
        Serial.println(F("Error reading channel"));
    }

    if (CONFIG::read_byte(EP_AUTH_TYPE, &bbuf )) {
        Serial.print(F("Authentification: "));
        if (byte(bbuf)==AUTH_OPEN) {
            Serial.println(F("None"));
        } else if (byte(bbuf)==AUTH_WEP) {
            Serial.println(F("WEP"));
        } else if (byte(bbuf)==AUTH_WPA_PSK) {
            Serial.println(F("WPA"));
        } else if (byte(bbuf)==AUTH_WPA2_PSK) {
            Serial.println(F("WPA2"));
        } else if (byte(bbuf)==AUTH_WPA_WPA2_PSK) {
            Serial.println(F("WPA/WPA2"));
        } else {
            Serial.println(F("???"));
        }
    } else {
        Serial.println(F("Error reading authentification"));
    }

    if (CONFIG::read_byte(EP_SSID_VISIBLE, &bbuf )) {
        Serial.print(F("SSID visibility: "));
        if (bbuf==0) {
            Serial.println(F("Hidden"));
        } else if (bbuf==1) {
            Serial.println(F("Visible"));
        } else {
            Serial.println(bbuf);
        }
    } else {
        Serial.println(F("Error reading SSID visibility"));
    }

    if (CONFIG::read_buffer(EP_WEB_PORT,  (byte *)&ibuf , INTEGER_LENGTH)) {
        Serial.print(F("Web port: "));
        Serial.println(ibuf);
    } else {
        Serial.println(F("Error reading web port"));
    }
    Serial.print(F("Data port: "));
#ifdef TCP_IP_DATA_FEATURE
    if (CONFIG::read_buffer(EP_DATA_PORT,  (byte *)&ibuf , INTEGER_LENGTH)) {
        Serial.println(ibuf);
    } else {
        Serial.println(F("Error reading data port"));
    }
#else
    Serial.println(F("Disabled"));
#endif
    if (CONFIG::read_byte(EP_REFRESH_PAGE_TIME, &bbuf )) {
        Serial.print(F("Web page refresh time: "));
        Serial.println(byte(bbuf));
    } else {
        Serial.println(F("Error reading refresh page"));
    }

    if (CONFIG::read_buffer(EP_XY_FEEDRATE,  (byte *)&ibuf , INTEGER_LENGTH)) {
        Serial.print(F("XY feed rate: "));
        Serial.println(ibuf);
    } else {
        Serial.println(F("Error reading XY feed rate"));
    }

    if (CONFIG::read_buffer(EP_Z_FEEDRATE,  (byte *)&ibuf , INTEGER_LENGTH)) {
        Serial.print(F("Z feed rate: "));
        Serial.println(ibuf);
    } else {
        Serial.println(F("Error reading Z feed rate"));
    }

    if (CONFIG::read_buffer(EP_E_FEEDRATE,  (byte *)&ibuf , INTEGER_LENGTH)) {
        Serial.print(F("E feed rate: "));
        Serial.println(ibuf);
    } else {
        Serial.println(F("Error reading E feed rate"));
    }

    Serial.print(F("Free memory: "));
    Serial.println(formatBytes(ESP.getFreeHeap()));

    Serial.print(F("Captive portal: "));
#ifdef CAPTIVE_PORTAL_FEATURE
    Serial.println(F("Enabled"));
#else
    Serial.println(F("Disabled"));
#endif
    Serial.print(F("SSDP: "));
#ifdef SSDP_FEATURE
    Serial.println(F("Enabled"));
#else
    Serial.println(F("Disabled"));
#endif
    Serial.print(F("NetBios: "));
#ifdef NETBIOS_FEATURE
    Serial.println(F("Enabled"));
#else
    Serial.println(F("Disabled"));
#endif
    Serial.print(F("mDNS: "));
#ifdef MDNS_FEATURE
    Serial.println(F("Enabled"));
#else
    Serial.println(F("Disabled"));
#endif
    Serial.print(F("Web update: "));
#ifdef WEB_UPDATE_FEATURE
    Serial.println(F("Enabled"));
#else
    Serial.println(F("Disabled"));
#endif
    Serial.print(F("Pin 2 Recovery: "));
#ifdef RECOVERY_FEATURE
    Serial.println(F("Enabled"));
#else
    Serial.println(F("Disabled"));
#endif
    Serial.print(F("Authentication: "));
#ifdef AUTHENTICATION_FEATURE
    Serial.println(F("Enabled"));
#else
    Serial.println(F("Disabled"));
#endif
    Serial.print(F("Target Firmware: "));
#if FIRMWARE_TARGET == REPETIER
    Serial.println(F("Repetier"));
#elif FIRMWARE_TARGET == REPETIER4DV
    Serial.println(F("Repetier for DaVinci"));
#elif FIRMWARE_TARGET == MALRLIN
    Serial.println(F("Marlin"));
#elif FIRMWARE_TARGET == SMOOTHIEWARE
    Serial.println(F("Smoothieware"));
#else
    Serial.println(F("???"));
#endif
  Serial.print(F("SD Card support: "));
#ifdef SDCARD_FEATURE
    Serial.println(F("Enabled"));
#else
    Serial.println(F("Disabled"));
#endif
#ifdef DEBUG_ESP3D
    Serial.print(F("Debug Enabled :"));
#ifdef DEBUG_OUTPUT_SPIFFS
    Serial.println(F("SPIFFS"));
#endif
#ifdef DEBUG_OUTPUT_SD
    Serial.println(F("SD"));
#endif
#ifdef DEBUG_OUTPUT_SERIAL
    Serial.println(F("serial"));
#endif
#endif
}
