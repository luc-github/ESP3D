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
#ifndef FS_NO_GLOBALS
#define FS_NO_GLOBALS
#endif
#include <FS.h>
#include <WiFiUdp.h>
#include "wificonf.h"
#ifdef ARDUINO_ARCH_ESP8266
extern "C" {
#include "user_interface.h"
}
#endif
#ifdef ARDUINO_ARCH_ESP32
#include <esp_ota_ops.h>
#include "Update.h"
#include "esp_wifi.h"
#endif
#include "espcom.h"
#ifdef TIMESTAMP_FEATURE
#include <time.h>
#endif

#ifdef DHT_FEATURE
#include "DHTesp.h"
extern DHTesp dht;
#endif

#ifdef NOTIFICATION_FEATURE
#include "notifications_service.h"
#endif

uint8_t CONFIG::FirmwareTarget = UNKNOWN_FW;
byte CONFIG::output_flag = DEFAULT_OUTPUT_FLAG;
bool  CONFIG::is_com_enabled = false;
#ifdef DHT_FEATURE
byte CONFIG::DHT_type  = DEFAULT_DHT_TYPE;
int CONFIG::DHT_interval = DEFAULT_DHT_INTERVAL;
#endif

//Watchdog feeder
void  CONFIG::wdtFeed()
{
#ifdef ARDUINO_ARCH_ESP8266
    ESP.wdtFeed();
#else
    void esp_task_wdt_feed();
#endif
}

void CONFIG::wait (uint32_t milliseconds)
{
#if defined(ASYNCWEBSERVER)
    uint32_t timeout = millis();
    while ( (millis() - timeout) < milliseconds) {
        wdtFeed();
    }
#else
    delay(milliseconds);
#endif
}

bool CONFIG::SetFirmwareTarget (uint8_t fw)
{
    if ( fw <= MAX_FW_ID) {
        FirmwareTarget = fw;
        return true;
    } else {
        return false;
    }
}
uint8_t CONFIG::GetFirmwareTarget()
{
    return FirmwareTarget;
}
const char* CONFIG::GetFirmwareTargetName()
{
    static String response;
    if ( CONFIG::FirmwareTarget == REPETIER4DV) {
        response = F ("Repetier for Davinci");
    } else if ( CONFIG::FirmwareTarget == REPETIER) {
        response = F ("Repetier");
    } else if ( CONFIG::FirmwareTarget == MARLIN) {
        response = F ("Marlin");
    } else if ( CONFIG::FirmwareTarget == MARLINKIMBRA) {
        response = F ("MarlinKimbra");
    } else if ( CONFIG::FirmwareTarget == SMOOTHIEWARE) {
        response = F ("Smoothieware");
    } else if ( CONFIG::FirmwareTarget == GRBL) {
        response = F ("Grbl");
    } else {
        response = F ("???");
    }
    return response.c_str();
}

const char* CONFIG::GetFirmwareTargetShortName()
{
    static String response;
    if ( CONFIG::FirmwareTarget == REPETIER4DV) {
        response = F ("repetier4davinci");
    } else if ( CONFIG::FirmwareTarget == REPETIER) {
        response = F ("repetier");
    } else if ( CONFIG::FirmwareTarget == MARLIN) {
        response = F ("marlin");
    } else if ( CONFIG::FirmwareTarget == MARLINKIMBRA) {
        response = F ("marlinkimbra");
    } else if ( CONFIG::FirmwareTarget == SMOOTHIEWARE) {
        response = F ("smoothieware");
    } else if ( CONFIG::FirmwareTarget == GRBL) {
        response = F ("grbl");
    } else {
        response = F ("???");
    }
    return response.c_str();
}

void CONFIG::InitFirmwareTarget()
{
    uint8_t b = UNKNOWN_FW;
    if (!CONFIG::read_byte (EP_TARGET_FW, &b ) ) {
        b = UNKNOWN_FW;
    }
    if (!SetFirmwareTarget (b) ) {
        SetFirmwareTarget (UNKNOWN_FW) ;
    }
}
void CONFIG::InitOutput()
{
    byte bflag = 0;
    if (!CONFIG::read_byte (EP_OUTPUT_FLAG, &bflag ) ) {
        bflag = 0;
    }
    CONFIG::output_flag = bflag;
}

bool  CONFIG::is_locked(byte flag)
{
    return ((CONFIG::output_flag & flag) == flag);
}

void CONFIG::InitDirectSD()
{
    CONFIG::is_direct_sd = false;

}

bool CONFIG::DisableSerial()
{
#ifdef USE_SERIAL_0
Serial.end();
#endif
#ifdef USE_SERIAL_1
Serial1.end();
#endif
#ifdef USE_SERIAL_2
Serial2.end();
#endif
CONFIG::is_com_enabled = false;
return true;
}

bool CONFIG::InitBaudrate(long value)
{
    long baud_rate = 0;
    if (value > 0) {
        baud_rate = value;
    } else {
        if ( !CONFIG::read_buffer (EP_BAUD_RATE,  (byte *) &baud_rate, INTEGER_LENGTH) ) {
            return false;
        }
    }
    if ( ! (baud_rate == 9600 || baud_rate == 19200 || baud_rate == 38400 || baud_rate == 57600 || baud_rate == 115200 || baud_rate == 230400 || baud_rate == 250000 || baud_rate == 500000 || baud_rate == 921600 ) ) {
        return false;
    }

    //setup serial
    //TODO define baudrate for each Serial
#ifdef USE_SERIAL_0
    if (Serial.baudRate() != baud_rate) {
#ifdef ARDUINO_ARCH_ESP8266
        Serial.begin (baud_rate);
#else
        Serial.begin (baud_rate, ESP_SERIAL_PARAM, ESP_RX_PIN, ESP_TX_PIN);
#endif

    }
#endif
#ifdef USE_SERIAL_1
    if (Serial1.baudRate() != baud_rate) {
#ifdef ARDUINO_ARCH_ESP8266
        Serial1.begin (baud_rate);
#else
        Serial1.begin (baud_rate, ESP_SERIAL_PARAM, ESP_RX_PIN, ESP_TX_PIN);
#endif
    }
#endif
#ifdef USE_SERIAL_2
    if (Serial2.baudRate() != baud_rate) {
#ifdef ARDUINO_ARCH_ESP8266
        Serial2.begin (baud_rate);
#else
        Serial2.begin (baud_rate, ESP_SERIAL_PARAM, ESP_RX_PIN, ESP_TX_PIN);
#endif
    }
#endif

//only Serial for ESP8266
#ifdef ARDUINO_ARCH_ESP8266
    Serial.setRxBufferSize (SERIAL_RX_BUFFER_SIZE);
#endif

    wifi_config.baud_rate = baud_rate;
    delay (100);
    CONFIG::is_com_enabled = true;
    return true;
}

bool CONFIG::InitExternalPorts()
{
    if (!CONFIG::read_buffer (EP_WEB_PORT,  (byte *) & (wifi_config.iweb_port), INTEGER_LENGTH) || !CONFIG::read_buffer (EP_DATA_PORT,  (byte *) & (wifi_config.idata_port), INTEGER_LENGTH) ) {
        return false;
    }
    if (wifi_config.iweb_port < DEFAULT_MIN_WEB_PORT || wifi_config.iweb_port > DEFAULT_MAX_WEB_PORT || wifi_config.idata_port < DEFAULT_MIN_DATA_PORT || wifi_config.idata_port > DEFAULT_MAX_DATA_PORT) {
        return false;
    }
    return true;
}

//warning if using from async function with async param
//restart will work but reason will be wrong
//better to use "web_interface->restartmodule = true;" instead
void CONFIG::esp_restart (bool async)
{
    LOG ("Restarting\r\n")
    ESPCOM::flush (DEFAULT_PRINTER_PIPE);
    SPIFFS.end();
    if (!async) {
        delay (1000);
    }
#ifdef ARDUINO_ARCH_ESP8266
    //ESP8266  has only serial
    Serial.swap();
#endif
    ESP.restart();
    while (1) {
        if (!async) {
            delay (1);
        }
    };
}
#ifdef DHT_FEATURE
void  CONFIG::InitDHT(bool refresh)
{
    if (!refresh) {
        byte bflag = DEFAULT_DHT_TYPE;
        int ibuf = DEFAULT_DHT_INTERVAL;
        if (!CONFIG::read_byte (EP_DHT_TYPE, &bflag ) ) {
            bflag = DEFAULT_DHT_TYPE;
        }
        CONFIG::DHT_type = bflag;
        if (!CONFIG::read_buffer (EP_DHT_INTERVAL,  (byte *) &ibuf, INTEGER_LENGTH) ) {
            ibuf = DEFAULT_DHT_INTERVAL;
        }
        CONFIG::DHT_interval = ibuf;
    }
    if (CONFIG::DHT_type != 255) {
        dht.setup(ESP_DHT_PIN,(DHTesp::DHT_MODEL_t)CONFIG::DHT_type);    // Connect DHT sensor to GPIO ESP_DHT_PIN
    }
}
#endif
void  CONFIG::InitPins()
{
#ifdef RECOVERY_FEATURE
    pinMode (RESET_CONFIG_PIN, INPUT);
#endif

#ifdef DHT_FEATURE
    CONFIG::InitDHT();
#endif
}

