/*
  wifi.cpp - esp8266 configuration class

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

#include "wifi.h"
#include "config.h"
#include "ESP8266WiFi.h"
#include "IPAddress.h"
#ifdef MDNS_FEATURE
#include <ESP8266mDNS.h>
#endif
#ifdef CAPTIVE_PORTAL_FEATURE
#include <DNSServer.h>
extern DNSServer dnsServer;
#endif
extern "C" {
#include "user_interface.h"
}
WIFI_CONFIG::WIFI_CONFIG()
{
    iweb_port=DEFAULT_WEB_PORT;
    idata_port=DEFAULT_DATA_PORT;
    baud_rate=DEFAULT_BAUD_RATE;
    sleep_mode=DEFAULT_SLEEP_MODE;
    _hostname[0]=0;
}

const char * WIFI_CONFIG::get_hostname()
{
    if (WiFi.hostname().length()==0) {
        if (!CONFIG::read_string(EP_HOSTNAME, _hostname , MAX_HOSTNAME_LENGTH)) {
            strcpy(_hostname,get_default_hostname());
        }
    } else {
        strcpy(_hostname,WiFi.hostname().c_str());
    }
    return _hostname;
}

const char * WIFI_CONFIG::get_default_hostname()
{
    static char hostname[13];
    uint8_t mac [WL_MAC_ADDR_LENGTH];
    WiFi.macAddress(mac);
    if (0>sprintf(hostname,"ESP_%02X%02X%02X",mac[3],mac[4],mac[5])) {
        strcpy (hostname, "ESP8266");
    }
    return hostname;
}

//helper to convert string to IP
//do not use IPAddress.fromString() because lack of check point and error result 
//return number of parts
byte WIFI_CONFIG::split_ip (const char * ptr,byte * part)
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
char * WIFI_CONFIG::mac2str(uint8_t mac [WL_MAC_ADDR_LENGTH])
{
    static char macstr [18];
    if (0>sprintf(macstr,"%02X:%02X:%02X:%02X:%02X:%02X",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5])) {
        strcpy (macstr, "00:00:00:00:00:00");
    }
    return macstr;
}

//safe setup if no connection 
void  WIFI_CONFIG::Safe_Setup()
{
#ifdef CAPTIVE_PORTAL_FEATURE
    dnsServer.stop();
    delay(100);
#endif

    WiFi.disconnect();
    //setup Soft AP
    WiFi.mode(WIFI_AP);
    IPAddress local_ip (DEFAULT_IP_VALUE[0],DEFAULT_IP_VALUE[1],DEFAULT_IP_VALUE[2],DEFAULT_IP_VALUE[3]);
    IPAddress gateway (DEFAULT_GATEWAY_VALUE[0],DEFAULT_GATEWAY_VALUE[1],DEFAULT_GATEWAY_VALUE[2],DEFAULT_GATEWAY_VALUE[3]);
    IPAddress subnet (DEFAULT_MASK_VALUE[0],DEFAULT_MASK_VALUE[1],DEFAULT_MASK_VALUE[2],DEFAULT_MASK_VALUE[3]);
    String ssid = FPSTR(DEFAULT_SSID);
    String pwd = FPSTR(DEFAULT_PASSWORD);
    WiFi.softAP(ssid.c_str(),pwd.c_str());
    delay(500);
    WiFi.softAPConfig( local_ip,  gateway,  subnet);
    delay(1000);
    Serial.println(F("M117 Safe mode started"));
}

//Read configuration settings and apply them
bool WIFI_CONFIG::Setup()
{
    char pwd[MAX_PASSWORD_LENGTH+1];
    char sbuf[MAX_SSID_LENGTH+1];
    char hostname [MAX_HOSTNAME_LENGTH+1];
    int wstatus;
    IPAddress currentIP;
    byte bflag=0;
    byte bmode=0;
    //system_update_cpu_freq(SYS_CPU_160MHZ);
    //set the sleep mode
    if (!CONFIG::read_byte(EP_SLEEP_MODE, &bflag )) {
        return false;
    }
    WiFi.setSleepMode ((WiFiSleepType_t)bflag);
    sleep_mode=bflag;
    //AP or client ?
    if (!CONFIG::read_byte(EP_WIFI_MODE, &bmode ) ||  !CONFIG::read_string(EP_SSID, sbuf , MAX_SSID_LENGTH) ||!CONFIG::read_string(EP_PASSWORD, pwd , MAX_PASSWORD_LENGTH)) {
        return false;
    }
    if (!CONFIG::read_string(EP_HOSTNAME, hostname , MAX_HOSTNAME_LENGTH)) {
        strcpy(hostname,get_default_hostname());
    }
        //DHCP or Static IP ?
    if (!CONFIG::read_byte(EP_IP_MODE, &bflag )) {
        return false;
    }
     if (bflag==STATIC_IP_MODE) {
        byte ip_buf[4];
        //get the IP
        if (!CONFIG::read_buffer(EP_IP_VALUE,ip_buf , IP_LENGTH)) {
            return false;
        }
        IPAddress local_ip (ip_buf[0],ip_buf[1],ip_buf[2],ip_buf[3]);
        //get the gateway
        if (!CONFIG::read_buffer(EP_GATEWAY_VALUE,ip_buf , IP_LENGTH)) {
            return false;
        }
        IPAddress gateway (ip_buf[0],ip_buf[1],ip_buf[2],ip_buf[3]);
        //get the mask
        if (!CONFIG::read_buffer(EP_MASK_VALUE,ip_buf , IP_LENGTH)) {
            return false;
        }
        IPAddress subnet (ip_buf[0],ip_buf[1],ip_buf[2],ip_buf[3]);
        //apply according active wifi mode
        if (bmode==AP_MODE) {
            WiFi.softAPConfig( local_ip,  gateway,  subnet);
        } else {
            WiFi.config( local_ip,  gateway,  subnet);
        }
    }
    //this is AP mode
    if (bmode==AP_MODE) {
		WiFi.enableSTA(true);
        //setup Soft AP
        WiFi.mode(WIFI_AP);
        WiFi.softAP(sbuf, pwd);
        //setup PHY_MODE
        if (!CONFIG::read_byte(EP_PHY_MODE, &bflag )) {
            return false;
        }
        WiFi.setPhyMode((WiFiPhyMode_t)bflag);
        //get current config
        struct softap_config apconfig;
        wifi_softap_get_config(&apconfig);
        //set the chanel
        if (!CONFIG::read_byte(EP_CHANNEL, &bflag )) {
            return false;
        }
        apconfig.channel=bflag;
        //set Authentification type
        if (!CONFIG::read_byte(EP_AUTH_TYPE, &bflag )) {
            return false;
        }
        apconfig.authmode=(AUTH_MODE)bflag;
        //set the visibility of SSID
        if (!CONFIG::read_byte(EP_SSID_VISIBLE, &bflag )) {
            return false;
        }
        apconfig.ssid_hidden=!bflag;
        //no need to add these settings to configuration just use default ones
        apconfig.max_connection=DEFAULT_MAX_CONNECTIONS;
        apconfig.beacon_interval=DEFAULT_BEACON_INTERVAL;
        //apply settings to current and to default
        if (!wifi_softap_set_config(&apconfig) || !wifi_softap_set_config_current(&apconfig)) {
            Serial.println(F("M117 Error Wifi AP!"));
            delay(1000);
        }
    } else {
		WiFi.enableAP(false);
        //setup station mode
        WiFi.mode(WIFI_STA);
        WiFi.begin(sbuf, pwd);
        delay(500);
        //setup PHY_MODE
        if (!CONFIG::read_byte(EP_PHY_MODE, &bflag )) {
            return false;
        }
        WiFi.setPhyMode((WiFiPhyMode_t)bflag);
        delay(500);
        byte i=0;
        //try to connect
        while (WiFi.status() != WL_CONNECTED && i<40) {
            switch(WiFi.status()) {
            case 1:
                Serial.print(FPSTR(M117_));
                Serial.println(F("No SSID found!"));
                break;

            case 4:
                Serial.print(FPSTR(M117_));
                Serial.println(F("No Connection!"));
                break;

            default:
                Serial.print(FPSTR(M117_));
                Serial.println(F("Connecting..."));
                break;
            }
            delay(500);
            i++;
        }
        if (WiFi.status() != WL_CONNECTED) {
            return false;
        }
        WiFi.hostname(hostname);
    }


#ifdef MDNS_FEATURE
    // Set up mDNS responder:
    if (!mdns.begin(hostname)) {
        Serial.print(FPSTR(M117_));
        Serial.println(F("Error with mDNS!"));
        delay(1000);
    }
#endif
    //Get IP
    if (WiFi.getMode()==WIFI_STA) {
        currentIP=WiFi.localIP();
    } else {
        currentIP=WiFi.softAPIP();
    }
    Serial.print(FPSTR(M117_));
    Serial.println(currentIP);
    return true;
}
WIFI_CONFIG wifi_config;
