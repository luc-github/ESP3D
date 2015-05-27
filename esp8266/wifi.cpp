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
  if (0>sprintf(macstr,"%02X:%02X:%02X:%02X:%02X:%02X",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5])) strcpy (macstr, "00:00:00:00:00:00");
  return macstr;
}

//just simple helper to convert IP address to string
char * WIFI_CONFIG::ip2str(IPAddress Ip )
{
  static char ipstr [16];
  if (0>sprintf(ipstr, "%i.%i.%i.%i",Ip[0],Ip[1],Ip[2],Ip[3])) strcpy (ipstr, "0.0.0.0");
  return ipstr;
}

//Read configuration settings and apply them
bool WIFI_CONFIG::Setup()
{
  char pwd[MAX_PASSWORD_LENGH+1];
  char sbuf[MAX_SSID_LENGH+1];
  int wstatus;
   IPAddress currentIP;
  byte bflag=0;
  //set the sleep mode
  if (!CONFIG::read_byte(EP_SLEEP_MODE, &bflag ))return false;
  wifi_set_sleep_type ((sleep_type)bflag);
  //AP or client ?
  if (!CONFIG::read_byte(EP_WIFI_MODE, &bflag ) ||  !CONFIG::read_string(EP_SSID, sbuf , MAX_SSID_LENGH) ||!CONFIG::read_string(EP_PASSWORD, pwd , MAX_PASSWORD_LENGH)) return false;
    //disconnect if connected
  WiFi.disconnect();
   //this is AP mode
  if (bflag==AP_MODE)
    {
		//setup Soft AP
      WiFi.mode(WIFI_AP);
      WiFi.softAP(sbuf, pwd);
      //setup PHY_MODE
	  if (!CONFIG::read_byte(EP_PHY_MODE, &bflag ))return false;
      wifi_set_phy_mode((phy_mode)bflag);
      //get current config
      struct softap_config apconfig;
       wifi_softap_get_config(&apconfig);
       //set the chanel
       if (!CONFIG::read_byte(EP_CHANNEL, &bflag ))return false;
       apconfig.channel=bflag;
       //set Authentification type
        if (!CONFIG::read_byte(EP_AUTH_TYPE, &bflag ))return false;
       apconfig.authmode=(AUTH_MODE)bflag;
       //set the visibility of SSID
        if (!CONFIG::read_byte(EP_SSID_VISIBLE, &bflag ))return false;
       apconfig.ssid_hidden=!bflag;
       //no need to add these settings to configuration just use default ones
       apconfig.max_connection=DEFAULT_MAX_CONNECTIONS;
       apconfig.beacon_interval=DEFAULT_BEACON_INTERVAL;
      //apply settings to current and to default
      if (!wifi_softap_set_config(&apconfig))Serial.println(F("Error Wifi AP"));
      if (!wifi_softap_set_config_current(&apconfig))Serial.println(F("Error Wifi AP"));
      wifi_softap_dhcps_start();
    }
  else
    {//setup station mode
      WiFi.mode(WIFI_STA);
      WiFi.begin(sbuf, pwd);
       //setup PHY_MODE
	   if (!CONFIG::read_byte(EP_PHY_MODE, &bflag ))return false;
	   wifi_set_phy_mode((phy_mode)bflag);
      byte i=0;
      //try to connect
      while (WiFi.status() != WL_CONNECTED && i<40) {
          delay(500);
          Serial.println(WiFi.status());
          i++;
          }
    }
  //DHCP or Static IP ?
  if (!CONFIG::read_byte(EP_IP_MODE, &bflag )) return false;
  if (bflag==STATIC_IP_MODE)
    {
      //get the IP 
      if (!CONFIG::read_buffer(EP_IP_VALUE,(byte *)sbuf , IP_LENGH))return false;
      IPAddress local_ip (sbuf[0],sbuf[1],sbuf[2],sbuf[3]);
      //get the gateway 
      if (!CONFIG::read_buffer(EP_GATEWAY_VALUE,(byte *)sbuf , IP_LENGH))return false;
      IPAddress gateway (sbuf[0],sbuf[1],sbuf[2],sbuf[3]);
      //get the mask 
      if (!CONFIG::read_buffer(EP_MASK_VALUE,(byte *)sbuf , IP_LENGH))return false;
      IPAddress subnet (sbuf[0],sbuf[1],sbuf[2],sbuf[3]);
     //apply according active wifi mode
      if (wifi_get_opmode()==WIFI_AP || wifi_get_opmode()==WIFI_AP_STA)  WiFi.softAPConfig( local_ip,  gateway,  subnet);
      else WiFi.config( local_ip,  gateway,  subnet); 
    }
    #ifdef MDNS_FEATURE
    //Get IP
    if (wifi_get_opmode()==WIFI_STA)currentIP=WiFi.localIP();
    else currentIP=WiFi.softAPIP();
    // Set up mDNS responder:
    // - first argument is the domain name, in this example
    //   the fully-qualified domain name is "esp8266.local"
    // - second argument is the IP address to advertise
    //   we send our IP address on the WiFi network
    //   Note: for AP mode we would use WiFi.softAPIP()!
  if (!mdns.begin(PROGMEM2CHAR(LOCAL_NAME), currentIP)) {
    Serial.println(F("Error setting up MDNS responder!"));
    }
    #endif
  return true;
}

WIFI_CONFIG wifi_config;
