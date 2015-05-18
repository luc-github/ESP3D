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

#define LIMIT_BUFFER		2500

const char  PAGE_HEAD_1[] PROGMEM =  "<html lang=\"en\">\n<head>\n<meta charset=\"utf-8\">\n<meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\">\n" \
                                      "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n";
const char  PAGE_HEAD_2[] PROGMEM = "<title>Wifi Configuration</title>" \
                                      "<link rel=\"stylesheet\" href=\"http://maxcdn.bootstrapcdn.com/bootstrap/3.3.4/css/bootstrap.min.css\">\n</head>\n<body>"\
                                      " <div class=\"container theme-showcase\" role=\"main\">";
const char NAV_START[] PROGMEM =  "<nav class=\"navbar navbar-inverse\">\n<div class=\"container\">\n";
 
const char NAV_LEFT_PART1 [] PROGMEM =  "<ul class=\"nav navbar-nav navbar-left\">\n<li ";
const char NAV_ELEMENT_ACTIVE [] PROGMEM =  "class=\"active\"";
const char  NAV_LEFT_PART2a[] PROGMEM =  "><a href=\"http://";
const char NAV_LEFT_PART2b[] PROGMEM =  "\">Home</a></li>\n<li ";
const char NAV_LEFT_PART3a[] PROGMEM = "><a href=\"http://";
const char NAV_LEFT_PART3b[] PROGMEM =  "/CONFIGSYS\">Configuration System</a></li>\n</ul>\n";
 
const char NAV_RIGHT_PART[] PROGMEM =  "<p class=\"navbar-text navbar-right\">&nbsp;&nbsp;&nbsp;&nbsp;</p>\n<ul class=\"nav navbar-nav navbar-right\">\n"\
                                                "<li><a href=\""  REPOSITORY "\">Github</a></li>\n</ul>\n"\
                                                "<p class=\"navbar-text navbar-right\">FW: " FW_VERSION "</p>\n";

const char NAV_END[] PROGMEM = "</div>\n</nav>";
                                      
const char PAGE_BOTTOM[] PROGMEM =  "</body>\n</html>\n" ;