#if defined(TIMESTAMP_FEATURE)
void CONFIG::init_time_client()
{
    String s1, s2, s3;
    int8_t t1;
    byte d1;
    if (!CONFIG::read_string (EP_TIME_SERVER1, s1, MAX_DATA_LENGTH) ) {
        s1 = FPSTR (DEFAULT_TIME_SERVER1);
    }
    if (!CONFIG::read_string (EP_TIME_SERVER2, s2, MAX_DATA_LENGTH) ) {
        s2 = FPSTR (DEFAULT_TIME_SERVER2);
    }
    if (!CONFIG::read_string (EP_TIME_SERVER3, s3, MAX_DATA_LENGTH) ) {
        s3 = FPSTR (DEFAULT_TIME_SERVER3);
    }
    if (!CONFIG::read_byte (EP_TIMEZONE, (byte *) &t1 ) ) {
        t1 = DEFAULT_TIME_ZONE;
    }
    if (!CONFIG::read_byte (EP_TIME_ISDST, &d1 ) ) {
        d1 = DEFAULT_TIME_DST;
    }
    configTime (3600 * (t1), d1 * 3600, s1.c_str(), s2.c_str(), s3.c_str() );
    time_t now = time(nullptr);
    if (WiFi.getMode() == WIFI_STA) {
        int nb = 0;
        while ((now < 8 * 3600 * 2) && (nb < 6)) {
            wait(500);
            nb++;
            now = time(nullptr);
        }
    }
}
#endif

bool CONFIG::is_direct_sd = false;


bool CONFIG::isHostnameValid (const char * hostname)
{
    //limited size
    char c;
    if (strlen (hostname) > MAX_HOSTNAME_LENGTH || strlen (hostname) < MIN_HOSTNAME_LENGTH) {
        return false;
    }
    //only letter and digit
    for (int i = 0; i < strlen (hostname); i++) {
        c = hostname[i];
        if (! (isdigit (c) || isalpha (c) || c == '_') ) {
            return false;
        }
        if (c == ' ') {
            return false;
        }
    }
    return true;
}

bool CONFIG::isSSIDValid (const char * ssid)
{
    //limited size
    //char c;
    if (strlen (ssid) > MAX_SSID_LENGTH || strlen (ssid) < MIN_SSID_LENGTH) {
        return false;
    }
    //only printable
    for (int i = 0; i < strlen (ssid); i++) {
        if (!isPrintable (ssid[i]) ) {
            return false;
        }
    }
    return true;
}

bool CONFIG::isPasswordValid (const char * password)
{
    //limited size
    if (strlen (password) > MAX_PASSWORD_LENGTH) {
        return false;
    }
#if MIN_PASSWORD_LENGTH > 0
    if (strlen (password) < MIN_PASSWORD_LENGTH) {
        ) return false;
    }
#endif
    //no space allowed
    for (int i = 0; i < strlen (password); i++)
        if (password[i] == ' ') {
            return false;
        }
    return true;
}

bool CONFIG::isLocalPasswordValid (const char * password)
{
    char c;
    //limited size
    if ( (strlen (password) > MAX_LOCAL_PASSWORD_LENGTH) ||  (strlen (password) < MIN_LOCAL_PASSWORD_LENGTH) ) {
        return false;
    }
    //no space allowed
    for (int i = 0; i < strlen (password); i++) {
        c = password[i];
        if (c == ' ') {
            return false;
        }
    }
    return true;
}

bool CONFIG::isIPValid (const char * IP)
{
    //limited size
    int internalcount = 0;
    int dotcount = 0;
    bool previouswasdot = false;
    char c;

    if (strlen (IP) > 15 || strlen (IP) == 0) {
        return false;
    }
    //cannot start with .
    if (IP[0] == '.') {
        return false;
    }
    //only letter and digit
    for (int i = 0; i < strlen (IP); i++) {
        c = IP[i];
        if (isdigit (c) ) {
            //only 3 digit at once
            internalcount++;
            previouswasdot = false;
            if (internalcount > 3) {
                return false;
            }
        } else if (c == '.') {
            //cannot have 2 dots side by side
            if (previouswasdot) {
                return false;
            }
            previouswasdot = true;
            internalcount = 0;
            dotcount++;
        }//if not a dot neither a digit it is wrong
        else {
            return false;
        }
    }
    //if not 3 dots then it is wrong
    if (dotcount != 3) {
        return false;
    }
    //cannot have the last dot as last char
    if (IP[strlen (IP) - 1] == '.') {
        return false;
    }
    return true;
}

char * CONFIG::intTostr (int value)
{
    static char result [12];
    sprintf (result, "%d", value);
    return result;
}

String CONFIG::formatBytes (uint64_t bytes)
{
    if (bytes < 1024) {
        return String ((uint16_t)bytes) + " B";
    } else if (bytes < (1024 * 1024) ) {
        return String ((float)(bytes / 1024.0)) + " KB";
    } else if (bytes < (1024 * 1024 * 1024) ) {
        return String ((float)(bytes / 1024.0 / 1024.0)) + " MB";
    } else {
        return String ((float)(bytes / 1024.0 / 1024.0 / 1024.0)) + " GB";
    }
}

//helper to convert string to IP
//do not use IPAddress.fromString() because lack of check point and error result
//return number of parts
byte CONFIG::split_ip (const char * ptr, byte * part)
{
    if (strlen (ptr) > 15 || strlen (ptr) < 7) {
        part[0] = 0;
        part[1] = 0;
        part[2] = 0;
        part[3] = 0;
        return 0;
    }

    char pstart [16];
    char * ptr2;
    strcpy (pstart, ptr);
    ptr2 = pstart;
    byte i = strlen (pstart);
    byte pos = 0;
    for (byte j = 0; j < i; j++) {
        if (pstart[j] == '.') {
            if (pos == 4) {
                part[0] = 0;
                part[1] = 0;
                part[2] = 0;
                part[3] = 0;
                return 0;
            }
            pstart[j] = 0x0;
            part[pos] = atoi (ptr2);
            pos++;
            ptr2 = &pstart[j + 1];
        }
    }
    part[pos] = atoi (ptr2);
    return pos + 1;
}

//just simple helper to convert mac address to string
char * CONFIG::mac2str (uint8_t mac [WL_MAC_ADDR_LENGTH])
{
    static char macstr [18];
    if (0 > sprintf (macstr, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]) ) {
        strcpy (macstr, "00:00:00:00:00:00");
    }
    return macstr;
}


bool CONFIG::set_EEPROM_version(uint8_t v)
{
    byte byte_buffer[6];
    byte_buffer[0]='E';
    byte_buffer[1]='S';
    byte_buffer[2]='P';
    byte_buffer[3]='3';
    byte_buffer[4]='D';
    byte_buffer[5]=v;
    return CONFIG::write_buffer (EP_EEPROM_VERSION, byte_buffer, 6);
}

uint8_t CONFIG::get_EEPROM_version()
{
    byte byte_buffer[6];
    long baud_rate;
    if (!CONFIG::read_buffer (EP_EEPROM_VERSION, byte_buffer, 6)) {
        return EEPROM_V0;
    }
    if ((byte_buffer[0]=='E') && (byte_buffer[1]=='S') && (byte_buffer[2]=='P')&& (byte_buffer[3]=='3') && (byte_buffer[4]=='D')) {
        return byte_buffer[5];
    }

    if ( !CONFIG::read_buffer (EP_BAUD_RATE,  (byte *) &baud_rate, INTEGER_LENGTH) ) {
        return EEPROM_V0;
    }
    if ((baud_rate == 9600 || baud_rate == 19200 || baud_rate == 38400 || baud_rate == 57600 || baud_rate == 115200 || baud_rate == 230400 || baud_rate == 250000 || baud_rate == 500000 || baud_rate == 921600 ) ) {
        return EEPROM_V1;
    }
    return EEPROM_V0;
}

