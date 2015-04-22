/* 
  webinterface.cpp - esp8266 configuration class

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
#include "webinterface.h"
#include "wifi.h"
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
extern "C" {
#include "user_interface.h"
}

const char  PAGE_HEAD[] PROGMEM =  "<html lang=\"en\">\n<head>\n<meta charset=\"utf-8\">\n<meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\">\n" \
                                      "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n<title>Wifi Configuration</title>" \
                                      "<link rel=\"stylesheet\" href=\"http://maxcdn.bootstrapcdn.com/bootstrap/3.3.4/css/bootstrap.min.css\">\n</head>\n<body>"\
                                      " <div class=\"container theme-showcase\" role=\"main\">";
const char NAV_START[] PROGMEM =  "<nav class=\"navbar navbar-inverse\">\n<div class=\"container\">\n";
 
const char NAV_LEFT_PART1 [] PROGMEM =  "<ul class=\"nav navbar-nav navbar-left\">\n<li ";
const char NAV_ELEMENT_ACTIVE [] PROGMEM =  "class=\"active\"";
const char  NAV_LEFT_PART2a[] PROGMEM =  "><a href=\"http://";
const char NAV_LEFT_PART2b[] PROGMEM =  "\">Home</a></li>\n<li ";
const char NAV_LEFT_PART3a[] PROGMEM = "><a href=\"http://";
const char NAV_LEFT_PART3b[] PROGMEM =  "/CONFIG\">Configuration</a></li>\n</ul>\n";
 
const char NAV_RIGHT_PART[] PROGMEM =  "<p class=\"navbar-text navbar-right\">&nbsp;&nbsp;&nbsp;&nbsp;</p>\n<ul class=\"nav navbar-nav navbar-right\">\n"\
                                                "<li><a href=\""  REPOSITORY "\">Github</a></li>\n</ul>\n"\
                                                "<p class=\"navbar-text navbar-right\">FW: " FW_VERSION "</p>\n";

const char NAV_END[] PROGMEM = "</div>\n</nav>";
                                      
const char PAGE_BOTTOM[] PROGMEM =  "</body>\n</html>" ;

const char PANEL_TOP[] PROGMEM = "<div class=\"panel panel-default\">\n<div class=\"panel-heading\">\n<h3 class=\"panel-title\">";
const char PANEL_START[] PROGMEM ="</h3>\n</div>\n<div class=\"panel-body\">";
const char PANEL_END[] PROGMEM = "</div>\n</div>";
const char LABEL_START[] PROGMEM = "<label>";
const char LABEL_COLOR[] PROGMEM = "</label><label class=\"text-info\">";
const char LABEL_END[] PROGMEM = "</label>";
const char BR[] PROGMEM = "<BR>\n";
#define LABEL(buffer, title, value)  buffer+= LABEL_START;  buffer+=title;buffer+=LABEL_COLOR;buffer+=value;buffer+=LABEL_END;buffer+=BR;
#define LABEL_UNITS(buffer, title, value,units)  buffer+= LABEL_START;  buffer+=title;buffer+=LABEL_COLOR;buffer+=value;buffer+=units;buffer+=LABEL_END;buffer+=BR;

//cannot put it in class then cast it as std::function<void(void)> so put outside
void handle_web_interface_root()
{
  String sbuf=PAGE_HEAD ;
  String IP;
  String sstatus;
  struct softap_config apconfig;
  struct ip_info info;
   int istatus;
  uint8_t mac [WL_MAC_ADDR_LENGTH];
  if (wifi_get_opmode()==WIFI_STA ) IP=wifi_config.ip2str(WiFi.localIP());
  else IP=wifi_config.ip2str(WiFi.softAPIP());
  //top bar
   sbuf+=NAV_START ;
  sbuf+=NAV_LEFT_PART1 ;
  sbuf+=NAV_ELEMENT_ACTIVE ;
  sbuf+=NAV_LEFT_PART2a ;
  sbuf+=IP;
  sbuf+=NAV_LEFT_PART2b ;
  sbuf+=NAV_LEFT_PART3a ;
  sbuf+=IP;
  sbuf+=NAV_LEFT_PART3b ;
  sbuf+=NAV_RIGHT_PART ;
  sbuf+=NAV_END ;
//system part
  sbuf+=PANEL_TOP;
  sbuf+=F("System");
  sbuf+=PANEL_START;
  LABEL(sbuf,F("Chip ID: "),String(system_get_chip_id()))
  LABEL_UNITS(sbuf,F("CPU Frequency: "),system_get_cpu_freq(),F("Hz"))
  LABEL_UNITS(sbuf,F("Free Memory: "),String(system_get_free_heap_size()),F(" octets"))
  LABEL(sbuf,F("SDK Version: "),system_get_sdk_version())
  #if MDNS_FEATURE
  sstatus = F("http://");
  sstatus+=LOCAL_NAME;
  LABEL_UNITS(sbuf,F("mDNS name: "),sstatus,F(".local"))
  #endif
  istatus = wifi_get_phy_mode();
  if (istatus==PHY_MODE_11B) sstatus=F("11b");
  else if (istatus==PHY_MODE_11G) sstatus=F("11g");
  else sstatus=F("11n");
  LABEL(sbuf,F("Network: "),sstatus)
  istatus = wifi_get_sleep_type();
  if (istatus==NONE_SLEEP_T) sstatus=F("None");
  else if (istatus==LIGHT_SLEEP_T) sstatus=F("Light");
  else  sstatus=F("Modem");
  LABEL(sbuf,F("Sleep mode: "),sstatus)
  //LABEL(sbuf,F("Boot mode: "),String(system_get_boot_mode())) //no meaning so far
  LABEL(sbuf,F("Boot version: "),String(system_get_boot_version()))
  sbuf+=PANEL_END;
  
  //split to not reach the sending size limit
 web_interface.WebServer.send(200, F("text/html"), sbuf);

 //access point
  sbuf=PANEL_TOP;
  sbuf+=F("Access Point");
  if(wifi_get_opmode()==WIFI_AP ||  wifi_get_opmode()==WIFI_AP_STA) sbuf+=F(" (enabled)");
  else sbuf+=F(" (disabled)");
  sbuf+=PANEL_START;
  LABEL(sbuf,F("Mac address: "),wifi_config.mac2str(WiFi.softAPmacAddress(mac));)
  if (wifi_softap_get_config(&apconfig))
    {
      LABEL(sbuf,F("SSID: "),(char *)(apconfig.ssid))
      if(apconfig.ssid_hidden==1)sstatus=F("No");
      else sstatus=F("Yes");
      LABEL(sbuf,F("Visible: "),sstatus)
      LABEL(sbuf,F("Channel: "),String(apconfig.channel))
      if (apconfig.authmode==AUTH_OPEN)sstatus=F("None");
      else if (apconfig.authmode==AUTH_WEP)sstatus=F("WEP");
      else if (apconfig.authmode==AUTH_WPA_PSK)sstatus=F("WPA");
      else if (apconfig.authmode==AUTH_WPA2_PSK)sstatus=F("WPA2");
      else if (apconfig.authmode==AUTH_WPA_WPA2_PSK)sstatus=F("WPA/WPA2");
      else sstatus=F("MAX"); //what is this one ? WPS ? Cannot find information
      LABEL(sbuf,F("Authentification: "),sstatus)
      LABEL(sbuf,F("Maximum connections : "),String(apconfig.max_connection))
    }
     if (wifi_softap_dhcps_status()==DHCP_STARTED)sstatus=F("Started");
     else sstatus=F("Stopped");
     LABEL(sbuf,F("DHCP Server: "),sstatus)
     if (wifi_get_ip_info(SOFTAP_IF,&info))
        {
          LABEL(sbuf,F("IP: "),wifi_config.ip2str(info.ip.addr))
          LABEL(sbuf,F("Gateway: "),wifi_config.ip2str(info.gw.addr))
          LABEL(sbuf,F("Subnet: "),wifi_config.ip2str(info.netmask.addr))
        }
    //List number of client
    istatus = 0;
    sstatus="";
    struct station_info * station = wifi_softap_get_station_info();
        while(station){
          istatus++;
          sstatus+=F("<TR><TD>Mac:</TD><TD>");
          sstatus+=wifi_config.mac2str(station->bssid);
          sstatus+=F("</TD><TD>IP:</TD><TD>");
          static char ipstr [16];
         if (0>sprintf(ipstr, IPSTR,IP2STR(&station->ip))) strcpy (ipstr, F("0.0.0.0"));
          sstatus+=ipstr;
           sstatus+=F("</TD></TR>");
          station = STAILQ_NEXT(station, next);
          }
   wifi_softap_free_station_info();
  LABEL(sbuf,F("Clients: "),String(istatus));
   sbuf+=F("<TABLE>");
   sbuf+=sstatus;
   sbuf+=F("</TABLE>");
  sbuf+=PANEL_END;
  //split to not reach the sending size limit
  web_interface.WebServer.client().print(sbuf);
  
  sbuf=PANEL_TOP;
  sbuf+=F("Station");
  if(wifi_get_opmode()==WIFI_STA ||  wifi_get_opmode()==WIFI_AP_STA) sbuf+=F(" (enabled)");
  else sbuf+=F(" (disabled)");
  sbuf+=PANEL_START;
  LABEL(sbuf,F("Mac address: "),wifi_config.mac2str(WiFi.macAddress(mac));)
  LABEL(sbuf,F("Connection to: "),WiFi.SSID())
  LABEL(sbuf,F("Channel: "),String(wifi_get_channel()))
  istatus = wifi_station_get_connect_status();
  if (istatus==STATION_GOT_IP) sstatus=F("Connected");
  else if  (istatus==STATION_NO_AP_FOUND) sstatus=F("SSID not Available!");
  else if  (istatus==STATION_CONNECT_FAIL) sstatus=F("Connecion failed!");
  else if  (istatus==STATION_WRONG_PASSWORD) sstatus=F("Connecion failed! (Wrong Password)");
  else if  (istatus==STATION_IDLE) sstatus=F("Idle");//should not happen
  else sstatus=F("Disconnected");
  LABEL(sbuf,F("Status: "),sstatus)
  if (wifi_station_dhcpc_status()==DHCP_STARTED)sstatus=F("Started");
  else sstatus=F("Stopped");
  LABEL(sbuf,F("DHCP Client: "),sstatus)
  LABEL(sbuf,F("IP: "),wifi_config.ip2str(WiFi.localIP()))
  LABEL(sbuf,F("Gateway: "),wifi_config.ip2str(WiFi.gatewayIP()))
  LABEL(sbuf,F("Subnet: "),wifi_config.ip2str(WiFi.subnetMask()))
  sbuf+=PANEL_END;
  //split to not reach the sending size limit (2920)
  web_interface.WebServer.client().print(sbuf);

  
   sbuf=PAGE_BOTTOM;
   //split to not reach the sending size limit
    web_interface.WebServer.client().print(sbuf);
}

void handle_web_interface_config()
{
  String sbuf=PAGE_HEAD ;
  String IP;
  if (wifi_get_opmode()==WIFI_STA ) IP=wifi_config.ip2str(WiFi.localIP());
  else IP=wifi_config.ip2str(WiFi.softAPIP());
  sbuf+=NAV_START ;
  sbuf+=NAV_LEFT_PART1 ;
  sbuf+=NAV_LEFT_PART2a ;
  sbuf+=IP;
  sbuf+=NAV_LEFT_PART2b ;
  sbuf+=NAV_ELEMENT_ACTIVE ;
  sbuf+=NAV_LEFT_PART3a ;
  sbuf+=IP;
  sbuf+=NAV_LEFT_PART3b ;
  sbuf+=NAV_RIGHT_PART ;
  sbuf+=NAV_END ;
  sbuf+="<div>Configuration</div>";
  sbuf+=PAGE_BOTTOM;
  web_interface.WebServer.send(200, F("text/html"), sbuf);
}

//URI Decoding function 
//no check if dst buffer is big enough to receive string so 
//use same size as src is a recommendation
void WEBINTERFACE_CLASS::urldecode(char *dst, const char *src)
{
  char a, b;
  if (dst==NULL) return;
  while (*src) {
    if ((*src == '%') &&
      ((a = src[1]) && (b = src[2])) &&
      (isxdigit(a) && isxdigit(b))) {
      if (a >= 'a')
        a -= 'a'-'A';
      if (a >= 'A')
        a -= ('A' - 10);
      else
        a -= '0';
      if (b >= 'a')
        b -= 'a'-'A';
      if (b >= 'A')
        b -= ('A' - 10);
      else
        b -= '0';
      *dst++ = 16*a+b;
      src+=3;
    } 
    else {
      *dst++ = *src++;
    }
  }
  *dst++ = '\0';
}

//constructor
WEBINTERFACE_CLASS::WEBINTERFACE_CLASS (int port):WebServer(port)
{
  //init what will handle "/"
  WebServer.on(F("/"),HTTP_ANY, handle_web_interface_root);
  WebServer.on(F("/CONFIG"),HTTP_ANY, handle_web_interface_config);
}

WEBINTERFACE_CLASS web_interface(80);

