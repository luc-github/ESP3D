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

#include <PgmSpace.h>
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



#define LIMIT_BUFFER		3500

const char  PAGE_HEAD_1[] PROGMEM =  "<html lang=\"en\">\n<head>\n<meta charset=\"utf-8\">\n<meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\">\n" \
                                      "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n";
const char  PAGE_HEAD_2[] PROGMEM = "<title>Wifi Configuration</title>" \
                                      "<link rel=\"stylesheet\" href=\"http://maxcdn.bootstrapcdn.com/bootstrap/3.3.4/css/bootstrap.min.css\">\n</head>\n<body>"\
                                      " <div class=\"container theme-showcase\" role=\"main\">";
const char NAV_START[] PROGMEM =  "<nav class=\"navbar navbar-inverse\">\n<div class=\"container\">\n";
 
const char NAV_LEFT_PART1 [] PROGMEM =  "<ul class=\"nav navbar-nav navbar-left\">\n<li ";
const char NAV_ELEMENT_ACTIVE [] PROGMEM =  "class=\"active\"";
const char NAV_LEFT_PART2a[] PROGMEM =  "><a href=\"http://";
const char NAV_LEFT_PART2b[] PROGMEM =  "\">Home</a></li>\n<li ><a href=\"http://";
const char NAV_LEFT_PART3b[] PROGMEM =  "/CONFIGSYS\">System Configuration</a></li>\n<li ><a href=\"http://";
const char NAV_LEFT_PART4b[] PROGMEM =  "/CONFIGAP\">AP Configuration</a></li>\n";
const char NAV_LEFT_PARTEND[] PROGMEM =  "</ul>\n";
 
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
const char TR_S[] PROGMEM = "<tr>\n";
const char TH_S[] PROGMEM = "<th>\n";
const char TH_R[] PROGMEM = "<th scope=\"row\">\n";
const char TD_S[] PROGMEM = "<td>\n";
const char TBODY_S[] PROGMEM = "<tbody>\n";
const char CAPTION_E [] PROGMEM = "</caption>\n";
const char THEAD_E[] PROGMEM = "</thead>\n";
const char TR_E[] PROGMEM = "</tr>\n";
const char TH_E[] PROGMEM = "</th>\n";
const char TD_E[] PROGMEM = "</td>\n";
const char TBODY_E[] PROGMEM = "</tbody>\n";
const char T404_PAGE[] PROGMEM = "<H1>Page not found!</H1><BR>Please try <a href=http://";
const char T404_PAGE_2[] PROGMEM = ">here</a>";
const char FORM_START[] PROGMEM = "<div class=\"col-sm-10\"><form METHOD=GET>\n";
const char FORM_END[] PROGMEM = "</form></div>\n";
const char FORM_INPUT_1[]  PROGMEM = "<div class=\"form-group\">\b<label for=\"";
const char FORM_INPUT_1_ERROR[]  PROGMEM = "<div class=\"form-group has-error\">\b<label class=\"control-label\" for=\"";
const char FORM_INPUT_2[]  PROGMEM = "\">";
const char FORM_INPUT_3[]  PROGMEM = "</label><BR>\n<input type=\"text\" class=\"form-control\" id=\"";
const char FORM_INPUT_3P[]  PROGMEM = "</label><BR>\n<input type=\"password\" class=\"form-control\" id=\"";
const char FORM_INPUT_4[]  PROGMEM = "\" name=\"";
const char FORM_INPUT_5[]  PROGMEM = "\" placeholder=\"";
const char FORM_INPUT_6[]  PROGMEM = "\" value=\"";
const char FORM_INPUT_7[]  PROGMEM = "\" style=\"width: auto;\" ></div>\n";

const char FORM_CHECKBOX_1[]  PROGMEM ="<div class=\"checkbox\"><label>\n<input type=\"checkbox\" NAME=\"";  
const char FORM_CHECKBOX_2[]  PROGMEM = "\" ";
const char FORM_CHECKBOX_3[]  PROGMEM = " >";
 const char FORM_CHECKBOX_4[]  PROGMEM = "\n</label>\n</div>\n";