bool CONFIG::adjust_EEPROM_settings()
{
    uint8_t v = get_EEPROM_version();
    bool bdone =false;
    if (v == EEPROM_CURRENT_VERSION) {
        return true;
    }
    if (v == 1) {
        bdone =true;
#ifdef SDCARD_FEATURE
        if (!CONFIG::write_byte (EP_SD_SPEED_DIV, DEFAULT_SDREADER_SPEED) ) {
            bdone =false;
        }
#endif
#ifdef DHT_FEATURE
        if (!CONFIG::write_buffer (EP_DHT_INTERVAL, (const byte *) &DEFAULT_DHT_INTERVAL, INTEGER_LENGTH) ) {
            bdone =false;
        }

        if (!CONFIG::write_byte (EP_DHT_TYPE, DEFAULT_DHT_TYPE) ) {
            bdone =false;
        }
#endif
        if (!CONFIG::write_byte (EP_OUTPUT_FLAG, DEFAULT_OUTPUT_FLAG) ) {
            bdone =false;
        }
    }
    if (bdone) {
        set_EEPROM_version(EEPROM_CURRENT_VERSION);
    }
    return bdone;
}

//read a string
//a string is multibyte + \0, this is won't work if 1 char is multibyte like chinese char
bool CONFIG::read_string (int pos, char byte_buffer[], int size_max)
{
    //check if parameters are acceptable
    if (size_max == 0 ||  pos + size_max + 1 > EEPROM_SIZE || byte_buffer == NULL) {
        LOG ("Error read string\r\n")
        return false;
    }
    EEPROM.begin (EEPROM_SIZE);
    byte b = 13; // non zero for the while loop below
    int i = 0;

    //read until max size is reached or \0 is found
    while (i < size_max && b != 0) {
        b = EEPROM.read (pos + i);
        byte_buffer[i] = b;
        i++;
    }

    // Be sure there is a 0 at the end.
    if (b != 0) {
        byte_buffer[i - 1] = 0x00;
    }
    EEPROM.end();

    return true;
}

bool CONFIG::read_string (int pos, String & sbuffer, int size_max)
{
    //check if parameters are acceptable
    if (size_max == 0 ||  pos + size_max + 1 > EEPROM_SIZE ) {
        LOG ("Error read string\r\n")
        return false;
    }
    byte b = 13; // non zero for the while loop below
    int i = 0;
    sbuffer = "";

    EEPROM.begin (EEPROM_SIZE);
    //read until max size is reached or \0 is found
    while (i < size_max && b != 0) {
        b = EEPROM.read (pos + i);
        if (b != 0) {
            sbuffer += char (b);
        }
        i++;
    }
    EEPROM.end();

    return true;
}

//read a buffer of size_buffer
bool CONFIG::read_buffer (int pos, byte byte_buffer[], int size_buffer)
{
    //check if parameters are acceptable
    if (size_buffer == 0 ||  pos + size_buffer > EEPROM_SIZE || byte_buffer == NULL) {
        LOG ("Error read buffer\r\n")
        return false;
    }
    int i = 0;
    EEPROM.begin (EEPROM_SIZE);
    //read until max size is reached
    while (i < size_buffer ) {
        byte_buffer[i] = EEPROM.read (pos + i);
        i++;
    }
    EEPROM.end();
    return true;
}

//read a flag / byte
bool CONFIG::read_byte (int pos, byte * value)
{
    //check if parameters are acceptable
    if (pos + 1 > EEPROM_SIZE) {
        LOG ("Error read byte\r\n")
        return false;
    }
    EEPROM.begin (EEPROM_SIZE);
    value[0] = EEPROM.read (pos);
    EEPROM.end();
    return true;
}

bool CONFIG::write_string (int pos, const __FlashStringHelper *str)
{
    String stmp = str;
    return write_string (pos, stmp.c_str() );
}

//write a string (array of byte with a 0x00  at the end)
bool CONFIG::write_string (int pos, const char * byte_buffer)
{
    int size_buffer;
    int maxsize = EEPROM_SIZE;
    size_buffer = strlen (byte_buffer);
    //check if parameters are acceptable
    switch (pos) {
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
    case EP_TIME_SERVER1:
    case EP_TIME_SERVER2:
    case EP_TIME_SERVER3:
        maxsize = MAX_DATA_LENGTH;
        break;
    case ESP_NOTIFICATION_TOKEN1:
    case ESP_NOTIFICATION_TOKEN2:
        maxsize = MAX_NOTIFICATION_TOKEN_LENGTH;
        break;
    case ESP_NOTIFICATION_SETTINGS:
        maxsize = MAX_NOTIFICATION_SETTINGS_LENGTH;
        break;
    default:
        maxsize = EEPROM_SIZE;
        break;
    }
    if ( pos + size_buffer + 1 > EEPROM_SIZE || size_buffer > maxsize  ) {
        LOG ("Error write string\r\n")
        return false;
    }

    if (!((pos == EP_STA_PASSWORD) || (pos == EP_AP_PASSWORD))) {
        if((size_buffer == 0 ) || (byte_buffer == NULL )) {
            LOG ("Error write string\r\n")
            return false;
        }
    }
    //copy the value(s)
    EEPROM.begin (EEPROM_SIZE);
    for (int i = 0; i < size_buffer; i++) {
        EEPROM.write (pos + i, byte_buffer[i]);
    }

    //0 terminal
    EEPROM.write (pos + size_buffer, 0x00);
    EEPROM.commit();
    EEPROM.end();
    return true;
}

//write a buffer
bool CONFIG::write_buffer (int pos, const byte * byte_buffer, int size_buffer)
{
    //check if parameters are acceptable
    if (size_buffer == 0 ||  pos + size_buffer > EEPROM_SIZE || byte_buffer == NULL) {
        LOG ("Error write buffer\r\n")
        return false;
    }
    EEPROM.begin (EEPROM_SIZE);
    //copy the value(s)
    for (int i = 0; i < size_buffer; i++) {
        EEPROM.write (pos + i, byte_buffer[i]);
    }
    EEPROM.commit();
    EEPROM.end();
    return true;
}

//read a flag / byte
bool CONFIG::write_byte (int pos, const byte value)
{
    //check if parameters are acceptable
    if (pos + 1 > EEPROM_SIZE) {
        LOG ("Error write byte\r\n")
        return false;
    }
    EEPROM.begin (EEPROM_SIZE);
    EEPROM.write (pos, value);
    EEPROM.commit();
    EEPROM.end();
    return true;
}