const char PANEL_TOP[] PROGMEM = "<div class=\"panel panel-default\">\n<div class=\"panel-heading\">\n<h3 class=\"panel-title\">";
const char PANEL_START[] PROGMEM ="</h3>\n</div>\n<div class=\"panel-body\">";
const char PANEL_END[] PROGMEM = "</div>\n</div>";
const char LABEL_START[] PROGMEM = "<label>";
const char LABEL_COLOR[] PROGMEM = "</label><label class=\"text-info\">";
const char LABEL_END[] PROGMEM = "</label>";
const char BR[] PROGMEM = "<BR>\n";
const char TABLE_START[] PROGMEM = "<div class=\"table-responsive\">\n<table class=\"table table-bordered table-striped\">";
const char TABLE_END [] PROGMEM = "</table></div>\n";
const char CAPTION_S [] PROGMEM = "<caption>\n";
const char THEAD_S[] PROGMEM = "<thead>\n";
const char  TR_S[] PROGMEM = "<tr>\n";
const char  TH_S[] PROGMEM = "<th>\n";
const char  TH_R[] PROGMEM = "<th scope=\"row\">\n";
const char  TD_S[] PROGMEM = "<td>\n";
const char  TBODY_S[] PROGMEM = "<tbody>\n";
const char CAPTION_E [] PROGMEM = "</caption>\n";
const char  THEAD_E[] PROGMEM = "</thead>\n";
const char  TR_E[] PROGMEM = "</tr>\n";
const char  TH_E[] PROGMEM = "</th>\n";
const char  TD_E[] PROGMEM = "</td>\n";
const char  TBODY_E[] PROGMEM = "</tbody>\n";
const char  T404_PAGE[] PROGMEM = "<H1>Page not found!</H1><BR>Please try <a href=http://";
const char  T404_PAGE_2[] PROGMEM = ">here</a>";
const char  FORM_START[] PROGMEM = "<div class=\"col-sm-10\"><form METHOD=POST>\n";
const char  FORM_END[] PROGMEM = "</form></div>\n";
const char  FORM_INPUT_1[]  PROGMEM = "<div class=\"form-group\">\b<label for=\"";
const char  FORM_INPUT_1_ERROR[]  PROGMEM = "<div class=\"form-group has-error\">\b<label for=\"";
const char  FORM_INPUT_2[]  PROGMEM = "\">";
const char  FORM_INPUT_3[]  PROGMEM = "</label><BR>\n<input type=\"text\" class=\"form-control\" id=\"";
const char  FORM_INPUT_4[]  PROGMEM = "\" name=\"";
const char  FORM_INPUT_5[]  PROGMEM = "\"placeholder=\"";
const char  FORM_INPUT_6[]  PROGMEM = "\" value=\"";
const char  FORM_INPUT_7[]  PROGMEM = "\"style=\"width: auto;\"></div>\n";
const char FORM_SELECT_1[] PROGMEM = "</label><BR>\n<select name=\"";
const char FORM_SELECT_2[] PROGMEM = "\" id=\"";
const char FORM_SELECT_3[] PROGMEM = "\" class=\"form-control\" style=\"width:auto;\">\n";
const char FORM_SELECT_END[] PROGMEM = "</select></div>\n";
const char FORM_OPTION_1[] PROGMEM = "<option value=\"";
const char FORM_OPTION_2[] PROGMEM = "\" ";
const char FORM_OPTION_3[] PROGMEM = " >";
const char FORM_OPTION_4[] PROGMEM = "</option>\n";
const char FORM_SUBMIT[] PROGMEM = "<BR><HR><input type=\"submit\" class=\"btn btn-primary\" name=\"SUBMIT\" value=\"Apply\">\n";
const char  ALERT_SUCCESS[]  PROGMEM = "<div class=\"alert alert-success\" role=\"alert\">\n";
const char  ALERT_ERROR[]  PROGMEM = "<div class=\"alert alert-danger\" role=\"alert\">\n";
const char  DIV_E[]  PROGMEM = "</div>\n";
const char BAUD_RATE[] PROGMEM = "BAUD_RATE";
const char REFRESH_1_5[] PROGMEM ="<META HTTP-EQUIV=\"Refresh\" CONTENT=\"5; URL=http://";
const char REFRESH_1_15[] PROGMEM ="<META HTTP-EQUIV=\"Refresh\" CONTENT=\"15; URL=http://";
const char REFRESH_2[] PROGMEM ="\">";
#define REFRESH5(ip) web_interface.add4send(REFRESH_1_5);web_interface.add4send(ip);web_interface.add4send(REFRESH_2);
#define REFRESH15(ip) web_interface.add4send(REFRESH_1_15);web_interface.add4send(ip);web_interface.add4send(REFRESH_2);
#define MSG_SUCCESS(msg) web_interface.add4send(ALERT_SUCCESS);web_interface.add4send(msg);web_interface.add4send(DIV_E);
#define MSG_ERROR(msg) web_interface.add4send(ALERT_ERROR);web_interface.add4send(msg);web_interface.add4send(DIV_E);
#define OPTION(value, selected,content) web_interface.add4send(FORM_OPTION_1);web_interface.add4send(value);web_interface.add4send(FORM_OPTION_2);web_interface.add4send(selected);web_interface.add4send(FORM_OPTION_3);web_interface.add4send(content);web_interface.add4send(FORM_OPTION_4);
#define SELECT_START(id,label,name) web_interface.add4send(FORM_INPUT_1);web_interface.add4send(id);web_interface.add4send(FORM_INPUT_2);web_interface.add4send(label);web_interface.add4send(FORM_SELECT_1);web_interface.add4send(name);web_interface.add4send(FORM_SELECT_2);web_interface.add4send(id);web_interface.add4send(FORM_SELECT_3);
#define SELECT_START_ERROR(id,label,name) web_interface.add4send(FORM_INPUT_1_ERROR);web_interface.add4send(id);web_interface.add4send(FORM_INPUT_2);web_interface.add4send(label);web_interface.add4send(FORM_SELECT_1);web_interface.add4send(name);web_interface.add4send(FORM_SELECT_2);web_interface.add4send(id);web_interface.add4send(FORM_SELECT_3);
#define SELECT_END  web_interface.add4send(FORM_SELECT_END);
#define INPUT_TEXT( id,label, name,placeholder,value)  web_interface.add4send(FORM_INPUT_1);web_interface.add4send(id);web_interface.add4send(FORM_INPUT_2);web_interface.add4send(label);web_interface.add4send(FORM_INPUT_3);web_interface.add4send(id);web_interface.add4send(FORM_INPUT_4);web_interface.add4send(name);web_interface.add4send(FORM_INPUT_5);web_interface.add4send(placeholder);web_interface.add4send(FORM_INPUT_6);web_interface.add4send(value);web_interface.add4send(FORM_INPUT_7);
#define INPUT_TEXT_ERROR( id,label, name,placeholder,value)  web_interface.add4send(FORM_INPUT_1_ERROR);web_interface.add4send(id);web_interface.add4send(FORM_INPUT_2);web_interface.add4send(label);web_interface.add4send(FORM_INPUT_3);web_interface.add4send(id);web_interface.add4send(FORM_INPUT_4);web_interface.add4send(name);web_interface.add4send(FORM_INPUT_5);web_interface.add4send(placeholder);web_interface.add4send(FORM_INPUT_6);web_interface.add4send(value);web_interface.add4send(FORM_INPUT_7);
#define LABEL( title, value)  web_interface.add4send(LABEL_START); web_interface.add4send(title);web_interface.add4send(LABEL_COLOR);web_interface.add4send(value);web_interface.add4send(LABEL_END);web_interface.add4send(BR);
#define LABEL_UNITS(title, value,units) web_interface.add4send(LABEL_START);  web_interface.add4send(title);web_interface.add4send(LABEL_COLOR);web_interface.add4send(value);web_interface.add4send(units);web_interface.add4send(LABEL_END);web_interface.add4send(BR);
#define TH_ENTRY(entry) web_interface.add4send(TH_S);web_interface.add4send(entry);web_interface.add4send(TH_E);
#define THR_ENTRY(entry) web_interface.add4send(TH_R);web_interface.add4send(entry);web_interface.add4send(TH_E);
#define TD_ENTRY(entry) web_interface.add4send(TD_S);web_interface.add4send(entry);web_interface.add4send(TD_E);

