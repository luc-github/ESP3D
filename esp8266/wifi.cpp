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
#if MDNS_FEATURE
#include <ESP8266mDNS.h>
#endif
extern "C" {
#include "user_interface.h"
}
//to get access to some function like
//wifi_get_opmode() in status

//no strtok so this is simplified version
//return number of part
byte WIFI_CONFIG::split_ip (char * ptr,byte * part)
{
  char * pstart = ptr;
  byte i = strlen(ptr);
  byte pos = 0;
  for (byte j=0;j<i;j++)
    {
      if (ptr[j]=='.')
        {
          ptr[j]=0x0;
          part[pos]=atoi(pstart);
          pos++;
          pstart = &ptr[j+1]; 
        }
    }
  part[pos]=atoi(pstart);
  return pos+1;  
}

//just simple helper to convert mac address to string
char * WIFI_CONFIG::mac2str(uint8_t mac [WL_MAC_ADDR_LENGTH])
{
  static char macstr [18];
  if (0>sprintf(macstr,F("%02X:%02X:%02X:%02X:%02X:%02X"),mac[0],mac[1],mac[2],mac[3],mac[4],mac[5])) strcpy (macstr, F("00:00:00:00:00:00"));
  return macstr;
}

//just simple helper to convert IP address to string
char * WIFI_CONFIG::ip2str(IPAddress Ip )
{
  static char ipstr [16];
  if (0>sprintf(ipstr, F("%i.%i.%i.%i"),Ip[0],Ip[1],Ip[2],Ip[3])) strcpy (ipstr, F("0.0.0.0"));
  return ipstr;
}

//Read configuration settings and apply them
bool WIFI_CONFIG::Setup()
{
  byte bbuf;
  char pwd[65];
  char sbuf[35];
  int wstatus;
  byte ip[4]={0,0,0,0};
  IPAddress currentIP;

  //AP or client ?
  if (!CONFIG::read_byte(EP_WIFI_MODE, &bbuf ) ||  !CONFIG::read_string(EP_SSID, sbuf , MAX_SSID_LENGH) ||!CONFIG::read_string(EP_PASSWORD, pwd , MAX_PASSWORD_LENGH)) return false;
    //disconnect if connected
  WiFi.disconnect();
  bbuf=AP_MODE;
   //this is AP mode
  if (bbuf==AP_MODE)
    {
      WiFi.mode(WIFI_AP);
      WiFi.softAP(sbuf, pwd);
      
       struct softap_config apconfig;
       wifi_softap_get_config(&apconfig);
       apconfig.channel=11;
       //apconfig.authmode=AUTH_OPEN;
       apconfig.ssid_hidden=0;
       apconfig.max_connection=4;
       apconfig.beacon_interval=100;
       wifi_set_phy_mode(PHY_MODE_11G);
      if (!wifi_softap_set_config(&apconfig))Serial.println(F("Error Wifi AP"));
      if (!wifi_softap_set_config_current(&apconfig))Serial.println(F("Error Wifi AP"));
      wifi_softap_dhcps_start();
      wifi_set_phy_mode(PHY_MODE_11G);
    }
  else
    {
      WiFi.mode(WIFI_STA);
      WiFi.begin(sbuf, pwd);
      byte i=0;
      while (WiFi.status() != WL_CONNECTED && i<40) {
          delay(500);
          Serial.println(WiFi.status());
          i++;
          }
    }
  //DHCP or Static IP ?
  if (!CONFIG::read_byte(EP_IP_MODE, &bbuf )) return false;
  if (bbuf==STATIC_IP_MODE)
    {
      //get the IP 
      if (!CONFIG::read_string(EP_IP_VALUE, sbuf , MAX_IP_LENGH))return false;
      //split in 4 parts
      split_ip (sbuf,ip); 
      IPAddress local_ip (ip[0],ip[1],ip[2],ip[3]);
      //get the gateway 
      if (!CONFIG::read_string(EP_GATEWAY_VALUE, sbuf , MAX_IP_LENGH))return false;
      //split in 4 parts
      split_ip (sbuf,ip); 
      IPAddress gateway (ip[0],ip[1],ip[2],ip[3]);
      //get the mask 
      if (!CONFIG::read_string(EP_MASK_VALUE, sbuf , MAX_IP_LENGH))return false;
      //split in 4 parts
      split_ip (sbuf,ip); 
      IPAddress subnet (ip[0],ip[1],ip[2],ip[3]);
     //apply according active wifi mode
      if (wifi_get_opmode()==WIFI_AP || wifi_get_opmode()==WIFI_AP_STA)  WiFi.softAPConfig( local_ip,  gateway,  subnet);
      else WiFi.config( local_ip,  gateway,  subnet); 
    }
    #if MDNS_FEATURE
    //Get IP
    if (wifi_get_opmode()==WIFI_STA)currentIP=WiFi.localIP();
    else currentIP=WiFi.softAPIP();
    // Set up mDNS responder:
  // - first argument is the domain name, in this example
  //   the fully-qualified domain name is "esp8266.local"
  // - second argument is the IP address to advertise
  //   we send our IP address on the WiFi network
  //   Note: for AP mode we would use WiFi.softAPIP()!
  if (!mdns.begin(LOCAL_NAME, currentIP)) {
    Serial.println(F("Error setting up MDNS responder!"));
    }
    #endif
    CONFIG::print_config();
  return true;
}

WIFI_CONFIG wifi_config;