bool CONFIG::reset_config()
{
    if (!CONFIG::write_byte (EP_WIFI_MODE, DEFAULT_WIFI_MODE) ) {
        return false;
    }
    if (!CONFIG::write_buffer (EP_BAUD_RATE, (const byte *) &DEFAULT_BAUD_RATE, INTEGER_LENGTH) ) {
        return false;
    }
    if (!CONFIG::write_string (EP_AP_SSID, FPSTR (DEFAULT_AP_SSID) ) ) {
        return false;
    }
    if (!CONFIG::write_string (EP_AP_PASSWORD, FPSTR (DEFAULT_AP_PASSWORD) ) ) {
        return false;
    }
    if (!CONFIG::write_string (EP_STA_SSID, FPSTR (DEFAULT_STA_SSID) ) ) {
        return false;
    }
    if (!CONFIG::write_string (EP_STA_PASSWORD, FPSTR (DEFAULT_STA_PASSWORD) ) ) {
        return false;
    }
    if (!CONFIG::write_byte (EP_AP_IP_MODE, DEFAULT_AP_IP_MODE) ) {
        return false;
    }
    if (!CONFIG::write_byte (EP_STA_IP_MODE, DEFAULT_STA_IP_MODE) ) {
        return false;
    }
    if (!CONFIG::write_buffer (EP_STA_IP_VALUE, DEFAULT_IP_VALUE, IP_LENGTH) ) {
        return false;
    }
    if (!CONFIG::write_buffer (EP_STA_MASK_VALUE, DEFAULT_MASK_VALUE, IP_LENGTH) ) {
        return false;
    }
    if (!CONFIG::write_buffer (EP_STA_GATEWAY_VALUE, DEFAULT_GATEWAY_VALUE, IP_LENGTH) ) {
        return false;
    }
    if (!CONFIG::write_byte (EP_STA_PHY_MODE, DEFAULT_PHY_MODE) ) {
        return false;
    }
    if (!CONFIG::write_buffer (EP_AP_IP_VALUE, DEFAULT_IP_VALUE, IP_LENGTH) ) {
        return false;
    }
    if (!CONFIG::write_buffer (EP_AP_MASK_VALUE, DEFAULT_MASK_VALUE, IP_LENGTH) ) {
        return false;
    }
    if (!CONFIG::write_buffer (EP_AP_GATEWAY_VALUE, DEFAULT_GATEWAY_VALUE, IP_LENGTH) ) {
        return false;
    }
    if (!CONFIG::write_byte (EP_AP_PHY_MODE, DEFAULT_PHY_MODE) ) {
        return false;
    }
    if (!CONFIG::write_byte (EP_SLEEP_MODE, DEFAULT_SLEEP_MODE) ) {
        return false;
    }
    if (!CONFIG::write_byte (EP_CHANNEL, DEFAULT_CHANNEL) ) {
        return false;
    }
    if (!CONFIG::write_byte (EP_AUTH_TYPE, DEFAULT_AUTH_TYPE) ) {
        return false;
    }
    if (!CONFIG::write_byte (EP_SSID_VISIBLE, DEFAULT_SSID_VISIBLE) ) {
        return false;
    }
    if (!CONFIG::write_buffer (EP_WEB_PORT, (const byte *) &DEFAULT_WEB_PORT, INTEGER_LENGTH) ) {
        return false;
    }
    if (!CONFIG::write_buffer (EP_DATA_PORT, (const byte *) &DEFAULT_DATA_PORT, INTEGER_LENGTH) ) {
        return false;
    }

    if (!CONFIG::write_string (EP_HOSTNAME, wifi_config.get_default_hostname() ) ) {
        return false;
    }

    if (!CONFIG::write_string (EP_ADMIN_PWD, FPSTR (DEFAULT_ADMIN_PWD) ) ) {
        return false;
    }
    if (!CONFIG::write_string (EP_USER_PWD, FPSTR (DEFAULT_USER_PWD) ) ) {
        return false;
    }

    if (!CONFIG::write_byte (EP_TARGET_FW, UNKNOWN_FW) ) {
        return false;
    }
#if defined(TIMESTAMP_FEATURE)
    if (!CONFIG::write_byte (EP_TIMEZONE, DEFAULT_TIME_ZONE) ) {
        return false;
    }

    if (!CONFIG::write_byte (EP_TIME_ISDST, DEFAULT_TIME_DST) ) {
        return false;
    }

    if (!CONFIG::write_string (EP_TIME_SERVER1, FPSTR (DEFAULT_TIME_SERVER1) ) ) {
        return false;
    }

    if (!CONFIG::write_string (EP_TIME_SERVER2, FPSTR (DEFAULT_TIME_SERVER2) ) ) {
        return false;
    }

    if (!CONFIG::write_string (EP_TIME_SERVER3, FPSTR (DEFAULT_TIME_SERVER3) ) ) {
        return false;
    }
#endif

    if (!CONFIG::write_byte (EP_OUTPUT_FLAG, DEFAULT_OUTPUT_FLAG) ) {
        return false;
    }
#ifdef DHT_FEATURE
    if (!CONFIG::write_buffer (EP_DHT_INTERVAL, (const byte *) &DEFAULT_DHT_INTERVAL, INTEGER_LENGTH) ) {
        return false;
    }

    if (!CONFIG::write_byte (EP_DHT_TYPE, DEFAULT_DHT_TYPE) ) {
        return false;
    }
#endif

#ifdef NOTIFICATION_FEATURE
    if (!CONFIG::write_byte (ESP_NOTIFICATION_TYPE, DEFAULT_NOTIFICATION_TYPE) ) {
        return false;
    }
    if (!CONFIG::write_string (ESP_NOTIFICATION_TOKEN1, DEFAULT_NOTIFICATION_TOKEN1 ) ) {
        return false;
    }
    if (!CONFIG::write_string (ESP_NOTIFICATION_TOKEN2, DEFAULT_NOTIFICATION_TOKEN2 ) ) {
        return false;
    }
    if (!CONFIG::write_string (ESP_NOTIFICATION_SETTINGS, DEFAULT_NOTIFICATION_SETTINGS ) ) {
        return false;
    }
    if (!CONFIG::write_byte (ESP_AUTO_NOTIFICATION, DEFAULT_AUTO_NOTIFICATION_STATE) ) {
        return false;
    }
#endif

    return set_EEPROM_version(EEPROM_CURRENT_VERSION);
}