//cannot put it in class then cast it as std::function<void(void)> so put outside
void handle_web_interface_root()
{
  String IP;
  String sstatus;
  struct softap_config apconfig;
  struct ip_info info;
   int istatus;
  uint8_t mac [WL_MAC_ADDR_LENGTH];
  if (wifi_get_opmode()==WIFI_STA ) IP=wifi_config.ip2str(WiFi.localIP());
  else IP=wifi_config.ip2str(WiFi.softAPIP());
  web_interface.add4send(PAGE_HEAD_1);
  web_interface.add4send(PAGE_HEAD_2);
  //top bar
  web_interface.add4send(NAV_START);
  web_interface.add4send(NAV_LEFT_PART1) ;
  web_interface.add4send(NAV_ELEMENT_ACTIVE) ;
  web_interface.add4send(NAV_LEFT_PART2a );
  web_interface.add4send(IP.c_str());
  web_interface.add4send(NAV_LEFT_PART2b );
  web_interface.add4send(NAV_LEFT_PART3a );
  web_interface.add4send(IP.c_str());
  web_interface.add4send(NAV_LEFT_PART3b) ;
  web_interface.add4send(NAV_RIGHT_PART) ;
  web_interface.add4send(NAV_END) ;
 //system part
  web_interface.add4send(PANEL_TOP);
  web_interface.add4send(F("System"));
  web_interface.add4send(PANEL_START);
  LABEL(F("Chip ID: "),String(system_get_chip_id()).c_str())
  LABEL_UNITS(F("CPU Frequency: "),String(system_get_cpu_freq()).c_str(),F("Hz"))
  LABEL_UNITS(F("Free Memory: "),String(system_get_free_heap_size()).c_str(),F(" octets"))
  LABEL(F("SDK Version: "),system_get_sdk_version())
  #ifdef MDNS_FEATURE
  sstatus = F("http://");
  sstatus+=LOCAL_NAME;
  LABEL_UNITS(F("mDNS name: "),sstatus.c_str(),F(".local"))
  #endif
  istatus = wifi_get_phy_mode();
  if (istatus==PHY_MODE_11B) sstatus=F("11b");
  else if (istatus==PHY_MODE_11G) sstatus=F("11g");
  else sstatus=F("11n");
  LABEL(F("Network: "),sstatus.c_str())
  istatus = wifi_get_sleep_type();
  if (istatus==NONE_SLEEP_T) sstatus=F("None");
  else if (istatus==LIGHT_SLEEP_T) sstatus=F("Light");
  else  sstatus=F("Modem");
  LABEL(F("Sleep mode: "),sstatus.c_str())
  //LABEL(sbuf,F("Boot mode: "),String(system_get_boot_mode())) //no meaning so far
  LABEL(F("Boot version: "),String(system_get_boot_version()).c_str())
  istatus=0;
  if (!CONFIG::read_buffer(EP_BAUD_RATE,  (byte *)&istatus , BAUD_LENGH))istatus=0;
  LABEL(F("Baud rate: "),String(istatus).c_str())
  web_interface.add4send(PANEL_END);
 //access point
  web_interface.add4send(PANEL_TOP);
  web_interface.add4send(F("Access Point"));
  if(wifi_get_opmode()==WIFI_AP ||  wifi_get_opmode()==WIFI_AP_STA) web_interface.add4send(F(" (enabled)"));
  else web_interface.add4send(F(" (disabled)"));
  web_interface.add4send(PANEL_START);
  LABEL(F("Mac address: "),wifi_config.mac2str(WiFi.softAPmacAddress(mac)))
  if (wifi_get_opmode()==WIFI_AP ||  wifi_get_opmode()==WIFI_AP_STA) 
	{
	  if (wifi_softap_get_config(&apconfig))
		{
		  LABEL(F("SSID: "),(char *)(apconfig.ssid))
		  if(apconfig.ssid_hidden==1)sstatus=F("No");
		  else sstatus=F("Yes");
		  LABEL(F("Visible: "),sstatus.c_str())
		  LABEL(F("Channel: "),String(apconfig.channel).c_str())
		  if (apconfig.authmode==AUTH_OPEN)sstatus=F("None");
		  else if (apconfig.authmode==AUTH_WEP)sstatus=F("WEP");
		  else if (apconfig.authmode==AUTH_WPA_PSK)sstatus=F("WPA");
		  else if (apconfig.authmode==AUTH_WPA2_PSK)sstatus=F("WPA2");
		  else if (apconfig.authmode==AUTH_WPA_WPA2_PSK)sstatus=F("WPA/WPA2");
		  else sstatus=F("MAX"); //what is this one ? WPS ? Cannot find information
		  LABEL(F("Authentification: "),sstatus.c_str())
		  LABEL(F("Maximum connections : "),String(apconfig.max_connection).c_str())
		}
		 if (wifi_softap_dhcps_status()==DHCP_STARTED)sstatus=F("Started");
		 else sstatus=F("Stopped");
		 LABEL(F("DHCP Server: "),sstatus.c_str())
		 if (wifi_get_ip_info(SOFTAP_IF,&info))
			{
			  LABEL(F("IP: "),wifi_config.ip2str(info.ip.addr))
			  LABEL(F("Gateway: "),wifi_config.ip2str(info.gw.addr))
			  LABEL(F("Subnet: "),wifi_config.ip2str(info.netmask.addr))
			}
		//List number of client
		istatus = 0;
		sstatus="";
		struct station_info * station = wifi_softap_get_station_info();
		struct station_info * stationtmp = station;
		//get total number of connected clients
		while(stationtmp)
			{
			istatus++;
			stationtmp = STAILQ_NEXT(stationtmp, next);
			}
		//start table as at least one connected
		 web_interface.add4send(TABLE_START);
		 web_interface.add4send(CAPTION_S);
		 web_interface.add4send(String(istatus).c_str());
		 web_interface.add4send(F(" connected station(s)"));
		 web_interface.add4send(CAPTION_S);
		 web_interface.add4send(THEAD_S);
		 web_interface.add4send(TR_S);
		TH_ENTRY(F("#"))
		TH_ENTRY(F("Mac"))
		TH_ENTRY(F("IP"))
		web_interface.add4send(TR_E);
		web_interface.add4send(THEAD_E);
		web_interface.add4send(TBODY_S);
		istatus=0;
		while(station)
			{
			istatus++;
			//display each client
			 web_interface.add4send(TR_S);
			 THR_ENTRY(String(istatus).c_str())
			 TD_ENTRY(wifi_config.mac2str(station->bssid))
			 static char ipstr [16];
			 if (0>sprintf(ipstr, IPSTR,IP2STR(&station->ip))) strcpy (ipstr, F("0.0.0.0"));
			 TD_ENTRY(ipstr)
			 web_interface.add4send(TR_E);
			station = STAILQ_NEXT(station, next);
			}
		 web_interface.add4send(TBODY_E);
		//close table
		 web_interface.add4send(TABLE_END);
		 wifi_softap_free_station_info();
	}
  web_interface.add4send(PANEL_END);
  web_interface.add4send(PANEL_TOP);
  web_interface.add4send(F("Station"));
  if(wifi_get_opmode()==WIFI_STA ||  wifi_get_opmode()==WIFI_AP_STA) web_interface.add4send(F(" (enabled)"));
  else web_interface.add4send(F(" (disabled)"));
  web_interface.add4send(PANEL_START);
  LABEL(F("Mac address: "),wifi_config.mac2str(WiFi.macAddress(mac)))
   if(wifi_get_opmode()==WIFI_STA ||  wifi_get_opmode()==WIFI_AP_STA)
   {
	  LABEL(F("Connection to: "),WiFi.SSID())
	  LABEL(F("Channel: "),String(wifi_get_channel()).c_str())
	  istatus = wifi_station_get_connect_status();
	  if (istatus==STATION_GOT_IP) sstatus=F("Connected");
	  else if  (istatus==STATION_NO_AP_FOUND) sstatus=F("SSID not Available!");
	  else if  (istatus==STATION_CONNECT_FAIL) sstatus=F("Connecion failed!");
	  else if  (istatus==STATION_WRONG_PASSWORD) sstatus=F("Connecion failed! (Wrong Password)");
	  else if  (istatus==STATION_IDLE) sstatus=F("Idle");//should not happen
	  else sstatus=F("Disconnected");
	  LABEL(F("Status: "),sstatus.c_str())
	  if (wifi_station_dhcpc_status()==DHCP_STARTED)sstatus=F("Started");
	  else sstatus=F("Stopped");
	  LABEL(F("DHCP Client: "),sstatus.c_str())
	  LABEL(F("IP: "),wifi_config.ip2str(WiFi.localIP()))
	  LABEL(F("Gateway: "),wifi_config.ip2str(WiFi.gatewayIP()))
	  LABEL(F("Subnet: "),wifi_config.ip2str(WiFi.subnetMask()))
	 }
  web_interface.add4send(PANEL_END);
  web_interface.add4send(PAGE_BOTTOM);
  web_interface.flushbuffer(); 
}

