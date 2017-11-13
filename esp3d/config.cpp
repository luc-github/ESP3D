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
#else
#include "Update.h"
#include "esp_wifi.h"
#endif
#include "bridge.h"

#ifdef ARDUINO_ARCH_ESP32
//This is output for ESP32 to avoid garbage
HardwareSerial Serial2(2);
#endif

uint8_t CONFIG::FirmwareTarget = UNKNOWN_FW;

bool CONFIG::SetFirmwareTarget(uint8_t fw){
    if ( fw >= 0 && fw <= MAX_FW_ID) {
        FirmwareTarget = fw;
        return true;
    } else return false;
}
uint8_t CONFIG::GetFirmwareTarget() {
    return FirmwareTarget;
}
const char* CONFIG::GetFirmwareTargetName() {
    static String response;
    if ( CONFIG::FirmwareTarget == REPETIER4DV)response = F("Repetier for Davinci");
    else if ( CONFIG::FirmwareTarget == REPETIER)response = F("Repetier");
    else if ( CONFIG::FirmwareTarget == MARLIN) response = F("Marlin");
    else if ( CONFIG::FirmwareTarget == MARLINKIMBRA) response = F("MarlinKimbra");
    else if ( CONFIG::FirmwareTarget == SMOOTHIEWARE) response = F("Smoothieware");
    else response = F("???");
    return response.c_str();
}

const char* CONFIG::GetFirmwareTargetShortName() {
    static String response;
    if ( CONFIG::FirmwareTarget == REPETIER4DV)response = F("repetier4davinci");
    else if ( CONFIG::FirmwareTarget == REPETIER)response = F("repetier");
    else if ( CONFIG::FirmwareTarget == MARLIN) response = F("marlin");
    else if ( CONFIG::FirmwareTarget == MARLINKIMBRA) response = F("marlinkimbra");
    else if ( CONFIG::FirmwareTarget == SMOOTHIEWARE) response = F("smoothieware");
    else response = F("???");
    return response.c_str();
}

void CONFIG::InitFirmwareTarget(){
    uint8_t b = UNKNOWN_FW;
    if (!CONFIG::read_byte(EP_TARGET_FW, &b )) {
        b = UNKNOWN_FW;
        }
    if (!SetFirmwareTarget(b))SetFirmwareTarget(UNKNOWN_FW) ;
}

void CONFIG::InitDirectSD(){
 CONFIG::is_direct_sd = false;

}

bool CONFIG::InitBaudrate(){
    long baud_rate=0;
     if ( !CONFIG::read_buffer(EP_BAUD_RATE,  (byte *)&baud_rate, INTEGER_LENGTH)) return false;
      if ( ! (baud_rate==9600 || baud_rate==19200 ||baud_rate==38400 ||baud_rate==57600 ||baud_rate==115200 ||baud_rate==230400 ||baud_rate==250000) ) return false;
     //setup serial
     if (ESP_SERIAL_OUT.baudRate() != baud_rate)ESP_SERIAL_OUT.begin(baud_rate);
#ifdef ARDUINO_ARCH_ESP8266
     ESP_SERIAL_OUT.setRxBufferSize(SERIAL_RX_BUFFER_SIZE);
#endif
     wifi_config.baud_rate=baud_rate;
     delay(1000);
     return true;
}

bool CONFIG::InitExternalPorts(){
    if (!CONFIG::read_buffer(EP_WEB_PORT,  (byte *)&(wifi_config.iweb_port), INTEGER_LENGTH) || !CONFIG::read_buffer(EP_DATA_PORT,  (byte *)&(wifi_config.idata_port), INTEGER_LENGTH)) return false;
    if (wifi_config.iweb_port < DEFAULT_MIN_WEB_PORT ||wifi_config.iweb_port > DEFAULT_MAX_WEB_PORT || wifi_config.idata_port < DEFAULT_MIN_DATA_PORT || wifi_config.idata_port > DEFAULT_MAX_DATA_PORT) return false;
    return true;
}

