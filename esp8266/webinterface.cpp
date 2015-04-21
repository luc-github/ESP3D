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

#define PAGE_HEAD  "<html lang=\"en\">\n<head>\n<meta charset=\"utf-8\">\n<meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\">\n" \
                                      "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n<title>Wifi Configuration</title>" \
                                      "<link rel=\"stylesheet\" href=\"http://maxcdn.bootstrapcdn.com/bootstrap/3.3.4/css/bootstrap.min.css\">\n</head>\n<body>"\
                                      " <div class=\"container theme-showcase\" role=\"main\">"
#define NAV_START "<nav class=\"navbar navbar-inverse\">\n<div class=\"container\">\n"
 
#define NAV_LEFT_PART1  "<ul class=\"nav navbar-nav navbar-left\">\n<li "
#define NAV_ELEMENT_ACTIVE "class=\"active\""
#define NAV_LEFT_PART2a "><a href=\"http://"
#define NAV_LEFT_PART2b "\">Home</a></li>\n<li "
#define NAV_LEFT_PART3a "><a href=\"http://"
#define NAV_LEFT_PART3b "/CONFIG\">Configuration</a></li>\n</ul>\n"
 
 #define NAV_RIGHT_PART "<p class=\"navbar-text navbar-right\">&nbsp;&nbsp;&nbsp;&nbsp;</p>\n<ul class=\"nav navbar-nav navbar-right\">\n"\
                                                "<li><a href=\""  REPOSITORY "\">Github</a></li>\n</ul>\n"\
                                                "<p class=\"navbar-text navbar-right\">FW: " FW_VERSION "</p>\n"

 #define NAV_END     "</div>\n</nav>"
                                      
#define PAGE_BOTTOM "</body>\n</html>" 



//cannot put it in class then cast it as std::function<void(void)> so put outside
void handle_web_interface_root()
{
  String sbuf=PAGE_HEAD ;
  String IP;
  if (wifi_get_opmode()==WIFI_STA ) IP=wifi_config.ip2str(WiFi.localIP());
  else IP=wifi_config.ip2str(WiFi.softAPIP());
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
 /* if (web_interface.WebServer.arg("myinput").length()> 0)
  {
    char buf[250];
    char buf1[250];
    web_interface.WebServer.arg("myinput").toCharArray(buf1, 249);
    web_interface.urldecode(buf,buf1);
    sbuf="<FORM METHOD=POST><INPUT TYPE=TEXT NAME=myinput value=\"";
    sbuf+=buf;
    sbuf+="\"><BUTTON TYPE=SUBMIT NAME=Submit VALUE=\"Submit\"></FORM>";
  }
  else
  {
    sbuf += "No Data";
  }*/
  sbuf+="<div>Information<BR>";
  sbuf+="<BR>CPU Freq:";
  sbuf+=String(system_get_cpu_freq());
  sbuf+="<BR>Mem:";
  sbuf+=String(system_get_free_heap_size());
    sbuf+="<BR>ID:";
  sbuf+=String(system_get_chip_id());
  
  sbuf+="<BR>SDK:";
  sbuf+=system_get_sdk_version();
  sbuf+="<BR>Mac:";
  uint8_t mac [WL_MAC_ADDR_LENGTH];
  sbuf+=wifi_config.mac2str(WiFi.macAddress(mac));
  sbuf+="</div>";
  sbuf+=PAGE_BOTTOM;
  web_interface.WebServer.send(200, "text/html", sbuf);
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
  web_interface.WebServer.send(200, "text/html", sbuf);
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
  WebServer.on("/",HTTP_ANY, handle_web_interface_root);
  WebServer.on("/CONFIG",HTTP_ANY, handle_web_interface_config);
}

WEBINTERFACE_CLASS web_interface(80);