const char FORM_SELECT_1[] PROGMEM = "</label><BR>\n<select name=\"";
const char FORM_SELECT_2[] PROGMEM = "\" id=\"";
const char FORM_SELECT_3[] PROGMEM = "\" class=\"form-control\" style=\"width:auto;\">\n";
const char FORM_SELECT_END[] PROGMEM = "</select></div>\n";
const char FORM_OPTION_1[] PROGMEM = "<option value=\"";
const char FORM_OPTION_2[] PROGMEM = "\" ";
const char FORM_OPTION_3[] PROGMEM = " >";
const char FORM_OPTION_4[] PROGMEM = "</option>\n";
const char FORM_SUBMIT[] PROGMEM = "<BR><HR><input type=\"submit\" class=\"btn btn-primary\" name=\"SUBMIT\" value=\"Apply\">\n";
const char ALERT_SUCCESS[]  PROGMEM = "<div class=\"alert alert-success\" role=\"alert\">\n";
const char ALERT_ERROR[]  PROGMEM = "<div class=\"alert alert-danger\" role=\"alert\">\n";
const char DIV_E[]  PROGMEM = "</div>\n";
const char BAUD_RATE_ID[] PROGMEM = "BAUD_RATE";
const char SLEEP_MODE_ID[] PROGMEM = "SLEEP_MODE";
const char NETWORK_ID[] PROGMEM = "NETWORK";
const char TITLE_SYSTEM[] PROGMEM = "System";
const char CHIP_ID_TITLE[] PROGMEM = "Chip ID: ";
const char CPU_FREQ_TITLE[] PROGMEM = "CPU Frequency: ";
const char FREE_MEM_TITLE[] PROGMEM = "Free Memory: ";
const char UNIT_HZ [] PROGMEM = "Hz";
const char UNIT_OCTET[] PROGMEM = " octets";
const char SDK_VERSION_TITLE[] PROGMEM = "SDK Version: ";
const char HTTP_START[] PROGMEM = "http://";
const char HTTP_MDNS_NAME[] PROGMEM = "mDNS name: ";
const char HTTP_END[] PROGMEM = ".local";
const char VALUE_11B[] PROGMEM = "11b";
const char VALUE_11N[] PROGMEM = "11n";
const char VALUE_11G[] PROGMEM = "11g";
const char NETWORK_TITLE[] PROGMEM = "Network: ";
const char VALUE_NONE[] PROGMEM = "None";
const char VALUE_LIGHT[] PROGMEM = "Light";
const char VALUE_MODEM[] PROGMEM = "Modem";
const char SLEEP_MODE_TITLE[] PROGMEM = "Sleep mode: ";
const char BOOT_VERSION_TITLE[] PROGMEM = "Boot version: ";
const char BAUD_RATE_TITLE[] PROGMEM = "Baud rate: ";
const char ACCESS_POINT_TITLE[] PROGMEM = "Access Point";
const char VALUE_ENABLED[] PROGMEM = " (enabled)";
const char VALUE_DISABLED[] PROGMEM = " (disabled)";
const char MAC_ADDRESS_TITLE[] PROGMEM = "Mac address: ";
const char MAC_LABEL[] PROGMEM = "Mac";
const char SSID_TITLE[] PROGMEM = "SSID: ";
const char VALUE_NO[] PROGMEM = "No";
const char VALUE_YES[] PROGMEM = "Yes";
const char VISIBLE_TITLE[] PROGMEM = "Visible: ";
const char CHANNEL_TITLE[] PROGMEM = "Channel: ";
const char STATUS_TITLE[] PROGMEM = "Status: ";
const char VALUE_WEP[] PROGMEM = "WEP";
const char VALUE_WPA[] PROGMEM = "WPA";
const char VALUE_WPA2[] PROGMEM = "WPA2";
const char VALUE_WPAWPA2[] PROGMEM = "WPA/WPA2";
const char VALUE_MAX[] PROGMEM = "MAX";
const char AUTENTIFICATION_TITLE[] PROGMEM = "Authentification: ";
const char MAX_CONNECTION_TITLE[] PROGMEM = "Maximum connections : ";
const char VALUE_STARTED[] PROGMEM = "Started";
const char VALUE_STOPPED[] PROGMEM = "Stopped";
const char DHCP_SERVER_TITLE[] PROGMEM = "DHCP Server: ";
const char IP_TITLE[] PROGMEM = "IP: ";
const char GATEWAY_TITLE[] PROGMEM = "Gateway: ";
const char SUBNET_TITLE[] PROGMEM = "Subnet: ";
const char CONNECTED_STATIONS[] PROGMEM = " connected station(s)";
const char NUMBER_LABEL[] PROGMEM = "#";
const char IP_LABEL[] PROGMEM = "IP";
const char STATION_TITLE[] PROGMEM = "Station";
const char CONNECTION_TO_TITLE[] PROGMEM = "Connection to: ";
const char VALUE_CONNECTED[] PROGMEM = "Connected";
const char VALUE_NO_SSID[] PROGMEM = "SSID not Available!";
const char VALUE_CONNECTION_FAILED[] PROGMEM = "Connection failed!";
const char VALUE_CONNECTION_FAILED2[] PROGMEM = "Connection failed! (Wrong Password)";
const char VALUE_IDLE[] PROGMEM = "Idle";
const char VALUE_DISCONNECTED[] PROGMEM = "Disconnected";
const char DHCP_CLIENT_TITLE[] PROGMEM = "DHCP Client: ";
const char ERROR_QUERY[] PROGMEM = "Error in query!!";
const char ERROR_WRITING_CHANGES[] PROGMEM = "Error in writing changes!!";
const char SAVED_CHANGES[] PROGMEM = "Change saved, restarting module...";
const char SUBMIT_ID[] PROGMEM = "SUBMIT";
const char SYS_1_ID[] PROGMEM = "SYS1";
const char BAUD_RATE_NAME[] PROGMEM = "Baud rate";
const char NETWORK_NAME[] PROGMEM = "Network";
const char SLEEP_MODE_NAME[] PROGMEM = "Sleep Mode";
const char VALUE_9600[] PROGMEM = "9600";
const char VALUE_19200[] PROGMEM = "19200";
const char VALUE_38400[] PROGMEM = "38400";
const char VALUE_57600[] PROGMEM = "57600";
const char VALUE_115200[] PROGMEM = "115200";
const char VALUE_230400[] PROGMEM = "230400";
const char SYS_2_ID[] PROGMEM = "SYS2";
const char SYS_3_ID[] PROGMEM = "SYS3";
const char VALUE_SELECTED[] PROGMEM = "selected";
const char AP_1_ID[] PROGMEM = "AP1";
const char AP_2_ID[] PROGMEM = "AP2";
const char AP_3_ID[] PROGMEM = "AP3";
const char AP_4_ID[] PROGMEM = "AP4";
const char AP_5_ID[] PROGMEM = "AP5";
const char AP_6_ID[] PROGMEM = "AP6";
const char AP_7_ID[] PROGMEM = "AP7";
const char AP_8_ID[] PROGMEM = "AP8";
const char AP_9_ID[] PROGMEM = "AP9";
const char AP_10_ID[] PROGMEM = "AP10";
const char SSID_ID[] PROGMEM = "SSID";
const char PASSWORD_TITLE[] PROGMEM = "Password :";
const char PASSWORD_NAME[] PROGMEM = "Password";
const char PASSWORD_ID[] PROGMEM = "PASSWORD";
const char CHECKED_VALUE[] PROGMEM = "checked";
const char VISIBLE_NAME[] PROGMEM = "VISIBLE";
const char VISIBLE_LABEL[] PROGMEM = "Visible";
const char AUTENTIFICATION_ID[] PROGMEM = "AUTHENTIFICATION";
const char STATIC_IP_LABEL[] PROGMEM = "Static IP";
const char STATIC_IP_NAME[] PROGMEM = "STATIC_IP";
const char IP_NAME[] PROGMEM = "IP";
const char GATEWAY_ID[] PROGMEM = "GATEWAY";
const char CHANNEL_ID[] PROGMEM = "CHANNEL";
const char SUBNET_ID[] PROGMEM = "SUBNET";
const char GATEWAY_NAME[] PROGMEM = "Gateway";
const char SUBNET_NAME[] PROGMEM = "Subnet";
const char ERROR_INCORRECT_SSID[] PROGMEM = "Incorrect SSID :only char and digit, no space, limited to 33 char length";
const char ERROR_INCORRECT_PASSWORD[] PROGMEM = "Incorrect password : space not allowed, limited to 8~64 char length<BR>";
const char ERROR_INCORRECT_IP_FORMAT[] PROGMEM = "Incorrect IP format, should be : xxx.xxx.xxx.xxx<BR>";
const char SHOW_IP_BLOCK[] PROGMEM = "<div NAME=\"IP_BLOCK\" >";
const char HIDE_IP_BLOCK[] PROGMEM = "<div NAME=\"IP_BLOCK\" style=\"visibility:none;\">";