void handle_web_interface_configSys()
{
  String stmp,smsg;
  int istatus=0;
  bool msg_alert_error=false;
  bool msg_alert_success=false;
  int ibaud=0;
  //check is it is a submission or a display
  if (web_interface.WebServer.hasArg(F("SUBMIT")))
  {   //is there a baude rate
	  if (web_interface.WebServer.hasArg(BAUD_RATE))
		{   //is value correct ?
			ibaud  = atoi(web_interface.WebServer.arg(BAUD_RATE).c_str());
			if (!(ibaud==9600 || ibaud==19200|| ibaud==38400|| ibaud==57600|| ibaud==115200|| ibaud==230400))
				{
					msg_alert_error=true;
					smsg=F("Error in query!!");
			    }
	  }
	  else
	   {
		msg_alert_error=true;
		smsg=F("Error in query!!");
	   }
	   //if no error apply the changes
		if (msg_alert_error!=true)
			{
			 if(!CONFIG::write_buffer(EP_BAUD_RATE,(const byte *)&ibaud,BAUD_LENGH))
				{
					msg_alert_error=true;
					smsg=F("Error in writing changes!!");
				}
			else
			if (!msg_alert_error)
				{
					msg_alert_success=true;
					smsg=F("Change saved, restarting module...");
				}
			}
		
  }
  if (!CONFIG::read_buffer(EP_BAUD_RATE,  (byte *)&istatus , BAUD_LENGH))istatus=0;
  if (wifi_get_opmode()==WIFI_STA ) stmp=wifi_config.ip2str(WiFi.localIP());
  else stmp=wifi_config.ip2str(WiFi.softAPIP());
  web_interface.add4send(PAGE_HEAD_1);
  web_interface.add4send(PAGE_HEAD_2);
  web_interface.add4send(NAV_START) ;
  web_interface.add4send(NAV_LEFT_PART1) ;
  web_interface.add4send(NAV_LEFT_PART2a) ;
  web_interface.add4send(stmp.c_str());
  web_interface.add4send(NAV_LEFT_PART2b);
  web_interface.add4send(NAV_ELEMENT_ACTIVE) ;
  web_interface.add4send(NAV_LEFT_PART3a) ;
  web_interface.add4send(stmp.c_str());
  web_interface.add4send(NAV_LEFT_PART3b) ;
  web_interface.add4send(NAV_RIGHT_PART) ;
  web_interface.add4send(NAV_END) ;
  web_interface.add4send(PANEL_TOP);
  web_interface.add4send(F("System"));
  web_interface.add4send(PANEL_START);
  web_interface.add4send(FORM_START);
 
  SELECT_START(F("SYS1"),F("Baud rate"),F("BAUD_RATE"))
  if (istatus==9600)stmp = F("selected");
  else stmp="";
  OPTION(F("9600"), stmp.c_str(),F("9600"))
   if (istatus==19200)stmp = F("selected");
  else stmp="";
  OPTION(F("19200"), stmp.c_str(),F("19200"))
   if (istatus==38400)stmp = F("selected");
  else stmp="";
  OPTION(F("38400"), stmp.c_str(),F("38400"))
   if (istatus==57600)stmp = F("selected");
  else stmp="";
  OPTION(F("57600"), stmp.c_str(),F("57600"))
   if (istatus==115200)stmp = F("selected");
  else stmp="";
  OPTION(F("115200"), stmp.c_str(),F("115200"))
   if (istatus==230400)stmp = F("selected");
  else stmp="";
  OPTION(F("230400"), stmp.c_str(),F("230400"))
  SELECT_END
  
  web_interface.add4send(FORM_SUBMIT); 
  web_interface.add4send(FORM_END);
  web_interface.add4send(PANEL_END); 
  if(msg_alert_error) 
	{
		MSG_ERROR(smsg.c_str()) 
	}
if(msg_alert_success) 
	{
		MSG_SUCCESS(smsg.c_str()) 
	}
  web_interface.add4send(PAGE_BOTTOM);
  web_interface.flushbuffer();
  if (msg_alert_success && !msg_alert_error)
	{
		system_restart();
	}
}