#ifdef SDCARD_CONFIG_FEATURE
bool CONFIG::update_settings(){
    String filename = SDCARD_CONFIG_FILENAME;
    filename.toLowerCase();
    //there is a config file
    LOG("Check ")
    LOG(filename)
    LOG("\r\n")
    bool answer = true;
    if(SD.exists((const char *)filename.c_str())) {
        bool success = true;
        bool localerror = false;
        const size_t bufferLen = 250;
        char buffer[bufferLen];
        byte ip_sav[4];
        String stmp;
        int itmp;
        long baud_rate=0;
        byte bflag;
        LOG("Found config file\r\n")
        String newfilename = filename;
        IniFile espconfig((char *)filename.c_str());
        //validate file is correct
        if (espconfig.open()) {
            if (!espconfig.validate(buffer, bufferLen)) {
                success = false;
                LOG("Invalid config file\r\n")
            } else { //file format is correct - let parse the settings
                //Section [wifi]
                //Hostname
                //hostname = myesp
                if (espconfig.getValue("network", "hostname", buffer, bufferLen)) {
                    if (!CONFIG::isHostnameValid(buffer)) {
                        success = false;
                    } else if(!CONFIG::write_string(EP_HOSTNAME,buffer)) {
                        success = false;
                    }
                }
                //baudrate = 115200
                if (espconfig.getValue("network", "baudrate", buffer, bufferLen, baud_rate)) {
                    if(!CONFIG::write_buffer(EP_BAUD_RATE,(const byte *)&baud_rate,INTEGER_LENGTH)) {
                        success = false;
                    }
                }
                //webport = 80
                if (espconfig.getValue("network", "webport", buffer, bufferLen, itmp)) {
                    if(!CONFIG::write_buffer(EP_WEB_PORT,(const byte *)&itmp,INTEGER_LENGTH)) {
                        success = false;
                    }
                }
                //dataport = 8888
                if (espconfig.getValue("network", "dataport", buffer, bufferLen, itmp)) {
                    if(!CONFIG::write_buffer(EP_DATA_PORT,(const byte *)&itmp,INTEGER_LENGTH)) {
                        success = false;
                    }
                }
                //adminpwd = admin
                if (espconfig.getValue("network", "adminpwd", buffer, bufferLen)) {
                    if (!CONFIG::isLocalPasswordValid(buffer)) {
                        success = false;
                    } else if(!CONFIG::write_string(EP_ADMIN_PWD,buffer)) {
                        success = false;
                    }
                }
                //userpwd = user
                if (espconfig.getValue("network", "userpwd", buffer, bufferLen)) {
                    if (!CONFIG::isLocalPasswordValid(buffer)) {
                        success = false;
                    } else if(!CONFIG::write_string(EP_USER_PWD,buffer)) {
                        success = false;
                    }
                }
                //sleep = none
                if (espconfig.getValue("network", "sleep", buffer, bufferLen)) {
                    stmp = buffer;
                    localerror = false;
                    stmp.toLowerCase();
                    bflag = 0;
                    if (stmp =="none") {
                        bflag = WIFI_NONE_SLEEP;
#ifdef ARDUINO_ARCH_ESP8266
                    } else  if (stmp =="light") {
                        bflag = WIFI_LIGHT_SLEEP;
#endif
                    } else  if (stmp =="modem") {
                        bflag = WIFI_MODEM_SLEEP;
                    } else {
                        localerror = true;
                        success = false;
                    }
                    if (!localerror) {
                        if(!CONFIG::write_byte(EP_SLEEP_MODE,bflag)) {
                            success = false;
                        }
                    }
                }
                //wifimode = STA
                if (espconfig.getValue("network", "wifimode", buffer, bufferLen)) {
                    stmp = buffer;
                    localerror = false;
                    stmp.toLowerCase();
                    bflag = 0;
                    if (stmp =="sta") {
                        bflag = CLIENT_MODE;
                    } else  if (stmp =="ap") {
                        bflag = AP_MODE;
                    } else {
                        localerror = true;
                        success = false;
                    }
                    if (!localerror) {
                        if(!CONFIG::write_byte(EP_WIFI_MODE,bflag)) {
                            success = false;
                        }
                    }
                }
                //STAssid = NETGEAR_2GEXT_OFFICE2
                if (espconfig.getValue("network", "STAssid", buffer, bufferLen)) {
                    if (!CONFIG::isSSIDValid(buffer)) {
                        success = false;
                    } else if(!CONFIG::write_string(EP_STA_SSID,buffer)) {
                        success = false;
                    }
                }
                //STApassword = mypassword
                if (espconfig.getValue("network", "STApassword", buffer, bufferLen)) {
                    if (!CONFIG::isPasswordValid(buffer)) {
                        success = false;
                    } else if(!CONFIG::write_string(EP_STA_PASSWORD,buffer)) {
                        success = false;
                    }
                }
                //STAipmode = DHCP
                if (espconfig.getValue("network", "STAipmode", buffer, bufferLen)) {
                    stmp = buffer;
                    localerror = false;
                    stmp.toLowerCase();
                    bflag = 0;
                    if (stmp =="dhcp") {
                        bflag = DHCP_MODE;
                    } else  if (stmp =="static") {
                        bflag = STATIC_IP_MODE;
                    } else {
                        localerror = true;
                        success = false;
                    }
                    if (!localerror) {
                        if(!CONFIG::write_byte(EP_STA_IP_MODE,bflag)) {
                            success = false;
                        }
                    }
                }
                //STAstatic_ip = 192.168.0.1
                if (espconfig.getValue("network", "STAstatic_ip", buffer, bufferLen)) {
                    if (!CONFIG::isIPValid(buffer)) {
                        success = false;
                    } else {
                        CONFIG::split_ip(buffer,ip_sav);
                        if(!CONFIG::write_buffer(EP_STA_IP_VALUE,ip_sav,IP_LENGTH)) {
                            success = false;
                        }
                    }
                }
                //STAstatic_mask = 255.255.255.0
                if (espconfig.getValue("network", "STAstatic_mask", buffer, bufferLen)) {
                    if (!CONFIG::isIPValid(buffer)) {
                        success = false;
                    } else {
                        CONFIG::split_ip(buffer,ip_sav);
                        if(!CONFIG::write_buffer(EP_STA_MASK_VALUE,ip_sav,IP_LENGTH)) {
                            success = false;
                        }
                    }
                }
                //STAstatic_gateway = 192.168.0.1
                if (espconfig.getValue("network", "STAstatic_gateway", buffer, bufferLen)) {
                    if (!CONFIG::isIPValid(buffer)) {
                        success = false;
                    } else {
                        CONFIG::split_ip(buffer,ip_sav);
                        if(!CONFIG::write_buffer(EP_STA_GATEWAY_VALUE,ip_sav,IP_LENGTH)) {
                            success = false;
                        }
                    }
                }
                //STANetwork_mode = 11g
                if (espconfig.getValue("network", "STANetwork_mode", buffer, bufferLen)) {
                    stmp = buffer;
                    localerror = false;
                    stmp.toLowerCase();
                    bflag = 0;
                    if (stmp =="11b") {
                        bflag = WIFI_PHY_MODE_11B;
                    } else  if (stmp =="11g") {
                        bflag = WIFI_PHY_MODE_11G;
                    } else  if (stmp =="11n") {
                        bflag = WIFI_PHY_MODE_11N;
                    } else {
                        localerror = true;
                        success = false;
                    }
                    if (!localerror) {
                        if(!CONFIG::write_byte(EP_STA_PHY_MODE,bflag)) {
                            success = false;
                        }
                    }
                }
                //APssid = NETGEAR_2GEXT_OFFICE2
                if (espconfig.getValue("network", "APssid", buffer, bufferLen)) {
                    if (!CONFIG::isSSIDValid(buffer)) {
                        success = false;
                    } else if(!CONFIG::write_string(EP_AP_SSID,buffer)) {
                        success = false;
                    }
                }
                //APpassword = 12345678
                if (espconfig.getValue("network", "APpassword", buffer, bufferLen)) {
                    if (!CONFIG::isPasswordValid(buffer)) {
                        success = false;
                    } else if(!CONFIG::write_string(EP_AP_PASSWORD,buffer)) {
                        success = false;
                    }
                }
                //APipmode = STATIC
                if (espconfig.getValue("network", "APipmode", buffer, bufferLen)) {
                    stmp = buffer;
                    localerror = false;
                    stmp.toLowerCase();
                    bflag = 0;
                    if (stmp =="dhcp") {
                        bflag = DHCP_MODE;
                    } else  if (stmp =="static") {
                        bflag = STATIC_IP_MODE;
                    } else {
                        localerror = true;
                        success = false;
                    }
                    if (!localerror) {
                        if(!CONFIG::write_byte(EP_AP_IP_MODE,bflag)) {
                            success = false;
                        }
                    }
                }
                //APstatic_ip = 192.168.0.1
                if (espconfig.getValue("network", "APstatic_ip", buffer, bufferLen)) {
                    if (!CONFIG::isIPValid(buffer)) {
                        success = false;
                    } else {
                        CONFIG::split_ip(buffer,ip_sav);
                        if(!CONFIG::write_buffer(EP_AP_IP_VALUE,ip_sav,IP_LENGTH)) {
                            success = false;
                        }
                    }
                }
                //APstatic_mask = 255.255.255.0
                if (espconfig.getValue("network", "APstatic_mask", buffer, bufferLen)) {
                    if (!CONFIG::isIPValid(buffer)) {
                        success = false;
                    } else {
                        CONFIG::split_ip(buffer,ip_sav);
                        if(!CONFIG::write_buffer(EP_AP_MASK_VALUE,ip_sav,IP_LENGTH)) {
                            success = false;
                        }
                    }
                }
                //APstatic_gateway = 192.168.0.1
                if (espconfig.getValue("network", "APstatic_gateway", buffer, bufferLen)) {
                    if (!CONFIG::isIPValid(buffer)) {
                        success = false;
                    } else {
                        CONFIG::split_ip(buffer,ip_sav);
                        if(!CONFIG::write_buffer(EP_AP_GATEWAY_VALUE,ip_sav,IP_LENGTH)) {
                            success = false;
                        }
                    }
                }
                //AP_channel = 1
                if (espconfig.getValue("network", "AP_channel", buffer, bufferLen, itmp)) {
                    bflag = itmp;
                    if(!CONFIG::write_byte(EP_CHANNEL,bflag)) {
                        success = false;
                    }
                }
                //AP_visible = yes
                if (espconfig.getValue("network", "AP_visible", buffer, bufferLen)) {
                    stmp = buffer;
                    localerror = false;
                    stmp.toLowerCase();
                    bflag = 0;
                    if (stmp =="yes") {
                        bflag = 1;
                    } else  if (stmp =="no") {
                        bflag = 0;
                    } else {
                        localerror = true;
                        success = false;
                    }
                    if (!localerror) {
                        if(!CONFIG::write_byte(EP_SSID_VISIBLE,bflag)) {
                            success = false;
                        }
                    }
                }
                //AP_auth = WPA
                if (espconfig.getValue("network", "AP_auth", buffer, bufferLen)) {
                    stmp = buffer;
                    localerror = false;
                    stmp.toLowerCase();
                    bflag = 0;
                    if (stmp =="open") {
                        bflag = AUTH_OPEN;
                    } else  if (stmp =="wpa") {
                        bflag = AUTH_WPA_PSK;
                    } else  if (stmp =="wpa2") {
                        bflag = AUTH_WPA2_PSK;
                    } else  if (stmp =="wpa_wpa2") {
                        bflag = AUTH_WPA_WPA2_PSK;
                    } else {
                        localerror = true;
                        success = false;
                    }
                    if (!localerror) {
                        if(!CONFIG::write_byte(EP_AUTH_TYPE,bflag)) {
                            success = false;
                        }
                    }
                }
                //APNetwork_mode = 11g
                if (espconfig.getValue("network", "APNetwork_mode", buffer, bufferLen)) {
                    stmp = buffer;
                    localerror = false;
                    stmp.toLowerCase();
                    bflag = 0;
                    if (stmp =="11b") {
                        bflag = WIFI_PHY_MODE_11B;
                    } else  if (stmp =="11g") {
                        bflag = WIFI_PHY_MODE_11G;
                    } else {
                        localerror = true;
                        success = false;
                    }
                    if (!localerror) {
                        if(!CONFIG::write_byte(EP_AP_PHY_MODE,bflag)) {
                            success = false;
                        }
                    }
                }
                
                //Timezone
                if (espconfig.getValue("network", "time_zone", buffer, bufferLen, itmp)) {
                    bflag = itmp;
                    if(!CONFIG::write_byte(EP_TIMEZONE,bflag)) {
                        success = false;
                    }
                }
                
                //day saving time
                if (espconfig.getValue("network", "day_st", buffer, bufferLen)) {
                    stmp = buffer;
                    localerror = false;
                    stmp.toLowerCase();
                    bflag = 0;
                    if (stmp =="yes") {
                        bflag = 1;
                    } else  if (stmp =="no") {
                        bflag = 0;
                    } else {
                        localerror = true;
                        success = false;
                    }
                    if (!localerror) {
                        if(!CONFIG::write_byte(EP_TIME_ISDST,bflag)) {
                            success = false;
                        }
                    }
                }
                //time server 1
                if (espconfig.getValue("network", "time_server_1", buffer, bufferLen)) {
                    if (strlen(buffer) > 128) {
                        success = false;
                    } else if(!CONFIG::write_string(EP_TIME_SERVER1,buffer)) {
                        success = false;
                    }
                }
                
                //time server 2
                if (espconfig.getValue("network", "time_server_2", buffer, bufferLen)) {
                    if (strlen(buffer) > 128) {
                        success = false;
                    } else if(!CONFIG::write_string(EP_TIME_SERVER2,buffer)) {
                        success = false;
                    }
                }
                
                //time server 3
                if (espconfig.getValue("network", "time_server_3", buffer, bufferLen)) {
                    if (strlen(buffer) > 128) {
                        success = false;
                    } else if(!CONFIG::write_string(EP_TIME_SERVER3,buffer)) {
                        success = false;
                    }
                }
                
                //Section [printer]
                //data = 192.168.1.1/video.cgi?user=luc&pwd=mypassword
                if (espconfig.getValue("printer", "data", buffer, bufferLen)) {
                    if (strlen(buffer) > 128) {
                        success = false;
                    } else if(!CONFIG::write_string(EP_DATA_STRING,buffer)) {
                        success = false;
                    }
                }
                //refreshrate = 3
                if (espconfig.getValue("printer", "refreshrate", buffer, bufferLen, itmp)) {
                    bflag = itmp;
                    if(!CONFIG::write_byte(EP_REFRESH_PAGE_TIME,bflag)) {
                        success = false;
                    }
                }
                 //refreshrate2 = 3
                if (espconfig.getValue("printer", "refreshrate2", buffer, bufferLen, itmp)) {
                    bflag = itmp;
                    if(!CONFIG::write_byte(EP_REFRESH_PAGE_TIME2,bflag)) {
                        success = false;
                    }
                }
                //XY_feedrate = 1000
                if (espconfig.getValue("printer", "XY_feedrate", buffer, bufferLen, itmp)) {
                    if(!CONFIG::write_buffer(EP_XY_FEEDRATE,(const byte *)&itmp,INTEGER_LENGTH)) {
                        success = false;
                    }
                }
                //Z_feedrate = 100
                if (espconfig.getValue("printer", "Z_feedrate", buffer, bufferLen, itmp)) {
                    if(!CONFIG::write_buffer(EP_Z_FEEDRATE,(const byte *)&itmp,INTEGER_LENGTH)) {
                        success = false;
                    }
                }
                //E_feedrate = 400
                if (espconfig.getValue("printer", "E_feedrate", buffer, bufferLen, itmp)) {
                    if(!CONFIG::write_buffer(EP_E_FEEDRATE,(const byte *)&itmp,INTEGER_LENGTH)) {
                        success = false;
                    }
                }
                
                 //target_printer= smoothieware
                if (espconfig.getValue("printer", "target_printer", buffer, bufferLen)) {
                    stmp = buffer;
                    bflag = UNKNOWN_FW;
                    if ( stmp.equalsIgnoreCase(String(F("repetier"))))bflag = REPETIER;
                    else  if ( stmp.equalsIgnoreCase(String(F("repetier4davinci"))))bflag = REPETIER4DV;
                    else  if ( stmp.equalsIgnoreCase(String(F("marlin"))))bflag = MARLIN;
                    else  if ( stmp.equalsIgnoreCase(String(F("marlinkimbra"))))bflag = MARLINKIMBRA;
                    else  if ( stmp.equalsIgnoreCase(String(F("smoothieware"))))bflag = SMOOTHIEWARE;
                    else  success = false;
                    if(!(success && CONFIG::write_byte(EP_TARGET_FW,bflag))) {
                        success = false;
                    }
                }
                
                //direct sd connection
                if (espconfig.getValue("printer", "direct_sd", buffer, bufferLen)) {
                    stmp = buffer;
                    localerror = false;
                    stmp.toLowerCase();
                    bflag = 0;
                    if (stmp =="yes") {
                        bflag = 1;
                    } else  if (stmp =="no") {
                        bflag = 0;
                    } else {
                        localerror = true;
                        success = false;
                    }
                    if (!localerror) {
                        if(!CONFIG::write_byte(EP_IS_DIRECT_SD,bflag)) {
                            success = false;
                        }
                    }
                }
                 //sd check update at boot
                if (espconfig.getValue("printer", "sd_check_update", buffer, bufferLen)) {
                    stmp = buffer;
                    localerror = false;
                    stmp.toLowerCase();
                    bflag = 0;
                    if (stmp =="yes") {
                        bflag = 1;
                    } else  if (stmp =="no") {
                        bflag = 0;
                    } else {
                        localerror = true;
                        success = false;
                    }
                    if (!localerror) {
                        if(!CONFIG::write_byte(EP_SD_CHECK_UPDATE_AT_BOOT,bflag)) {
                            success = false;
                        }
                    }
                }
                 //direct sd boot check
                if (espconfig.getValue("printer", "direct_sd_boot_check", buffer, bufferLen)) {
                    stmp = buffer;
                    localerror = false;
                    stmp.toLowerCase();
                    bflag = 0;
                    if (stmp =="yes") {
                        bflag = 1;
                    } else  if (stmp =="no") {
                        bflag = 0;
                    } else {
                        localerror = true;
                        success = false;
                    }
                    if (!localerror) {
                        if(!CONFIG::write_byte(EP_DIRECT_SD_CHECK,bflag)) {
                            success = false;
                        }
                    }
                }
                
                //primary sd directory
                if (espconfig.getValue("printer", "primary_sd ", buffer, bufferLen)) {
                    stmp = buffer;
                    localerror = false;
                    stmp.toLowerCase();
                    bflag = 0;
                    if (stmp =="none") {
                        bflag = 0;
                    } else  if (stmp =="sd") {
                        bflag = 1;
                    } else  if (stmp =="ext") {
                        bflag = 2;                        
                    } else {
                        localerror = true;
                        success = false;
                    }
                    if (!localerror) {
                        if(!CONFIG::write_byte(EP_PRIMARY_SD,bflag)) {
                            success = false;
                        }
                    }
                }
            
                 //secondary sd directory
                if (espconfig.getValue("printer", "secondary_sd  ", buffer, bufferLen)) {
                    stmp = buffer;
                    localerror = false;
                    stmp.toLowerCase();
                    bflag = 0;
                    if (stmp =="none") {
                        bflag = 0;
                    } else  if (stmp =="sd") {
                        bflag = 1;
                    } else  if (stmp =="ext") {
                        bflag = 2;                      
                    } else {
                        localerror = true;
                        success = false;
                    }
                    if (!localerror) {
                        if(!CONFIG::write_byte(EP_SECONDARY_SD,bflag)) {
                            success = false;
                        }
                    }
                }
            
            }
            espconfig.close();
            if(success) {
                newfilename.replace(String(".txt"), String(".ok"));
            } else {
                newfilename.replace(String(".txt"), String(".bad"));
                answer = false;
            }
            //rename if name is already used
            if (!CONFIG::renameFile (newfilename)) {
                LOG("Failed to rename previous file\r\n")
            }
            if (SD.rename((const char *)filename.c_str(),(const char *)newfilename.c_str())) {
                LOG("File renamed, restarted file\r\n")
            } else {
                 return false;
                LOG("Failed to rename file\r\n")
            }
            } else {
                 return false;
                LOG("Failed to open config file\r\n")
            }
        } else {
            return false;
            LOG("No config file\r\n")
        }
    return answer;
}
#endif
void CONFIG::esp_restart()
{
    LOG("Restarting\r\n")
    ESP_SERIAL_OUT.flush();
    delay(500);
#ifdef ARDUINO_ARCH_ESP8266
    ESP_SERIAL_OUT.swap();
#endif
    delay(100);
    ESP.restart();
    while (1) {
        delay(1);
    };
}