void CONFIG::print_config (tpipe output, bool plaintext, ESPResponseStream  *espresponse)
{
    if (!plaintext) {
        ESPCOM::print (F ("{\"chip_id\":\""), output, espresponse);
    } else {
        ESPCOM::print (F ("Chip ID: "), output, espresponse);
    }
#ifdef ARDUINO_ARCH_ESP8266
    ESPCOM::print (String (ESP.getChipId() ).c_str(), output, espresponse);
#else
    ESPCOM::print (String ( (uint16_t) (ESP.getEfuseMac() >> 32) ).c_str(), output, espresponse);
#endif
    if (!plaintext) {
        ESPCOM::print (F ("\","), output, espresponse);
    } else {
        ESPCOM::print (F ("\n"), output, espresponse);
    }

    if (!plaintext) {
        ESPCOM::print (F ("\"cpu\":\""), output, espresponse);
    } else {
        ESPCOM::print (F ("CPU Frequency: "), output, espresponse);
    }
    ESPCOM::print (String (ESP.getCpuFreqMHz() ).c_str(), output, espresponse);
    if (plaintext) {
        ESPCOM::print (F ("Mhz"), output, espresponse);
    }
    if (!plaintext) {
        ESPCOM::print (F ("\","), output, espresponse);
    } else {
        ESPCOM::print (F ("\n"), output, espresponse);
    }
#ifdef ARDUINO_ARCH_ESP32
    if (!plaintext) {
        ESPCOM::print (F ("\"cpu_temp\":\""), output, espresponse);
    } else {
        ESPCOM::print (F ("CPU Temperature: "), output, espresponse);
    }
    ESPCOM::print (String (temperatureRead(), 1).c_str(), output, espresponse);
    if (plaintext) {
        ESPCOM::print (F ("C"), output, espresponse);
    }
    if (!plaintext) {
        ESPCOM::print (F ("\","), output, espresponse);
    } else {
        ESPCOM::print (F ("\n"), output, espresponse);
    }
#endif
    if (!plaintext) {
        ESPCOM::print (F ("\"freemem\":\""), output, espresponse);
    } else {
        ESPCOM::print (F ("Free memory: "), output, espresponse);
    }
    ESPCOM::print (formatBytes (ESP.getFreeHeap() ).c_str(), output, espresponse);
    if (!plaintext) {
        ESPCOM::print (F ("\","), output, espresponse);
    } else {
        ESPCOM::print (F ("\n"), output, espresponse);
    }

    if (!plaintext) {
        ESPCOM::print (F ("\""), output, espresponse);
    }
    ESPCOM::print (F ("SDK"), output, espresponse);
    if (!plaintext) {
        ESPCOM::print (F ("\":\""), output, espresponse);
    } else {
        ESPCOM::print (F (": "), output, espresponse);
    }
    ESPCOM::print (ESP.getSdkVersion(), output, espresponse);
    if (!plaintext) {
        ESPCOM::print (F ("\","), output, espresponse);
    } else {
        ESPCOM::print (F ("\n"), output, espresponse);
    }

    if (!plaintext) {
        ESPCOM::print (F ("\"flash_size\":\""), output, espresponse);
    } else {
        ESPCOM::print (F ("Flash Size: "), output, espresponse);
    }
    ESPCOM::print (formatBytes (ESP.getFlashChipSize() ).c_str(), output, espresponse);
    if (!plaintext) {
        ESPCOM::print (F ("\","), output, espresponse);
    } else {
        ESPCOM::print (F ("\n"), output, espresponse);
    }
#ifdef ARDUINO_ARCH_ESP8266
    if (!plaintext) {
        ESPCOM::print (F ("\"update_size\":\""), output, espresponse);
    } else {
        ESPCOM::print (F ("Available Size for update: "), output, espresponse);
    }
    uint32_t  flashsize = ESP.getFlashChipSize();
    fs::FSInfo info;
    SPIFFS.info (info);
    //if higher than 1MB take out SPIFFS
    if (flashsize > 1024 * 1024) {
        flashsize = (1024 * 1024)-ESP.getSketchSize()-1024;
    } else {
        flashsize = flashsize - ESP.getSketchSize()-info.totalBytes-1024;
    }
    ESPCOM::print(formatBytes(flashsize).c_str(), output, espresponse);
    if (!plaintext) {
        ESPCOM::print (F ("\","), output, espresponse);
    } else {
        if (flashsize > ( ESP.getSketchSize())) {
            ESPCOM::println(F("(Ok)"), output, espresponse);
        } else {
            ESPCOM::println(F ("(Not enough)"), output, espresponse);
        }
    }

    if (!plaintext) {
        ESPCOM::print (F ("\"spiffs_size\":\""), output, espresponse);
    } else {
        ESPCOM::print (F ("Available Size for SPIFFS: "), output, espresponse);
    }
    ESPCOM::print (formatBytes (info.totalBytes).c_str(), output, espresponse);
    if (!plaintext) {
        ESPCOM::print (F ("\","), output, espresponse);
    } else {
        ESPCOM::print (F ("\n"), output, espresponse);
    }
#else
    if (!plaintext) {
        ESPCOM::print (F ("\"update_size\":\""), output, espresponse);
    } else {
        ESPCOM::print (F ("Available Size for update: "), output, espresponse);
    }
     size_t flashsize = 0;
    if (esp_ota_get_running_partition()) {
        const esp_partition_t* partition = esp_ota_get_next_update_partition(NULL);
        if (partition) {
            flashsize = partition->size;
        }
    } 
    ESPCOM::print (formatBytes (flashsize).c_str(), output, espresponse);
    if (!plaintext) {
        ESPCOM::print (F ("\","), output, espresponse);
    } else {
        if (flashsize  > 0x0) {
            ESPCOM::println (F ("(Ok)"), output, espresponse);
        } else {
            ESPCOM::print (F ("(Not enough)"), output, espresponse);
        }
    }
    if (!plaintext) {
        ESPCOM::print (F ("\"spiffs_size\":\""), output, espresponse);
    } else {
        ESPCOM::print (F ("Available Size for SPIFFS: "), output, espresponse);
    }
    ESPCOM::print (formatBytes (SPIFFS.totalBytes() ).c_str(), output, espresponse);
    if (!plaintext) {
        ESPCOM::print (F ("\","), output, espresponse);
    } else {
        ESPCOM::print (F ("\n"), output, espresponse);
    }
#endif
    if (!plaintext) {
        ESPCOM::print (F ("\"baud_rate\":\""), output, espresponse);
    } else {
        ESPCOM::print (F ("Baud rate: "), output, espresponse);
    }
    uint32_t br = ESPCOM::baudRate(DEFAULT_PRINTER_PIPE);
    ESPCOM::print (String (br).c_str(), output, espresponse);
    if (!plaintext) {
        ESPCOM::print (F ("\","), output, espresponse);
    } else {
        ESPCOM::print (F ("\n"), output, espresponse);
    }

    if (!plaintext) {
        ESPCOM::print (F ("\"sleep_mode\":\""), output, espresponse);
    } else {
        ESPCOM::print (F ("Sleep mode: "), output, espresponse);
    }
#ifdef ARDUINO_ARCH_ESP32
    wifi_ps_type_t ps_type;
    esp_wifi_get_ps (&ps_type);
#else
    WiFiSleepType_t ps_type;
    ps_type = WiFi.getSleepMode();
#endif
    if (ps_type == WIFI_NONE_SLEEP) {
        ESPCOM::print (F ("None"), output, espresponse);
    } else if (ps_type == WIFI_LIGHT_SLEEP) {
        ESPCOM::print (F ("Light"), output, espresponse);
    } else if (ps_type == WIFI_MODEM_SLEEP) {
        ESPCOM::print (F ("Modem"), output, espresponse);
    } else {
        ESPCOM::print (F ("???"), output, espresponse);
    }
    if (!plaintext) {
        ESPCOM::print (F ("\","), output, espresponse);
    } else {
        ESPCOM::print (F ("\n"), output, espresponse);
    }

    if (!plaintext) {
        ESPCOM::print (F ("\"channel\":\""), output, espresponse);
    } else {
        ESPCOM::print (F ("Channel: "), output, espresponse);
    }
    ESPCOM::print (String (WiFi.channel() ).c_str(), output, espresponse);
    if (!plaintext) {
        ESPCOM::print (F ("\","), output, espresponse);
    } else {
        ESPCOM::print (F ("\n"), output, espresponse);
    }
#ifdef ARDUINO_ARCH_ESP32
    uint8_t PhyMode;
    if (WiFi.getMode() == WIFI_STA) {
        esp_wifi_get_protocol (ESP_IF_WIFI_STA, &PhyMode);
    } else {
        esp_wifi_get_protocol (ESP_IF_WIFI_AP, &PhyMode);
    }
#else
    WiFiPhyMode_t PhyMode = WiFi.getPhyMode();
#endif
    if (!plaintext) {
        ESPCOM::print (F ("\"phy_mode\":\""), output, espresponse);
    } else {
        ESPCOM::print (F ("Phy Mode: "), output, espresponse);
    }
    if (PhyMode == (WIFI_PHY_MODE_11G) ) {
        ESPCOM::print (F ("11g"), output, espresponse);
    } else if (PhyMode == (WIFI_PHY_MODE_11B) ) {
        ESPCOM::print (F ("11b"), output, espresponse);
    } else if (PhyMode == (WIFI_PHY_MODE_11N) ) {
        ESPCOM::print (F ("11n"), output, espresponse);
    } else {
        ESPCOM::print (F ("???"), output, espresponse);
    }
    if (!plaintext) {
        ESPCOM::print (F ("\","), output, espresponse);
    } else {
        ESPCOM::print (F ("\n"), output, espresponse);
    }

    if (!plaintext) {
        ESPCOM::print (F ("\"web_port\":\""), output, espresponse);
    } else {
        ESPCOM::print (F ("Web port: "), output, espresponse);
    }
    ESPCOM::print (String (wifi_config.iweb_port).c_str(), output, espresponse);
    if (!plaintext) {
        ESPCOM::print (F ("\","), output, espresponse);
    } else {
        ESPCOM::print (F ("\n"), output, espresponse);
    }

    if (!plaintext) {
        ESPCOM::print (F ("\"data_port\":\""), output, espresponse);
    } else {
        ESPCOM::print (F ("Data port: "), output, espresponse);
    }
#ifdef TCP_IP_DATA_FEATURE
    ESPCOM::print (String (wifi_config.idata_port).c_str(), output, espresponse);
#else
    ESPCOM::print (F ("Disabled"), output, espresponse);
#endif
    if (!plaintext) {
        ESPCOM::print (F ("\","), output, espresponse);
    } else {
        ESPCOM::print (F ("\n"), output, espresponse);
    }

    if (WiFi.getMode() == WIFI_STA || WiFi.getMode() == WIFI_AP_STA) {
        if (!plaintext) {
            ESPCOM::print (F ("\"hostname\":\""), output, espresponse);
        } else {
            ESPCOM::print (F ("Hostname: "), output, espresponse);
        }
#ifdef ARDUINO_ARCH_ESP32
        ESPCOM::print (WiFi.getHostname(), output, espresponse);
#else
        ESPCOM::print (WiFi.hostname().c_str(), output, espresponse);
#endif
        if (!plaintext) {
            ESPCOM::print (F ("\","), output, espresponse);
        } else {
            ESPCOM::print (F ("\n"), output, espresponse);
        }
    }

    if (!plaintext) {
        ESPCOM::print (F ("\"active_mode\":\""), output, espresponse);
    } else {
        ESPCOM::print (F ("Active Mode: "), output, espresponse);
    }
    if (WiFi.getMode() == WIFI_STA) {
        ESPCOM::print (F ("STA ("), output, espresponse);
        ESPCOM::print (WiFi.macAddress().c_str(), output, espresponse);
        ESPCOM::print (F (")"), output, espresponse);
        if (!plaintext) {
            ESPCOM::print (F ("\","), output, espresponse);
        } else {
            ESPCOM::print (F ("\n"), output, espresponse);
        }
        if (WiFi.isConnected() ) {
            if (!plaintext) {
                ESPCOM::print (F ("\"connected_ssid\":\""), output, espresponse);
            } else {
                ESPCOM::print (F ("Connected to: "), output, espresponse);
            }
            ESPCOM::print (WiFi.SSID().c_str(), output, espresponse);
            if (!plaintext) {
                ESPCOM::print (F ("\","), output, espresponse);
            } else {
                ESPCOM::print (F ("\n"), output, espresponse);
            }
            if (!plaintext) {
                ESPCOM::print (F ("\"connected_signal\":\""), output, espresponse);
            } else {
                ESPCOM::print (F ("Signal: "), output, espresponse);
            }
            ESPCOM::print (String (wifi_config.getSignal (WiFi.RSSI() ) ).c_str(), output, espresponse);
            ESPCOM::print (F ("%"), output, espresponse);
            if (!plaintext) {
                ESPCOM::print (F ("\","), output, espresponse);
            } else {
                ESPCOM::print (F ("\n"), output, espresponse);
            }
        } else {
            if (!plaintext) {
                ESPCOM::print (F ("\"connection_status\":\""), output, espresponse);
            } else {
                ESPCOM::print (F ("Connection Status: "), output, espresponse);
            }
            ESPCOM::print (F ("Connection Status: "), output, espresponse);
            if (WiFi.status() == WL_DISCONNECTED) {
                ESPCOM::print (F ("Disconnected"), output, espresponse);
            } else if (WiFi.status() == WL_CONNECTION_LOST) {
                ESPCOM::print (F ("Connection lost"), output, espresponse);
            } else if (WiFi.status() == WL_CONNECT_FAILED) {
                ESPCOM::print (F ("Connection failed"), output, espresponse);
            } else if (WiFi.status() == WL_NO_SSID_AVAIL) {
                ESPCOM::print (F ("No connection"), output, espresponse);
            } else if (WiFi.status() == WL_IDLE_STATUS   ) {
                ESPCOM::print (F ("Idle"), output, espresponse);
            } else {
                ESPCOM::print (F ("Unknown"), output, espresponse);
            }
            if (!plaintext) {
                ESPCOM::print (F ("\","), output, espresponse);
            } else {
                ESPCOM::print (F ("\n"), output, espresponse);
            }
        }
        if (!plaintext) {
            ESPCOM::print (F ("\"ip_mode\":\""), output, espresponse);
        } else {
            ESPCOM::print (F ("IP Mode: "), output, espresponse);
        }
#ifdef ARDUINO_ARCH_ESP32
        tcpip_adapter_dhcp_status_t dhcp_status;
        tcpip_adapter_dhcpc_get_status (TCPIP_ADAPTER_IF_STA, &dhcp_status);
        if (dhcp_status == TCPIP_ADAPTER_DHCP_STARTED)
#else
        if (wifi_station_dhcpc_status() == DHCP_STARTED)
#endif
        {
            ESPCOM::print (F ("DHCP"), output, espresponse);
        } else {
            ESPCOM::print (F ("Static"), output, espresponse);
        }
        if (!plaintext) {
            ESPCOM::print (F ("\","), output, espresponse);
        } else {
            ESPCOM::print (F ("\n"), output, espresponse);
        }

        if (!plaintext) {
            ESPCOM::print (F ("\"ip\":\""), output, espresponse);
        } else {
            ESPCOM::print (F ("IP: "), output, espresponse);
        }
        ESPCOM::print (WiFi.localIP().toString().c_str(), output, espresponse);
        if (!plaintext) {
            ESPCOM::print (F ("\","), output, espresponse);
        } else {
            ESPCOM::print (F ("\n"), output, espresponse);
        }

        if (!plaintext) {
            ESPCOM::print (F ("\"gw\":\""), output, espresponse);
        } else {
            ESPCOM::print (F ("Gateway: "), output, espresponse);
        }
        ESPCOM::print (WiFi.gatewayIP().toString().c_str(), output, espresponse);
        if (!plaintext) {
            ESPCOM::print (F ("\","), output, espresponse);
        } else {
            ESPCOM::print (F ("\n"), output, espresponse);
        }

        if (!plaintext) {
            ESPCOM::print (F ("\"msk\":\""), output, espresponse);
        } else {
            ESPCOM::print (F ("Mask: "), output, espresponse);
        }
        ESPCOM::print (WiFi.subnetMask().toString().c_str(), output, espresponse);
        if (!plaintext) {
            ESPCOM::print (F ("\","), output, espresponse);
        } else {
            ESPCOM::print (F ("\n"), output, espresponse);
        }

        if (!plaintext) {
            ESPCOM::print (F ("\"dns\":\""), output, espresponse);
        } else {
            ESPCOM::print (F ("DNS: "), output, espresponse);
        }
        ESPCOM::print (WiFi.dnsIP().toString().c_str(), output, espresponse);
        if (!plaintext) {
            ESPCOM::print (F ("\","), output, espresponse);
        } else {
            ESPCOM::print (F ("\n"), output, espresponse);
        }

        if (!plaintext) {
            ESPCOM::print (F ("\"disabled_mode\":\""), output, espresponse);
        } else {
            ESPCOM::print (F ("Disabled Mode: "), output, espresponse);
        }
        ESPCOM::print (F ("AP ("), output, espresponse);
        ESPCOM::print (WiFi.softAPmacAddress().c_str(), output, espresponse);
        ESPCOM::print (F (")"), output, espresponse);
        if (!plaintext) {
            ESPCOM::print (F ("\","), output, espresponse);
        } else {
            ESPCOM::print (F ("\n"), output, espresponse);
        }

    } else if (WiFi.getMode() == WIFI_AP) {
        ESPCOM::print (F ("AP ("), output, espresponse);
        ESPCOM::print (WiFi.softAPmacAddress().c_str(), output, espresponse);
        ESPCOM::print (F (")"), output, espresponse);
        if (!plaintext) {
            ESPCOM::print (F ("\","), output, espresponse);
        } else {
            ESPCOM::print (F ("\n"), output, espresponse);
        }

        //get current config
#ifdef ARDUINO_ARCH_ESP32
        wifi_ap_config_t apconfig;
        wifi_config_t conf;
        esp_wifi_get_config (ESP_IF_WIFI_AP, &conf);
        apconfig.ssid_hidden = conf.ap.ssid_hidden;
        apconfig.authmode = conf.ap.authmode;
        apconfig.max_connection = conf.ap.max_connection;
#else
        struct softap_config apconfig;
        wifi_softap_get_config (&apconfig);
#endif
        if (!plaintext) {
            ESPCOM::print (F ("\"ap_ssid\":\""), output, espresponse);
        } else {
            ESPCOM::print (F ("SSID: "), output, espresponse);
        }
#ifdef ARDUINO_ARCH_ESP32
        ESPCOM::print ( (const char*) conf.ap.ssid, output, espresponse);
#else
        ESPCOM::print ( (const char*) apconfig.ssid, output, espresponse);
#endif
        if (!plaintext) {
            ESPCOM::print (F ("\","), output, espresponse);
        } else {
            ESPCOM::print (F ("\n"), output, espresponse);
        }

        if (!plaintext) {
            ESPCOM::print (F ("\"ssid_visible\":\""), output, espresponse);
        } else {
            ESPCOM::print (F ("Visible: "), output, espresponse);
        }
        ESPCOM::print ( (apconfig.ssid_hidden == 0) ? F ("Yes") : F ("No"), output, espresponse);
        if (!plaintext) {
            ESPCOM::print (F ("\","), output, espresponse);
        } else {
            ESPCOM::print (F ("\n"), output, espresponse);
        }

        if (!plaintext) {
            ESPCOM::print (F ("\"ssid_authentication\":\""), output, espresponse);
        } else {
            ESPCOM::print (F ("Authentication: "), output, espresponse);
        }
        if (apconfig.authmode == AUTH_OPEN) {
            ESPCOM::print (F ("None"), output, espresponse);
        } else if (apconfig.authmode == AUTH_WEP) {
            ESPCOM::print (F ("WEP"), output, espresponse);
        } else if (apconfig.authmode == AUTH_WPA_PSK) {
            ESPCOM::print (F ("WPA"), output, espresponse);
        } else if (apconfig.authmode == AUTH_WPA2_PSK) {
            ESPCOM::print (F ("WPA2"), output, espresponse);
        } else {
            ESPCOM::print (F ("WPA/WPA2"), output, espresponse);
        }
        if (!plaintext) {
            ESPCOM::print (F ("\","), output, espresponse);
        } else {
            ESPCOM::print (F ("\n"), output, espresponse);
        }

        if (!plaintext) {
            ESPCOM::print (F ("\"ssid_max_connections\":\""), output, espresponse);
        } else {
            ESPCOM::print (F ("Max Connections: "), output, espresponse);
        }
        ESPCOM::print (String (apconfig.max_connection).c_str(), output, espresponse);
        if (!plaintext) {
            ESPCOM::print (F ("\","), output, espresponse);
        } else {
            ESPCOM::print (F ("\n"), output, espresponse);
        }

        if (!plaintext) {
            ESPCOM::print (F ("\"ssid_dhcp\":\""), output, espresponse);
        } else {
            ESPCOM::print (F ("DHCP Server: "), output, espresponse);
        }
#ifdef ARDUINO_ARCH_ESP32
        tcpip_adapter_dhcp_status_t dhcp_status;
        tcpip_adapter_dhcps_get_status (TCPIP_ADAPTER_IF_AP, &dhcp_status);
        if (dhcp_status == TCPIP_ADAPTER_DHCP_STARTED)
#else
        if (wifi_softap_dhcps_status() == DHCP_STARTED)
#endif
        {
            ESPCOM::print (F ("Started"), output, espresponse);
        } else {
            ESPCOM::print (F ("Stopped"), output, espresponse);
        }
        if (!plaintext) {
            ESPCOM::print (F ("\","), output, espresponse);
        } else {
            ESPCOM::print (F ("\n"), output, espresponse);
        }

        if (!plaintext) {
            ESPCOM::print (F ("\"ip\":\""), output, espresponse);
        } else {
            ESPCOM::print (F ("IP: "), output, espresponse);
        }
        ESPCOM::print (WiFi.softAPIP().toString().c_str(), output, espresponse);
        if (!plaintext) {
            ESPCOM::print (F ("\","), output, espresponse);
        } else {
            ESPCOM::print (F ("\n"), output, espresponse);
        }
#ifdef ARDUINO_ARCH_ESP32
        tcpip_adapter_ip_info_t ip;
        tcpip_adapter_get_ip_info (TCPIP_ADAPTER_IF_AP, &ip);
#else
        struct ip_info ip;
        wifi_get_ip_info (SOFTAP_IF, &ip);
#endif
        if (!plaintext) {
            ESPCOM::print (F ("\"gw\":\""), output, espresponse);
        } else {
            ESPCOM::print (F ("Gateway: "), output, espresponse);
        }
        ESPCOM::print (IPAddress (ip.gw.addr).toString().c_str(), output, espresponse);
        if (!plaintext) {
            ESPCOM::print (F ("\","), output, espresponse);
        } else {
            ESPCOM::print (F ("\n"), output, espresponse);
        }

        if (!plaintext) {
            ESPCOM::print (F ("\"msk\":\""), output, espresponse);
        } else {
            ESPCOM::print (F ("Mask: "), output, espresponse);
        }
        ESPCOM::print (IPAddress (ip.netmask.addr).toString().c_str(), output, espresponse);
        if (!plaintext) {
            ESPCOM::print (F ("\","), output, espresponse);
        } else {
            ESPCOM::print (F ("\n"), output, espresponse);
        }


        if (!plaintext) {
            ESPCOM::print (F ("\"connected_clients\":["), output, espresponse);
        } else {
            ESPCOM::print (F ("Connected clients: "), output, espresponse);
        }
        int client_counter = 0;
#ifdef ARDUINO_ARCH_ESP32
        wifi_sta_list_t station;
        tcpip_adapter_sta_list_t tcpip_sta_list;
        esp_wifi_ap_get_sta_list (&station);
        tcpip_adapter_get_sta_list (&station, &tcpip_sta_list);
#else
        struct station_info * station;
        station = wifi_softap_get_station_info();
#endif
        String stmp = "";
#ifdef ARDUINO_ARCH_ESP32
        for (int i = 0; i < station.num; i++) {
#else
        while (station) {
#endif
            if (stmp.length() > 0) {
                if (!plaintext) {
                    stmp += F (",");
                } else {
                    stmp += F ("\n");
                }

            }
            if (!plaintext) {
                stmp += F ("{\"bssid\":\"");
            }
            //BSSID
#ifdef ARDUINO_ARCH_ESP32
            stmp += CONFIG::mac2str (tcpip_sta_list.sta[i].mac);
#else
            stmp += CONFIG::mac2str (station->bssid);
#endif
            if (!plaintext) {
                stmp += F ("\",\"ip\":\"");
            } else {
                stmp += F (" ");
            }
            //IP
#ifdef ARDUINO_ARCH_ESP32
            stmp += IPAddress (tcpip_sta_list.sta[i].ip.addr).toString().c_str();
#else
            stmp += IPAddress ( (const uint8_t *) &station->ip).toString().c_str();
#endif
            if (!plaintext) {
                stmp += F ("\"}");
            }
            //increment counter
            client_counter++;
#ifdef ARDUINO_ARCH_ESP32
        }
#else
            //go next record
            station = STAILQ_NEXT (station, next);
        }
        wifi_softap_free_station_info();
#endif
        if (!plaintext) {
            ESPCOM::print (stmp.c_str(), output, espresponse);
            ESPCOM::print (F ("],"), output, espresponse);
        } else {
            //display number of client
            ESPCOM::println (String (client_counter).c_str(), output, espresponse);
            //display list if any
            if (stmp.length() > 0) {
                ESPCOM::println (stmp.c_str(), output, espresponse);
            }
        }

        if (!plaintext) {
            ESPCOM::print (F ("\"disabled_mode\":\""), output, espresponse);
        } else {
            ESPCOM::print (F ("Disabled Mode: "), output, espresponse);
        }
        ESPCOM::print (F ("STA ("), output, espresponse);
        ESPCOM::print (WiFi.macAddress().c_str(), output, espresponse);
        ESPCOM::print (F (") is disabled"), output, espresponse);
        if (!plaintext) {
            ESPCOM::print (F ("\","), output, espresponse);
        } else {
            ESPCOM::print (F ("\n"), output, espresponse);
        }

    } else if (WiFi.getMode() == WIFI_AP_STA)
    {
        ESPCOM::print (F ("Mixed"), output, espresponse);
        if (!plaintext) {
            ESPCOM::print (F ("\","), output, espresponse);
        } else {
            ESPCOM::print (F ("\n"), output, espresponse);
        }
        if (!plaintext) {
            ESPCOM::print (F ("\"active_mode\":\""), output, espresponse);
        } else {
            ESPCOM::print (F ("Active Mode: "), output, espresponse);
        }
        ESPCOM::print (F ("AP ("), output, espresponse);
        ESPCOM::print (WiFi.softAPmacAddress().c_str(), output, espresponse);
        ESPCOM::println (F (")"), output, espresponse);
        ESPCOM::print (F ("STA ("), output, espresponse);
        ESPCOM::print (WiFi.macAddress().c_str(), output, espresponse);
        ESPCOM::print (F (")"), output, espresponse);
        if (!plaintext) {
            ESPCOM::print (F ("\","), output, espresponse);
        } else {
            ESPCOM::print (F ("\n"), output, espresponse);
        }
    } else
    {
        ESPCOM::print ("Wifi Off", output, espresponse);
        if (!plaintext) {
            ESPCOM::print (F ("\","), output, espresponse);
        } else {
            ESPCOM::print (F ("\n"), output, espresponse);
        }
    }

    if (!plaintext)
    {
        ESPCOM::print (F ("\"captive_portal\":\""), output, espresponse);
    } else
    {
        ESPCOM::print (F ("Captive portal: "), output, espresponse);
    }
#ifdef CAPTIVE_PORTAL_FEATURE
    ESPCOM::print (F ("Enabled"), output, espresponse);
#else
    ESPCOM::print (F ("Disabled"), output, espresponse);
#endif
    if (!plaintext)
    {
        ESPCOM::print (F ("\","), output, espresponse);
    } else
    {
        ESPCOM::print (F ("\n"), output, espresponse);
    }

    if (!plaintext)
    {
        ESPCOM::print (F ("\"ssdp\":\""), output, espresponse);
    } else
    {
        ESPCOM::print (F ("SSDP: "), output, espresponse);
    }
#ifdef SSDP_FEATURE
    ESPCOM::print (F ("Enabled"), output, espresponse);
#else
    ESPCOM::print (F ("Disabled"), output, espresponse);
#endif
    if (!plaintext)
    {
        ESPCOM::print (F ("\","), output, espresponse);
    } else
    {
        ESPCOM::print (F ("\n"), output, espresponse);
    }

    if (!plaintext)
    {
        ESPCOM::print (F ("\"netbios\":\""), output, espresponse);
    } else
    {
        ESPCOM::print (F ("NetBios: "), output, espresponse);
    }
#ifdef NETBIOS_FEATURE
    ESPCOM::print (F ("Enabled"), output, espresponse);
#else
    ESPCOM::print (F ("Disabled"), output, espresponse);
#endif
    if (!plaintext)
    {
        ESPCOM::print (F ("\","), output, espresponse);
    } else
    {
        ESPCOM::print (F ("\n"), output, espresponse);
    }

    if (!plaintext)
    {
        ESPCOM::print (F ("\"mdns\":\""), output, espresponse);
    } else
    {
        ESPCOM::print (F ("mDNS: "), output, espresponse);
    }
#ifdef MDNS_FEATURE
    ESPCOM::print (F ("Enabled"), output, espresponse);
#else
    ESPCOM::print (F ("Disabled"), output, espresponse);
#endif
    if (!plaintext)
    {
        ESPCOM::print (F ("\","), output, espresponse);
    } else
    {
        ESPCOM::print (F ("\n"), output, espresponse);
    }

    if (!plaintext)
    {
        ESPCOM::print (F ("\"web_update\":\""), output, espresponse);
    } else
    {
        ESPCOM::print (F ("Web Update: "), output, espresponse);
    }
#ifdef WEB_UPDATE_FEATURE
    ESPCOM::print (F ("Enabled"), output, espresponse);
#else
    ESPCOM::print (F ("Disabled"), output, espresponse);
#endif
    if (!plaintext)
    {
        ESPCOM::print (F ("\","), output, espresponse);
    } else
    {
        ESPCOM::print (F ("\n"), output, espresponse);
    }

    if (!plaintext)
    {
        ESPCOM::print (F ("\"pin recovery\":\""), output, espresponse);
    } else
    {
        ESPCOM::print (F ("Pin Recovery: "), output, espresponse);
    }
#ifdef RECOVERY_FEATURE
    ESPCOM::print (F ("Enabled"), output, espresponse);
#else
    ESPCOM::print (F ("Disabled"), output, espresponse);
#endif
    if (!plaintext)
    {
        ESPCOM::print (F ("\","), output, espresponse);
    } else
    {
        ESPCOM::print (F ("\n"), output, espresponse);
    }

    if (!plaintext)
    {
        ESPCOM::print (F ("\"autentication\":\""), output, espresponse);
    } else
    {
        ESPCOM::print (F ("Authentication: "), output, espresponse);
    }
#ifdef AUTHENTICATION_FEATURE
    ESPCOM::print (F ("Enabled"), output, espresponse);
#else
    ESPCOM::print (F ("Disabled"), output, espresponse);
#endif
    if (!plaintext)
    {
        ESPCOM::print (F ("\","), output, espresponse);
    } else
    {
        ESPCOM::print (F ("\n"), output, espresponse);
    }

    if (!plaintext)
    {
        ESPCOM::print (F ("\"target_fw\":\""), output, espresponse);
    } else
    {
        ESPCOM::print (F ("Target Firmware: "), output, espresponse);
    }
    ESPCOM::print (CONFIG::GetFirmwareTargetName(), output, espresponse);
    if (!plaintext)
    {
        ESPCOM::print (F ("\","), output, espresponse);
    } else
    {
        ESPCOM::print (F ("\n"), output, espresponse);
    }
    //flag M117
    if (!plaintext)
    {
        ESPCOM::print (F ("\"M117_output\":\""), output, espresponse);
    } else
    {
        ESPCOM::print (F ("M117 output: "), output, espresponse);
    }
    if (!CONFIG::is_locked(FLAG_BLOCK_M117))
    {
        ESPCOM::print (F ("Enabled"), output, espresponse);
    } else
    {
        ESPCOM::print (F ("Disabled"), output, espresponse);
    }

    if (!plaintext)
    {
        ESPCOM::print (F ("\","), output, espresponse);
    } else
    {
        ESPCOM::print (F ("\n"), output, espresponse);
    }
#ifdef NOTIFICATION_FEATURE
    if (!plaintext)
    {
        ESPCOM::print (F ("\"Notifications\":\""), output, espresponse);
    } else
    {
        ESPCOM::print (F ("Notifications: "), output, espresponse);
    }
    if (notificationsservice.started())
    {
        ESPCOM::print (notificationsservice.getTypeString(), output, espresponse);
    } else
    {
        ESPCOM::print (F ("Disabled"), output, espresponse);
    }

    if (!plaintext)
    {
        ESPCOM::print (F ("\","), output, espresponse);
    } else
    {
        ESPCOM::print (F ("\n"), output, espresponse);
    }
#endif

    //Flag Oled
#ifdef ESP_OLED_FEATURE
    if (!plaintext)
    {
        ESPCOM::print (F ("\"Oled_output\":\""), output, espresponse);
    } else
    {
        ESPCOM::print (F ("Oled output: "), output, espresponse);
    }
    if (!CONFIG::is_locked(FLAG_BLOCK_OLED))
    {
        ESPCOM::print (F ("Enabled"), output, espresponse);
    } else
    {
        ESPCOM::print (F ("Disabled"), output, espresponse);
    }

    if (!plaintext)
    {
        ESPCOM::print (F ("\","), output, espresponse);
    } else
    {
        ESPCOM::print (F ("\n"), output, espresponse);
    }
#endif

    //flag serial
    if (!plaintext)
    {
        ESPCOM::print (F ("\"Serial_output\":\""), output, espresponse);
    } else
    {
        ESPCOM::print (F ("Serial output: "), output, espresponse);
    }
    if (!CONFIG::is_locked(FLAG_BLOCK_SERIAL))
    {
        ESPCOM::print (F ("Enabled"), output, espresponse);
    } else
    {
        ESPCOM::print (F ("Disabled"), output, espresponse);
    }

    if (!plaintext)
    {
        ESPCOM::print (F ("\","), output, espresponse);
    } else
    {
        ESPCOM::print (F ("\n"), output, espresponse);
    }

#ifdef WS_DATA_FEATURE
    //flag websocket
    if (!plaintext)
    {
        ESPCOM::print (F ("\"Websocket_output\":\""), output, espresponse);
    } else
    {
        ESPCOM::print (F ("Web socket  output: "), output, espresponse);
    }
    if (!CONFIG::is_locked(FLAG_BLOCK_WSOCKET))
    {
        ESPCOM::print (F ("Enabled"), output, espresponse);
    } else
    {
        ESPCOM::print (F ("Disabled"), output, espresponse);
    }

    if (!plaintext)
    {
        ESPCOM::print (F ("\","), output, espresponse);
    } else
    {
        ESPCOM::print (F ("\n"), output, espresponse);
    }
#endif
#ifdef TCP_IP_DATA_FEATURE
    //flag tcp
    if (!plaintext)
    {
        ESPCOM::print (F ("\"TCP_output\":\""), output, espresponse);
    } else
    {
        ESPCOM::print (F ("TCP output: "), output, espresponse);
    }
    if (!CONFIG::is_locked(FLAG_BLOCK_TCP))
    {
        ESPCOM::print (F ("Enabled"), output, espresponse);
    } else
    {
        ESPCOM::print (F ("Disabled"), output, espresponse);
    }

    if (!plaintext)
    {
        ESPCOM::print (F ("\","), output, espresponse);
    } else
    {
        ESPCOM::print (F ("\n"), output, espresponse);
    }
#endif
#ifdef DEBUG_ESP3D
    if (!plaintext)
    {
        ESPCOM::print (F ("\"debug\":\""), output, espresponse);
    } else
    {
        ESPCOM::print (F ("Debug: "), output, espresponse);
    }
    ESPCOM::print (F ("Debug Enabled :"), output, espresponse);
#ifdef DEBUG_OUTPUT_SPIFFS
    ESPCOM::print (F ("SPIFFS"), output, espresponse);
#endif
#ifdef DEBUG_OUTPUT_SD
    ESPCOM::print (F ("SD"), output, espresponse);
#endif
#ifdef DEBUG_OUTPUT_SERIAL
    ESPCOM::print (F ("serial"), output, espresponse);
#endif
#ifdef DEBUG_OUTPUT_TCP
    ESPCOM::print (F ("TCP"), output, espresponse);
#endif
    if (!plaintext)
    {
        ESPCOM::print (F ("\","), output, espresponse);
    } else
    {
        ESPCOM::print (F ("\n"), output, espresponse);
    }
#endif
    if (!plaintext)
    {
        ESPCOM::print (F ("\"fw\":\""), output, espresponse);
    } else
    {
        ESPCOM::print (F ("FW version: "), output, espresponse);
    }
    ESPCOM::print (FW_VERSION, output, espresponse);
#ifdef ARDUINO_ARCH_ESP8266
    ESPCOM::print (" ESP8266/8586", output, espresponse);
#else
    ESPCOM::print (" ESP32", output, espresponse);
#endif
    if (!plaintext)
    {
        ESPCOM::print (F ("\"}"), output, espresponse);
    } else
    {
        ESPCOM::print (F ("\n"), output, espresponse);
    }
}