void handle_not_found()
{
  String IP;
  if (wifi_get_opmode()==WIFI_STA ) IP=wifi_config.ip2str(WiFi.localIP());
  else IP=wifi_config.ip2str(WiFi.softAPIP());
	 web_interface.add4send(T404_PAGE);
	 web_interface.add4send(IP.c_str());
	 web_interface.add4send(T404_PAGE_2);
	 web_interface.flushbuffer();
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

//send immediatly the current data
void WEBINTERFACE_CLASS::flushbuffer()
{   //empty buffer
	 web_interface.add4send("", true);
	 //reset header part
	 bsendingstarted=false;
}

//check size of buffer 
//add to buffer if under buffer limit
//send if buffer limit is reached or will be reached
void WEBINTERFACE_CLASS::add4send(const char * data2send, bool bimmediatsend)
{
	//we store as no need to send right now
	if (((buffer2send.length() + strlen(data2send)) <LIMIT_BUFFER) && !bimmediatsend)
		{
			buffer2send = buffer2send + data2send;
		}
	else //or buffer is over or need immediate
		{//send current buffer first
		  if(!bsendingstarted)
			{ //no header send so first send using header 2500 limit allow to add extra without calculation
				web_interface.WebServer.send(200, F("text/html"), buffer2send);
				bsendingstarted=true;
			}
		 else
			{ //direct data send
				web_interface.WebServer.client().print(buffer2send);
			}
		buffer2send=data2send;
		//do we need to purge or to store ?
		if (bimmediatsend)
			{//if some data flush
				if (buffer2send.length()>0)
					{
					web_interface.WebServer.client().print(buffer2send);
					//reset buffer
					buffer2send="";
					}
			//reset header
			bsendingstarted=false;
			}
		}
}

//constructor
WEBINTERFACE_CLASS::WEBINTERFACE_CLASS (int port):WebServer(port)
{
  //init what will handle "/"
  WebServer.on(F("/"),HTTP_ANY, handle_web_interface_root);
  WebServer.on(F("/CONFIGSYS"),HTTP_ANY, handle_web_interface_configSys);
  WebServer.onNotFound( handle_not_found);
  buffer2send="";
}

WEBINTERFACE_CLASS web_interface(80);