#define MSG_SUCCESS(msg) web_interface.add4send(PROGMEM2CHAR(ALERT_SUCCESS));web_interface.add4send(msg);web_interface.add4send(PROGMEM2CHAR(DIV_E));
#define MSG_ERROR(msg) web_interface.add4send(PROGMEM2CHAR(ALERT_ERROR));web_interface.add4send(msg);web_interface.add4send(PROGMEM2CHAR(DIV_E));
#define OPTION(value, selected,content) web_interface.add4send(PROGMEM2CHAR(FORM_OPTION_1));web_interface.add4send(value);web_interface.add4send(PROGMEM2CHAR(FORM_OPTION_2));web_interface.add4send(selected);web_interface.add4send(PROGMEM2CHAR(FORM_OPTION_3));web_interface.add4send(content);web_interface.add4send(PROGMEM2CHAR(FORM_OPTION_4));
#define SELECT_START(id,label,name) web_interface.add4send(PROGMEM2CHAR(FORM_INPUT_1));web_interface.add4send(id);web_interface.add4send(PROGMEM2CHAR(FORM_INPUT_2));web_interface.add4send(label);web_interface.add4send(PROGMEM2CHAR(FORM_SELECT_1));web_interface.add4send(name);web_interface.add4send(PROGMEM2CHAR(FORM_SELECT_2));web_interface.add4send(id);web_interface.add4send(PROGMEM2CHAR(FORM_SELECT_3));
#define SELECT_START_ERROR(id,label,name) web_interface.add4send(PROGMEM2CHAR(FORM_INPUT_1_ERROR));web_interface.add4send(id);web_interface.add4send(PROGMEM2CHAR(FORM_INPUT_2));web_interface.add4send(label);web_interface.add4send(PROGMEM2CHAR(FORM_SELECT_1));web_interface.add4send(name);web_interface.add4send(PROGMEM2CHAR(FORM_SELECT_2));web_interface.add4send(id);web_interface.add4send(PROGMEM2CHAR(FORM_SELECT_3));
#define SELECT_END  web_interface.add4send(PROGMEM2CHAR(FORM_SELECT_END));
#define INPUT_TEXT( id,label, name,placeholder,value)  web_interface.add4send(PROGMEM2CHAR(FORM_INPUT_1));web_interface.add4send(id);web_interface.add4send(PROGMEM2CHAR(FORM_INPUT_2));web_interface.add4send(label);web_interface.add4send(PROGMEM2CHAR(FORM_INPUT_3));web_interface.add4send(id);web_interface.add4send(PROGMEM2CHAR(FORM_INPUT_4));web_interface.add4send(name);web_interface.add4send(PROGMEM2CHAR(FORM_INPUT_5));web_interface.add4send(placeholder);web_interface.add4send(PROGMEM2CHAR(FORM_INPUT_6));web_interface.add4send(value);web_interface.add4send(PROGMEM2CHAR(FORM_INPUT_7));
#define INPUT_TEXT_ERROR( id,label, name,placeholder,value)  web_interface.add4send(PROGMEM2CHAR(FORM_INPUT_1_ERROR));web_interface.add4send(id);web_interface.add4send(PROGMEM2CHAR(FORM_INPUT_2));web_interface.add4send(label);web_interface.add4send(PROGMEM2CHAR(FORM_INPUT_3));web_interface.add4send(id);web_interface.add4send(PROGMEM2CHAR(FORM_INPUT_4));web_interface.add4send(name);web_interface.add4send(PROGMEM2CHAR(FORM_INPUT_5));web_interface.add4send(placeholder);web_interface.add4send(PROGMEM2CHAR(FORM_INPUT_6));web_interface.add4send(value);web_interface.add4send(PROGMEM2CHAR(FORM_INPUT_7));
#define INPUT_PASSWORD( id,label, name,placeholder,value)  web_interface.add4send(PROGMEM2CHAR(FORM_INPUT_1));web_interface.add4send(id);web_interface.add4send(PROGMEM2CHAR(FORM_INPUT_2));web_interface.add4send(label);web_interface.add4send(PROGMEM2CHAR(FORM_INPUT_3P));web_interface.add4send(id);web_interface.add4send(PROGMEM2CHAR(FORM_INPUT_4));web_interface.add4send(name);web_interface.add4send(PROGMEM2CHAR(FORM_INPUT_5));web_interface.add4send(placeholder);web_interface.add4send(PROGMEM2CHAR(FORM_INPUT_6));web_interface.add4send(value);web_interface.add4send(PROGMEM2CHAR(FORM_INPUT_7));
#define INPUT_PASSWORD_ERROR( id,label, name,placeholder,value)  web_interface.add4send(PROGMEM2CHAR(FORM_INPUT_1_ERROR));web_interface.add4send(id);web_interface.add4send(PROGMEM2CHAR(FORM_INPUT_2));web_interface.add4send(label);web_interface.add4send(PROGMEM2CHAR(FORM_INPUT_3P));web_interface.add4send(id);web_interface.add4send(PROGMEM2CHAR(FORM_INPUT_4));web_interface.add4send(name);web_interface.add4send(PROGMEM2CHAR(FORM_INPUT_5));web_interface.add4send(placeholder);web_interface.add4send(PROGMEM2CHAR(FORM_INPUT_6));web_interface.add4send(value);web_interface.add4send(PROGMEM2CHAR(FORM_INPUT_7));
#define INPUT_CHECKBOX(name,label,status)  web_interface.add4send(PROGMEM2CHAR(FORM_CHECKBOX_1));web_interface.add4send(name);web_interface.add4send(PROGMEM2CHAR(FORM_CHECKBOX_2));web_interface.add4send(status);web_interface.add4send(PROGMEM2CHAR(FORM_CHECKBOX_3));web_interface.add4send(label);web_interface.add4send(PROGMEM2CHAR(FORM_CHECKBOX_4));
#define LABEL( title, value)  web_interface.add4send(PROGMEM2CHAR(LABEL_START)); web_interface.add4send(title);web_interface.add4send(PROGMEM2CHAR(LABEL_COLOR));web_interface.add4send(value);web_interface.add4send(PROGMEM2CHAR(LABEL_END));web_interface.add4send(PROGMEM2CHAR(BR));
#define LABEL_UNITS(title, value,units) web_interface.add4send(PROGMEM2CHAR(LABEL_START));  web_interface.add4send(title);web_interface.add4send(PROGMEM2CHAR(LABEL_COLOR));web_interface.add4send(value);web_interface.add4send(units);web_interface.add4send(PROGMEM2CHAR(LABEL_END));web_interface.add4send(PROGMEM2CHAR(BR));
#define TH_ENTRY(entry) web_interface.add4send(PROGMEM2CHAR(TH_S));web_interface.add4send(entry);web_interface.add4send(PROGMEM2CHAR(TH_E));
#define THR_ENTRY(entry) web_interface.add4send(PROGMEM2CHAR(TH_R));web_interface.add4send(entry);web_interface.add4send(PROGMEM2CHAR(TH_E));
#define TD_ENTRY(entry) web_interface.add4send(PROGMEM2CHAR(TD_S));web_interface.add4send(entry);web_interface.add4send(PROGMEM2CHAR(TD_E));
#define TOPBAR(IP)  web_interface.add4send(PROGMEM2CHAR(NAV_START)); web_interface.add4send(PROGMEM2CHAR(NAV_LEFT_PART1)) ;  web_interface.add4send(PROGMEM2CHAR(NAV_ELEMENT_ACTIVE)) ;  web_interface.add4send(PROGMEM2CHAR(NAV_LEFT_PART2a) );  web_interface.add4send(IP);  web_interface.add4send(PROGMEM2CHAR(NAV_LEFT_PART2b)); web_interface.add4send(IP);  web_interface.add4send(PROGMEM2CHAR(NAV_LEFT_PART3b));  web_interface.add4send(IP);  web_interface.add4send(PROGMEM2CHAR(NAV_LEFT_PART4b));web_interface.add4send(PROGMEM2CHAR(NAV_LEFT_PARTEND)); web_interface.add4send(PROGMEM2CHAR(NAV_RIGHT_PART)) ;  web_interface.add4send(PROGMEM2CHAR(NAV_END));


char * progmem2char(const char* src)
{
  static char buffer[300];
  strcpy_P(buffer,src);
  return buffer;
}

bool WEBINTERFACE_CLASS::isSSIDValid(const char * ssid)
{  //limited size 
	char c;
	if (strlen(ssid)>MAX_SSID_LENGH || strlen(ssid)<MIN_SSID_LENGH) return false;
	//only letter and digit
	for (int i=0;i < strlen(ssid);i++)
		{
			c = ssid[i];
			if (!(isdigit(c) || isalpha(c))) return false;
		}
	return true;
}

bool WEBINTERFACE_CLASS::isPasswordValid(const char * password)
{
	char c;
	 //limited size 
	if ((strlen(password)>MAX_PASSWORD_LENGH)||  (strlen(password)<MIN_PASSWORD_LENGH)) return false;
	//no space allowed
	for (int i=0;i < strlen(password);i++)
		{
			c= password[i];
			if (c==' ') return false;
		}
	return true;
}

