/*
  CONFIG.CPP- esp8266 configuration class

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

//read a string
//a string is multibyte + \0, this is won't work if 1 char is multibyte like chinese char
bool CONFIG::read_string(int pos, char byte_buffer[], int size_max)
{
    //check if parameters are acceptable
    if (size_max==0 ||  pos+size_max+1 > EEPROM_SIZE || byte_buffer== NULL) {
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
        return false;
    }
    byte b = 13; // non zero for the while loop below
    int i=0;
    sbuffer="";

    EEPROM.begin(EEPROM_SIZE);
    //read until max size is reached or \0 is found
    while (i < size_max && b != 0) {
        b = EEPROM.read(pos+i);
        sbuffer+=char(b);
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
    size_buffer= strlen(byte_buffer);
    //check if parameters are acceptable
    if (size_buffer==0 ||  pos+size_buffer+1 > EEPROM_SIZE || byte_buffer== NULL) {
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
    if(!CONFIG::write_string(EP_SSID,FPSTR(DEFAULT_SSID))) {
        return false;
    }
    if(!CONFIG::write_string(EP_PASSWORD,FPSTR(DEFAULT_PASSWORD))) {
        return false;
    }
    if(!CONFIG::write_byte(EP_IP_MODE,DEFAULT_IP_MODE)) {
        return false;
    }
    if(!CONFIG::write_buffer(EP_IP_VALUE,DEFAULT_IP_VALUE,IP_LENGTH)) {
        return false;
    }
    if(!CONFIG::write_buffer(EP_MASK_VALUE,DEFAULT_MASK_VALUE,IP_LENGTH)) {
        return false;
    }
    if(!CONFIG::write_buffer(EP_GATEWAY_VALUE,DEFAULT_GATEWAY_VALUE,IP_LENGTH)) {
        return false;
    }
    if(!CONFIG::write_buffer(EP_BAUD_RATE,(const byte *)&DEFAULT_BAUD_RATE,INTEGER_LENGTH)) {
        return false;
    }
    if(!CONFIG::write_byte(EP_PHY_MODE,DEFAULT_PHY_MODE)) {
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
    if(!CONFIG::write_string(EP_ADMIN_PWD,FPSTR(DEFAULT_ADMIN))) {
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
    if (CONFIG::read_byte(EP_WIFI_MODE, &bbuf )) {
        Serial.print(F("Mode: "));
        if (byte(bbuf) == CLIENT_MODE) {
            Serial.println(F("Station"));
            Serial.print(F("Signal: "));
            Serial.print(100+WiFi.RSSI());
            Serial.println(F("%"));
        } else if (byte(bbuf)==AP_MODE) {
            Serial.println(F("Access Point"));
        } else {
            Serial.println("???");
        }
    } else {
        Serial.println(F("Error reading mode"));
    }

    if (CONFIG::read_string(EP_SSID, sbuf , MAX_SSID_LENGTH)) {
        Serial.print(F("SSID: "));
        Serial.println(sbuf);
    } else {
        Serial.println(F("Error reading SSID"));
    }

    if (CONFIG::read_byte(EP_IP_MODE, &bbuf )) {
        Serial.print(F("IP Mode: "));
        if (byte(bbuf)==STATIC_IP_MODE) {
            Serial.println(F("Static"));
        } else  if (byte(bbuf)==DHCP_MODE) {
            Serial.println(F("DHCP"));
        } else {
            Serial.println(F("???"));
        }
    } else {
        Serial.println(F("Error reading IP mode"));
    }

    if (CONFIG::read_buffer(EP_IP_VALUE,(byte *)ipbuf , IP_LENGTH)) {
        Serial.print(F("IP: "));
        Serial.println(IPAddress(ipbuf).toString());
    } else {
        Serial.println(F("Error reading IP"));
    }

    if (CONFIG::read_buffer(EP_MASK_VALUE, (byte *)ipbuf  , IP_LENGTH)) {
        Serial.print(F("Subnet: "));
        Serial.println(IPAddress(ipbuf).toString());
    } else {
        Serial.println(F("Error reading subnet"));
    }

    if (CONFIG::read_buffer(EP_GATEWAY_VALUE, (byte *)ipbuf  , IP_LENGTH)) {
        Serial.print(F("Gateway: "));
        Serial.println(IPAddress(ipbuf).toString());
    } else {
        Serial.println(F("Error reading gateway"));
    }

    if (CONFIG::read_buffer(EP_BAUD_RATE,  (byte *)&ibuf , INTEGER_LENGTH)) {
        Serial.print(F("Baud rate: "));
        Serial.println(ibuf);
    } else {
        Serial.println(F("Error reading baud rate"));
    }

    if (CONFIG::read_byte(EP_PHY_MODE, &bbuf )) {
		Serial.print(F("Phy mode: "));
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

    if (CONFIG::read_string(EP_HOSTNAME, sbuf , MAX_HOSTNAME_LENGTH)) {
        Serial.print(F("Hostname: "));
        Serial.println(sbuf);
    } else {
        Serial.println(F("Error reading hostname"));
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
 
    
}
