/*
  settings_esp3d.cpp -  settings esp3d functions class

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

#include "../include/esp3d_config.h"
#include "settings_esp3d.h"
#include "esp3doutput.h"
#if defined(SETTINGS_IN_EEPROM)
#include <EEPROM.h>
//EEPROM SIZE (Up to 4096)
#define EEPROM_SIZE		1024 //max is 1024
#endif //SETTINGS_IN_EEPROM

#if defined (WIFI_FEATURE) || defined(ETH_FEATURE)
#include "../modules/network/netconfig.h"
#if defined (WIFI_FEATURE)
#include "../modules/wifi/wificonfig.h"
#endif //WIFI_FEATURE
#endif //WIFI_FEATURE || ETH_FEATURE


#if defined(SETTINGS_IN_PREFERENCES)
#include <Preferences.h>
#define NAMESPACE "ESP3D"
#endif // SETTINGS_IN_PREFERENCES

//Current Settings Version
#define CURRENT_SETTINGS_VERSION "ESP3D03"

//boundaries
#define MAX_DHT_INTERVAL            1000
#define MIN_DHT_INTERVAL            0
#define MAX_LOCAL_PASSWORD_LENGTH   20
#define MIN_LOCAL_PASSWORD_LENGTH   1
#define MAX_VERSION_LENGTH          7 //ESP3DXX


//default byte values
#ifdef WIFI_FEATURE
#define DEFAULT_ESP_RADIO_MODE  ESP_WIFI_AP
#else //WIFI_FEATURE
#ifdef BLUETOOTH_FEATURE
#define DEFAULT_ESP_RADIO_MODE  ESP_BT
#else //BLUETOOTH_FEATURE
#ifdef ETH_FEATURE
#define DEFAULT_ESP_RADIO_MODE  ESP_ETH_STA
#else //BLUETOOTH_FEATURE
#define DEFAULT_ESP_RADIO_MODE  NO_NETWORK
#endif //ETH_FEATURE
#endif //BLUETOOTH_FEATURE
#endif //WIFI_FEATURE


#define DEFAULT_ESP_BYTE        0
#define DEFAULT_ESP_STRING_SIZE 0
#if defined (WIFI_FEATURE) || defined (ETH_FEATURE)
#define DEFAULT_STA_IP_MODE     DHCP_MODE
#endif //WIFI_FEATURE || ETH_FEATURE
//#define DEFAULT_PHY_MODE        WIFI_PHY_MODE_11G
//#define DEFAULT_SLEEP_MODE      WIFI_MODEM_SLEEP
#define DEFAULT_AP_CHANNEL         11
#define DEFAULT_AUTH_TYPE       AUTH_WPA_PSK
#define DEFAULT_SSID_VISIBLE    1
#define DEFAULT_OUTPUT_FLAG     ESP_ALL_CLIENTS
#define DEFAULT_SDREADER_SPEED  4
#define DEFAULT_FW              UNKNOWN_FW
#define DEFAULT_TIME_ZONE       0
#define DEFAULT_TIME_DST        0
#define DEFAULT_PRIMARY_SD      2
#define DEFAULT_SECONDARY_SD    1
#define DEFAULT_DIRECT_SD_CHECK 0
#define DEFAULT_SD_CHECK_UPDATE_AT_BOOT 1
#define DEFAULT_DHT_TYPE        255
#define DEFAULT_IS_DIRECT_SD    0
#define DEFAULT_HTTP_ON         1
#define DEFAULT_TELNET_ON       1


//default int values
#define DEFAULT_ESP_INT         0L
#define DEFAULT_BAUD_RATE       115200L
#define DEFAULT_HTTP_PORT       80L
#define DEFAULT_TELNET_PORT     23L
#define DEFAULT_DHT_INTERVAL    30L

//default string values
const char DEFAULT_AP_SSID []  PROGMEM =        "ESP3D";
const char DEFAULT_AP_PASSWORD [] PROGMEM =     "12345678";
const char DEFAULT_STA_SSID []  PROGMEM =       "ESP3D";
const char DEFAULT_STA_PASSWORD [] PROGMEM =    "12345678";
const char DEFAULT_HOSTNAME []  PROGMEM =       "esp3d";
const char DEFAULT_ESP_STRING []  =             "";
const char DEFAULT_ADMIN_PWD []  PROGMEM =      "admin";
const char DEFAULT_USER_PWD []  PROGMEM =       "user";
const char DEFAULT_TIME_SERVER1 []  PROGMEM =	"time.nist.gov";
const char DEFAULT_TIME_SERVER2 []  PROGMEM =	"0.pool.ntp.org";
const char DEFAULT_TIME_SERVER3 []  PROGMEM =	"1.pool.ntp.org";
const char DEFAULT_SETTINGS_VERSION []  PROGMEM =	"ESP3D";

//default IP values
const uint8_t DEFAULT_IP_VALUE[]   =       {192, 168, 0, 1};
const uint8_t DEFAULT_MASK_VALUE[]  =      {255, 255, 255, 0};
#define DEFAULT_GATEWAY_VALUE           DEFAULT_IP_VALUE
const uint8_t DEFAULT_ADDRESS_VALUE[]   =  {0, 0, 0, 0};

uint8_t Settings_ESP3D::_FirmwareTarget = UNKNOWN_FW;
bool Settings_ESP3D::_directSD = false;

Settings_ESP3D::Settings_ESP3D()
{
}
Settings_ESP3D::~Settings_ESP3D()
{
}

bool Settings_ESP3D::begin()
{
    if(GetSettingsVersion() == -1) {
        return false;
    }
    //get target FW
    Settings_ESP3D::GetFirmwareTarget(true);
    //is direct SD
    Settings_ESP3D::isDirectSD(true);
    return true;
}

uint8_t Settings_ESP3D::GetFirmwareTarget(bool fromsettings)
{
    if(fromsettings) {
        _FirmwareTarget = read_byte (ESP_TARGET_FW);
    }
    return _FirmwareTarget;
}

bool Settings_ESP3D::isDirectSD(bool fromsettings)
{
    if(fromsettings) {
#ifdef SDCARD_FEATURE
        _directSD = read_byte (ESP_IS_DIRECT_SD);
#else // !SDCARD_FEATURE
        _directSD = false;
#endif //SDCARD_FEATURE
    }
    return _directSD;
}

const char* Settings_ESP3D::GetFirmwareTargetShortName()
{
    static String response;
    if ( _FirmwareTarget == REPETIER4DV) {
        response = F ("repetier4davinci");
    } else if ( _FirmwareTarget == REPETIER) {
        response = F ("repetier");
    } else if ( _FirmwareTarget == MARLIN) {
        response = F ("marlin");
    } else if ( _FirmwareTarget == MARLINKIMBRA) {
        response = F ("marlinkimbra");
    } else if ( _FirmwareTarget == SMOOTHIEWARE) {
        response = F ("smoothieware");
    } else if ( _FirmwareTarget == GRBL) {
        response = F ("grbl");
    } else {
        response = F ("???");
    }
    return response.c_str();
}

//Default value for a byte setting
uint8_t Settings_ESP3D::get_default_byte_value(int pos)
{
    uint8_t res;
    switch(pos) {
    case ESP_RADIO_MODE:
        res = DEFAULT_ESP_RADIO_MODE;
        break;
#if defined (WIFI_FEATURE) || defined (ETH_FEATURE)
    case ESP_STA_IP_MODE:
        res = DEFAULT_STA_IP_MODE;
        break;
#endif //WIFI_FEATURE || ETH_FEATURE
        //case ESP_AP_PHY_MODE:
        //case ESP_STA_PHY_MODE:
        //    res = DEFAULT_PHY_MODE;
        //    break;
        //case ESP_SLEEP_MODE:
        //    res = DEFAULT_SLEEP_MODE;
        //    break;
#if defined (WIFI_FEATURE)
    case ESP_AP_CHANNEL:
        res = DEFAULT_AP_CHANNEL;
        break;
        //case ESP_AP_AUTH_TYPE:
        //    res = DEFAULT_AUTH_TYPE;
        //    break;
        //case ESP_SSID_VISIBLE:
        //    res = DEFAULT_SSID_VISIBLE;
        //    break;
#endif //WIFI_FEATURE 
    case ESP_OUTPUT_FLAG:
        res = DEFAULT_OUTPUT_FLAG;
        break;
    case ESP_HTTP_ON:
        res = DEFAULT_HTTP_ON;
        break;
    case ESP_TELNET_ON:
        res = DEFAULT_TELNET_ON;
        break;
    case ESP_SD_SPEED_DIV:
        res = DEFAULT_SDREADER_SPEED;
        break;
    case ESP_TARGET_FW:
        res = DEFAULT_FW;
        break;
    case ESP_TIMEZONE:
        res = DEFAULT_TIME_ZONE;
        break;
    case ESP_TIME_IS_DST:
        res = DEFAULT_TIME_DST;
        break;
    case ESP_IS_DIRECT_SD:
        res = DEFAULT_IS_DIRECT_SD;
        break;
    case ESP_PRIMARY_SD:
        res = DEFAULT_PRIMARY_SD;
        break;
    case ESP_SECONDARY_SD:
        res = DEFAULT_SECONDARY_SD;
        break;
    case ESP_DIRECT_SD_CHECK:
        res = DEFAULT_DIRECT_SD_CHECK;
        break;
    case ESP_DHT_TYPE:
        res = DEFAULT_DHT_TYPE;
        break;
    case ESP_SD_CHECK_UPDATE_AT_BOOT:
        res = DEFAULT_SD_CHECK_UPDATE_AT_BOOT;
        break;
    default:
        res = DEFAULT_ESP_BYTE;
    }
    return res;
}

//Default value for a int32 setting
uint32_t Settings_ESP3D::get_default_int32_value(int pos)
{
    uint32_t res;
    switch(pos) {
    case ESP_BAUD_RATE:
        res = DEFAULT_BAUD_RATE;
        break;
#if defined (WIFI_FEATURE) || defined (ETH_FEATURE)
    case ESP_AP_IP_VALUE:
    case ESP_STA_IP_VALUE:
        res = IPAddress(DEFAULT_IP_VALUE);
        break;
    case ESP_STA_MASK_VALUE:
        res = IPAddress(DEFAULT_MASK_VALUE);
        break;
    case ESP_STA_GATEWAY_VALUE:
        res = IPAddress(DEFAULT_GATEWAY_VALUE);
        break;
    case ESP_HTTP_PORT:
        res = DEFAULT_HTTP_PORT;
        break;
    case ESP_TELNET_PORT:
        res = DEFAULT_TELNET_PORT;
        break;
#endif //WIFI_FEATURE || ETH_FEATURE
    case ESP_DHT_INTERVAL:
        res = DEFAULT_DHT_INTERVAL;
        break;
    default:
        res = DEFAULT_ESP_INT;
    }
    return res;
}

//Max value for a int32 setting
uint32_t Settings_ESP3D::get_max_int32_value(int pos)
{
    uint32_t res;
    switch(pos) {
#if defined (WIFI_FEATURE) || defined (ETH_FEATURE)
    case ESP_HTTP_PORT:
        res = MAX_HTTP_PORT;
        break;
    case ESP_TELNET_PORT:
        res = MAX_TELNET_PORT;
        break;
#endif //WIFI_FEATURE || ETH_FEATURE
    case ESP_DHT_INTERVAL:
        res = MAX_DHT_INTERVAL;
        break;
    default:
        res = DEFAULT_ESP_INT;
    }
    return res;
}

//Min value for a int32 setting
uint32_t Settings_ESP3D::get_min_int32_value(int pos)
{
    uint32_t res;
    switch(pos) {
#if defined (WIFI_FEATURE) || defined (ETH_FEATURE)
    case ESP_HTTP_PORT:
        res = MIN_HTTP_PORT;
        break;
    case ESP_TELNET_PORT:
        res = MIN_TELNET_PORT;
        break;
#endif //WIFI_FEATURE || ETH_FEATURE
    case ESP_DHT_INTERVAL:
        res = MIN_DHT_INTERVAL;
        break;
    default:
        res = DEFAULT_ESP_INT;
    }
    return res;
}

uint8_t Settings_ESP3D::get_max_byte(int pos)
{
    uint8_t res;
    switch(pos) {
#if defined (WIFI_FEATURE)
    case ESP_AP_CHANNEL:
        res = MAX_CHANNEL;
        break;
#endif //WIFI_FEATURE
    default:
        res = 255;
    }
    return res;
}
uint8_t Settings_ESP3D::get_min_byte(int pos)
{
    uint8_t res;
    switch(pos) {
#if defined (WIFI_FEATURE)
    case ESP_AP_CHANNEL:
        res = MIN_CHANNEL;
        break;
#endif //WIFI_FEATURE || ETH_FEATURE
    default:
        res = 0;
    }
    return res;
}


//Default value for a ip setting
uint32_t Settings_ESP3D::get_default_IP_value(int pos)
{
    return get_default_int32_value(pos);
}

//Default value for a byte setting
const String & Settings_ESP3D::get_default_string_value(int pos)
{
    static String res;
    switch(pos) {
#if defined (WIFI_FEATURE) || defined (ETH_FEATURE)
    case ESP_HOSTNAME:
        res = DEFAULT_HOSTNAME;
        break;
    case ESP_TIME_SERVER1:
        res = DEFAULT_TIME_SERVER1;
        break;
    case ESP_TIME_SERVER2:
        res = DEFAULT_TIME_SERVER2;
        break;
    case ESP_TIME_SERVER3:
        res = DEFAULT_TIME_SERVER3;
        break;
#endif //WIFI_FEATURE || ETH_FEATURE
#if defined (WIFI_FEATURE)
    case ESP_STA_SSID:
        res = DEFAULT_STA_SSID;
        break;
    case ESP_AP_SSID:
        res = DEFAULT_AP_SSID;
        break;
    case ESP_STA_PASSWORD:
        res = DEFAULT_STA_PASSWORD;
        break;
    case ESP_AP_PASSWORD:
        res = DEFAULT_AP_PASSWORD;
        break;
#endif //WIFI_FEATURE 
    case ESP_ADMIN_PWD:
        res = DEFAULT_ADMIN_PWD;
        break;
    case ESP_USER_PWD:
        res = DEFAULT_USER_PWD;
        break;

    case ESP_SETTINGS_VERSION:
        res = DEFAULT_SETTINGS_VERSION;
        break;
    default:
        res = DEFAULT_ESP_STRING;
    }
    return res;
}

//Max size of for a string setting
uint8_t Settings_ESP3D::get_max_string_size(int pos)
{
    uint8_t res;
    switch(pos) {
#if defined (WIFI_FEATURE) || defined (ETH_FEATURE)

    case ESP_HOSTNAME:
        res = MAX_HOSTNAME_LENGTH;
        break;
    case ESP_TIME_SERVER1:
    case ESP_TIME_SERVER2:
    case ESP_TIME_SERVER3:
        res =  MAX_SERVER_ADDRESS_LENGTH;
        break;
#endif //WIFI_FEATURE || ETH_FEATURE 
#if defined (WIFI_FEATURE)
    case ESP_STA_SSID:
    case ESP_AP_SSID:
        res = MAX_SSID_LENGTH;
        break;
    case ESP_STA_PASSWORD:
    case ESP_AP_PASSWORD:
        res = MAX_PASSWORD_LENGTH;
        break;
#endif //WIFI_FEATURE 
    case ESP_ADMIN_PWD:
    case ESP_USER_PWD:
        res = MAX_LOCAL_PASSWORD_LENGTH;
        break;

    case ESP_SETTINGS_VERSION:
        res = MAX_VERSION_LENGTH;
        break;
    default:
        res = DEFAULT_ESP_STRING_SIZE;
    }
    return res;
}

//Min size of for a string setting
uint8_t Settings_ESP3D::get_min_string_size(int pos)
{
    uint8_t res;
    switch(pos) {
#if defined (WIFI_FEATURE) || defined (ETH_FEATURE)
    case ESP_HOSTNAME:
        res = MIN_HOSTNAME_LENGTH;
        break;
    case ESP_TIME_SERVER1:
    case ESP_TIME_SERVER2:
    case ESP_TIME_SERVER3:
        res =  MIN_SERVER_ADDRESS_LENGTH;
        break;
#endif //WIFI_FEATURE || ETH_FEATURE
#if defined (WIFI_FEATURE)
    case ESP_STA_SSID:
    case ESP_AP_SSID:
        res = MIN_SSID_LENGTH;
        break;
    case ESP_STA_PASSWORD:
    case ESP_AP_PASSWORD:
        res = MIN_PASSWORD_LENGTH;
        break;
#endif
    case ESP_ADMIN_PWD:
    case ESP_USER_PWD:
        res = MIN_LOCAL_PASSWORD_LENGTH;
        break;

    default:
        res = DEFAULT_ESP_STRING_SIZE;
    }
    return res;
}

uint8_t Settings_ESP3D::read_byte (int pos, bool * haserror)
{
    if(haserror) {
        *haserror = true;
    }
    uint8_t value = get_default_byte_value(pos);
#if defined( SETTINGS_IN_EEPROM)
//check if parameters are acceptable
    if ((pos + 1 > EEPROM_SIZE) )  {
        log_esp3d("Error read byte %d", pos);
        return value;
    }
//read byte
    EEPROM.begin (EEPROM_SIZE);
    value = EEPROM.read (pos);
    EEPROM.end();
#endif //SETTINGS_IN_EEPROM
#if defined(SETTINGS_IN_PREFERENCES)
    Preferences prefs;
    if (!prefs.begin(NAMESPACE, true)) {
        log_esp3d("Error opening %s", NAMESPACE);
        return value;
    }
    String p = "P_" + String(pos);
    value  = prefs.getChar(p.c_str(), get_default_byte_value(pos));
    prefs.end();
#endif //SETTINGS_IN_PREFERENCES
    if(haserror) {
        *haserror = false;
    }
    return value;
}

//write a flag / byte
bool Settings_ESP3D::write_byte (int pos, const uint8_t value)
{
#if defined( SETTINGS_IN_EEPROM)
    //check if parameters are acceptable
    if (pos + 1 > EEPROM_SIZE) {
        log_esp3d("Error read byte %d", pos);
        return false;
    }
    EEPROM.begin (EEPROM_SIZE);
    EEPROM.write (pos, value);
    if (!EEPROM.commit()) {
        log_esp3d("Error commit %d", pos);
        return false;
    }
    EEPROM.end();
#endif //SETTINGS_IN_EEPROM
#if defined(SETTINGS_IN_PREFERENCES)
    Preferences prefs;
    if (!prefs.begin(NAMESPACE, false)) {
        log_esp3d("Error opening %s", NAMESPACE);
        return false;
    }
    String p = "P_" + String(pos);
    uint8_t r = prefs.putChar(p.c_str(), value);
    prefs.end();
    if (r == 0) {
        log_esp3d("Error commit %s", p.c_str());
        return false;
    }
#endif //SETTINGS_IN_PREFERENCES 
    return true;
}

//read a string
//a string is multibyte + \0, this is won't work if 1 char is multibyte like chinese char
const char * Settings_ESP3D::read_string (int pos, bool *haserror)
{
    uint8_t size_max = get_max_string_size(pos);
    if (haserror) {
        *haserror = true;
    }
    if (size_max == 0) {
        log_esp3d("Error size string %d", pos);
        return DEFAULT_ESP_STRING;
    }
#if defined(SETTINGS_IN_EEPROM)
    static char * byte_buffer = NULL;
    size_max++;//do not forget the 0x0 for the end
    if (byte_buffer) {
        free (byte_buffer);
        byte_buffer = NULL;
    }
    //check if parameters are acceptable
    if (pos + size_max + 1 > EEPROM_SIZE) {
        log_esp3d("Error read string %d", pos);
        return DEFAULT_ESP_STRING;
    }
    byte_buffer = (char *)malloc(size_max+1);
    if (!byte_buffer) {
        log_esp3d("Error mem read string %d", pos);
        return DEFAULT_ESP_STRING;
    }
    EEPROM.begin (EEPROM_SIZE);
    byte b = 1; // non zero for the while loop below
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
    if (haserror) {
        *haserror = false;
    }
    return byte_buffer;
#endif //SETTINGS_IN_EEPROM
#if defined(SETTINGS_IN_PREFERENCES)
    Preferences prefs;
    static String res;

    if (!prefs.begin(NAMESPACE, true)) {
        log_esp3d("Error opening %s", NAMESPACE);
        return "";
    }
    String p = "P_" + String(pos);
    res = prefs.getString(p.c_str(), get_default_string_value(pos));
    prefs.end();
    if(haserror) {
        *haserror = false;
    }
    if (res.length() > size_max) {
        log_esp3d("String too long %d vs %d", res.length(), size_max);
        res = res.substring(0,size_max-1);
    }
    return res.c_str();
#endif //SETTINGS_IN_PREFERENCES
}

//write a string (array of byte with a 0x00  at the end)
bool Settings_ESP3D::write_string (int pos, const char * byte_buffer)
{
    int size_buffer = strlen (byte_buffer);
    uint8_t size_max = get_max_string_size(pos);
    //check if parameters are acceptable
    if (size_max == 0) {
        log_esp3d("Error unknow entry %d", pos);
        return false;
    }
    if (size_max < size_buffer) {
        log_esp3d("Error string too long %d, %d", pos, size_buffer);
        return false;
    }
#if defined(SETTINGS_IN_EEPROM)
    if (  pos + size_buffer + 1 > EEPROM_SIZE  || byte_buffer == NULL) {
        log_esp3d("Error write string %d", pos);
        return false;
    }
    //copy the value(s)
    EEPROM.begin (EEPROM_SIZE);
    for (int i = 0; i < size_buffer; i++) {
        EEPROM.write (pos + i, byte_buffer[i]);
    }
    //0 terminal
    EEPROM.write (pos + size_buffer, 0x00);
    if (!EEPROM.commit()) {
        log_esp3d("Error commit %d", pos);
        return false;
    }
    EEPROM.end();
#endif //SETTINGS_IN_EEPROM
#if defined(SETTINGS_IN_PREFERENCES)
    Preferences prefs;
    if (!prefs.begin(NAMESPACE, false)) {
        log_esp3d("Error opening %s", NAMESPACE);
        return false;
    }
    String p = "P_" + String(pos);
    uint8_t r = prefs.putString(p.c_str(), byte_buffer);
    prefs.end();
    if (r == 0) {
        log_esp3d("Error commit %s", p.c_str());
        return false;
    }
#endif //SETTINGS_IN_PREFERENCES
    return true;
}

//read a uint32
uint32_t Settings_ESP3D::read_uint32(int pos, bool * haserror)
{
    if (haserror) {
        *haserror = true;
    }
    uint32_t res = get_default_int32_value(pos);
#if defined(SETTINGS_IN_EEPROM)
    //check if parameters are acceptable
    uint8_t size_buffer = sizeof(uint32_t);
    if ( pos + size_buffer > EEPROM_SIZE ) {
        log_esp3d("Error read int %d", pos);
        return res;
    }
    uint8_t i = 0;
    EEPROM.begin (EEPROM_SIZE);
    //read until max size is reached
    while (i < size_buffer ) {
        ((uint8_t *)(&res))[i] = EEPROM.read (pos + i);
        i++;
    }
    EEPROM.end();
#endif //SETTINGS_IN_EEPROM
#if defined(SETTINGS_IN_PREFERENCES)
    Preferences prefs;
    if (!prefs.begin(NAMESPACE, true)) {
        log_esp3d("Error opening %s", NAMESPACE);
        return res;
    }
    String p = "P_" + String(pos);
    res = prefs.getUInt(p.c_str(), res);
    prefs.end();
#endif //SETTINGS_IN_PREFERENCES
    if (haserror) {
        *haserror = false;
    }
    return res;
}

//read an IP
uint32_t Settings_ESP3D::read_IP(int pos, bool * haserror)
{
    return read_uint32(pos,haserror);
}

//read an IP
String Settings_ESP3D::read_IP_String(int pos, bool * haserror)
{
    return IPtoString(read_uint32(pos,haserror));
}

//write a uint32
bool Settings_ESP3D::write_uint32(int pos, const uint32_t value)
{
#if defined(SETTINGS_IN_EEPROM)
    uint8_t size_buffer = sizeof(uint32_t);
    //check if parameters are acceptable
    if (pos + size_buffer > EEPROM_SIZE) {
        log_esp3d("Error invalid entry %d", pos);
        return false;
    }
    EEPROM.begin (EEPROM_SIZE);
    //copy the value(s)
    for (int i = 0; i < size_buffer; i++) {
        EEPROM.write (pos + i, ((uint8_t *)(&value))[i]);
    }
    if (!EEPROM.commit()) {
        log_esp3d("Error commit %d", pos);
        return false;
    }
    EEPROM.end();
#endif //SETTINGS_IN_EEPROM
#if defined(SETTINGS_IN_PREFERENCES)
    Preferences prefs;
    if (!prefs.begin(NAMESPACE, false)) {
        log_esp3d("Error opening %s", NAMESPACE);
        return false;
    }
    String p = "P_" + String(pos);
    uint8_t r = prefs.putUInt(p.c_str(), value);
    prefs.end();
    if (r == 0) {
        log_esp3d("Error commit %s", p.c_str());
        return false;
    }
#endif //SETTINGS_IN_PREFERENCES
    return true;
}

//write a IP
bool Settings_ESP3D::write_IP(int pos, const uint32_t value)
{
    return write_uint32(pos, value);
}

//clear all entries
bool Settings_ESP3D::reset()
{
    bool res = true;
    log_esp3d("Reset Settings");
#if defined(SETTINGS_IN_PREFERENCES)
    Preferences prefs;
    if (!prefs.begin(NAMESPACE, false)) {
        return false;
    }
    res = prefs.clear();
    prefs.end();
#endif //SETTINGS_IN_PREFERENCES 
//for EEPROM need to overwrite all settings
#if defined(SETTINGS_IN_EEPROM)
#if defined (WIFI_FEATURE) || defined (BLUETOOTH_FEATURE) || defined (ETH_FEATURE)
    //Hostname
    Settings_ESP3D::write_string(ESP_HOSTNAME,Settings_ESP3D::get_default_string_value(ESP_HOSTNAME).c_str());
#endif //WIFI_FEATURE ||  BLUETOOTH_FEATURE
    //radio mode
    Settings_ESP3D::write_byte(ESP_RADIO_MODE,Settings_ESP3D::get_default_byte_value(ESP_RADIO_MODE));
#if defined (WIFI_FEATURE) || defined (ETH_FEATURE)
    //STA SSID
    Settings_ESP3D::write_string(ESP_STA_SSID,Settings_ESP3D::get_default_string_value(ESP_STA_SSID).c_str());
    //STA pwd
    Settings_ESP3D::write_string(ESP_STA_PASSWORD,Settings_ESP3D::get_default_string_value(ESP_STA_PASSWORD).c_str());
    //STA Network Mode
    //Settings_ESP3D::write_byte(ESP_STA_PHY_MODE,Settings_ESP3D::get_default_byte_value(ESP_STA_PHY_MODE));
    //STA IP mode
    Settings_ESP3D::write_byte(ESP_STA_IP_MODE,Settings_ESP3D::get_default_byte_value(ESP_STA_IP_MODE));
    //STA static IP
    Settings_ESP3D::write_IP(ESP_STA_IP_VALUE, Settings_ESP3D::get_default_IP_value(ESP_STA_IP_VALUE));
    //STA static Gateway
    Settings_ESP3D::write_IP(ESP_STA_GATEWAY_VALUE, Settings_ESP3D::get_default_IP_value(ESP_STA_GATEWAY_VALUE));
    //STA static Mask
    Settings_ESP3D::write_IP(ESP_STA_MASK_VALUE, Settings_ESP3D::get_default_IP_value(ESP_STA_MASK_VALUE));
    //AP SSID
    Settings_ESP3D::write_string(ESP_AP_SSID,Settings_ESP3D::get_default_string_value(ESP_AP_SSID).c_str());
    //AP password
    Settings_ESP3D::write_string(ESP_AP_PASSWORD,Settings_ESP3D::get_default_string_value(ESP_AP_PASSWORD).c_str());
    //AP static IP
    Settings_ESP3D::write_IP(ESP_AP_IP_VALUE, Settings_ESP3D::get_default_IP_value(ESP_AP_IP_VALUE));
    //AP Channel
    Settings_ESP3D::write_byte(ESP_AP_CHANNEL,Settings_ESP3D::get_default_byte_value(ESP_AP_CHANNEL));
    //AP Network Mode (PHY)
    //Settings_ESP3D::write_byte(ESP_AP_PHY_MODE,Settings_ESP3D::get_default_byte_value(ESP_AP_PHY_MODE));
    //AP Authentication
    //Settings_ESP3D::write_byte(ESP_AP_AUTH_TYPE,Settings_ESP3D::get_default_byte_value(ESP_AP_AUTH_TYPE));
    //AP SSID visibility
    //Settings_ESP3D::write_byte(ESP_SSID_VISIBLE,Settings_ESP3D::get_default_byte_value(ESP_SSID_VISIBLE));
#ifdef HTTP_FEATURE
    //HTTP On
    Settings_ESP3D::write_byte(ESP_HTTP_ON,Settings_ESP3D::get_default_byte_value(ESP_HTTP_ON));
    //HTTP Port
    Settings_ESP3D::write_uint32 (ESP_HTTP_PORT, Settings_ESP3D::get_default_int32_value(ESP_HTTP_PORT));
#endif //HTTP_FEATURE

#ifdef TELNET_FEATURE
    //TELNET On
    Settings_ESP3D::write_byte(ESP_TELNET_ON,Settings_ESP3D::get_default_byte_value(ESP_TELNET_ON));
    //TELNET Port
    Settings_ESP3D::write_uint32 (ESP_TELNET_PORT, Settings_ESP3D::get_default_int32_value(ESP_TELNET_PORT));
#endif //TELNET
#endif //WIFI_FEATURE
#ifdef AUTHENTICATION_FEATURE
    //Admin password
    Settings_ESP3D::write_string(ESP_ADMIN_PWD,Settings_ESP3D::get_default_string_value(ESP_ADMIN_PWD).c_str());
    //User password
    Settings_ESP3D::write_string(ESP_USER_PWD,Settings_ESP3D::get_default_string_value(ESP_USER_PWD).c_str());
#endif //AUTHENTICATION_FEATURE
    //Target FW
    Settings_ESP3D::write_byte(ESP_TARGET_FW,Settings_ESP3D::get_default_byte_value(ESP_TARGET_FW));
    //Output flag
    Settings_ESP3D::write_byte(ESP_OUTPUT_FLAG,Settings_ESP3D::get_default_byte_value(ESP_OUTPUT_FLAG));
#ifdef SDCARD_FEATURE
    //Direct SD
    Settings_ESP3D::write_byte(ESP_IS_DIRECT_SD,Settings_ESP3D::get_default_byte_value(ESP_IS_DIRECT_SD));
#endif //SDCARD_FEATURE      
#endif //SETTINGS_IN_EEPROM
    //set version in settings
    if (res) {
        res =  Settings_ESP3D::write_string(ESP_SETTINGS_VERSION, CURRENT_SETTINGS_VERSION);
    }
    return res;
}

//Get Settings Version
// * -1 means no version detected
// * 00 / 01 Not used
// * 03 and up is version
int8_t Settings_ESP3D::GetSettingsVersion()
{
    int8_t v = -1;
    String version  = Settings_ESP3D::read_string(ESP_SETTINGS_VERSION);
    if ((version == "ESP3D") ||( version.length() != 7) || (version.indexOf("ESP3D")!=0)) {
        log_esp3d("Invalid Settings Version %s",version.c_str());
        return v;
    }
    v = version.substring(5).toInt();
    log_esp3d("Settings Version %d", v);
    return v;
}

//write a IP from string
bool Settings_ESP3D::write_IP_String(int pos, const char * value)
{
    return write_uint32(pos, StringtoIP(value));
}

//Helper to convert  IP string to int
uint32_t Settings_ESP3D::StringtoIP(const char *s)
{
    uint32_t ip_int = 0;
    IPAddress ipaddr;
    if (ipaddr.fromString(s)) {
        ip_int = ipaddr;
    }
    return ip_int;
}

// Helper to convert int to IP string
String Settings_ESP3D::IPtoString(uint32_t ip_int)
{
    static IPAddress ipaddr;
    ipaddr = ip_int;
    return ipaddr.toString();
}

const char * Settings_ESP3D::TargetBoard()
{
#ifdef ARDUINO_ARCH_ESP32
    return "ESP32";
#endif //ARDUINO_ARCH_ESP32
#ifdef ARDUINO_ARCH_ESP8266
    return "ESP82XX";
#endif //ARDUINO_ARCH_ESP8266
}

bool Settings_ESP3D::isLocalPasswordValid (const char * password)
{
    char c;
    //limited size
    if ( (strlen (password) > MAX_LOCAL_PASSWORD_LENGTH) ||  (strlen (password) <= MIN_LOCAL_PASSWORD_LENGTH) ) {
        return false;
    }
    //no space allowed
    for (uint8_t i = 0; i < strlen (password); i++) {
        c = password[i];
        if (c == ' ') {
            return false;
        }
    }
    return true;
}