bool WEBINTERFACE_CLASS::isIPValid(const char * IP)
{  //limited size 
	int internalcount=0;
	int dotcount = 0;
	bool previouswasdot=false;
	char c;
	if (strlen(IP)>15 || strlen(IP)==0) return false;
	//cannot start with .
	if (IP[0]=='.')return false;
	//only letter and digit
	for (int i=0;i < strlen(IP);i++)
		{
			c = IP[i];
			if (isdigit(c))
				{//only 3 digit at once
					internalcount++;
					previouswasdot=false;
					if (internalcount>3)return false;
				}
			else if(c=='.')
				{   //cannot have 2 dots side by side
					if (previouswasdot)return false;
					previouswasdot=true;
					internalcount=0;
					dotcount++;
				}//if not a dot neither a digit it is wrong
			else return false;
		}
	//if not 3 dots then it is wrong
	if (dotcount!=3)return false;
	//cannot have the last dot as last char
	if (IP[strlen(IP)-1]=='.')return false;
	return true;
}

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
  web_interface.add4send(PROGMEM2CHAR(PAGE_HEAD_1));
  web_interface.add4send(PROGMEM2CHAR(PAGE_HEAD_2));
  //top bar
  TOPBAR(IP.c_str())
 //system part
  web_interface.add4send(PROGMEM2CHAR(PANEL_TOP));
  web_interface.add4send(PROGMEM2CHAR(TITLE_SYSTEM));
  web_interface.add4send(PROGMEM2CHAR(PANEL_START));
  LABEL(PROGMEM2CHAR(CHIP_ID_TITLE),String(system_get_chip_id()).c_str())
  LABEL_UNITS(PROGMEM2CHAR(CPU_FREQ_TITLE),String(system_get_cpu_freq()).c_str(),PROGMEM2CHAR(UNIT_HZ))
  LABEL_UNITS(PROGMEM2CHAR(FREE_MEM_TITLE),String(system_get_free_heap_size()).c_str(),PROGMEM2CHAR(UNIT_OCTET))
  LABEL(PROGMEM2CHAR(SDK_VERSION_TITLE),system_get_sdk_version())
  #ifdef MDNS_FEATURE
  sstatus = PROGMEM2CHAR(HTTP_START);
  sstatus+=PROGMEM2CHAR(LOCAL_NAME);
  LABEL_UNITS(PROGMEM2CHAR(HTTP_MDNS_NAME),sstatus.c_str(),PROGMEM2CHAR(HTTP_END))
  #endif
  istatus = wifi_get_phy_mode();
  if (istatus==PHY_MODE_11B) sstatus=PROGMEM2CHAR(VALUE_11B);
  else if (istatus==PHY_MODE_11G) sstatus=PROGMEM2CHAR(VALUE_11G);
  else sstatus=PROGMEM2CHAR(VALUE_11N);
  LABEL(PROGMEM2CHAR(NETWORK_TITLE),sstatus.c_str())
  istatus = wifi_get_sleep_type();
  if (istatus==NONE_SLEEP_T) sstatus=PROGMEM2CHAR(VALUE_NONE);
  else if (istatus==LIGHT_SLEEP_T) sstatus=PROGMEM2CHAR(VALUE_LIGHT);
  else  sstatus=PROGMEM2CHAR(VALUE_MODEM);
  LABEL(PROGMEM2CHAR(SLEEP_MODE_TITLE),sstatus.c_str())
  //LABEL(sbuf,"Boot mode: ",String(system_get_boot_mode())) //no meaning so far
  LABEL(PROGMEM2CHAR(BOOT_VERSION_TITLE),String(system_get_boot_version()).c_str())
  istatus=0;
  if (!CONFIG::read_buffer(EP_BAUD_RATE,  (byte *)&istatus , BAUD_LENGH))istatus=0;
  LABEL(PROGMEM2CHAR(BAUD_RATE_TITLE),String(istatus).c_str())
  web_interface.add4send(PROGMEM2CHAR(PANEL_END));
 //access point
  web_interface.add4send(PROGMEM2CHAR(PANEL_TOP));
  web_interface.add4send(PROGMEM2CHAR(ACCESS_POINT_TITLE));
  if(wifi_get_opmode()==WIFI_AP ||  wifi_get_opmode()==WIFI_AP_STA) web_interface.add4send(PROGMEM2CHAR(VALUE_ENABLED));
  else web_interface.add4send(PROGMEM2CHAR(VALUE_DISABLED));
  web_interface.add4send(PROGMEM2CHAR(PANEL_START));
  LABEL(PROGMEM2CHAR(MAC_ADDRESS_TITLE),wifi_config.mac2str(WiFi.softAPmacAddress(mac)))
  if (wifi_get_opmode()==WIFI_AP ||  wifi_get_opmode()==WIFI_AP_STA) 
	{
	  if (wifi_softap_get_config(&apconfig))
		{
		  LABEL(PROGMEM2CHAR(SSID_TITLE),(char *)(apconfig.ssid))
		  if(apconfig.ssid_hidden==1)sstatus=PROGMEM2CHAR(VALUE_NO);
		  else sstatus=PROGMEM2CHAR(VALUE_YES);
		  LABEL(PROGMEM2CHAR(VISIBLE_TITLE),sstatus.c_str())
		  LABEL(PROGMEM2CHAR(CHANNEL_TITLE),String(apconfig.channel).c_str())
		  if (apconfig.authmode==AUTH_OPEN)sstatus=PROGMEM2CHAR(VALUE_NONE);
		  else if (apconfig.authmode==AUTH_WEP)sstatus=PROGMEM2CHAR(VALUE_WEP);
		  else if (apconfig.authmode==AUTH_WPA_PSK)sstatus=PROGMEM2CHAR(VALUE_WPA);
		  else if (apconfig.authmode==AUTH_WPA2_PSK)sstatus=PROGMEM2CHAR(VALUE_WPA2);
		  else if (apconfig.authmode==AUTH_WPA_WPA2_PSK)sstatus=PROGMEM2CHAR(VALUE_WPAWPA2);
		  else sstatus=PROGMEM2CHAR(VALUE_MAX); //what is this one ? WPS ? Cannot find information
		  LABEL(PROGMEM2CHAR(AUTENTIFICATION_TITLE),sstatus.c_str())
		  LABEL(PROGMEM2CHAR(MAX_CONNECTION_TITLE),String(apconfig.max_connection).c_str())
		}
		 if (wifi_softap_dhcps_status()==DHCP_STARTED)sstatus=PROGMEM2CHAR(VALUE_STARTED);
		 else sstatus=PROGMEM2CHAR(VALUE_STOPPED);
		 LABEL(PROGMEM2CHAR(DHCP_SERVER_TITLE),sstatus.c_str())
		 if (wifi_get_ip_info(SOFTAP_IF,&info))
			{
			  LABEL(PROGMEM2CHAR(IP_TITLE),wifi_config.ip2str(info.ip.addr))
			  LABEL(PROGMEM2CHAR(GATEWAY_TITLE),wifi_config.ip2str(info.gw.addr))
			  LABEL(PROGMEM2CHAR(SUBNET_TITLE),wifi_config.ip2str(info.netmask.addr))
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
		 web_interface.add4send(PROGMEM2CHAR(TABLE_START));
		 web_interface.add4send(PROGMEM2CHAR(CAPTION_S));
		 web_interface.add4send(String(istatus).c_str());
		 web_interface.add4send(PROGMEM2CHAR(CONNECTED_STATIONS));
		 web_interface.add4send(PROGMEM2CHAR(CAPTION_E));
		 web_interface.add4send(PROGMEM2CHAR(THEAD_S));
		 web_interface.add4send(PROGMEM2CHAR(TR_S));
		TH_ENTRY(PROGMEM2CHAR(NUMBER_LABEL))
		TH_ENTRY(PROGMEM2CHAR(MAC_LABEL))
		TH_ENTRY(PROGMEM2CHAR(IP_LABEL))
		web_interface.add4send(PROGMEM2CHAR(TR_E));
		web_interface.add4send(PROGMEM2CHAR(THEAD_E));
		web_interface.add4send(PROGMEM2CHAR(TBODY_S));
		istatus=0;
		while(station)
			{
			istatus++;
			//display each client
			 web_interface.add4send(PROGMEM2CHAR(TR_S));
			 THR_ENTRY(String(istatus).c_str())
			 TD_ENTRY(wifi_config.mac2str(station->bssid))
			 TD_ENTRY(wifi_config.ip2str((byte *)&station->ip))
			 web_interface.add4send(PROGMEM2CHAR(TR_E));
			station = STAILQ_NEXT(station, next);
			}
		 web_interface.add4send(PROGMEM2CHAR(TBODY_E));
		//close table
		 web_interface.add4send(PROGMEM2CHAR(TABLE_END));
		 wifi_softap_free_station_info();
	}
  web_interface.add4send(PROGMEM2CHAR(PANEL_END));
  web_interface.add4send(PROGMEM2CHAR(PANEL_TOP));
  web_interface.add4send(PROGMEM2CHAR(STATION_TITLE));
  if(wifi_get_opmode()==WIFI_STA ||  wifi_get_opmode()==WIFI_AP_STA) web_interface.add4send(PROGMEM2CHAR(VALUE_ENABLED));
  else web_interface.add4send(PROGMEM2CHAR(VALUE_DISABLED));
  web_interface.add4send(PROGMEM2CHAR(PANEL_START));
  LABEL(PROGMEM2CHAR(MAC_ADDRESS_TITLE),wifi_config.mac2str(WiFi.macAddress(mac)))
   if(wifi_get_opmode()==WIFI_STA ||  wifi_get_opmode()==WIFI_AP_STA)
   {
	  LABEL(PROGMEM2CHAR(CONNECTION_TO_TITLE),WiFi.SSID())
	  LABEL(PROGMEM2CHAR(CHANNEL_TITLE),String(wifi_get_channel()).c_str())
	  istatus = wifi_station_get_connect_status();
	  if (istatus==STATION_GOT_IP) sstatus=PROGMEM2CHAR(VALUE_CONNECTED);
	  else if  (istatus==STATION_NO_AP_FOUND) sstatus=PROGMEM2CHAR(VALUE_NO_SSID);
	  else if  (istatus==STATION_CONNECT_FAIL) sstatus=PROGMEM2CHAR(VALUE_CONNECTION_FAILED);
	  else if  (istatus==STATION_WRONG_PASSWORD) sstatus=PROGMEM2CHAR(VALUE_CONNECTION_FAILED2);
	  else if  (istatus==STATION_IDLE) sstatus=PROGMEM2CHAR(VALUE_IDLE);//should not happen
	  else sstatus=PROGMEM2CHAR(VALUE_DISCONNECTED);
	  LABEL(PROGMEM2CHAR(STATUS_TITLE),sstatus.c_str())
	  if (wifi_station_dhcpc_status()==DHCP_STARTED)sstatus=PROGMEM2CHAR(VALUE_STARTED);
	  else sstatus=PROGMEM2CHAR(VALUE_STOPPED);
	  LABEL(PROGMEM2CHAR(DHCP_CLIENT_TITLE),sstatus.c_str())
	  LABEL(PROGMEM2CHAR(IP_TITLE),wifi_config.ip2str(WiFi.localIP()))
	  LABEL(PROGMEM2CHAR(GATEWAY_TITLE),wifi_config.ip2str(WiFi.gatewayIP()))
	  LABEL(PROGMEM2CHAR(SUBNET_TITLE),wifi_config.ip2str(WiFi.subnetMask()))
	 }
  web_interface.add4send(PROGMEM2CHAR(PANEL_END));
  web_interface.add4send(PROGMEM2CHAR(PAGE_BOTTOM));
  web_interface.flushbuffer(); 
}

