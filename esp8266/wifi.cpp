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

//Set IP configurstion to AP
void WIFI_CONFIG::configAP(IPAddress local_ip, IPAddress gateway, IPAddress subnet)
{
 //no helper function to change AP IP so do it manually
      struct ip_info info;
      info.ip.addr = static_cast<uint32_t>(local_ip);
      info.gw.addr = static_cast<uint32_t>(gateway);
      info.netmask.addr = static_cast<uint32_t>(subnet);
      wifi_softap_dhcps_stop();
      wifi_set_ip_info(SOFTAP_IF, &info);
      wifi_softap_dhcps_start();
}
//just simple helper to convert mac address to string
char * WIFI_CONFIG::mac2str(uint8_t mac [WL_MAC_ADDR_LENGTH])
{
  static char macstr [18];
  if (0>sprintf(macstr, "%02X:%02X:%02X:%02X:%02X:%02X",mac[5],mac[4],mac[3],mac[2],mac[1],mac[0])) strcpy (macstr, "00:00:00:00:00:00");
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
  byte bbuf;
  char pwd[65];
  char sbuf[35];
  int wstatus;
  byte ip[4]={0,0,0,0};
  //AP or client ?
  if (!CONFIG::read_byte(EP_WIFI_MODE, &bbuf ) ||  !CONFIG::read_string(EP_SSID, sbuf , MAX_SSID_LENGH) ||!CONFIG::read_string(EP_PASSWORD, pwd , MAX_PASSWORD_LENGH)) return false;
    //disconnect if connected
  WiFi.disconnect();
    //this is AP mode
  if (bbuf==AP_MODE)
    {
      WiFi.mode(WIFI_AP);
      WiFi.softAP(sbuf, pwd);
    }
  else
    {
      WiFi.mode(WIFI_STA);
      WiFi.begin(sbuf, pwd);
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
      if (wifi_get_opmode()==WIFI_AP || wifi_get_opmode()==WIFI_AP_STA)  configAP( local_ip,  gateway,  subnet);
      else WiFi.config( local_ip,  gateway,  subnet); 
    }
  return true;
}

WIFI_CONFIG wifi_config;
