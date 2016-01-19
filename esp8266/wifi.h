/* 
  wifi.h - esp8266 configuration class

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

#ifndef WIFI_h
#define WIFI_h
#include <Arduino.h>
#include "config.h"
#include "IPAddress.h"
#include <ESP8266WiFi.h>

#ifdef MDNS_FEATURE
#include <ESP8266mDNS.h>
#endif

class WIFI_CONFIG
{
  public:
  // multicast DNS responder feature
  #ifdef MDNS_FEATURE
	MDNSResponder mdns;
  #endif
  WIFI_CONFIG();
  int iweb_port;
  int idata_port;
  long baud_rate;
  int sleep_mode;
  bool Setup();
  void Safe_Setup();
  char * mac2str(uint8_t mac [WL_MAC_ADDR_LENGTH]);
  char * ip2str(IPAddress Ip );
  byte split_ip (const char * ptr,byte * part);
  const char * get_default_hostname();
  const char * get_hostname();
  private:
  char _hostname[33];
};

extern WIFI_CONFIG wifi_config;
#endif