void handle_web_interface_configSys()
{
  String stmp,smsg;
  int istatus=0;
  byte bflag=0;
  bool msg_alert_error=false;
  bool msg_alert_success=false;
  int ibaud=0;
  byte bnetwork=0;
  byte bsleepmode=0;
  //check is it is a submission or a display
  if (web_interface.WebServer.hasArg(PROGMEM2CHAR(SUBMIT_ID)))
  {   //is there a correct list of values?
	  if (web_interface.WebServer.hasArg(PROGMEM2CHAR(BAUD_RATE_ID)) && web_interface.WebServer.hasArg(PROGMEM2CHAR(SLEEP_MODE_ID))&& web_interface.WebServer.hasArg(PROGMEM2CHAR(NETWORK_ID)))
		{   //is each value correct ?
			ibaud  = atoi(web_interface.WebServer.arg(PROGMEM2CHAR(BAUD_RATE_ID)).c_str());
			bnetwork  = atoi(web_interface.WebServer.arg(PROGMEM2CHAR(NETWORK_ID)).c_str());
			bsleepmode = atoi(web_interface.WebServer.arg(PROGMEM2CHAR(SLEEP_MODE_ID)).c_str());
			if (!(ibaud==9600 || ibaud==19200|| ibaud==38400|| ibaud==57600|| ibaud==115200|| ibaud==230400) ||
			    !(bnetwork==PHY_MODE_11B||bnetwork==PHY_MODE_11G||bnetwork==PHY_MODE_11N) ||
			    !(bsleepmode==NONE_SLEEP_T ||bsleepmode==LIGHT_SLEEP_T ||bsleepmode==MODEM_SLEEP_T ))
				{
					msg_alert_error=true;
					smsg=PROGMEM2CHAR(ERROR_QUERY);
			    }
	  }
	  else
	   {
		msg_alert_error=true;
		smsg=PROGMEM2CHAR(ERROR_QUERY);
	   }
	   //if no error apply the changes
		if (msg_alert_error!=true)
			{
			 if(!CONFIG::write_buffer(EP_BAUD_RATE,(const byte *)&ibaud,BAUD_LENGH) ||!CONFIG::write_byte(EP_PHY_MODE,bnetwork) ||!CONFIG::write_byte(EP_SLEEP_MODE,bsleepmode))
				{
					msg_alert_error=true;
					smsg=PROGMEM2CHAR(ERROR_WRITING_CHANGES);
				}
			else
			if (!msg_alert_error)
				{
					msg_alert_success=true;
					smsg=PROGMEM2CHAR(SAVED_CHANGES);
				}
			}
		
  }
  if (!CONFIG::read_buffer(EP_BAUD_RATE,  (byte *)&istatus , BAUD_LENGH))istatus=0;
  if (wifi_get_opmode()==WIFI_STA ) stmp=wifi_config.ip2str(WiFi.localIP());
  else stmp=wifi_config.ip2str(WiFi.softAPIP());
  web_interface.add4send(PROGMEM2CHAR(PAGE_HEAD_1));
  web_interface.add4send(PROGMEM2CHAR(PAGE_HEAD_2));
  TOPBAR(stmp.c_str())
  web_interface.add4send(PROGMEM2CHAR(PANEL_TOP));
  web_interface.add4send(PROGMEM2CHAR(TITLE_SYSTEM));
  web_interface.add4send(PROGMEM2CHAR(PANEL_START));
  web_interface.add4send(PROGMEM2CHAR(FORM_START));
 
  SELECT_START(PROGMEM2CHAR(SYS_1_ID),PROGMEM2CHAR(BAUD_RATE_NAME),PROGMEM2CHAR(BAUD_RATE_ID))
  if (istatus==9600)stmp = PROGMEM2CHAR(VALUE_SELECTED);
  else stmp="";
  OPTION(PROGMEM2CHAR(VALUE_9600), stmp.c_str(),PROGMEM2CHAR(VALUE_9600))
   if (istatus==19200)stmp = PROGMEM2CHAR(VALUE_SELECTED);
  else stmp="";
  OPTION(PROGMEM2CHAR(VALUE_19200), stmp.c_str(),PROGMEM2CHAR(VALUE_19200))
   if (istatus==38400)stmp = PROGMEM2CHAR(VALUE_SELECTED);
  else stmp="";
  OPTION(PROGMEM2CHAR(VALUE_38400), stmp.c_str(),PROGMEM2CHAR(VALUE_38400))
   if (istatus==57600)stmp = PROGMEM2CHAR(VALUE_SELECTED);
  else stmp="";
  OPTION(PROGMEM2CHAR(VALUE_57600), stmp.c_str(),PROGMEM2CHAR(VALUE_57600))
   if (istatus==115200)stmp = PROGMEM2CHAR(VALUE_SELECTED);
  else stmp="";
  OPTION(PROGMEM2CHAR(VALUE_115200), stmp.c_str(),PROGMEM2CHAR(VALUE_115200))
   if (istatus==230400)stmp = PROGMEM2CHAR(VALUE_SELECTED);
  else stmp="";
  OPTION(PROGMEM2CHAR(VALUE_230400), stmp.c_str(),PROGMEM2CHAR(VALUE_230400))
  SELECT_END
  
  //web_interface.add4send(PROGMEM2CHAR(BR)); 
  
  if (!CONFIG::read_byte(EP_PHY_MODE, &bflag ))bflag=0;
  SELECT_START(PROGMEM2CHAR(SYS_2_ID),PROGMEM2CHAR(NETWORK_NAME),PROGMEM2CHAR(NETWORK_ID))
  if (bflag==PHY_MODE_11B)stmp = PROGMEM2CHAR(VALUE_SELECTED);
  else stmp="";
  OPTION(String(PHY_MODE_11B).c_str(), stmp.c_str(),PROGMEM2CHAR(VALUE_11B))
  if (bflag==PHY_MODE_11G)stmp = PROGMEM2CHAR(VALUE_SELECTED);
  else stmp="";
  OPTION(String(PHY_MODE_11G).c_str(), stmp.c_str(),PROGMEM2CHAR(VALUE_11G))
  if (bflag==PHY_MODE_11N)stmp = PROGMEM2CHAR(VALUE_SELECTED);
  else stmp="";
  if (wifi_get_opmode()==WIFI_STA )
	{
		OPTION(String(PHY_MODE_11N).c_str(), stmp.c_str(),PROGMEM2CHAR(VALUE_11N))
	}
  SELECT_END
  
 // web_interface.add4send(PROGMEM2CHAR(BR)); 
  
  if (!CONFIG::read_byte(EP_SLEEP_MODE, &bflag ))bflag=0;
  SELECT_START(PROGMEM2CHAR(SYS_3_ID),PROGMEM2CHAR(SLEEP_MODE_NAME),PROGMEM2CHAR(SLEEP_MODE_ID))
  if (bflag==NONE_SLEEP_T)stmp = PROGMEM2CHAR(VALUE_SELECTED);
  else stmp="";
  OPTION(String(NONE_SLEEP_T).c_str(), stmp.c_str(),PROGMEM2CHAR(VALUE_NONE))
  if (bflag==LIGHT_SLEEP_T)stmp = PROGMEM2CHAR(VALUE_SELECTED);
  else stmp="";
  OPTION(String(LIGHT_SLEEP_T).c_str(), stmp.c_str(),PROGMEM2CHAR(VALUE_LIGHT))
  if (bflag==MODEM_SLEEP_T)stmp = PROGMEM2CHAR(VALUE_SELECTED);
  else stmp="";
  OPTION(String(MODEM_SLEEP_T).c_str(), stmp.c_str(),PROGMEM2CHAR(VALUE_MODEM))
  SELECT_END
  
  if(msg_alert_error) 
	{
		MSG_ERROR(smsg.c_str()) 
		web_interface.add4send(PROGMEM2CHAR(FORM_SUBMIT)); 
	}
  else if(msg_alert_success) 
		{
			MSG_SUCCESS(smsg.c_str()) 
		}
	else web_interface.add4send(PROGMEM2CHAR(FORM_SUBMIT)); 
	
  web_interface.add4send(PROGMEM2CHAR(FORM_END));
  web_interface.add4send(PROGMEM2CHAR(PANEL_END)); 
  web_interface.add4send(PROGMEM2CHAR(PAGE_BOTTOM));
  web_interface.flushbuffer();
  if (msg_alert_success && !msg_alert_error)
	{
		system_restart();
	}
}