void  CONFIG::InitPins(){
#ifdef RECOVERY_FEATURE
    pinMode(RESET_CONFIG_PIN, INPUT);
#endif
}

bool CONFIG::is_direct_sd = false;

bool CONFIG::isHostnameValid(const char * hostname)
{
    //limited size
    char c;
    if (strlen(hostname)>MAX_HOSTNAME_LENGTH || strlen(hostname) < MIN_HOSTNAME_LENGTH) {
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
        if (!isPrintable(ssid[i]))return false;
        //if (!(isdigit(c) || isalpha(c))) return false;
        //if (c==' ') {
        //     return false;
        //}
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

String CONFIG::formatBytes(uint32_t bytes)
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
        LOG("Error read string\r\n")
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
        LOG("Error read string\r\n")
        return false;
    }
    byte b = 13; // non zero for the while loop below
    int i=0;
    sbuffer="";

    EEPROM.begin(EEPROM_SIZE);
    //read until max size is reached or \0 is found
    while (i < size_max && b != 0) {
        b = EEPROM.read(pos+i);
        if (b!=0) {
            sbuffer+=char(b);
        }
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
        LOG("Error read buffer\r\n")
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
        LOG("Error read byte\r\n")
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

bool CONFIG::check_update_presence( ){ 
     bool result = false;
     if (CONFIG::is_direct_sd) { 
         long baud_rate=0;
         if (!CONFIG::read_buffer(EP_BAUD_RATE,  (byte *)&baud_rate, INTEGER_LENGTH)) return false;
         if (ESP_SERIAL_OUT.baudRate() != baud_rate)ESP_SERIAL_OUT.begin(baud_rate);
         CONFIG::InitFirmwareTarget();
         delay(500);
         String cmd = "M20";
         //By default M20 should be applied
         //if (CONFIG::FirmwareTarget == UNKNOWN_FW) return false;
         if (CONFIG::FirmwareTarget == SMOOTHIEWARE) {
             byte sd_dir = 0;
             if (!CONFIG::read_byte(EP_PRIMARY_SD, &sd_dir )) sd_dir = DEFAULT_PRIMARY_SD;
             if (sd_dir == SD_DIRECTORY) cmd = "ls /sd";
             else if (sd_dir == EXT_DIRECTORY) cmd = "ls /ext";
             else return false;
         }
        String tmp;
        int count ;
        //send command to serial as no need to transfer ESP command
        //to avoid any pollution if Uploading file to SDCard
        //block every query
        //empty the serial buffer and incoming data
        if(ESP_SERIAL_OUT.available()) {
            BRIDGE::processFromSerial2TCP();
            delay(1);
        }
        //Send command
        ESP_SERIAL_OUT.println(cmd);
        count = 0;
        String current_buffer;
        String current_line;
        int pos;
        int temp_counter = 0;
       
        //pickup the list
        while (count < MAX_TRY) {
            //give some time between each buffer
            if (ESP_SERIAL_OUT.available()) {
                count = 0;
                size_t len = ESP_SERIAL_OUT.available();
                uint8_t sbuf[len+1];
                //read buffer
                ESP_SERIAL_OUT.readBytes(sbuf, len);
                //change buffer as string
                sbuf[len]='\0';
                //add buffer to current one if any
                current_buffer += (char * ) sbuf;
                while (current_buffer.indexOf("\n") !=-1) {
                    //remove the possible "\r"
                    current_buffer.replace("\r","");
                    pos = current_buffer.indexOf("\n");
                    //get line
                    current_line = current_buffer.substring(0,current_buffer.indexOf("\n"));
                    //if line is command ack - just exit so save the time out period
                    if ((current_line == "ok") || (current_line == "wait")) {
                        count = MAX_TRY;
                        break;
                    }
                    //check line
                    //save time no need to continue
                    if (current_line.indexOf("busy:") > -1 || current_line.indexOf("T:") > -1 || current_line.indexOf("B:") > -1) {
                        temp_counter++;
                    } else {
                    
                    }
                    if (temp_counter > 5) {
                        break;
                    }
                    //current remove line from buffer
                    tmp = current_buffer.substring(current_buffer.indexOf("\n")+1,current_buffer.length());
                    current_buffer = tmp;
                    delay(0);
                }
                delay (0);
            } else {
                delay(1);
            }
            //it is sending too many temp status should be heating so let's exit the loop
            if (temp_counter > 5) {
                count = MAX_TRY;
            }
            count++;
        }
        if(ESP_SERIAL_OUT.available()) {
            BRIDGE::processFromSerial2TCP();
            delay(1);
        }
    }
    return result;
}


//write a string (array of byte with a 0x00  at the end)
bool CONFIG::write_string(int pos, const char * byte_buffer)
{
    int size_buffer;
    int maxsize = EEPROM_SIZE;
    size_buffer= strlen(byte_buffer);
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
    case EP_DATA_STRING:
        maxsize = MAX_DATA_LENGTH;
        break;
    default:
        maxsize = EEPROM_SIZE;
        break;
    }
    if ((size_buffer==0 && !(pos == EP_DATA_STRING)) ||  pos+size_buffer+1 > EEPROM_SIZE || size_buffer > maxsize  || byte_buffer== NULL) {
        LOG("Error write string\r\n")
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
        LOG("Error write buffer\r\n")
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
        LOG("Error write byte\r\n")
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
    if(!CONFIG::write_string(EP_DATA_STRING,"")) {
        return false;
    }
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
    if(!CONFIG::write_byte(EP_REFRESH_PAGE_TIME2,DEFAULT_REFRESH_PAGE_TIME)) {
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
    
    if(!CONFIG::write_byte(EP_TARGET_FW, UNKNOWN_FW)) {
        return false;
    }
    
    if(!CONFIG::write_byte(EP_TIMEZONE, DEFAULT_TIME_ZONE)) {
        return false;
    }
    
    if(!CONFIG::write_byte(EP_TIME_ISDST, DEFAULT_TIME_DST)) {
        return false;
    }
    
     if(!CONFIG::write_byte(EP_PRIMARY_SD, DEFAULT_PRIMARY_SD)) {
        return false;
    }
    
     if(!CONFIG::write_byte(EP_SECONDARY_SD, DEFAULT_SECONDARY_SD)) {
        return false;
    }
    
    if(!CONFIG::write_byte(EP_IS_DIRECT_SD, DEFAULT_IS_DIRECT_SD)) {
        return false;
    }
    
     if(!CONFIG::write_byte(EP_DIRECT_SD_CHECK, DEFAULT_DIRECT_SD_CHECK)) {
        return false;
    }
    
    if(!CONFIG::write_byte(EP_SD_CHECK_UPDATE_AT_BOOT, DEFAULT_SD_CHECK_UPDATE_AT_BOOT)) {
        return false;
    }

    if(!CONFIG::write_string(EP_TIME_SERVER1,FPSTR(DEFAULT_TIME_SERVER1))) {
        return false;
    }
    
    if(!CONFIG::write_string(EP_TIME_SERVER2,FPSTR(DEFAULT_TIME_SERVER2))) {
        return false;
    }
    
    if(!CONFIG::write_string(EP_TIME_SERVER3,FPSTR(DEFAULT_TIME_SERVER3))) {
        return false;
    }
    return true;
}

void CONFIG::print_config(tpipe output, bool plaintext)
{    
    if (!plaintext)BRIDGE::print(F("{\"chip_id\":\""), output);
    else BRIDGE::print(F("Chip ID: "), output);
#ifdef ARDUINO_ARCH_ESP8266
    BRIDGE::print(String(ESP.getChipId()).c_str(), output);
#else
	BRIDGE::print(String((uint16_t)(ESP.getEfuseMac()>>32)).c_str(), output);
#endif
    if (!plaintext)BRIDGE::print(F("\","), output);
    else BRIDGE::print(F("\n"), output);
    
    if (!plaintext)BRIDGE::print(F("\"cpu\":\""), output);
    else BRIDGE::print(F("CPU Frequency: "), output);
    BRIDGE::print(String(ESP.getCpuFreqMHz()).c_str(), output);
    if (plaintext)BRIDGE::print(F("Mhz"), output);
    if (!plaintext)BRIDGE::print(F("\","), output);
    else BRIDGE::print(F("\n"), output);
#ifdef ARDUINO_ARCH_ESP32 
    if (!plaintext)BRIDGE::print(F("\"cpu_temp\":\""), output);
    else BRIDGE::print(F("CPU Temperature: "), output);
    BRIDGE::print(String(temperatureRead(),1).c_str(), output);
    if (plaintext)BRIDGE::print(F("C"), output);
    if (!plaintext)BRIDGE::print(F("\","), output);
    else BRIDGE::print(F("\n"), output);
#endif  
    if (!plaintext)BRIDGE::print(F("\"freemem\":\""), output);
    else BRIDGE::print(F("Free memory: "), output);
    BRIDGE::print(formatBytes(ESP.getFreeHeap()).c_str(), output);
    if (!plaintext)BRIDGE::print(F("\","), output);
    else BRIDGE::print(F("\n"), output);
    
    if (!plaintext)BRIDGE::print(F("\""), output);
    BRIDGE::print(F("SDK"), output);
    if (!plaintext)BRIDGE::print(F("\":\""), output);
    else BRIDGE::print(F(": "), output);
    BRIDGE::print(ESP.getSdkVersion(), output);
    if (!plaintext)BRIDGE::print(F("\","), output);
    else BRIDGE::print(F("\n"), output);
      
    if (!plaintext)BRIDGE::print(F("\"flash_size\":\""), output);
    else BRIDGE::print(F("Flash Size: "), output);
    BRIDGE::print(formatBytes(ESP.getFlashChipSize()).c_str(), output);
    if (!plaintext)BRIDGE::print(F("\","), output);
    else BRIDGE::print(F("\n"), output);
#ifdef ARDUINO_ARCH_ESP8266    
    if (!plaintext)BRIDGE::print(F("\"update_size\":\""), output);
    else BRIDGE::print(F("Available Size for update: "), output);
    uint32_t  flashsize = ESP.getFlashChipSize();
    if (flashsize > 1024 * 1024) flashsize = 1024 * 1024;
    BRIDGE::print(formatBytes(flashsize - ESP.getSketchSize()).c_str(), output);
    if (!plaintext)BRIDGE::print(F("\","), output);
    else {
        if ((flashsize - ESP.getSketchSize()) > (flashsize / 2)) BRIDGE::println(F("(Ok)"), output);
        else BRIDGE::print(F("(Not enough)"), output);
        }

    if (!plaintext)BRIDGE::print(F("\"spiffs_size\":\""), output);
    else BRIDGE::print(F("Available Size for SPIFFS: "), output);
    fs::FSInfo info;
    SPIFFS.info(info);
    BRIDGE::print(formatBytes(info.totalBytes).c_str(), output);
    if (!plaintext)BRIDGE::print(F("\","), output);
    else BRIDGE::print(F("\n"), output);
#else 
	if (!plaintext)BRIDGE::print(F("\"update_size\":\""), output);
    else BRIDGE::print(F("Available Size for update: "), output);
    uint32_t  flashsize = ESP.getFlashChipSize();
//Not OTA on 2Mb board per spec
    if (flashsize > 0x20000) flashsize = 0x140000;
    else flashsize = 0x0;
    BRIDGE::print(formatBytes(flashsize).c_str(), output);
    if (!plaintext)BRIDGE::print(F("\","), output);
    else {
        if (flashsize  > 0x0) BRIDGE::println(F("(Ok)"), output);
        else BRIDGE::print(F("(Not enough)"), output);
        }
    if (!plaintext)BRIDGE::print(F("\"spiffs_size\":\""), output);
    else BRIDGE::print(F("Available Size for SPIFFS: "), output);
    BRIDGE::print(formatBytes(SPIFFS.totalBytes()).c_str(), output);
    if (!plaintext)BRIDGE::print(F("\","), output);
    else BRIDGE::print(F("\n"), output);
#endif
    if (!plaintext)BRIDGE::print(F("\"baud_rate\":\""), output);
    else BRIDGE::print(F("Baud rate: "), output);
    uint32_t br = ESP_SERIAL_OUT.baudRate();
#ifdef ARDUINO_ARCH_ESP32
    //workaround for ESP32
    if(br == 115201)br = 115200;
    if(br == 230423)br = 230400;
#endif
    BRIDGE::print(String(br).c_str(), output);
    if (!plaintext)BRIDGE::print(F("\","), output);
    else BRIDGE::print(F("\n"), output);

    if (!plaintext)BRIDGE::print(F("\"sleep_mode\":\""), output);
    else BRIDGE::print(F("Sleep mode: "), output);
#ifdef ARDUINO_ARCH_ESP32
    wifi_ps_type_t ps_type;
    esp_wifi_get_ps(&ps_type);
#else
    WiFiSleepType_t ps_type;
    ps_type = WiFi.getSleepMode();
#endif
    if (ps_type == WIFI_NONE_SLEEP) {
        BRIDGE::print(F("None"), output);
#ifdef ARDUINO_ARCH_ESP8266
    } else if (ps_type == WIFI_LIGHT_SLEEP) {
        BRIDGE::print(F("Light"), output);
#endif
    } else if (ps_type == WIFI_MODEM_SLEEP) {
        BRIDGE::print(F("Modem"), output);
    } else {
        BRIDGE::print(F("???"), output);
    }
    if (!plaintext)BRIDGE::print(F("\","), output);
    else BRIDGE::print(F("\n"), output);
    
    if (!plaintext)BRIDGE::print(F("\"channel\":\""), output);
    else BRIDGE::print(F("Channel: "), output);
    BRIDGE::print(String(WiFi.channel()).c_str(), output);
    if (!plaintext)BRIDGE::print(F("\","), output);
    else BRIDGE::print(F("\n"), output);
#ifdef ARDUINO_ARCH_ESP32
    uint8_t PhyMode;
    if (WiFi.getMode() == WIFI_STA)esp_wifi_get_protocol(ESP_IF_WIFI_STA, &PhyMode);
    else esp_wifi_get_protocol(ESP_IF_WIFI_AP, &PhyMode);
#else   
    WiFiPhyMode_t PhyMode = WiFi.getPhyMode();
#endif
    if (!plaintext)BRIDGE::print(F("\"phy_mode\":\""), output);
    else BRIDGE::print(F("Phy Mode: "), output);
    if (PhyMode == WIFI_PHY_MODE_11G )BRIDGE::print(F("11g"), output);
    else if (PhyMode == WIFI_PHY_MODE_11B )BRIDGE::print(F("11b"), output);
    else if (PhyMode == WIFI_PHY_MODE_11N )BRIDGE::print(F("11n"), output);
    else BRIDGE::print(F("???"), output);
    if (!plaintext)BRIDGE::print(F("\","), output);
    else BRIDGE::print(F("\n"), output);
    
    if (!plaintext)BRIDGE::print(F("\"web_port\":\""), output);
    else BRIDGE::print(F("Web port: "), output);
    BRIDGE::print(String(wifi_config.iweb_port).c_str(), output);
    if (!plaintext)BRIDGE::print(F("\","), output);
    else BRIDGE::print(F("\n"), output);
    
    if (!plaintext)BRIDGE::print(F("\"data_port\":\""), output);
    else BRIDGE::print(F("Data port: "), output);
#ifdef TCP_IP_DATA_FEATURE
    BRIDGE::print(String(wifi_config.idata_port).c_str(), output);
#else
    BRIDGE::print(F("Disabled"), output);
#endif
    if (!plaintext)BRIDGE::print(F("\","), output);
    else BRIDGE::print(F("\n"), output);
    
    if (WiFi.getMode() == WIFI_STA || WiFi.getMode() == WIFI_AP_STA) {
        if (!plaintext)BRIDGE::print(F("\"hostname\":\""), output);
        else BRIDGE::print(F("Hostname: "), output);
#ifdef ARDUINO_ARCH_ESP32
        BRIDGE::print(WiFi.getHostname(), output);
#else
        BRIDGE::print(WiFi.hostname().c_str(), output);
#endif
        if (!plaintext)BRIDGE::print(F("\","), output);
        else BRIDGE::print(F("\n"), output);
    }

    if (!plaintext)BRIDGE::print(F("\"active_mode\":\""), output);
    else BRIDGE::print(F("Active Mode: "), output);
    if (WiFi.getMode() == WIFI_STA) {
        BRIDGE::print(F("Station ("), output);
        BRIDGE::print(WiFi.macAddress().c_str(), output);
        BRIDGE::print(F(")"), output);
        if (!plaintext)BRIDGE::print(F("\","), output);
        else BRIDGE::print(F("\n"), output);
        if (WiFi.isConnected()) {
             if (!plaintext)BRIDGE::print(F("\"connected_ssid\":\""), output);
            else BRIDGE::print(F("Connected to: "), output);
            BRIDGE::print(WiFi.SSID().c_str(), output);
            if (!plaintext)BRIDGE::print(F("\","), output);
            else BRIDGE::print(F("\n"), output);
            if (!plaintext)BRIDGE::print(F("\"connected_signal\":\""), output);
            else BRIDGE::print(F("Signal: "), output);
            BRIDGE::print(String(wifi_config.getSignal(WiFi.RSSI())).c_str(), output);
            BRIDGE::print(F("%"), output);
            if (!plaintext)BRIDGE::print(F("\","), output);
            else BRIDGE::print(F("\n"), output);
            }
        else {
             if (!plaintext)BRIDGE::print(F("\"connection_status\":\""), output);
            else BRIDGE::print(F("Connection Status: "), output);
            BRIDGE::print(F("Connection Status: "), output);
            if (WiFi.status() == WL_DISCONNECTED) {
                BRIDGE::print(F("Disconnected"), output);
            } else if (WiFi.status() == WL_CONNECTION_LOST) {
                BRIDGE::print(F("Connection lost"), output);
            } else if (WiFi.status() == WL_CONNECT_FAILED) {
                BRIDGE::print(F("Connection failed"), output);
            } else if (WiFi.status() == WL_NO_SSID_AVAIL) {
                BRIDGE::print(F("No connection"), output);
            } else if (WiFi.status() == WL_IDLE_STATUS   ) {
                BRIDGE::print(F("Idle"), output);
            } else  BRIDGE::print(F("Unknown"), output);
            if (!plaintext)BRIDGE::print(F("\","), output);
            else BRIDGE::print(F("\n"), output);
        }
         if (!plaintext)BRIDGE::print(F("\"ip_mode\":\""), output);
        else BRIDGE::print(F("IP Mode: "), output);
#ifdef ARDUINO_ARCH_ESP32
        tcpip_adapter_dhcp_status_t dhcp_status;
        tcpip_adapter_dhcpc_get_status(TCPIP_ADAPTER_IF_STA, &dhcp_status);
        if (dhcp_status == TCPIP_ADAPTER_DHCP_STARTED)
#else
        if (wifi_station_dhcpc_status()==DHCP_STARTED) 
#endif
        {
			BRIDGE::print(F("DHCP"), output);
		}
        else BRIDGE::print(F("Static"), output);
         if (!plaintext)BRIDGE::print(F("\","), output);
        else BRIDGE::print(F("\n"), output);
        
        if (!plaintext)BRIDGE::print(F("\"ip\":\""), output);
        else BRIDGE::print(F("IP: "), output);
        BRIDGE::print(WiFi.localIP().toString().c_str(), output);
        if (!plaintext)BRIDGE::print(F("\","), output);
        else BRIDGE::print(F("\n"), output);
        
        if (!plaintext)BRIDGE::print(F("\"gw\":\""), output);
        else BRIDGE::print(F("Gateway: "), output);
        BRIDGE::print(WiFi.gatewayIP().toString().c_str(), output);
        if (!plaintext)BRIDGE::print(F("\","), output);
        else BRIDGE::print(F("\n"), output);
        
        if (!plaintext)BRIDGE::print(F("\"msk\":\""), output);
        else BRIDGE::print(F("Mask: "), output);
        BRIDGE::print(WiFi.subnetMask().toString().c_str(), output);
        if (!plaintext)BRIDGE::print(F("\","), output);
        else BRIDGE::print(F("\n"), output);
        
        if (!plaintext)BRIDGE::print(F("\"dns\":\""), output);
        else BRIDGE::print(F("DNS: "), output);
        BRIDGE::print(WiFi.dnsIP().toString().c_str(), output);
        if (!plaintext)BRIDGE::print(F("\","), output);
        else BRIDGE::print(F("\n"), output);
        
        if (!plaintext)BRIDGE::print(F("\"disabled_mode\":\""), output);
        else BRIDGE::print(F("Disabled Mode: "), output);
        BRIDGE::print(F("Access Point ("), output);
        BRIDGE::print(WiFi.softAPmacAddress().c_str(), output);
        BRIDGE::print(F(")"), output);
        if (!plaintext)BRIDGE::print(F("\","), output);
        else BRIDGE::print(F("\n"), output);
        
    } else if (WiFi.getMode() == WIFI_AP) {
        BRIDGE::print(F("Access Point ("), output);
        BRIDGE::print(WiFi.softAPmacAddress().c_str(), output);
        BRIDGE::print(F(")"), output);
        if (!plaintext)BRIDGE::print(F("\","), output);
        else BRIDGE::print(F("\n"), output);
        
        //get current config
#ifdef ARDUINO_ARCH_ESP32
        wifi_ap_config_t apconfig;
        wifi_config_t conf;
        esp_wifi_get_config(ESP_IF_WIFI_AP, &conf);
        apconfig.ssid_hidden = conf.ap.ssid_hidden;
        apconfig.authmode = conf.ap.authmode;
        apconfig.max_connection = conf.ap.max_connection;
#else
        struct softap_config apconfig;
        wifi_softap_get_config(&apconfig);
#endif
        if (!plaintext)BRIDGE::print(F("\"ap_ssid\":\""), output);
        else BRIDGE::print(F("SSID: "), output);
#ifdef ARDUINO_ARCH_ESP32
        BRIDGE::print((const char*)conf.ap.ssid, output);
#else
		BRIDGE::print((const char*)apconfig.ssid, output);
#endif
        if (!plaintext)BRIDGE::print(F("\","), output);
        else BRIDGE::print(F("\n"), output);
        
        if (!plaintext)BRIDGE::print(F("\"ssid_visible\":\""), output);
        else BRIDGE::print(F("Visible: "), output);
        BRIDGE::print((apconfig.ssid_hidden == 0)?F("Yes"):F("No"), output);
        if (!plaintext)BRIDGE::print(F("\","), output);
        else BRIDGE::print(F("\n"), output);
        
        if (!plaintext)BRIDGE::print(F("\"ssid_authentication\":\""), output);
        else BRIDGE::print(F("Authentication: "), output);
        if (apconfig.authmode==AUTH_OPEN) {
            BRIDGE::print(F("None"), output);
        } else if (apconfig.authmode==AUTH_WEP) {
            BRIDGE::print(F("WEP"), output);
        } else if (apconfig.authmode==AUTH_WPA_PSK) {
           BRIDGE::print(F("WPA"), output);
        } else if (apconfig.authmode==AUTH_WPA2_PSK) {
            BRIDGE::print(F("WPA2"), output);
        } else {
            BRIDGE::print(F("WPA/WPA2"), output);
        }
        if (!plaintext)BRIDGE::print(F("\","), output);
        else BRIDGE::print(F("\n"), output);
        
        if (!plaintext)BRIDGE::print(F("\"ssid_max_connections\":\""), output);
        else BRIDGE::print(F("Max Connections: "), output);
        BRIDGE::print(String(apconfig.max_connection).c_str(), output);
        if (!plaintext)BRIDGE::print(F("\","), output);
        else BRIDGE::print(F("\n"), output);
        
        if (!plaintext)BRIDGE::print(F("\"ssid_dhcp\":\""), output);
        else BRIDGE::print(F("DHCP Server: "), output);
#ifdef ARDUINO_ARCH_ESP32
        tcpip_adapter_dhcp_status_t dhcp_status;
        tcpip_adapter_dhcps_get_status(TCPIP_ADAPTER_IF_AP, &dhcp_status);
        if (dhcp_status == TCPIP_ADAPTER_DHCP_STARTED)
#else
        if(wifi_softap_dhcps_status() == DHCP_STARTED) 
#endif
        {
			BRIDGE::print(F("Started"), output);
		}
        else BRIDGE::print(F("Stopped"), output);
        if (!plaintext)BRIDGE::print(F("\","), output);
        else BRIDGE::print(F("\n"), output);
        
        if (!plaintext)BRIDGE::print(F("\"ip\":\""), output);
        else BRIDGE::print(F("IP: "), output);
        BRIDGE::print(WiFi.softAPIP().toString().c_str(), output);
        if (!plaintext)BRIDGE::print(F("\","), output);
        else BRIDGE::print(F("\n"), output);
#ifdef ARDUINO_ARCH_ESP32      
        tcpip_adapter_ip_info_t ip;
        tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_AP, &ip);
#else    
        struct ip_info ip;
        wifi_get_ip_info(SOFTAP_IF, &ip);
#endif
        if (!plaintext)BRIDGE::print(F("\"gw\":\""), output);
        else BRIDGE::print(F("Gateway: "), output);
        BRIDGE::print(IPAddress(ip.gw.addr).toString().c_str(), output);
        if (!plaintext)BRIDGE::print(F("\","), output);
        else BRIDGE::print(F("\n"), output);
        
        if (!plaintext)BRIDGE::print(F("\"msk\":\""), output);
        else BRIDGE::print(F("Mask: "), output);
        BRIDGE::print(IPAddress(ip.netmask.addr).toString().c_str(), output);
        if (!plaintext)BRIDGE::print(F("\","), output);
        else BRIDGE::print(F("\n"), output);
        
        
        if (!plaintext)BRIDGE::print(F("\"connected_clients\":["), output);
        else BRIDGE::print(F("Connected clients: "), output);
        int client_counter=0;
#ifdef ARDUINO_ARCH_ESP32     
        wifi_sta_list_t station;
        tcpip_adapter_sta_list_t tcpip_sta_list;
        esp_wifi_ap_get_sta_list(&station);
        tcpip_adapter_get_sta_list(&station, &tcpip_sta_list);
#else       
        struct station_info * station;
        station = wifi_softap_get_station_info();
#endif
        String stmp ="";
#ifdef ARDUINO_ARCH_ESP32
		for (int i=0; i < station.num; i++){
#else
        while(station) {
#endif
            if(stmp.length() > 0) {
                if (!plaintext)stmp+=F(",");
                else stmp+=F("\n");
                
            }
            if (!plaintext)stmp+=F("{\"bssid\":\"");
            //BSSID
#ifdef ARDUINO_ARCH_ESP32
            stmp += CONFIG::mac2str(tcpip_sta_list.sta[i].mac);
#else
			stmp += CONFIG::mac2str(station->bssid);
#endif
            if (!plaintext)stmp+=F("\",\"ip\":\"");
            else stmp += F(" ");
            //IP
#ifdef ARDUINO_ARCH_ESP32
            stmp += IPAddress(tcpip_sta_list.sta[i].ip.addr).toString().c_str();
#else
			stmp += IPAddress((const uint8_t *)&station->ip).toString().c_str();
#endif
            if (!plaintext)stmp+=F("\"}");
            //increment counter
            client_counter++;
#ifdef ARDUINO_ARCH_ESP32
		}
#else
            //go next record
            station =STAILQ_NEXT(station,	next);
        }
        wifi_softap_free_station_info();
#endif
        if (!plaintext) {
            BRIDGE::print(stmp.c_str(), output);
            BRIDGE::print(F("],"), output);
            }
        else {
                //display number of client
                BRIDGE::println(String(client_counter).c_str(), output);
                //display list if any
                if(stmp.length() > 0)BRIDGE::println(stmp.c_str(), output);
            }
        
        if (!plaintext)BRIDGE::print(F("\"disabled_mode\":\""), output);
        else BRIDGE::print(F("Disabled Mode: "), output);
        BRIDGE::print(F("Station ("), output);
        BRIDGE::print(WiFi.macAddress().c_str(), output);
        BRIDGE::print(F(") is disabled"), output);
        if (!plaintext)BRIDGE::print(F("\","), output);
        else BRIDGE::print(F("\n"), output);
        
    } else if (WiFi.getMode() == WIFI_AP_STA) {
        BRIDGE::print(F("Mixed"), output);
        if (!plaintext)BRIDGE::print(F("\","), output);
        else BRIDGE::print(F("\n"), output);
        if (!plaintext)BRIDGE::print(F("\"active_mode\":\""), output);
        else BRIDGE::print(F("Active Mode: "), output);
        BRIDGE::print(F("Access Point ("), output);
        BRIDGE::print(WiFi.softAPmacAddress().c_str(), output);
        BRIDGE::println(F(")"), output);
        BRIDGE::print(F("Station ("), output);
        BRIDGE::print(WiFi.macAddress().c_str(), output);
        BRIDGE::print(F(")"), output);
         if (!plaintext)BRIDGE::print(F("\","), output);
        else BRIDGE::print(F("\n"), output);
    } else {
        BRIDGE::print("Wifi Off", output);
        if (!plaintext)BRIDGE::print(F("\","), output);
        else BRIDGE::print(F("\n"), output);
    }

    if (!plaintext)BRIDGE::print(F("\"captive_portal\":\""), output);
    else BRIDGE::print(F("Captive portal: "), output);
#ifdef CAPTIVE_PORTAL_FEATURE
    BRIDGE::print(F("Enabled"), output);
#else
    BRIDGE::print(F("Disabled"), output);
#endif
    if (!plaintext)BRIDGE::print(F("\","), output);
    else BRIDGE::print(F("\n"), output);
    
    if (!plaintext)BRIDGE::print(F("\"ssdp\":\""), output);
    else BRIDGE::print(F("SSDP: "), output);
#ifdef SSDP_FEATURE
    BRIDGE::print(F("Enabled"), output);
#else
    BRIDGE::print(F("Disabled"), output);
#endif
    if (!plaintext)BRIDGE::print(F("\","), output);
    else BRIDGE::print(F("\n"), output);
    
    if (!plaintext)BRIDGE::print(F("\"netbios\":\""), output);
    else BRIDGE::print(F("NetBios: "), output);
#ifdef NETBIOS_FEATURE
    BRIDGE::print(F("Enabled"), output);
#else
    BRIDGE::print(F("Disabled"), output);
#endif
    if (!plaintext)BRIDGE::print(F("\","), output);
    else BRIDGE::print(F("\n"), output);
    
    if (!plaintext)BRIDGE::print(F("\"mdns\":\""), output);
    else BRIDGE::print(F("mDNS: "), output);
#ifdef MDNS_FEATURE
    BRIDGE::print(F("Enabled"), output);
#else
    BRIDGE::print(F("Disabled"), output);
#endif
    if (!plaintext)BRIDGE::print(F("\","), output);
    else BRIDGE::print(F("\n"), output);

    if (!plaintext)BRIDGE::print(F("\"web_update\":\""), output);
    else BRIDGE::print(F("Web Update: "), output);
#ifdef WEB_UPDATE_FEATURE
    BRIDGE::print(F("Enabled"), output);
#else
    BRIDGE::print(F("Disabled"), output);
#endif
    if (!plaintext)BRIDGE::print(F("\","), output);
    else BRIDGE::print(F("\n"), output);

    if (!plaintext)BRIDGE::print(F("\"pin recovery\":\""), output);
    else BRIDGE::print(F("Pin Recovery: "), output);
#ifdef RECOVERY_FEATURE
    BRIDGE::print(F("Enabled"), output);
#else
    BRIDGE::print(F("Disabled"), output);
#endif
    if (!plaintext)BRIDGE::print(F("\","), output);
    else BRIDGE::print(F("\n"), output);

    if (!plaintext)BRIDGE::print(F("\"autentication\":\""), output);
    else BRIDGE::print(F("Authentication: "), output);
#ifdef AUTHENTICATION_FEATURE
    BRIDGE::print(F("Enabled"), output);
#else
    BRIDGE::print(F("Disabled"), output);
#endif
    if (!plaintext)BRIDGE::print(F("\","), output);
    else BRIDGE::print(F("\n"), output);
    
    if (!plaintext)BRIDGE::print(F("\"target_fw\":\""), output);
    else BRIDGE::print(F("Target Firmware: "), output);
    BRIDGE::print(CONFIG::GetFirmwareTargetName(), output);
    if (!plaintext)BRIDGE::print(F("\","), output);
    else BRIDGE::print(F("\n"), output);
#ifdef DEBUG_ESP3D
    if (!plaintext)BRIDGE::print(F("\"debug\":\""), output);
    else BRIDGE::print(F("Debug: "), output);
    BRIDGE::print(F("Debug Enabled :"), output);
#ifdef DEBUG_OUTPUT_SPIFFS
    BRIDGE::print(F("SPIFFS"), output);
#endif
#ifdef DEBUG_OUTPUT_SD
    BRIDGE::print(F("SD"), output);
#endif
#ifdef DEBUG_OUTPUT_SERIAL
    BRIDGE::print(F("serial"), output);
#endif
#ifdef DEBUG_OUTPUT_TCP
    BRIDGE::print(F("TCP"), output);
#endif
    if (plaintext)BRIDGE::print(F("\","), output);
    else BRIDGE::print(F("\n"), output);
#endif
    if (!plaintext)BRIDGE::print(F("\"fw\":\""), output);
    else BRIDGE::print(F("FW version: "), output);
    BRIDGE::print(FW_VERSION, output);
    if (!plaintext)BRIDGE::print(F("\"}"), output);
    else BRIDGE::print(F("\n"), output);
}