#ifdef DEBUG_OUTPUT_SOCKET
#if defined(ARDUINO_ARCH_ESP8266)
#define NODEBUG_WEBSOCKETS
#include <WebSocketsServer.h>
extern WebSocketsServer * socket_server;
const char * pathToFileName(const char * path)
{
    size_t i = 0;
    size_t pos = 0;
    char * p = (char *)path;
    while(*p) {
        i++;
        if(*p == '/' || *p == '\\') {
            pos = i;
        }
        p++;
    }
    return path+pos;
}
#endif //ARDUINO_ARCH_ESP8266 

void log_socket(const char *format, ...){
    if(socket_server){
        char loc_buf[255];
        char * temp = loc_buf;
        va_list arg;
        va_list copy;
        va_start(arg, format);
        va_copy(copy, arg);
        size_t len = vsnprintf(NULL, 0, format, arg);
        va_end(copy);
        if(len >= sizeof(loc_buf)){
            temp = new char[len+1];
            if(temp == NULL) {
                return;
            }
        }
        len = vsnprintf(temp, len+1, format, arg);
        socket_server->sendBIN(ESPCOM::current_socket_id,(uint8_t *)temp,strlen(temp));
        va_end(arg);
        if(len > 255){
            delete[] temp;
        }
    }
}
#endif