void handle_web_interface_configAP()
{
  String stmp,smsg;
  char sbuf[MAX_PASSWORD_LENGH+1];
  char error_display[9]={0,0,0,0,0,0,0,0,0};
  char password_buf[MAX_PASSWORD_LENGH+1];
  char ssid_buf[MAX_SSID_LENGH+1];
  char ip_buf[15+1];
  byte ip_sav[4];
  byte gw_sav[4];
  byte msk_sav[4];
  char gw_buf[15+1];
  char msk_buf[15+1];
  byte visible_buf;
  byte static_ip_buf;
  byte auth_buf;
  byte channel_buf;
  byte phy_mode_buf;
  int istatus=0;
  byte bflag=0;
  bool msg_alert_error=false;
  bool msg_alert_success=false;
  //check is it is a submission or a display
  smsg="";
  if (web_interface.WebServer.hasArg(PROGMEM2CHAR(SUBMIT_ID)))
  {   //is there a correct list of values?
	  if (web_interface.WebServer.hasArg(PROGMEM2CHAR(SSID_ID)) && web_interface.WebServer.hasArg(PROGMEM2CHAR(PASSWORD_ID))&& web_interface.WebServer.hasArg(PROGMEM2CHAR(NETWORK_ID))&& web_interface.WebServer.hasArg(PROGMEM2CHAR(AUTENTIFICATION_ID))&& web_interface.WebServer.hasArg(PROGMEM2CHAR(IP_NAME))&& web_interface.WebServer.hasArg(PROGMEM2CHAR(GATEWAY_ID))&& web_interface.WebServer.hasArg(PROGMEM2CHAR(SUBNET_ID))&& web_interface.WebServer.hasArg(PROGMEM2CHAR(CHANNEL_ID)))
		{	//ssid
			if (web_interface.WebServer.arg(PROGMEM2CHAR(SSID_ID)).length() > MAX_SSID_LENGH)
				{
				stmp = 	web_interface.WebServer.arg(PROGMEM2CHAR(SSID_ID)).substring(0,MAX_SSID_LENGH);
				msg_alert_error=true;
				error_display[0]=1;
				smsg+=PROGMEM2CHAR(ERROR_INCORRECT_SSID);
				web_interface.urldecode(ssid_buf,stmp.c_str());
				}
			else
				web_interface.urldecode(ssid_buf,web_interface.WebServer.arg(PROGMEM2CHAR(SSID_ID)).c_str());
			if (!web_interface.isSSIDValid(ssid_buf))
				{
				msg_alert_error=true;
				smsg+=PROGMEM2CHAR(ERROR_INCORRECT_SSID);
				error_display[0]=1;	
				}

			if (web_interface.WebServer.arg(PROGMEM2CHAR(PASSWORD_ID)).length() > MAX_PASSWORD_LENGH)
				{
				stmp = 	web_interface.WebServer.arg(PROGMEM2CHAR(PASSWORD_ID)).substring(0,MAX_PASSWORD_LENGH);
				msg_alert_error=true;
				error_display[0]=2;
				smsg+=PROGMEM2CHAR(ERROR_INCORRECT_PASSWORD);
				web_interface.urldecode(password_buf,stmp.c_str());
				}
			else
				web_interface.urldecode(password_buf,web_interface.WebServer.arg(PROGMEM2CHAR(PASSWORD_ID)).c_str());
				Serial.println(password_buf);
			if (!web_interface.isPasswordValid(password_buf))
				{
				msg_alert_error=true;
				smsg+=PROGMEM2CHAR(ERROR_INCORRECT_PASSWORD);
				error_display[2]=1;	
				}
			//ssid visible ?	
			if (web_interface.WebServer.hasArg(PROGMEM2CHAR(VISIBLE_NAME))) 
				{
					visible_buf=1;
				}
			else
				{
					visible_buf=0;
				}
			//phy mode
			phy_mode_buf  = atoi(web_interface.WebServer.arg(PROGMEM2CHAR(NETWORK_ID)).c_str());
			if (!(phy_mode_buf==PHY_MODE_11B||phy_mode_buf==PHY_MODE_11G) )
				 {
					msg_alert_error=true;
					smsg+=PROGMEM2CHAR(ERROR_QUERY);
				 }
			//channel
			channel_buf  = atoi(web_interface.WebServer.arg(PROGMEM2CHAR(CHANNEL_ID)).c_str());
			if (channel_buf< 1|| channel_buf>11) 
				 {
					msg_alert_error=true;
					smsg+=PROGMEM2CHAR(ERROR_QUERY);
				 }
			//authentification
			auth_buf  = atoi(web_interface.WebServer.arg(PROGMEM2CHAR(AUTENTIFICATION_ID)).c_str());
			if (!(auth_buf==AUTH_OPEN||auth_buf==AUTH_WEP||auth_buf==AUTH_WPA_PSK||auth_buf==AUTH_WPA2_PSK||auth_buf==AUTH_WPA_WPA2_PSK||auth_buf==AUTH_MAX) )
				 {
					msg_alert_error=true;
					smsg+=PROGMEM2CHAR(ERROR_QUERY);
				 }	
		    //Static IP ?	
			if (web_interface.WebServer.hasArg(PROGMEM2CHAR(STATIC_IP_NAME))) 
				{
					static_ip_buf=STATIC_IP_MODE;
				}
			else
				{
					static_ip_buf=DHCP_MODE;
				}
				
			//IP
			if (web_interface.WebServer.arg(PROGMEM2CHAR(IP_NAME)).length() > MAX_SSID_LENGH)
				{
				stmp = 	web_interface.WebServer.arg(PROGMEM2CHAR(IP_NAME)).substring(0,MAX_SSID_LENGH);
				msg_alert_error=true;
				error_display[6]=1;
				smsg+=PROGMEM2CHAR(ERROR_INCORRECT_IP_FORMAT);
				web_interface.urldecode(ip_buf,stmp.c_str());
				}
			else
				web_interface.urldecode(ip_buf,web_interface.WebServer.arg(PROGMEM2CHAR(IP_NAME)).c_str());
			if (!web_interface.isIPValid(ip_buf))
				{
				msg_alert_error=true;
				smsg+=PROGMEM2CHAR(ERROR_INCORRECT_IP_FORMAT);
				error_display[6]=1;	
				}
				
			//Gateway
			if (web_interface.WebServer.arg(PROGMEM2CHAR(GATEWAY_ID)).length() > MAX_SSID_LENGH)
				{
				stmp = 	web_interface.WebServer.arg(PROGMEM2CHAR(GATEWAY_ID)).substring(0,MAX_SSID_LENGH);
				msg_alert_error=true;
				error_display[7]=1;
				smsg+=PROGMEM2CHAR(ERROR_INCORRECT_IP_FORMAT);
				web_interface.urldecode(gw_buf,stmp.c_str());
				}
			else
				web_interface.urldecode(gw_buf,web_interface.WebServer.arg(PROGMEM2CHAR(GATEWAY_ID)).c_str());
			if (!web_interface.isIPValid(gw_buf))
				{
				msg_alert_error=true;
				smsg+=PROGMEM2CHAR(ERROR_INCORRECT_IP_FORMAT);
				error_display[7]=1;	
				}
			//subnet
			if (web_interface.WebServer.arg(PROGMEM2CHAR(SUBNET_ID)).length() > MAX_SSID_LENGH)
				{
				stmp = 	web_interface.WebServer.arg(PROGMEM2CHAR(SUBNET_ID)).substring(0,MAX_SSID_LENGH);
				msg_alert_error=true;
				error_display[8]=1;
				smsg+=PROGMEM2CHAR(ERROR_INCORRECT_IP_FORMAT);
				web_interface.urldecode(msk_buf,stmp.c_str());
				}
			else
				web_interface.urldecode(msk_buf,web_interface.WebServer.arg(PROGMEM2CHAR(SUBNET_ID)).c_str());
			if (!web_interface.isIPValid(msk_buf))
				{
				msg_alert_error=true;
				smsg+=PROGMEM2CHAR(ERROR_INCORRECT_IP_FORMAT);
				error_display[8]=1;	
				}	 	    	  	    
		}
      else
	   {
		msg_alert_error=true;
		smsg=PROGMEM2CHAR(ERROR_QUERY);
	   }
	   
	   
	   //no error ? then save
	   if (msg_alert_error==false)
		{
		//save
		 wifi_config.split_ip(ip_buf,ip_sav);
		 wifi_config.split_ip(gw_buf,gw_sav);
		 wifi_config.split_ip(msk_buf,msk_sav);
		 if((!CONFIG::write_byte(EP_WIFI_MODE,AP_MODE))||
			(!CONFIG::write_string(EP_SSID,ssid_buf,strlen(ssid_buf)))||
			(!CONFIG::write_string(EP_PASSWORD,password_buf,strlen(password_buf)))||
			(!CONFIG::write_byte(EP_SSID_VISIBLE,visible_buf))||
			(!CONFIG::write_byte(EP_PHY_MODE,phy_mode_buf))||
			(!CONFIG::write_byte(EP_CHANNEL,channel_buf)) ||
			(!CONFIG::write_byte(EP_AUTH_TYPE,auth_buf)) ||
			(!CONFIG::write_byte(EP_IP_MODE,static_ip_buf)) ||
			(!CONFIG::write_buffer(EP_IP_VALUE,ip_sav,IP_LENGH))||
			(!CONFIG::write_buffer(EP_GATEWAY_VALUE,gw_sav,IP_LENGH))||
			(!CONFIG::write_buffer(EP_MASK_VALUE,msk_sav,IP_LENGH)))msg_alert_error=true;
		
		if (msg_alert_error)smsg=PROGMEM2CHAR(ERROR_WRITING_CHANGES);
		if (!msg_alert_error)
			{
				msg_alert_success=true;
				smsg=PROGMEM2CHAR(SAVED_CHANGES);
				
			}
		}
	   
	}
	else
	{
	//ssid
	if (!CONFIG::read_string(EP_SSID, ssid_buf , MAX_SSID_LENGH) )strcpy(ssid_buf,PROGMEM2CHAR(DEFAULT_SSID));
	//password 
	if (!CONFIG::read_string(EP_PASSWORD, password_buf , MAX_PASSWORD_LENGH) )strcpy(password_buf,PROGMEM2CHAR(DEFAULT_PASSWORD));
	//ssid visible ?
	if (!CONFIG::read_byte(EP_SSID_VISIBLE, &visible_buf ))visible_buf=DEFAULT_SSID_VISIBLE;
	//phy mode
	if (!CONFIG::read_byte(EP_PHY_MODE, &phy_mode_buf ))phy_mode_buf=DEFAULT_PHY_MODE;
	//authentification
	if (!CONFIG::read_byte(EP_AUTH_TYPE, &auth_buf ))auth_buf=DEFAULT_AUTH_TYPE;
	//channel
	if (!CONFIG::read_byte(EP_CHANNEL, &channel_buf ))channel_buf=DEFAULT_CHANNEL;
	//static IP ?
	if (!CONFIG::read_byte(EP_IP_MODE, &static_ip_buf ))static_ip_buf=DEFAULT_IP_MODE;
	//IP for static IP
	if (!CONFIG::read_buffer(EP_IP_VALUE,(byte *)sbuf , IP_LENGH) )
		strcpy(ip_buf,wifi_config.ip2str((byte *)DEFAULT_IP_VALUE));
	else
		strcpy(ip_buf,wifi_config.ip2str((byte *)sbuf));  
	//GW for static IP
	if (!CONFIG::read_buffer(EP_GATEWAY_VALUE,(byte *)sbuf , IP_LENGH) )
		strcpy(gw_buf,wifi_config.ip2str((byte *)DEFAULT_GATEWAY_VALUE));
	else
		strcpy(gw_buf,wifi_config.ip2str((byte *)sbuf));  
	
	//Subnet for static IP
	if (!CONFIG::read_buffer(EP_MASK_VALUE,(byte *)sbuf , IP_LENGH) )
		strcpy(msk_buf,wifi_config.ip2str((byte *)DEFAULT_MASK_VALUE));
	else
		strcpy(msk_buf,wifi_config.ip2str((byte *)sbuf));  
	}
  
  //display page
  if (wifi_get_opmode()==WIFI_STA ) stmp=wifi_config.ip2str(WiFi.localIP());
  else stmp=wifi_config.ip2str(WiFi.softAPIP());
  web_interface.add4send(PROGMEM2CHAR(PAGE_HEAD_1));
  web_interface.add4send(PROGMEM2CHAR(PAGE_HEAD_2));
  TOPBAR(stmp.c_str())
  web_interface.add4send(PROGMEM2CHAR(PANEL_TOP));
  web_interface.add4send(PROGMEM2CHAR(ACCESS_POINT_TITLE));
  web_interface.add4send(PROGMEM2CHAR(PANEL_START));
  web_interface.add4send(PROGMEM2CHAR(FORM_START));
  //ssid
   if(error_display[0]==0)
    {
		INPUT_TEXT( PROGMEM2CHAR(AP_1_ID),PROGMEM2CHAR(SSID_TITLE), PROGMEM2CHAR(SSID_ID),PROGMEM2CHAR(SSID_ID),ssid_buf)
	}
    else
    {
		INPUT_TEXT_ERROR( PROGMEM2CHAR(AP_1_ID),PROGMEM2CHAR(SSID_TITLE), PROGMEM2CHAR(SSID_ID),PROGMEM2CHAR(SSID_ID),ssid_buf)
	}
  //password 
   if(error_display[1]==0)
    {
		INPUT_PASSWORD( PROGMEM2CHAR(AP_2_ID),PROGMEM2CHAR(PASSWORD_TITLE), PROGMEM2CHAR(PASSWORD_ID),PROGMEM2CHAR(PASSWORD_NAME),password_buf)
	}
   else
    {
		INPUT_PASSWORD_ERROR( PROGMEM2CHAR(AP_2_ID),PROGMEM2CHAR(PASSWORD_TITLE), PROGMEM2CHAR(PASSWORD_ID),PROGMEM2CHAR(PASSWORD_NAME),password_buf)
	}
  //ssid visible ?
  if (visible_buf==1)stmp=PROGMEM2CHAR(CHECKED_VALUE);
  else stmp="";
  INPUT_CHECKBOX( PROGMEM2CHAR(VISIBLE_NAME),PROGMEM2CHAR(VISIBLE_LABEL),stmp.c_str())
  //Phy mode
  SELECT_START(PROGMEM2CHAR(AP_4_ID),PROGMEM2CHAR(NETWORK_TITLE),PROGMEM2CHAR(NETWORK_ID))
  if (phy_mode_buf==PHY_MODE_11B)stmp = PROGMEM2CHAR(VALUE_SELECTED);
  else stmp="";
  OPTION(String(PHY_MODE_11B).c_str(), stmp.c_str(),PROGMEM2CHAR(VALUE_11B))
  if (phy_mode_buf==PHY_MODE_11G)stmp = PROGMEM2CHAR(VALUE_SELECTED);
  else stmp="";
  OPTION(String(PHY_MODE_11G).c_str(), stmp.c_str(),PROGMEM2CHAR(VALUE_11G))
  SELECT_END

  //CHANNEL
  SELECT_START(PROGMEM2CHAR(AP_10_ID),PROGMEM2CHAR(CHANNEL_TITLE),PROGMEM2CHAR(CHANNEL_ID))
  for (int c=1;c < 12;c++)
	{
	if (channel_buf==c)stmp = PROGMEM2CHAR(VALUE_SELECTED);
	else stmp="";
	OPTION(String(c).c_str(), stmp.c_str(),String(c).c_str())
	}
  SELECT_END
	
  //Authentification
  SELECT_START(PROGMEM2CHAR(AP_5_ID),PROGMEM2CHAR(AUTENTIFICATION_TITLE),PROGMEM2CHAR(AUTENTIFICATION_ID))
  if (auth_buf==AUTH_OPEN)stmp = PROGMEM2CHAR(VALUE_SELECTED);
  else stmp="";
  OPTION(String(AUTH_OPEN).c_str(), stmp.c_str(),PROGMEM2CHAR(VALUE_NONE))
  if (auth_buf==AUTH_WEP)stmp = PROGMEM2CHAR(VALUE_SELECTED);
  else stmp="";
  OPTION(String(AUTH_WEP).c_str(), stmp.c_str(),PROGMEM2CHAR(VALUE_WEP))
  if (auth_buf==AUTH_WPA_PSK)stmp = PROGMEM2CHAR(VALUE_SELECTED);
  else stmp="";
  OPTION(String(AUTH_WPA_PSK).c_str(), stmp.c_str(),PROGMEM2CHAR(VALUE_WPA))
  if (auth_buf==AUTH_WPA2_PSK)stmp = PROGMEM2CHAR(VALUE_SELECTED);
  else stmp="";
  OPTION(String(AUTH_WPA2_PSK).c_str(), stmp.c_str(),PROGMEM2CHAR(VALUE_WPA2))
  if (auth_buf==AUTH_WPA_WPA2_PSK)stmp = PROGMEM2CHAR(VALUE_SELECTED);
  else stmp="";
  OPTION(String(AUTH_WPA_WPA2_PSK).c_str(), stmp.c_str(),PROGMEM2CHAR(VALUE_WPAWPA2))
  if (auth_buf==AUTH_MAX)stmp = PROGMEM2CHAR(VALUE_SELECTED);
  else stmp="";
  OPTION(String(AUTH_MAX).c_str(), stmp.c_str(),PROGMEM2CHAR(VALUE_MAX))
  SELECT_END
  //static IP ?
  if (static_ip_buf==STATIC_IP_MODE)stmp=PROGMEM2CHAR(CHECKED_VALUE);
  else stmp="";
  INPUT_CHECKBOX(PROGMEM2CHAR(STATIC_IP_NAME), PROGMEM2CHAR(STATIC_IP_LABEL),stmp.c_str())
  // if (static_ip_buf==STATIC_IP_MODE)web_interface.add4send(PROGMEM2CHAR(SHOW_IP_BLOCK));
  //else web_interface.add4send(PROGMEM2CHAR(HIDE_IP_BLOCK));
  //IP for static IP
   if(error_display[6]==0)
    {
		INPUT_TEXT( PROGMEM2CHAR(AP_7_ID),PROGMEM2CHAR(IP_TITLE), PROGMEM2CHAR(IP_NAME),PROGMEM2CHAR(IP_NAME),ip_buf)
	}
   else
	{
		INPUT_TEXT_ERROR( PROGMEM2CHAR(AP_7_ID),PROGMEM2CHAR(IP_TITLE), PROGMEM2CHAR(IP_NAME),PROGMEM2CHAR(IP_NAME),ip_buf)
	}
 //Gateway for static IP
   if(error_display[7]==0)
    {
		INPUT_TEXT( PROGMEM2CHAR(AP_8_ID),PROGMEM2CHAR(GATEWAY_TITLE), PROGMEM2CHAR(GATEWAY_ID),PROGMEM2CHAR(GATEWAY_NAME),gw_buf)
	}
   else
    {
		INPUT_TEXT_ERROR( PROGMEM2CHAR(AP_8_ID),PROGMEM2CHAR(GATEWAY_TITLE), PROGMEM2CHAR(GATEWAY_ID),PROGMEM2CHAR(GATEWAY_NAME),gw_buf)
	}
  //Mask for static IP
   if(error_display[8]==0)
    {
	INPUT_TEXT( PROGMEM2CHAR(AP_9_ID),PROGMEM2CHAR(SUBNET_TITLE), PROGMEM2CHAR(SUBNET_ID),PROGMEM2CHAR(SUBNET_NAME),msk_buf)
	}
   else
    {
	INPUT_TEXT_ERROR( PROGMEM2CHAR(AP_9_ID),PROGMEM2CHAR(SUBNET_TITLE), PROGMEM2CHAR(SUBNET_ID),PROGMEM2CHAR(SUBNET_NAME),msk_buf)
	}
	//web_interface.add4send(PROGMEM2CHAR(DIV_E));

  
  if(msg_alert_error) 
	{
		MSG_ERROR(smsg.c_str()) 
		web_interface.add4send(PROGMEM2CHAR(FORM_SUBMIT)); 
	}
  else if(msg_alert_success) 
		{
			MSG_SUCCESS(smsg.c_str()) 
		}
	else web_interface.add4send(PROGMEM2CHAR(FORM_SUBMIT)); 
		
 
  web_interface.add4send(PROGMEM2CHAR(FORM_END));
  web_interface.add4send(PROGMEM2CHAR(PANEL_END)); 
  web_interface.add4send(PROGMEM2CHAR(PAGE_BOTTOM));
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
	 web_interface.add4send(PROGMEM2CHAR(T404_PAGE));
	 web_interface.add4send(IP.c_str());
	 web_interface.add4send(PROGMEM2CHAR(T404_PAGE_2));
	 web_interface.flushbuffer();
}

//URI Decoding function 
//no check if dst buffer is big enough to receive string so 
//use same size as src is a recommendation
void WEBINTERFACE_CLASS::urldecode(char *dst, const char *src)
{
  char a, b,c;
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
		c = *src++;
		if(c=='+')c=' ';
      *dst++ = c;
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
				web_interface.WebServer.send(200, "text/html", buffer2send);
				bsendingstarted=true;
			}
		 else
			{ //direct data send
				web_interface.WebServer.sendContent(buffer2send);
			}
		buffer2send=data2send;
		//do we need to purge or to store ?
		if (bimmediatsend)
			{//if some data flush
				if (buffer2send.length()>0)
					{
					web_interface.WebServer.sendContent(buffer2send);
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
  WebServer.on("/",HTTP_ANY, handle_web_interface_root);
  WebServer.on("/CONFIGSYS",HTTP_ANY, handle_web_interface_configSys);
   WebServer.on("/CONFIGAP",HTTP_ANY, handle_web_interface_configAP);
  WebServer.onNotFound( handle_not_found);
  buffer2send="";
}

WEBINTERFACE_CLASS web_interface(80);

