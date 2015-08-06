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

#include <pgmspace.h>
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

#ifdef SSDP_FEATURE
#include <ESP8266SSDP.h>
#endif
const char CSS[] PROGMEM = "html{font-family:sans-serif;-webkit-text-size-adjust:100%;-ms-text-size-adjust:100%;}body{margin:0;}main,menu,nav,summary{display:block;}[hidden],template{display:none;}a{background-color:transparent;}a:active,a:hover{outline:0;}h1{margin:.67em 0;font-size:2em;}img{border:0;}svg:not(:root){overflow:hidden;}hr{height:0;-webkit-box-sizing:content-box; -moz-box-sizing:content-box;box-sizing:content-box;}button,input,select{margin:0;font:inherit;color:inherit;}button{overflow:visible;}button,select{text-transform:none;}button,html input[type=\nbutton\n],input[type=\nreset\n],input[type=\nsubmit\n]{-webkit-appearance:button;cursor:pointer;}input{line-height:normal;}input[type=\ncheckbox\n],input[type=\nradio\n]{-webkit-box-sizing:border-box; -moz-box-sizing:border-box;box-sizing:border-box;padding:0;}input[type=\nnumber\n]::-webkit-inner-spin-button,input[type=\nnumber\n]::-webkit-outer-spin-button{height:auto;}table{border-spacing:0;border-collapse:collapse;}td,th{padding:0;}*{-webkit-box-sizing:border-box; -moz-box-sizing:border-box;box-sizing:border-box;}*:before,*:after{-webkit-box-sizing:border-box; -moz-box-sizing:border-box;box-sizing:border-box;}html{font-size:10px;-webkit-tap-highlight-color:rgba(0, 0, 0, 0);}"\
"body{font-family:\nHelvetica Neue\n, Helvetica, Arial, sans-serif;font-size:14px;line-height:1.42857143;color:#333;background-color:#fff;}input,button,select,a{color:#337ab7;text-decoration:none;}a:hover,a:focus{color:#23527c;text-decoration:underline;}a:focus{outline:thin dotted;outline:5px auto -webkit-focus-ring-color;outline-offset:-2px;}img{vertical-align:middle;}hr{margin-top:20px;margin-bottom:20px;border:0;border-top:1px solid #eee;}[role=\nbutton\n]{cursor:pointer;}h1,h3,.h1,.h3{font-family:inherit;font-weight:500;line-height:1.1;color:inherit;}h1,.h1,h3,.h3{margin-top:20px;margin-bottom:10px;}h1,.h1{font-size:36px;}h3,.h3{font-size:24px;}p{margin:0 0 10px;}@media (min-width:768px){.lead{font-size:21px;}}.text-left{text-align:left;}.text-right{text-align:right;}.text-center{text-align:center;}.text-primary{color:#337ab7;}.text-success{color:#3c763d;}.text-info{color:#31708f;}.page-header{padding-bottom:9px;margin:40px 0 20px;border-bottom:1px solid #eee;}.container{padding-right:15px;padding-left:15px;margin-right:auto;margin-left:auto;}@media (min-width:768px){.container{width:750px;}}@media (min-width:992px){.container{width:970px;}}@media (min-width:1200px){.container{width:1170px;}}.row{margin-right:-15px;margin-left:-15px;}"\
"table{background-color:transparent;}caption{padding-top:8px;padding-bottom:8px;color:#777;text-align:left;}th{text-align:left;}.table{width:100%;max-width:100%;margin-bottom:20px;}.table>thead>tr>th,.table>tbody>tr>th,.table>tfoot>tr>th,.table>thead>tr>td,.table>tbody>tr>td,.table>tfoot>tr>td{padding:8px;line-height:1.42857143;vertical-align:top;border-top:1px solid #ddd;}.table>thead>tr>th{vertical-align:bottom;border-bottom:2px solid #ddd;}.table>caption + thead>tr:first-child>th,.table>colgroup + thead>tr:first-child>th,.table>thead:first-child>tr:first-child>th,.table>caption + thead>tr:first-child>td,.table>colgroup + thead>tr:first-child>td,.table>thead:first-child>tr:first-child>td{border-top:0;}.table>tbody + tbody{border-top:2px solid #ddd;}.table .table{background-color:#fff;}.table-bordered{border:1px solid #ddd;}.table-bordered>thead>tr>th,.table-bordered>tbody>tr>th,.table-bordered>tfoot>tr>th,.table-bordered>thead>tr>td,.table-bordered>tbody>tr>td,.table-bordered>tfoot>tr>td{border:1px solid #ddd;}.table-bordered>thead>tr>th,.table-bordered>thead>tr>td{border-bottom-width:2px;}.table-striped>tbody>tr:nth-of-type(odd){background-color:#f9f9f9;}.table-hover>tbody>tr:hover{background-color:#f5f5f5;}"\
"table col[class*=\ncol-\n]{position:static;display:table-column;float:none;}table td[class*=\ncol-\n],table th[class*=\ncol-\n]{position:static;display:table-cell;float:none;}.table>thead>tr>td.active,.table>tbody>tr>td.active,.table>tfoot>tr>td.active,.table>thead>tr>th.active,.table>tbody>tr>th.active,.table>tfoot>tr>th.active,.table>thead>tr.active>td,.table>tbody>tr.active>td,.table>tfoot>tr.active>td,.table>thead>tr.active>th,.table>tbody>tr.active>th,.table>tfoot>tr.active>th{background-color:#f5f5f5;}.table-hover>tbody>tr>td.active:hover,.table-hover>tbody>tr>th.active:hover,.table-hover>tbody>tr.active:hover>td,.table-hover>tbody>tr:hover>.active,.table-hover>tbody>tr.active:hover>th{background-color:#e8e8e8;}label{display:inline-block;max-width:100%;margin-bottom:5px;font-weight:bold;}input[type=\nradio\n],input[type=\ncheckbox\n]{margin:4px 0 0;margin-top:1px;line-height:normal;}input[type=\nfile\n]{display:block;}input[type=\nrange\n]{display:block;width:100%;}input[type=\nfile\n]:focus,input[type=\nradio\n]:focus,input[type=\ncheckbox\n]:focus{outline:thin dotted;outline:5px auto -webkit-focus-ring-color;outline-offset:-2px;}"\
".form-control{display:block;width:100%;height:34px;padding:6px 12px;font-size:14px;line-height:1.42857143;color:#555;background-color:#fff;background-image:none;border:1px solid #ccc;border-radius:4px;-webkit-box-shadow:inset 0 1px 1px rgba(0, 0, 0, .075);box-shadow:inset 0 1px 1px rgba(0, 0, 0, .075);-webkit-transition:border-color ease-in-out .15s, -webkit-box-shadow ease-in-out .15s; -o-transition:border-color ease-in-out .15s, box-shadow ease-in-out .15s;transition:border-color ease-in-out .15s, box-shadow ease-in-out .15s;}.form-control:focus{border-color:#66afe9;outline:0;-webkit-box-shadow:inset 0 1px 1px rgba(0,0,0,.075), 0 0 8px rgba(102, 175, 233, .6);box-shadow:inset 0 1px 1px rgba(0,0,0,.075), 0 0 8px rgba(102, 175, 233, .6);}.form-control::-moz-placeholder{color:#999;opacity:1;}.form-control:-ms-input-placeholder{color:#999;}.form-control::-webkit-input-placeholder{color:#999;}.form-control[readonly]{background-color:#eee;opacity:1;}textarea.form-control{height:auto;}.form-group{margin-bottom:15px;}.radio,.checkbox{position:relative;display:block;margin-top:10px;margin-bottom:10px;}.radio label,.checkbox label{min-height:20px;padding-left:20px;margin-bottom:0;font-weight:normal;cursor:pointer;}"\
".radio input[type=\nradio\n],.checkbox input[type=\ncheckbox\n]{position:absolute;margin-top:4px;margin-left:-20px;}.radio + .radio,.checkbox + .checkbox{margin-top:-5px;}.has-error .control-label{color:#a94442;}.has-error .form-control{border-color:#a94442;-webkit-box-shadow:inset 0 1px 1px rgba(0, 0, 0, .075);box-shadow:inset 0 1px 1px rgba(0, 0, 0, .075);}.has-error .form-control:focus{border-color:#843534;-webkit-box-shadow:inset 0 1px 1px rgba(0, 0, 0, .075), 0 0 6px #ce8483;box-shadow:inset 0 1px 1px rgba(0, 0, 0, .075), 0 0 6px #ce8483;}.btn{display:inline-block;padding:6px 12px;margin-bottom:0;font-size:14px;font-weight:normal;line-height:1.42857143;text-align:center;white-space:nowrap;vertical-align:middle;-ms-touch-action:manipulation;touch-action:manipulation;cursor:pointer;-webkit-user-select:none; -moz-user-select:none;-ms-user-select:none;user-select:none;background-image:none;border:1px solid transparent;border-radius:4px;}.btn:focus,.btn:active:focus,.btn.active:focus,.btn.focus,.btn:active.focus,.btn.active.focus{outline:thin dotted;outline:5px auto -webkit-focus-ring-color;outline-offset:-2px;}.btn:hover,.btn:focus,.btn.focus{color:#333;text-decoration:none;}.btn:active,.btn.active{background-image:none;outline:0;-webkit-box-shadow:inset 0 3px 5px rgba(0, 0, 0, .125);box-shadow:inset 0 3px 5px rgba(0, 0, 0, .125);}"\
".btn-primary{color:#fff;background-color:#337ab7;border-color:#2e6da4;}.btn-primary:hover,.btn-primary:focus,.btn-primary.focus,.btn-primary:active,.btn-primary.active{color:#fff;background-color:#286090;border-color:#204d74;}.btn-primary:active,.btn-primary.active{background-image:none;}.btn-danger{color:#fff;background-color:#d9534f;border-color:#d43f3a;}.btn-danger:hover,.btn-danger:focus,.btn-danger.focus,.btn-danger:active,.btn-danger.active{color:#fff;background-color:#c9302c;border-color:#ac2925;}.btn-danger:active,.btn-danger.active{background-image:none;}.nav{padding-left:0;margin-bottom:0;list-style:none;}.nav>li{position:relative;display:block;}.nav>li>a{position:relative;display:block;padding:10px 15px;}.nav>li>a:hover,.nav>li>a:focus{text-decoration:none;background-color:#eee;}.nav-tabs{border-bottom:1px solid #ddd;}.nav-tabs>li{float:left;margin-bottom:-1px;}.nav-tabs>li>a{margin-right:2px;line-height:1.42857143;border:1px solid transparent;border-radius:4px 4px 0 0;}.nav-tabs>li>a:hover{border-color:#eee #eee #ddd;}.nav-tabs>li.active>a,.nav-tabs>li.active>a:hover,.nav-tabs>li.active>a:focus{color:#555;cursor:default;background-color:#fff;border:1px solid #ddd;border-bottom-color:transparent;}"\
".navbar{position:relative;min-height:50px;margin-bottom:20px;border:1px solid transparent;}@media (min-width:768px){.navbar{border-radius:4px;}}@media (min-width:768px){.navbar-header{float:left;}}.container>.navbar-header{margin-right:-15px;margin-left:-15px;}@media (min-width:768px){.container>.navbar-header{margin-right:0;margin-left:0;}}.navbar-nav{margin:7.5px -15px;}.navbar-nav>li>a{padding-top:10px;padding-bottom:10px;line-height:20px;}@media (min-width:768px){.navbar-nav{float:left;margin:0;}.navbar-nav>li{float:left;}.navbar-nav>li>a{padding-top:15px;padding-bottom:15px;}}.navbar-text{margin-top:15px;margin-bottom:15px;}@media (min-width:768px){.navbar-text{float:left;margin-right:15px;margin-left:15px;}}@media (min-width:768px){.navbar-left{float:left !important;}.navbar-right{float:right !important;margin-right:-15px;}.navbar-right ~ .navbar-right{margin-right:0;}}.navbar-inverse{background-color:#222;border-color:#080808;}.navbar-inverse .navbar-text{color:#9d9d9d;}.navbar-inverse .navbar-nav>li>a{color:#9d9d9d;}.navbar-inverse .navbar-nav>li>a:hover,.navbar-inverse .navbar-nav>li>a:focus{color:#fff;background-color:transparent;}"\
".navbar-inverse .navbar-nav>.active>a,.navbar-inverse .navbar-nav>.active>a:hover,.navbar-inverse .navbar-nav>.active>a:focus{color:#fff;background-color:#080808;}.navbar-inverse .navbar-link{color:#9d9d9d;}.navbar-inverse .navbar-link:hover{color:#fff;}.navbar-inverse .btn-link{color:#9d9d9d;}.navbar-inverse .btn-link:hover,.navbar-inverse .btn-link:focus{color:#fff;}.label{display:inline;padding:.2em .6em .3em;font-size:75%;font-weight:bold;line-height:1;color:#fff;text-align:center;white-space:nowrap;vertical-align:baseline;border-radius:.25em;}a.label:hover,a.label:focus{color:#fff;text-decoration:none;cursor:pointer;}.label:empty{display:none;}.btn .label{position:relative;top:-1px;}.label-info{background-color:#5bc0de;}.label-info[href]:hover,.label-info[href]:focus{background-color:#31b0d5;}.alert{padding:15px;margin-bottom:20px;border:1px solid transparent;border-radius:4px;}.alert h4{margin-top:0;color:inherit;}.alert-success{color:#3c763d;background-color:#dff0d8;border-color:#d6e9c6;}.alert-success hr{border-top-color:#c9e2b3;}.alert-success .alert-link{color:#2b542c;}.alert-danger{color:#a94442;background-color:#f2dede;border-color:#ebccd1;}.alert-danger hr{border-top-color:#e4b9c0;}.alert-danger .alert-link{color:#843534;}"\
".panel{margin-bottom:20px;background-color:#fff;border:1px solid transparent;border-radius:4px;-webkit-box-shadow:0 1px 1px rgba(0, 0, 0, .05);box-shadow:0 1px 1px rgba(0, 0, 0, .05);}.panel-body{padding:15px;}.panel-heading{padding:10px 15px;border-bottom:1px solid transparent;border-top-left-radius:3px;border-top-right-radius:3px;}.panel-title{margin-top:0;margin-bottom:0;font-size:16px;color:inherit;}.panel-footer{padding:10px 15px;background-color:#f5f5f5;border-top:1px solid #ddd;border-bottom-right-radius:3px;border-bottom-left-radius:3px;}.panel>.table{margin-bottom:0;}.panel>.table caption{padding-right:15px;padding-left:15px;}..panel>.table-bordered{border:0;}.panel-default{border-color:#ddd;}.panel-default>.panel-heading{color:#333;background-color:#f5f5f5;border-color:#ddd;}@-ms-viewport{width:device-width;}";
const char  PAGE_HEAD_1[] PROGMEM =  "<html lang=\"en\">\n<head>\n<meta charset=\"utf-8\">\n<meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\">\n" \
                                      "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n"\
                                      "<meta http-equiv=\"Cache-control\" content=\"public\">";
const char  PAGE_HEAD_2[] PROGMEM = "<title>Wifi Configuration</title>" \
  									  "<link rel=\"stylesheet\" href=\"/mincss.css\" type=\"text/css\">\n</head>\n<body>"\
                                      " <div class=\"container theme-showcase\" role=\"main\">";
const char NAV_START[] PROGMEM =  "<nav class=\"navbar navbar-inverse\">\n<div class=\"container\">\n";
 
const char NAV_LEFT_PART1 [] PROGMEM =  "<ul class=\"nav navbar-nav navbar-left\">\n<li ";
const char NAV_ELEMENT_ACTIVE [] PROGMEM =  "class=\"active\"";
const char NAV_LEFT_PART2a[] PROGMEM =  "><a href=\"http://";
const char NAV_LEFT_PART2b[] PROGMEM =  "\">Home</a></li>\n<li ";
const char NAV_LEFT_PART3a[] PROGMEM =  "><a href=\"http://";
const char NAV_LEFT_PART3b[] PROGMEM =  "/CONFIGSYS\">System Configuration</a></li>\n<li ";
const char NAV_LEFT_PART4a[] PROGMEM =  "><a href=\"http://";
const char NAV_LEFT_PART4b[] PROGMEM =  "/CONFIGAP\">AP Configuration</a></li>\n<li ";
const char NAV_LEFT_PART5a[] PROGMEM =  "><a href=\"http://";
const char NAV_LEFT_PART5b[] PROGMEM =  "/CONFIGSTA\">Station Configuration</a></li>\n<li ";
const char NAV_LEFT_PART6a[] PROGMEM =  "><a href=\"http://";
const char NAV_LEFT_PART6b[] PROGMEM =  "/PRINTER\">Printer Status</a></li>\n";
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
const char FORM_START[] PROGMEM = "<form METHOD=POST>\n";
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
const char UNIT_SECONDS[]  PROGMEM = " second(s)";
const char SDK_VERSION_TITLE[] PROGMEM = "SDK Version: ";
const char HTTP_START[] PROGMEM = "http://";
const char HTTP_MDNS_NAME[] PROGMEM = "mDNS name: ";
const char HTTP_END[] PROGMEM = ".local";
const char SSDP_PROTOCOL_NAME[] PROGMEM = "SSDP Protocol： ";
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
const char BAUD_RATE_NAME[] PROGMEM = "Baud rate";
const char NETWORK_NAME[] PROGMEM = "Network";
const char SLEEP_MODE_NAME[] PROGMEM = "Sleep Mode";
const char VALUE_1[] PROGMEM = "1";
const char VALUE_2[] PROGMEM = "2";
const char VALUE_3[] PROGMEM = "3";
const char VALUE_4[] PROGMEM = "4";
const char VALUE_5[] PROGMEM = "5";
const char VALUE_10[] PROGMEM = "10";
const char VALUE_30[] PROGMEM = "30";
const char VALUE_60[] PROGMEM = "60";
const char VALUE_9600[] PROGMEM = "9600";
const char VALUE_19200[] PROGMEM = "19200";
const char VALUE_38400[] PROGMEM = "38400";
const char VALUE_57600[] PROGMEM = "57600";
const char VALUE_115200[] PROGMEM = "115200";
const char VALUE_230400[] PROGMEM = "230400";
const char VALUE_250000[] PROGMEM = "250000";
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
const char ERROR_INCORRECT_SSID[] PROGMEM = "Incorrect SSID : no space, limited to 33 char length";
const char ERROR_INCORRECT_PASSWORD[] PROGMEM = "Incorrect password : space not allowed, limited to 8~64 char length<BR>";
const char ERROR_INCORRECT_PORT[] PROGMEM = "Incorrect port : 1~65000 only<BR>";
const char ERROR_INCORRECT_PORT2[] PROGMEM = "Incorrect port : web port aannot be same as data port<BR>";
const char ERROR_INCORRECT_IP_FORMAT[] PROGMEM = "Incorrect IP format, should be : xxx.xxx.xxx.xxx<BR>";
const char SHOW_IP_BLOCK[] PROGMEM = "<div NAME=\"IP_BLOCK\" >";
const char HIDE_IP_BLOCK[] PROGMEM = "<div NAME=\"IP_BLOCK\" style=\"visibility:none;\">";
const char AVAILABLE_APS[] PROGMEM = " AP(s) available";
const char RSSI_NAME[] PROGMEM = "Signal";
const char PROTECTED_NAME[] PROGMEM = "Protected";
const char WEB_PORT_TITLE[] PROGMEM = "Web port:";
const char DATA_PORT_TITLE[] PROGMEM = "Data port:";
const char WEB_PORT_ID[] PROGMEM = "WEBPORT";
const char DATA_PORT_ID[] PROGMEM = "DATAPORT";
const char PORT_DESC[]PROGMEM = "1~65000";

const char TEMP_SVG_1[]PROGMEM ="<svg height=\"30px\" width=\"300px\" xmlns=\"http://wwww.w3.org/2000/svg\">\n<linearGradient id=\"gradient\">\n<stop class=\"begin\" style=\"stop-color:green;\" offset=\"0%\"/>\n";
const char TEMP_SVG_2[]PROGMEM ="<stop class=\"middle\" style=\"stop-color:yellow;\" offset=\"100%\"/>\n</linearGradient>\n<linearGradient id=\"gradient2\">\n";
const char TEMP_SVG_3[]PROGMEM ="<stop class=\"middle\" style=\"stop-color:yellow;\" offset=\"0%\"/>\n<stop class=\"end\" style=\"stop-color:red;\" offset=\"100%\"/>\n";
const char TEMP_SVG_4[]PROGMEM ="</linearGradient>\n<rect x=\"10\" y=\"4\" width=\"24\" height=\"21\" style=\"fill:url(#gradient)\" />\n<rect x=\"34\" y=\"4\" width=\"280\" height=\"21\" style=\"fill:url(#gradient2)\" />\n";
const char TEMP_SVG_5[]PROGMEM ="<line x1=\"";
const char TEMP_SVG_6[]PROGMEM ="\" y1=\"4\" x2=\"";
const char TEMP_SVG_7[]PROGMEM ="\" y2=\"25\" style=\"stroke:rgb(255,255,255);stroke-width:1\" />\n";
const char TEMP_SVG_8[]PROGMEM ="<path d=\"M";
const char TEMP_SVG_9[]PROGMEM =" 0 L";
const char TEMP_SVG_10[]PROGMEM =" 0 L";
const char TEMP_SVG_11[]PROGMEM =" 8 Z\" stroke=\"white\" stroke-width=\"1\" />\n<path d=\"M";
const char TEMP_SVG_12[]PROGMEM =" 30 L";
const char TEMP_SVG_13[]PROGMEM =" 30 L";
const char TEMP_SVG_14[]PROGMEM =" 22 Z\" stroke=\"white\" stroke-width=\"1\"/>\n<text x=\"10\" y=\"19\" fill=\"black\" style=\"font-family: calibri; font-size:10pt;\">\n";
const char TEMP_SVG_15[]PROGMEM =" &#176;C</text>\n";
const char TEMP_SVG_16[]PROGMEM ="</svg>";
const char DATA_S[]PROGMEM ="<HTML><BODY><DIV NAME=\"data\" ID=\"data\" >";
const char DATA_E[]PROGMEM ="</BODY>\n</HTML>";
const char DIV_FLOW[]PROGMEM = "<DIV NAME=\"flow\" ID=\"flow\">";
const char DIV_SPEED[]PROGMEM ="<DIV NAME=\"speed\" ID=\"speed\">";
const char DIV_POSITION[]PROGMEM ="<DIV NAME=\"position\" ID=\"position\">";
const char DIV_STATUS[]PROGMEM ="<DIV NAME=\"status\" ID=\"status\">";
const char STATUS_1[]PROGMEM ="<svg width=\"20\" height=\"20\">\n<circle cx=\"10\" cy=\"10\" r=\"8\" stroke=\"black\" stroke-width=\"2\" fill=\"";
const char STATUS_2[]PROGMEM ="\" />\n</svg>";

const char PRINTER_1a[]PROGMEM ="<TABLE><TR style=\"vertical-align:top\"><TD><TABLE BORDER=0 ><TR><TD ID=\"display_data\" NAME=\"display_data\"></TD><TD>&nbsp;&nbsp;</TD>\n";
const char PRINTER_1b[]PROGMEM ="<TD ID=\"status\" NAME=\"status\" ></TD><TR></TABLE>\n";
const char PRINTER_1c[]PROGMEM ="<BR><TABLE BORDER=0><TR><TD ID=\"position\" NAME=\"position\"></TD>\n";
const char PRINTER_1d[]PROGMEM ="<TD><LABEL>Speed:</LABEL><LABEL ID=\"speed\" NAME=\"speed\" class=\"text-info\"></LABEL><LABEL class=\"text-info\">&#37;</LABEL>&nbsp;&nbsp;\n";
const char PRINTER_1e[]PROGMEM ="<LABEL>Flow:</LABEL><LABEL ID=\"flow\" NAME=\"flow\" class=\"text-info\"></LABEL><LABEL class=\"text-info\">&#37;</LABEL></TD></TR></TABLE><BR>\n";
const char PRINTER_1f[]PROGMEM ="<TABLE BORDER=0><TR style=\"vertical-align:top\" ><TD><LABEL>Info:</LABEL></TD><TD ID=\"infomsg\" NAME=\"infomsg\" class=\"text-info\"></TD></TR></TABLE><HR>\n";
const char PRINTER_1g[]PROGMEM ="<TABLE BORDER=0><TR style=\"vertical-align:top\"><TD><LABEL>Error:</LABEL></TD><TD ID=\"errormsg\" NAME=\"errormsg\" class=\"text-info\"></TD></TR></TABLE><HR>\n";
const char PRINTER_1h[]PROGMEM ="<TABLE BORDER=0><TR style=\"vertical-align:top\"><TD><LABEL>Status:</LABEL></TD><TD ID=\"statusmsg\" NAME=\"statusmsg\" class=\"text-info\"></TD></TR></TABLE>\n";
const char PRINTER_1_a_1[]PROGMEM="</TD><TD>&nbsp;&nbsp;&nbsp;</TD><TD><BUTTON TYPE=\"BUTTON\" class=\"btn btn-danger\" VALUE=\"Emergency Stop\" Onclick=\"window.open('http://";
const char PRINTER_1_a_2[]PROGMEM="/CMD?COM=M112','frmcmd');\">Emergency Stop</BUTTON>";
const char PRINTER_2[]PROGMEM ="</TD></TR></TABLE><iframe ID=\"dataframe\" NAME=\"dataframe\"src=\"http://";
const char PRINTER_3[]PROGMEM ="/STATUS\" frameborder=0 width=\"2\" height=\"2\" style=\"visibility:hidden;\"></iframe>\n<IFRAME width=\"2\" height=\"2\" style=\"visibility:hidden\" ID=\"frmcmd\" NAME=\"frmcmd\" ></IFRAME>\n";
const char PRINTER_4[]PROGMEM ="<SCRIPT TYPE=\"text/javascript\">\n document.getElementById(\"dataframe\").onload=function(){\n";
const char PRINTER_5[]PROGMEM ="var ifrm=document.getElementById(\"dataframe\");\nvar doc=ifrm.contentDocument?ifrm.contentDocument:ifrm.contentWindow.document;\n";
const char PRINTER_6a[]PROGMEM ="document.getElementById(\"display_data\").innerHTML=doc.getElementById(\"data\").innerHTML;\n";
const char PRINTER_6b[]PROGMEM ="document.getElementById(\"position\").innerHTML=doc.getElementById(\"position\").innerHTML;\n";
const char PRINTER_6c[]PROGMEM ="document.getElementById(\"speed\").innerHTML=doc.getElementById(\"speed\").innerHTML;\n";
const char PRINTER_6d[]PROGMEM ="document.getElementById(\"flow\").innerHTML=doc.getElementById(\"flow\").innerHTML;\n";
const char PRINTER_6e[]PROGMEM ="document.getElementById(\"status\").innerHTML=doc.getElementById(\"status\").innerHTML;\n";
const char PRINTER_6f[]PROGMEM ="document.getElementById(\"infomsg\").innerHTML=doc.getElementById(\"infomsg\").innerHTML;\n";
const char PRINTER_6g[]PROGMEM ="document.getElementById(\"errormsg\").innerHTML=doc.getElementById(\"errormsg\").innerHTML;\n";
const char PRINTER_6h[]PROGMEM ="document.getElementById(\"statusmsg\").innerHTML=doc.getElementById(\"statusmsg\").innerHTML;\n";
const char PRINTER_7[]PROGMEM ="}\n";
const char PRINTER_8[]PROGMEM ="setInterval(function(){";
const char PRINTER_9[]PROGMEM ="var ifrm=document.getElementById(\"dataframe\");var doc=ifrm.contentDocument?ifrm.contentDocument:ifrm.contentWindow.document;";
const char PRINTER_10[]PROGMEM ="doc.location.reload(true);";
const char PRINTER_11a[]PROGMEM ="},";
const char PRINTER_11b[]PROGMEM =");\n";
const char PRINTER_12[]PROGMEM ="</SCRIPT>\n";
const char DIV_ERRORMSG[]PROGMEM ="<DIV ID=\"errormsg\" NAME=\"errormsg\">\n";
const char DIV_INFOMSG[]PROGMEM ="<DIV ID=\"infomsg\" NAME=\"infomsg\">\n";
const char DIV_STATUSMSG[]PROGMEM ="<DIV ID=\"statusmsg\" NAME=\"statusmsg\">\n";
const char COMMAND_ID[]PROGMEM ="COM";
const char PARAM_ID[]PROGMEM ="PARAM";
const char POLLING_TITLE[]PROGMEM ="Refresh Web page (s):";
const char POLLING_NAME[]PROGMEM ="Refresh printer status every :";
const char POLLING_ID[]PROGMEM ="POLLING";
const char TEXT_HTML[]PROGMEM ="text/html";
const char RESTARTCMD [] PROGMEM ="<script>setTimeout(function(){window.location.href='/RESTART'},10000);</script>";
const char RESTARTINGMSG [] PROGMEM = "<CENTER>Restarting, please wait.... </CENTER><script>setTimeout(function(){window.location.href='/'},20000);</script>";




#define TEMP_SVG(temperature,target,description) buffer2send+=(PROGMEM2CHAR(TEMP_SVG_1));buffer2send+=(PROGMEM2CHAR(TEMP_SVG_2));buffer2send+=(PROGMEM2CHAR(TEMP_SVG_3));buffer2send+=(PROGMEM2CHAR(TEMP_SVG_4));buffer2send+=(PROGMEM2CHAR(TEMP_SVG_5));buffer2send+=String(target+10); buffer2send+=(PROGMEM2CHAR(TEMP_SVG_6));buffer2send+=String(target+10); buffer2send+=(PROGMEM2CHAR(TEMP_SVG_7));buffer2send+=(PROGMEM2CHAR(TEMP_SVG_8));buffer2send+=String(temperature+5);buffer2send+=(PROGMEM2CHAR(TEMP_SVG_9));buffer2send+=String(temperature+15);buffer2send+=(PROGMEM2CHAR(TEMP_SVG_10));buffer2send+=String(temperature+10);buffer2send+=(PROGMEM2CHAR(TEMP_SVG_11));buffer2send+=String(temperature+5);buffer2send+=(PROGMEM2CHAR(TEMP_SVG_12));buffer2send+=String(temperature+15);buffer2send+=(PROGMEM2CHAR(TEMP_SVG_13));buffer2send+=String(temperature+10);buffer2send+=(PROGMEM2CHAR(TEMP_SVG_14));buffer2send+=description;buffer2send+=(PROGMEM2CHAR(TEMP_SVG_15));buffer2send+=(PROGMEM2CHAR(TEMP_SVG_16));

#define MSG_SUCCESS(msg) buffer2send+=(PROGMEM2CHAR(ALERT_SUCCESS));buffer2send+=(msg);buffer2send+=(PROGMEM2CHAR(DIV_E));
#define MSG_ERROR(msg) buffer2send+=(PROGMEM2CHAR(ALERT_ERROR));buffer2send+=(msg);buffer2send+=(PROGMEM2CHAR(DIV_E));
#define OPTION(value, selected,content) buffer2send+=(PROGMEM2CHAR(FORM_OPTION_1));buffer2send+=(value);buffer2send+=(PROGMEM2CHAR(FORM_OPTION_2));buffer2send+=(selected);buffer2send+=(PROGMEM2CHAR(FORM_OPTION_3));buffer2send+=(content);buffer2send+=(PROGMEM2CHAR(FORM_OPTION_4));
#define SELECT_START(id,label,name) buffer2send+=(PROGMEM2CHAR(FORM_INPUT_1));buffer2send+=(id);buffer2send+=(PROGMEM2CHAR(FORM_INPUT_2));buffer2send+=(label);buffer2send+=(PROGMEM2CHAR(FORM_SELECT_1));buffer2send+=(name);buffer2send+=(PROGMEM2CHAR(FORM_SELECT_2));buffer2send+=(id);buffer2send+=(PROGMEM2CHAR(FORM_SELECT_3));
#define SELECT_START_ERROR(id,label,name) buffer2send+=(PROGMEM2CHAR(FORM_INPUT_1_ERROR));buffer2send+=(id);buffer2send+=(PROGMEM2CHAR(FORM_INPUT_2));buffer2send+=(label);buffer2send+=(PROGMEM2CHAR(FORM_SELECT_1));buffer2send+=(name);buffer2send+=(PROGMEM2CHAR(FORM_SELECT_2));buffer2send+=(id);buffer2send+=(PROGMEM2CHAR(FORM_SELECT_3));
#define SELECT_END  buffer2send+=(PROGMEM2CHAR(FORM_SELECT_END));
#define INPUT_TEXT( id,label, name,placeholder,value)  buffer2send+=(PROGMEM2CHAR(FORM_INPUT_1));buffer2send+=(id);buffer2send+=(PROGMEM2CHAR(FORM_INPUT_2));buffer2send+=(label);buffer2send+=(PROGMEM2CHAR(FORM_INPUT_3));buffer2send+=(id);buffer2send+=(PROGMEM2CHAR(FORM_INPUT_4));buffer2send+=(name);buffer2send+=(PROGMEM2CHAR(FORM_INPUT_5));buffer2send+=(placeholder);buffer2send+=(PROGMEM2CHAR(FORM_INPUT_6));buffer2send+=(value);buffer2send+=(PROGMEM2CHAR(FORM_INPUT_7));
#define INPUT_TEXT_ERROR( id,label, name,placeholder,value)  buffer2send+=(PROGMEM2CHAR(FORM_INPUT_1_ERROR));buffer2send+=(id);buffer2send+=(PROGMEM2CHAR(FORM_INPUT_2));buffer2send+=(label);buffer2send+=(PROGMEM2CHAR(FORM_INPUT_3));buffer2send+=(id);buffer2send+=(PROGMEM2CHAR(FORM_INPUT_4));buffer2send+=(name);buffer2send+=(PROGMEM2CHAR(FORM_INPUT_5));buffer2send+=(placeholder);buffer2send+=(PROGMEM2CHAR(FORM_INPUT_6));buffer2send+=(value);buffer2send+=(PROGMEM2CHAR(FORM_INPUT_7));
#define INPUT_PASSWORD( id,label, name,placeholder,value)  buffer2send+=(PROGMEM2CHAR(FORM_INPUT_1));buffer2send+=(id);buffer2send+=(PROGMEM2CHAR(FORM_INPUT_2));buffer2send+=(label);buffer2send+=(PROGMEM2CHAR(FORM_INPUT_3P));buffer2send+=(id);buffer2send+=(PROGMEM2CHAR(FORM_INPUT_4));buffer2send+=(name);buffer2send+=(PROGMEM2CHAR(FORM_INPUT_5));buffer2send+=(placeholder);buffer2send+=(PROGMEM2CHAR(FORM_INPUT_6));buffer2send+=(value);buffer2send+=(PROGMEM2CHAR(FORM_INPUT_7));
#define INPUT_PASSWORD_ERROR( id,label, name,placeholder,value)  buffer2send+=(PROGMEM2CHAR(FORM_INPUT_1_ERROR));buffer2send+=(id);buffer2send+=(PROGMEM2CHAR(FORM_INPUT_2));buffer2send+=(label);buffer2send+=(PROGMEM2CHAR(FORM_INPUT_3P));buffer2send+=(id);buffer2send+=(PROGMEM2CHAR(FORM_INPUT_4));buffer2send+=(name);buffer2send+=(PROGMEM2CHAR(FORM_INPUT_5));buffer2send+=(placeholder);buffer2send+=(PROGMEM2CHAR(FORM_INPUT_6));buffer2send+=(value);buffer2send+=(PROGMEM2CHAR(FORM_INPUT_7));
#define INPUT_CHECKBOX(name,label,status)  buffer2send+=(PROGMEM2CHAR(FORM_CHECKBOX_1));buffer2send+=(name);buffer2send+=(PROGMEM2CHAR(FORM_CHECKBOX_2));buffer2send+=(status);buffer2send+=(PROGMEM2CHAR(FORM_CHECKBOX_3));buffer2send+=(label);buffer2send+=(PROGMEM2CHAR(FORM_CHECKBOX_4));
#define LABEL( title, value)  buffer2send+=(PROGMEM2CHAR(LABEL_START)); buffer2send+=(title);buffer2send+=(PROGMEM2CHAR(LABEL_COLOR));buffer2send+=(value);buffer2send+=(PROGMEM2CHAR(LABEL_END));buffer2send+=(PROGMEM2CHAR(BR));
#define LABEL_UNITS(title, value,units) buffer2send+=(PROGMEM2CHAR(LABEL_START));  buffer2send+=(title);buffer2send+=(PROGMEM2CHAR(LABEL_COLOR));buffer2send+=(value);buffer2send+=(units);buffer2send+=(PROGMEM2CHAR(LABEL_END));buffer2send+=(PROGMEM2CHAR(BR));
#define TH_ENTRY(entry) buffer2send+=(PROGMEM2CHAR(TH_S));buffer2send+=(entry);buffer2send+=(PROGMEM2CHAR(TH_E));
#define THR_ENTRY(entry) buffer2send+=(PROGMEM2CHAR(TH_R));buffer2send+=(entry);buffer2send+=(PROGMEM2CHAR(TH_E));
#define TD_ENTRY(entry) buffer2send+=(PROGMEM2CHAR(TD_S));buffer2send+=(entry);buffer2send+=(PROGMEM2CHAR(TD_E));
#define TOPBAR(IP,menu)  buffer2send+=(PROGMEM2CHAR(NAV_START)); buffer2send+=(PROGMEM2CHAR(NAV_LEFT_PART1)) ;  buffer2send+=(menu==1?PROGMEM2CHAR(NAV_ELEMENT_ACTIVE):"") ;  buffer2send+=(PROGMEM2CHAR(NAV_LEFT_PART2a) );  buffer2send+=(IP);  buffer2send+=(PROGMEM2CHAR(NAV_LEFT_PART2b));buffer2send+=(menu==2?PROGMEM2CHAR(NAV_ELEMENT_ACTIVE):"") ;  buffer2send+=(PROGMEM2CHAR(NAV_LEFT_PART3a) ); buffer2send+=(IP);  buffer2send+=(PROGMEM2CHAR(NAV_LEFT_PART3b));buffer2send+=(menu==3?PROGMEM2CHAR(NAV_ELEMENT_ACTIVE):"") ;  buffer2send+=(PROGMEM2CHAR(NAV_LEFT_PART4a) );  buffer2send+=(IP);  buffer2send+=(PROGMEM2CHAR(NAV_LEFT_PART4b)); buffer2send+=(menu==4?PROGMEM2CHAR(NAV_ELEMENT_ACTIVE):"") ;  buffer2send+=(PROGMEM2CHAR(NAV_LEFT_PART5a) ); buffer2send+=(IP);  buffer2send+=(PROGMEM2CHAR(NAV_LEFT_PART5b));buffer2send+=(menu==5?PROGMEM2CHAR(NAV_ELEMENT_ACTIVE):"") ;  buffer2send+=(PROGMEM2CHAR(NAV_LEFT_PART6a) ); buffer2send+=(IP);  buffer2send+=(PROGMEM2CHAR(NAV_LEFT_PART6b));buffer2send+=(PROGMEM2CHAR(NAV_LEFT_PARTEND)); buffer2send+=(PROGMEM2CHAR(NAV_RIGHT_PART)) ;  buffer2send+=(PROGMEM2CHAR(NAV_END));
#define STATUS_SVG(color) buffer2send+=(PROGMEM2CHAR(STATUS_1));buffer2send+=(color);buffer2send+=(PROGMEM2CHAR(STATUS_2));


char * progmem2char(const char* src)
{
  static char buffer[300];
  buffer[0]=0;
  if(strlen_P(src)<299)strcpy_P(buffer,src);
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
			//if (!(isdigit(c) || isalpha(c))) return false;
			if (c==' ') return false;
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
  String buffer2send="";
  String sstatus;
  struct softap_config apconfig;
  struct ip_info info;
  byte bbuf;
  int istatus;
  int lstatus;
  uint8_t mac [WL_MAC_ADDR_LENGTH];
  if (wifi_get_opmode()==WIFI_STA ) IP=wifi_config.ip2str(WiFi.localIP());
  else IP=wifi_config.ip2str(WiFi.softAPIP());
  if (wifi_config.iweb_port!=80)
	{
		IP+=":";
		IP+=String(wifi_config.iweb_port);
	}
  buffer2send+=(PROGMEM2CHAR(PAGE_HEAD_1));
  buffer2send+=(PROGMEM2CHAR(PAGE_HEAD_2));
  //top bar
  TOPBAR(IP.c_str(),1)
 //system part
  buffer2send+=(PROGMEM2CHAR(PANEL_TOP));
  buffer2send+=(PROGMEM2CHAR(TITLE_SYSTEM));
  buffer2send+=(PROGMEM2CHAR(PANEL_START));
  LABEL(PROGMEM2CHAR(CHIP_ID_TITLE),String(system_get_chip_id()).c_str())
  LABEL_UNITS(PROGMEM2CHAR(CPU_FREQ_TITLE),String(system_get_cpu_freq()).c_str(),PROGMEM2CHAR(UNIT_HZ))
  LABEL_UNITS(PROGMEM2CHAR(FREE_MEM_TITLE),String(system_get_free_heap_size()).c_str(),PROGMEM2CHAR(UNIT_OCTET))
  LABEL(PROGMEM2CHAR(SDK_VERSION_TITLE),system_get_sdk_version())
  #ifdef MDNS_FEATURE
  if (wifi_get_opmode()==WIFI_STA )
	{
	sstatus = PROGMEM2CHAR(HTTP_START);
	sstatus+=PROGMEM2CHAR(LOCAL_NAME);
	LABEL_UNITS(PROGMEM2CHAR(HTTP_MDNS_NAME),sstatus.c_str(),PROGMEM2CHAR(HTTP_END))
	}
  #endif
  #ifdef SSDP_FEATURE
  LABEL(PROGMEM2CHAR(SSDP_PROTOCOL_NAME),PROGMEM2CHAR(VALUE_YES))
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
  if (!CONFIG::read_buffer(EP_BAUD_RATE,  (byte *)&lstatus , INTEGER_LENGH))lstatus=0;
  LABEL(PROGMEM2CHAR(BAUD_RATE_TITLE),String(lstatus).c_str())
  istatus=0;
  if (!CONFIG::read_buffer(EP_WEB_PORT,  (byte *)&istatus , INTEGER_LENGH))istatus=0;
  LABEL(PROGMEM2CHAR(WEB_PORT_TITLE),String(istatus).c_str())
  istatus=0;
  if (!CONFIG::read_buffer(EP_DATA_PORT,  (byte *)&istatus , INTEGER_LENGH))istatus=0;
  LABEL(PROGMEM2CHAR(DATA_PORT_TITLE),String(istatus).c_str())
  if (!CONFIG::read_byte(EP_POLLING_TIME, &bbuf ))bbuf=DEFAULT_POLLING_TIME;
  LABEL_UNITS(PROGMEM2CHAR(POLLING_NAME),String(bbuf).c_str(),PROGMEM2CHAR(UNIT_SECONDS))
  buffer2send+=(PROGMEM2CHAR(PANEL_END));
 //access point
  buffer2send+=(PROGMEM2CHAR(PANEL_TOP));
  buffer2send+=(PROGMEM2CHAR(ACCESS_POINT_TITLE));
  if(wifi_get_opmode()==WIFI_AP ||  wifi_get_opmode()==WIFI_AP_STA) buffer2send+=(PROGMEM2CHAR(VALUE_ENABLED));
  else buffer2send+=(PROGMEM2CHAR(VALUE_DISABLED));
  buffer2send+=(PROGMEM2CHAR(PANEL_START));
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
			yield();
			}
		//start table as at least one connected
		 buffer2send+=(PROGMEM2CHAR(TABLE_START));
		 buffer2send+=(PROGMEM2CHAR(CAPTION_S));
		 buffer2send+=(String(istatus).c_str());
		 buffer2send+=(PROGMEM2CHAR(CONNECTED_STATIONS));
		 buffer2send+=(PROGMEM2CHAR(CAPTION_E));
		 buffer2send+=(PROGMEM2CHAR(THEAD_S));
		 buffer2send+=(PROGMEM2CHAR(TR_S));
		TH_ENTRY(PROGMEM2CHAR(NUMBER_LABEL))
		TH_ENTRY(PROGMEM2CHAR(MAC_LABEL))
		TH_ENTRY(PROGMEM2CHAR(IP_LABEL))
		buffer2send+=(PROGMEM2CHAR(TR_E));
		buffer2send+=(PROGMEM2CHAR(THEAD_E));
		buffer2send+=(PROGMEM2CHAR(TBODY_S));
		istatus=0;
		while(station)
			{
			yield();
			istatus++;
			//display each client
			 buffer2send+=(PROGMEM2CHAR(TR_S));
			 THR_ENTRY(String(istatus).c_str())
			 TD_ENTRY(wifi_config.mac2str(station->bssid))
			 TD_ENTRY(wifi_config.ip2str((byte *)&station->ip))
			 buffer2send+=(PROGMEM2CHAR(TR_E));
			station = STAILQ_NEXT(station, next);
			}
		 buffer2send+=(PROGMEM2CHAR(TBODY_E));
		//close table
		 buffer2send+=(PROGMEM2CHAR(TABLE_END));
		 wifi_softap_free_station_info();
	}
  buffer2send+=(PROGMEM2CHAR(PANEL_END));
  buffer2send+=(PROGMEM2CHAR(PANEL_TOP));
  buffer2send+=(PROGMEM2CHAR(STATION_TITLE));
  if(wifi_get_opmode()==WIFI_STA ||  wifi_get_opmode()==WIFI_AP_STA) buffer2send+=(PROGMEM2CHAR(VALUE_ENABLED));
  else buffer2send+=(PROGMEM2CHAR(VALUE_DISABLED));
  buffer2send+=(PROGMEM2CHAR(PANEL_START));
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
  buffer2send+=(PROGMEM2CHAR(PANEL_END));
  buffer2send+=(PROGMEM2CHAR(PAGE_BOTTOM));
  web_interface->WebServer.send(200, "text/html", buffer2send);
}

void handle_web_interface_configSys()
{
  String stmp,smsg;
  String buffer2send ="";
  byte bflag=0;
  char error_display[4]={0,0,0,0};
  bool msg_alert_error=false;
  bool msg_alert_success=false;
  long ibaud=0;
  int iweb_port =0;
  int idata_port =0;
  byte bsleepmode=0;
  byte polling_time=3;
  //check is it is a submission or a display
  if (web_interface->WebServer.hasArg(PROGMEM2CHAR(SUBMIT_ID)))
  {   //is there a correct list of values?
	  if (web_interface->WebServer.hasArg(PROGMEM2CHAR(BAUD_RATE_ID)) && web_interface->WebServer.hasArg(PROGMEM2CHAR(SLEEP_MODE_ID))&& web_interface->WebServer.hasArg(PROGMEM2CHAR(POLLING_ID))&& web_interface->WebServer.hasArg(PROGMEM2CHAR(WEB_PORT_ID))&& web_interface->WebServer.hasArg(PROGMEM2CHAR(DATA_PORT_ID)))
		{   //is each value correct ?
			ibaud  = atol(web_interface->WebServer.arg(PROGMEM2CHAR(BAUD_RATE_ID)).c_str());
			iweb_port  = atol(web_interface->WebServer.arg(PROGMEM2CHAR(WEB_PORT_ID)).c_str());
			idata_port  = atoi(web_interface->WebServer.arg(PROGMEM2CHAR(DATA_PORT_ID)).c_str());
			bsleepmode = atoi(web_interface->WebServer.arg(PROGMEM2CHAR(SLEEP_MODE_ID)).c_str());
			polling_time = atoi(web_interface->WebServer.arg(PROGMEM2CHAR(POLLING_ID)).c_str());
			if (!(iweb_port>0 && iweb_port<65001) ||
				!(idata_port>0 && idata_port<65001))
				{
					msg_alert_error=true;
					smsg=PROGMEM2CHAR(ERROR_INCORRECT_PORT);
			    }
			if (iweb_port== idata_port)
				{
					msg_alert_error=true;
					smsg=PROGMEM2CHAR(ERROR_INCORRECT_PORT2);
			    }
			if (!(ibaud==9600 || ibaud==19200|| ibaud==38400|| ibaud==57600|| ibaud==115200|| ibaud==230400 || ibaud==250000) ||
			    !(bsleepmode==NONE_SLEEP_T ||bsleepmode==LIGHT_SLEEP_T ||bsleepmode==MODEM_SLEEP_T )||
			    !(polling_time==1 || polling_time==2 ||polling_time==3 || polling_time==4 ||polling_time==5 ||polling_time==10 ||polling_time==30 ||polling_time==60))
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
			 if(!CONFIG::write_buffer(EP_BAUD_RATE,(const byte *)&ibaud,INTEGER_LENGH)||!CONFIG::write_buffer(EP_WEB_PORT,(const byte *)&iweb_port,INTEGER_LENGH)||!CONFIG::write_buffer(EP_DATA_PORT,(const byte *)&idata_port,INTEGER_LENGH)||!CONFIG::write_byte(EP_SLEEP_MODE,bsleepmode)||!CONFIG::write_byte(EP_POLLING_TIME,polling_time))
				{
					msg_alert_error=true;
					smsg=PROGMEM2CHAR(ERROR_WRITING_CHANGES);
				}
			else
			if (!msg_alert_error)
				{
					msg_alert_success=true;
					wifi_config.iweb_port=iweb_port;
					wifi_config.idata_port=idata_port;
					smsg=PROGMEM2CHAR(SAVED_CHANGES);
				}
			}
		
  }
  else
	{
	if (!CONFIG::read_buffer(EP_BAUD_RATE,  (byte *)&ibaud , INTEGER_LENGH))ibaud=DEFAULT_BAUD_RATE;
	if (!CONFIG::read_byte(EP_SLEEP_MODE, &bsleepmode ))bsleepmode=DEFAULT_SLEEP_MODE;
	if (!CONFIG::read_buffer(EP_WEB_PORT,  (byte *)&iweb_port , INTEGER_LENGH))ibaud=DEFAULT_WEB_PORT;
	if (!CONFIG::read_buffer(EP_DATA_PORT,  (byte *)&idata_port , INTEGER_LENGH))ibaud=DEFAULT_DATA_PORT;
	if (!CONFIG::read_byte(EP_POLLING_TIME, &polling_time ))polling_time=DEFAULT_POLLING_TIME;
	}
  if (wifi_get_opmode()==WIFI_STA ) stmp=wifi_config.ip2str(WiFi.localIP());
  else stmp=wifi_config.ip2str(WiFi.softAPIP());
  if (wifi_config.iweb_port!=80)
	{
		stmp+=":";
		stmp+=String(wifi_config.iweb_port);
	}
  buffer2send+=(PROGMEM2CHAR(PAGE_HEAD_1));
  buffer2send+=(PROGMEM2CHAR(PAGE_HEAD_2));
  TOPBAR(stmp.c_str(),2)
  buffer2send+=(PROGMEM2CHAR(PANEL_TOP));
  buffer2send+=(PROGMEM2CHAR(TITLE_SYSTEM));
  buffer2send+=(PROGMEM2CHAR(PANEL_START));
  buffer2send+=(PROGMEM2CHAR(FORM_START));
  //baud rate
  SELECT_START(PROGMEM2CHAR(AP_1_ID),PROGMEM2CHAR(BAUD_RATE_NAME),PROGMEM2CHAR(BAUD_RATE_ID))
  if (ibaud==9600)stmp = PROGMEM2CHAR(VALUE_SELECTED);
  else stmp="";
  OPTION(PROGMEM2CHAR(VALUE_9600), stmp.c_str(),PROGMEM2CHAR(VALUE_9600))
   if (ibaud==19200)stmp = PROGMEM2CHAR(VALUE_SELECTED);
  else stmp="";
  OPTION(PROGMEM2CHAR(VALUE_19200), stmp.c_str(),PROGMEM2CHAR(VALUE_19200))
   if (ibaud==38400)stmp = PROGMEM2CHAR(VALUE_SELECTED);
  else stmp="";
  OPTION(PROGMEM2CHAR(VALUE_38400), stmp.c_str(),PROGMEM2CHAR(VALUE_38400))
   if (ibaud==57600)stmp = PROGMEM2CHAR(VALUE_SELECTED);
  else stmp="";
  OPTION(PROGMEM2CHAR(VALUE_57600), stmp.c_str(),PROGMEM2CHAR(VALUE_57600))
   if (ibaud==115200)stmp = PROGMEM2CHAR(VALUE_SELECTED);
  else stmp="";
  OPTION(PROGMEM2CHAR(VALUE_115200), stmp.c_str(),PROGMEM2CHAR(VALUE_115200))
   if (ibaud==230400)stmp = PROGMEM2CHAR(VALUE_SELECTED);
  else stmp="";
  OPTION(PROGMEM2CHAR(VALUE_230400), stmp.c_str(),PROGMEM2CHAR(VALUE_230400))
  if (ibaud==250000)stmp = PROGMEM2CHAR(VALUE_SELECTED);
  else stmp="";
  OPTION(PROGMEM2CHAR(VALUE_250000), stmp.c_str(),PROGMEM2CHAR(VALUE_250000))
  SELECT_END
   
 // buffer2send+=(PROGMEM2CHAR(BR)); 
  
  //sleep mode
  SELECT_START(PROGMEM2CHAR(AP_2_ID),PROGMEM2CHAR(SLEEP_MODE_NAME),PROGMEM2CHAR(SLEEP_MODE_ID))
  if (bsleepmode==NONE_SLEEP_T)stmp = PROGMEM2CHAR(VALUE_SELECTED);
  else stmp="";
  OPTION(String(NONE_SLEEP_T).c_str(), stmp.c_str(),PROGMEM2CHAR(VALUE_NONE))
  if (bsleepmode==LIGHT_SLEEP_T)stmp = PROGMEM2CHAR(VALUE_SELECTED);
  else stmp="";
  OPTION(String(LIGHT_SLEEP_T).c_str(), stmp.c_str(),PROGMEM2CHAR(VALUE_LIGHT))
  if (bsleepmode==MODEM_SLEEP_T)stmp = PROGMEM2CHAR(VALUE_SELECTED);
  else stmp="";
  OPTION(String(MODEM_SLEEP_T).c_str(), stmp.c_str(),PROGMEM2CHAR(VALUE_MODEM))
  SELECT_END
  
  //web port
  if(error_display[2]==0)
    {
		INPUT_TEXT( PROGMEM2CHAR(AP_3_ID),PROGMEM2CHAR(WEB_PORT_TITLE), PROGMEM2CHAR(WEB_PORT_ID),PROGMEM2CHAR(PORT_DESC),String(iweb_port).c_str())
	}
   else
	{
		INPUT_TEXT_ERROR( PROGMEM2CHAR(AP_3_ID),PROGMEM2CHAR(WEB_PORT_TITLE), PROGMEM2CHAR(WEB_PORT_ID),PROGMEM2CHAR(PORT_DESC),String(iweb_port).c_str())
	}
  //data port
  if(error_display[3]==0)
    {
		INPUT_TEXT( PROGMEM2CHAR(AP_4_ID),PROGMEM2CHAR(DATA_PORT_TITLE), PROGMEM2CHAR(DATA_PORT_ID),PROGMEM2CHAR(PORT_DESC),String(idata_port).c_str())
	}
   else
	{
		INPUT_TEXT_ERROR( PROGMEM2CHAR(AP_4_ID),PROGMEM2CHAR(DATA_PORT_TITLE), PROGMEM2CHAR(DATA_PORT_ID),PROGMEM2CHAR(PORT_DESC),String(idata_port).c_str())
	}
	
  //polling
  SELECT_START(PROGMEM2CHAR(AP_10_ID),PROGMEM2CHAR(POLLING_TITLE),PROGMEM2CHAR(POLLING_ID))
  if (polling_time==1)stmp = PROGMEM2CHAR(VALUE_SELECTED);
  else stmp="";
  OPTION(PROGMEM2CHAR(VALUE_1), stmp.c_str(),PROGMEM2CHAR(VALUE_1))
   if (polling_time==2)stmp = PROGMEM2CHAR(VALUE_SELECTED);
  else stmp="";
  OPTION(PROGMEM2CHAR(VALUE_2), stmp.c_str(),PROGMEM2CHAR(VALUE_2))
   if (polling_time==3)stmp = PROGMEM2CHAR(VALUE_SELECTED);
  else stmp="";
  OPTION(PROGMEM2CHAR(VALUE_3), stmp.c_str(),PROGMEM2CHAR(VALUE_3))
   if (polling_time==4)stmp = PROGMEM2CHAR(VALUE_SELECTED);
  else stmp="";
  OPTION(PROGMEM2CHAR(VALUE_4), stmp.c_str(),PROGMEM2CHAR(VALUE_4))
   if (polling_time==5)stmp = PROGMEM2CHAR(VALUE_SELECTED);
  else stmp="";
  OPTION(PROGMEM2CHAR(VALUE_5), stmp.c_str(),PROGMEM2CHAR(VALUE_5))
   if (polling_time==10)stmp = PROGMEM2CHAR(VALUE_SELECTED);
  else stmp="";
  OPTION(PROGMEM2CHAR(VALUE_10), stmp.c_str(),PROGMEM2CHAR(VALUE_10))
   if (polling_time==30)stmp = PROGMEM2CHAR(VALUE_SELECTED);
  else stmp="";
  OPTION(PROGMEM2CHAR(VALUE_30), stmp.c_str(),PROGMEM2CHAR(VALUE_30))
   if (polling_time==60)stmp = PROGMEM2CHAR(VALUE_SELECTED);
  else stmp="";
  OPTION(PROGMEM2CHAR(VALUE_60), stmp.c_str(),PROGMEM2CHAR(VALUE_60))
  
  SELECT_END
  if(msg_alert_error) 
	{
		MSG_ERROR(smsg.c_str()) 
		buffer2send+=(PROGMEM2CHAR(FORM_SUBMIT)); 
	}
  else if(msg_alert_success) 
		{
			MSG_SUCCESS(smsg.c_str()) 
		}
	else buffer2send+=(PROGMEM2CHAR(FORM_SUBMIT)); 
	
	buffer2send+=(PROGMEM2CHAR(FORM_END));
	buffer2send+=(PROGMEM2CHAR(PANEL_END)); 
  if (msg_alert_success && !msg_alert_error)
	{
		buffer2send+= PROGMEM2CHAR(RESTARTCMD);
	}
	buffer2send+=(PROGMEM2CHAR(PAGE_BOTTOM));
	web_interface->WebServer.send(200, "text/html", buffer2send);
}

void handle_web_interface_configAP()
{
  String stmp,smsg;
  String buffer2send ="";
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
  byte bflag=0;
  bool msg_alert_error=false;
  bool msg_alert_success=false;
  //check is it is a submission or a display
  smsg="";
  if (web_interface->WebServer.hasArg(PROGMEM2CHAR(SUBMIT_ID)))
  {   //is there a correct list of values?
	  if (web_interface->WebServer.hasArg(PROGMEM2CHAR(SSID_ID)) && web_interface->WebServer.hasArg(PROGMEM2CHAR(PASSWORD_ID))&& web_interface->WebServer.hasArg(PROGMEM2CHAR(NETWORK_ID))&& web_interface->WebServer.hasArg(PROGMEM2CHAR(AUTENTIFICATION_ID))&& web_interface->WebServer.hasArg(PROGMEM2CHAR(IP_NAME))&& web_interface->WebServer.hasArg(PROGMEM2CHAR(GATEWAY_ID))&& web_interface->WebServer.hasArg(PROGMEM2CHAR(SUBNET_ID))&& web_interface->WebServer.hasArg(PROGMEM2CHAR(CHANNEL_ID)))
		{	//ssid
			if (web_interface->WebServer.arg(PROGMEM2CHAR(SSID_ID)).length() > MAX_SSID_LENGH)
				{
				stmp = 	web_interface->WebServer.arg(PROGMEM2CHAR(SSID_ID)).substring(0,MAX_SSID_LENGH);
				msg_alert_error=true;
				error_display[0]=1;
				smsg+=PROGMEM2CHAR(ERROR_INCORRECT_SSID);
				web_interface->urldecode(ssid_buf,stmp.c_str());
				}
			else
				web_interface->urldecode(ssid_buf,web_interface->WebServer.arg(PROGMEM2CHAR(SSID_ID)).c_str());
			if (!web_interface->isSSIDValid(ssid_buf))
				{
				msg_alert_error=true;
				smsg+=PROGMEM2CHAR(ERROR_INCORRECT_SSID);
				error_display[0]=1;	
				}

			if (web_interface->WebServer.arg(PROGMEM2CHAR(PASSWORD_ID)).length() > MAX_PASSWORD_LENGH)
				{
				stmp = 	web_interface->WebServer.arg(PROGMEM2CHAR(PASSWORD_ID)).substring(0,MAX_PASSWORD_LENGH);
				msg_alert_error=true;
				error_display[0]=2;
				smsg+=PROGMEM2CHAR(ERROR_INCORRECT_PASSWORD);
				web_interface->urldecode(password_buf,stmp.c_str());
				}
			else
				web_interface->urldecode(password_buf,web_interface->WebServer.arg(PROGMEM2CHAR(PASSWORD_ID)).c_str());
			if (!web_interface->isPasswordValid(password_buf))
				{
				msg_alert_error=true;
				smsg+=PROGMEM2CHAR(ERROR_INCORRECT_PASSWORD);
				error_display[2]=1;	
				}
			//ssid visible ?	
			if (web_interface->WebServer.hasArg(PROGMEM2CHAR(VISIBLE_NAME))) 
				{
					visible_buf=1;
				}
			else
				{
					visible_buf=0;
				}
			//phy mode
			phy_mode_buf  = atoi(web_interface->WebServer.arg(PROGMEM2CHAR(NETWORK_ID)).c_str());
			if (!(phy_mode_buf==PHY_MODE_11B||phy_mode_buf==PHY_MODE_11G) )
				 {
					msg_alert_error=true;
					smsg+=PROGMEM2CHAR(ERROR_QUERY);
				 }
			//channel
			channel_buf  = atoi(web_interface->WebServer.arg(PROGMEM2CHAR(CHANNEL_ID)).c_str());
			if (channel_buf< 1|| channel_buf>11) 
				 {
					msg_alert_error=true;
					smsg+=PROGMEM2CHAR(ERROR_QUERY);
				 }
			//authentification
			auth_buf  = atoi(web_interface->WebServer.arg(PROGMEM2CHAR(AUTENTIFICATION_ID)).c_str());
			if (!(auth_buf==AUTH_OPEN||auth_buf==AUTH_WEP||auth_buf==AUTH_WPA_PSK||auth_buf==AUTH_WPA2_PSK||auth_buf==AUTH_WPA_WPA2_PSK||auth_buf==AUTH_MAX) )
				 {
					msg_alert_error=true;
					smsg+=PROGMEM2CHAR(ERROR_QUERY);
				 }	
		    //Static IP ?	
			if (web_interface->WebServer.hasArg(PROGMEM2CHAR(STATIC_IP_NAME))) 
				{
					static_ip_buf=STATIC_IP_MODE;
				}
			else
				{
					static_ip_buf=DHCP_MODE;
				}
				
			//IP
			if (web_interface->WebServer.arg(PROGMEM2CHAR(IP_NAME)).length() > MAX_SSID_LENGH)
				{
				stmp = 	web_interface->WebServer.arg(PROGMEM2CHAR(IP_NAME)).substring(0,MAX_SSID_LENGH);
				msg_alert_error=true;
				error_display[6]=1;
				smsg+=PROGMEM2CHAR(ERROR_INCORRECT_IP_FORMAT);
				web_interface->urldecode(ip_buf,stmp.c_str());
				}
			else
				web_interface->urldecode(ip_buf,web_interface->WebServer.arg(PROGMEM2CHAR(IP_NAME)).c_str());
			if (!web_interface->isIPValid(ip_buf))
				{
				msg_alert_error=true;
				smsg+=PROGMEM2CHAR(ERROR_INCORRECT_IP_FORMAT);
				error_display[6]=1;	
				}
				
			//Gateway
			if (web_interface->WebServer.arg(PROGMEM2CHAR(GATEWAY_ID)).length() > MAX_SSID_LENGH)
				{
				stmp = 	web_interface->WebServer.arg(PROGMEM2CHAR(GATEWAY_ID)).substring(0,MAX_SSID_LENGH);
				msg_alert_error=true;
				error_display[7]=1;
				smsg+=PROGMEM2CHAR(ERROR_INCORRECT_IP_FORMAT);
				web_interface->urldecode(gw_buf,stmp.c_str());
				}
			else
				web_interface->urldecode(gw_buf,web_interface->WebServer.arg(PROGMEM2CHAR(GATEWAY_ID)).c_str());
			if (!web_interface->isIPValid(gw_buf))
				{
				msg_alert_error=true;
				smsg+=PROGMEM2CHAR(ERROR_INCORRECT_IP_FORMAT);
				error_display[7]=1;	
				}
			//subnet
			if (web_interface->WebServer.arg(PROGMEM2CHAR(SUBNET_ID)).length() > MAX_SSID_LENGH)
				{
				stmp = 	web_interface->WebServer.arg(PROGMEM2CHAR(SUBNET_ID)).substring(0,MAX_SSID_LENGH);
				msg_alert_error=true;
				error_display[8]=1;
				smsg+=PROGMEM2CHAR(ERROR_INCORRECT_IP_FORMAT);
				web_interface->urldecode(msk_buf,stmp.c_str());
				}
			else
				web_interface->urldecode(msk_buf,web_interface->WebServer.arg(PROGMEM2CHAR(SUBNET_ID)).c_str());
			if (!web_interface->isIPValid(msk_buf))
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
		 char ip_buftmp[15+1];
		 strcpy(ip_buftmp,ip_buf);
		 wifi_config.split_ip(ip_buftmp,ip_sav);
		 strcpy(ip_buftmp,gw_buf);
		 wifi_config.split_ip(ip_buftmp,gw_sav);
		 strcpy(ip_buftmp,msk_buf);
		 wifi_config.split_ip(ip_buftmp,msk_sav);
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
  if (wifi_config.iweb_port!=80)
	{
		stmp+=":";
		stmp+=String(wifi_config.iweb_port);
	}
  buffer2send+=(PROGMEM2CHAR(PAGE_HEAD_1));
  buffer2send+=(PROGMEM2CHAR(PAGE_HEAD_2));
  TOPBAR(stmp.c_str(),3)
  buffer2send+=(PROGMEM2CHAR(PANEL_TOP));
  buffer2send+=(PROGMEM2CHAR(ACCESS_POINT_TITLE));
  buffer2send+=(PROGMEM2CHAR(PANEL_START));
  buffer2send+=(PROGMEM2CHAR(FORM_START));
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
  // if (static_ip_buf==STATIC_IP_MODE)buffer2send+=(PROGMEM2CHAR(SHOW_IP_BLOCK));
  //else buffer2send+=(PROGMEM2CHAR(HIDE_IP_BLOCK));
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
	//buffer2send+=(PROGMEM2CHAR(DIV_E));

  
  if(msg_alert_error) 
	{
		MSG_ERROR(smsg.c_str()) 
		buffer2send+=(PROGMEM2CHAR(FORM_SUBMIT)); 
	}
  else if(msg_alert_success) 
		{
			MSG_SUCCESS(smsg.c_str()) 
		}
	else buffer2send+=(PROGMEM2CHAR(FORM_SUBMIT)); 
		
  
	buffer2send+=(PROGMEM2CHAR(FORM_END));
	buffer2send+=(PROGMEM2CHAR(PANEL_END)); 
	if (msg_alert_success && !msg_alert_error)
	{
		buffer2send+=PROGMEM2CHAR(RESTARTCMD);
	}
	buffer2send+=(PROGMEM2CHAR(PAGE_BOTTOM));
	web_interface->WebServer.send(200, "text/html", buffer2send);
}


void handle_web_interface_configSTA()
{
  String stmp,smsg;
  String buffer2send ="";
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
  byte bflag=0;
  bool revertSTA=false;
  bool msg_alert_error=false;
  bool msg_alert_success=false;
  //check is it is a submission or a display
  smsg="";
  if (web_interface->WebServer.hasArg(PROGMEM2CHAR(SUBMIT_ID)))
  {   //is there a correct list of values?
	  if (web_interface->WebServer.hasArg(PROGMEM2CHAR(SSID_ID)) && web_interface->WebServer.hasArg(PROGMEM2CHAR(PASSWORD_ID))&& web_interface->WebServer.hasArg(PROGMEM2CHAR(NETWORK_ID))&& web_interface->WebServer.hasArg(PROGMEM2CHAR(IP_NAME))&& web_interface->WebServer.hasArg(PROGMEM2CHAR(GATEWAY_ID))&& web_interface->WebServer.hasArg(PROGMEM2CHAR(SUBNET_ID)))
		{	//ssid
			if (web_interface->WebServer.arg(PROGMEM2CHAR(SSID_ID)).length() > MAX_SSID_LENGH)
				{
				stmp = 	web_interface->WebServer.arg(PROGMEM2CHAR(SSID_ID)).substring(0,MAX_SSID_LENGH);
				msg_alert_error=true;
				error_display[0]=1;
				smsg+=PROGMEM2CHAR(ERROR_INCORRECT_SSID);
				web_interface->urldecode(ssid_buf,stmp.c_str());
				}
			else
				web_interface->urldecode(ssid_buf,web_interface->WebServer.arg(PROGMEM2CHAR(SSID_ID)).c_str());
			if (!web_interface->isSSIDValid(ssid_buf))
				{
				msg_alert_error=true;
				smsg+=PROGMEM2CHAR(ERROR_INCORRECT_SSID);
				error_display[0]=1;	
				}

			if (web_interface->WebServer.arg(PROGMEM2CHAR(PASSWORD_ID)).length() > MAX_PASSWORD_LENGH)
				{
				stmp = 	web_interface->WebServer.arg(PROGMEM2CHAR(PASSWORD_ID)).substring(0,MAX_PASSWORD_LENGH);
				msg_alert_error=true;
				error_display[0]=2;
				smsg+=PROGMEM2CHAR(ERROR_INCORRECT_PASSWORD);
				web_interface->urldecode(password_buf,stmp.c_str());
				}
			else
				web_interface->urldecode(password_buf,web_interface->WebServer.arg(PROGMEM2CHAR(PASSWORD_ID)).c_str());
			if (!web_interface->isPasswordValid(password_buf))
				{
				msg_alert_error=true;
				smsg+=PROGMEM2CHAR(ERROR_INCORRECT_PASSWORD);
				error_display[2]=1;	
				}
			//phy modeString stmp,smsg;
			phy_mode_buf  = atoi(web_interface->WebServer.arg(PROGMEM2CHAR(NETWORK_ID)).c_str());
			if (!(phy_mode_buf==PHY_MODE_11B||phy_mode_buf==PHY_MODE_11G  ||phy_mode_buf==PHY_MODE_11N))
				 {
					msg_alert_error=true;
					smsg+=PROGMEM2CHAR(ERROR_QUERY);
				 }
		    //Static IP ?	
			if (web_interface->WebServer.hasArg(PROGMEM2CHAR(STATIC_IP_NAME))) 
				{
					static_ip_buf=STATIC_IP_MODE;
				}
			else
				{
					static_ip_buf=DHCP_MODE;
				}
				
			//IP
			if (web_interface->WebServer.arg(PROGMEM2CHAR(IP_NAME)).length() > MAX_SSID_LENGH)
				{
				stmp = 	web_interface->WebServer.arg(PROGMEM2CHAR(IP_NAME)).substring(0,MAX_SSID_LENGH);
				msg_alert_error=true;
				error_display[6]=1;
				smsg+=PROGMEM2CHAR(ERROR_INCORRECT_IP_FORMAT);
				web_interface->urldecode(ip_buf,stmp.c_str());
				}
			else
				web_interface->urldecode(ip_buf,web_interface->WebServer.arg(PROGMEM2CHAR(IP_NAME)).c_str());
			if (!web_interface->isIPValid(ip_buf))
				{
				msg_alert_error=true;
				smsg+=PROGMEM2CHAR(ERROR_INCORRECT_IP_FORMAT);
				error_display[6]=1;	
				}
				
			//Gateway
			if (web_interface->WebServer.arg(PROGMEM2CHAR(GATEWAY_ID)).length() > MAX_SSID_LENGH)
				{
				stmp = 	web_interface->WebServer.arg(PROGMEM2CHAR(GATEWAY_ID)).substring(0,MAX_SSID_LENGH);
				msg_alert_error=true;
				error_display[7]=1;
				smsg+=PROGMEM2CHAR(ERROR_INCORRECT_IP_FORMAT);
				web_interface->urldecode(gw_buf,stmp.c_str());
				}
			else
				web_interface->urldecode(gw_buf,web_interface->WebServer.arg(PROGMEM2CHAR(GATEWAY_ID)).c_str());
			if (!web_interface->isIPValid(gw_buf))
				{
				msg_alert_error=true;
				smsg+=PROGMEM2CHAR(ERROR_INCORRECT_IP_FORMAT);
				error_display[7]=1;	
				}
			//subnet
			if (web_interface->WebServer.arg(PROGMEM2CHAR(SUBNET_ID)).length() > MAX_SSID_LENGH)
				{
				stmp = 	web_interface->WebServer.arg(PROGMEM2CHAR(SUBNET_ID)).substring(0,MAX_SSID_LENGH);
				msg_alert_error=true;
				error_display[8]=1;
				smsg+=PROGMEM2CHAR(ERROR_INCORRECT_IP_FORMAT);
				web_interface->urldecode(msk_buf,stmp.c_str());
				}
			else
				web_interface->urldecode(msk_buf,web_interface->WebServer.arg(PROGMEM2CHAR(SUBNET_ID)).c_str());
			if (!web_interface->isIPValid(msk_buf))
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
		 char ip_buftmp[15+1];
		 strcpy(ip_buftmp,ip_buf);
		 wifi_config.split_ip(ip_buftmp,ip_sav);
		 strcpy(ip_buftmp,gw_buf);
		 wifi_config.split_ip(ip_buftmp,gw_sav);
		 strcpy(ip_buftmp,msk_buf);
		 wifi_config.split_ip(ip_buftmp,msk_sav);
		 if((!CONFIG::write_byte(EP_WIFI_MODE,CLIENT_MODE))||
			(!CONFIG::write_string(EP_SSID,ssid_buf,strlen(ssid_buf)))||
			(!CONFIG::write_string(EP_PASSWORD,password_buf,strlen(password_buf)))||
			(!CONFIG::write_byte(EP_PHY_MODE,phy_mode_buf))||
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
	//phy mode
	if (!CONFIG::read_byte(EP_PHY_MODE, &phy_mode_buf ))phy_mode_buf=DEFAULT_PHY_MODE;
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
  if (wifi_config.iweb_port!=80)
	{
		stmp+=":";
		stmp+=String(wifi_config.iweb_port);
	}
  buffer2send+=(PROGMEM2CHAR(PAGE_HEAD_1));
  buffer2send+=(PROGMEM2CHAR(PAGE_HEAD_2));
  TOPBAR(stmp.c_str(),4)
  buffer2send+=(PROGMEM2CHAR(PANEL_TOP));
  buffer2send+=(PROGMEM2CHAR(STATION_TITLE));
  buffer2send+=(PROGMEM2CHAR(PANEL_START));
  buffer2send+=(PROGMEM2CHAR(FORM_START));
  
   //if in AP mode switch to mixed mode to be able to scan
  if (wifi_get_opmode()!=WIFI_STA )
	{
		WiFi.mode(WIFI_AP_STA);
		revertSTA=true;
	}
	
  int n = WiFi.scanNetworks();
  buffer2send+=(PROGMEM2CHAR(TABLE_START));
  buffer2send+=(PROGMEM2CHAR(CAPTION_S));
  buffer2send+=(String(n).c_str());
  buffer2send+=(PROGMEM2CHAR(AVAILABLE_APS));
  buffer2send+=(PROGMEM2CHAR(CAPTION_E));
  buffer2send+=(PROGMEM2CHAR(THEAD_S));
  buffer2send+=(PROGMEM2CHAR(TR_S));
  TH_ENTRY(PROGMEM2CHAR(NUMBER_LABEL))
  TH_ENTRY(PROGMEM2CHAR(SSID_ID))
  TH_ENTRY(PROGMEM2CHAR(RSSI_NAME))
  TH_ENTRY(PROGMEM2CHAR(PROTECTED_NAME))
  buffer2send+=(PROGMEM2CHAR(TR_E));
  buffer2send+=(PROGMEM2CHAR(THEAD_E));
  buffer2send+=(PROGMEM2CHAR(TBODY_S));
  for (int i = 0; i < n; ++i)
    {
      // Print SSID and RSSI for each network found
      buffer2send+=(PROGMEM2CHAR(TR_S));
	  THR_ENTRY(String(i+1).c_str())
	  TD_ENTRY(WiFi.SSID(i))
	  stmp = String(100+WiFi.RSSI(i)) + "%";
	  TD_ENTRY(stmp.c_str())
	  TD_ENTRY((WiFi.encryptionType(i) == ENC_TYPE_NONE)?PROGMEM2CHAR(VALUE_NO):PROGMEM2CHAR(VALUE_YES))
    }
   //close table
   buffer2send+=(PROGMEM2CHAR(TABLE_END));
  
 
  //revert to pure softAP
   if (revertSTA) WiFi.mode(WIFI_AP);
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
  //Phy mode
  SELECT_START(PROGMEM2CHAR(AP_4_ID),PROGMEM2CHAR(NETWORK_TITLE),PROGMEM2CHAR(NETWORK_ID))
  if (phy_mode_buf==PHY_MODE_11B)stmp = PROGMEM2CHAR(VALUE_SELECTED);
  else stmp="";
  OPTION(String(PHY_MODE_11B).c_str(), stmp.c_str(),PROGMEM2CHAR(VALUE_11B))
  if (phy_mode_buf==PHY_MODE_11G)stmp = PROGMEM2CHAR(VALUE_SELECTED);
  else stmp="";
  OPTION(String(PHY_MODE_11G).c_str(), stmp.c_str(),PROGMEM2CHAR(VALUE_11G))
  if (phy_mode_buf==PHY_MODE_11N)stmp = PROGMEM2CHAR(VALUE_SELECTED);
  else stmp="";
  OPTION(String(PHY_MODE_11N).c_str(), stmp.c_str(),PROGMEM2CHAR(VALUE_11N))
  SELECT_END
  //static IP ?
  if (static_ip_buf==STATIC_IP_MODE)stmp=PROGMEM2CHAR(CHECKED_VALUE);
  else stmp="";
  INPUT_CHECKBOX(PROGMEM2CHAR(STATIC_IP_NAME), PROGMEM2CHAR(STATIC_IP_LABEL),stmp.c_str())
  // if (static_ip_buf==STATIC_IP_MODE)buffer2send+=(PROGMEM2CHAR(SHOW_IP_BLOCK));
  //else buffer2send+=(PROGMEM2CHAR(HIDE_IP_BLOCK));
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
	//buffer2send+=(PROGMEM2CHAR(DIV_E));

  
  if(msg_alert_error) 
	{
		MSG_ERROR(smsg.c_str()) 
		buffer2send+=(PROGMEM2CHAR(FORM_SUBMIT)); 
	}
  else if(msg_alert_success) 
		{
			MSG_SUCCESS(smsg.c_str()) 
		}
	else buffer2send+=(PROGMEM2CHAR(FORM_SUBMIT)); 
		
 
	buffer2send+=(PROGMEM2CHAR(FORM_END));
	buffer2send+=(PROGMEM2CHAR(PANEL_END)); 
  if (msg_alert_success && !msg_alert_error)
	{
		buffer2send+= PROGMEM2CHAR(RESTARTCMD);
	}
	buffer2send+=(PROGMEM2CHAR(PAGE_BOTTOM));
	web_interface->WebServer.send(200, "text/html", buffer2send);
}


void handle_not_found()
{
  String IP;
  String buffer2send ="";
  if (wifi_get_opmode()==WIFI_STA ) IP=wifi_config.ip2str(WiFi.localIP());
  else IP=wifi_config.ip2str(WiFi.softAPIP());
  if (wifi_config.iweb_port!=80)
	{
		IP+=":";
		IP+=String(wifi_config.iweb_port);
	}
  buffer2send+=(PROGMEM2CHAR(T404_PAGE));
  buffer2send+=(IP.c_str());
  buffer2send+=(PROGMEM2CHAR(T404_PAGE_2));
  web_interface->WebServer.send(200, "text/html", buffer2send);
}

void handle_web_interface_printer()
{
  String IP;
  String stmp;
  String buffer2send ="";
  byte polling_time = 3;
  if (!CONFIG::read_byte(EP_POLLING_TIME, &polling_time ))polling_time=DEFAULT_POLLING_TIME;
  //Serial.println("M114");
  Serial.println("M220");
  Serial.println("M221");
  //display page
  if (wifi_get_opmode()==WIFI_STA ) stmp=wifi_config.ip2str(WiFi.localIP());
  else stmp=wifi_config.ip2str(WiFi.softAPIP());
  if (wifi_config.iweb_port!=80)
	{
		stmp+=":";
		stmp+=String(wifi_config.iweb_port);
	}
  buffer2send+=(PROGMEM2CHAR(PAGE_HEAD_1));
  buffer2send+=(PROGMEM2CHAR(PAGE_HEAD_2));
  TOPBAR(stmp.c_str(),5)
  buffer2send+=(PROGMEM2CHAR(PRINTER_1a));
  buffer2send+=(PROGMEM2CHAR(PRINTER_1b));
  buffer2send+=(PROGMEM2CHAR(PRINTER_1c));
  buffer2send+=(PROGMEM2CHAR(PRINTER_1d));
  buffer2send+=(PROGMEM2CHAR(PRINTER_1e));
  buffer2send+=(PROGMEM2CHAR(PRINTER_1f));
  buffer2send+=(PROGMEM2CHAR(PRINTER_1g));
  buffer2send+=(PROGMEM2CHAR(PRINTER_1h));
  buffer2send+=(PROGMEM2CHAR(PRINTER_1_a_1));
  buffer2send+=stmp.c_str();
  buffer2send+=(PROGMEM2CHAR(PRINTER_1_a_2));
  buffer2send+=(PROGMEM2CHAR(PRINTER_2));
  buffer2send+=stmp.c_str();
  buffer2send+=(PROGMEM2CHAR(PRINTER_3));
  buffer2send+=(PROGMEM2CHAR(PRINTER_4));
  buffer2send+=(PROGMEM2CHAR(PRINTER_5));
  buffer2send+=(PROGMEM2CHAR(PRINTER_6a));
  buffer2send+=(PROGMEM2CHAR(PRINTER_6b));
  buffer2send+=(PROGMEM2CHAR(PRINTER_6c));
  buffer2send+=(PROGMEM2CHAR(PRINTER_6d));
  buffer2send+=(PROGMEM2CHAR(PRINTER_6e));
  buffer2send+=(PROGMEM2CHAR(PRINTER_6f));
  buffer2send+=(PROGMEM2CHAR(PRINTER_6g));
  buffer2send+=(PROGMEM2CHAR(PRINTER_6h));
  buffer2send+=(PROGMEM2CHAR(PRINTER_7));
  buffer2send+=(PROGMEM2CHAR(PRINTER_8));
  buffer2send+=(PROGMEM2CHAR(PRINTER_9));
  buffer2send+=(PROGMEM2CHAR(PRINTER_10));
  buffer2send+=(PROGMEM2CHAR(PRINTER_11a));
  stmp=String(1000*polling_time);
  buffer2send+=stmp.c_str();
  buffer2send+=(PROGMEM2CHAR(PRINTER_11b));
  buffer2send+=(PROGMEM2CHAR(PRINTER_12));
  buffer2send+=(PROGMEM2CHAR(PAGE_BOTTOM));
  web_interface->WebServer.send(200, "text/html", buffer2send);
}

void handle_web_interface_status()
{
	Serial.println("M114");
	String buffer2send =(PROGMEM2CHAR(DATA_S));
	String description;
	String status_color;
	static bool flashit = true;
	int temperature,target;
	flashit=!flashit;
	//request temperature only if no feedback
	if ((system_get_time()-web_interface->last_temp)>2000000)
		Serial.println("M105");
	if ((system_get_time()-web_interface->last_temp)<3200000)
		{
		if (flashit)status_color="lime";
		else status_color="darkgreen";
		}
	else if ((system_get_time()-web_interface->last_temp)<32000000)
		{
		if (flashit)status_color="#FFFF66";
		else status_color="gold";
		}
	else 
		{
		if (flashit)status_color="red";
		else status_color="darkred";
		}
		
	int Tpos = web_interface->answer4M105.indexOf("T:");
	int slashpos = web_interface->answer4M105.indexOf(" /",Tpos);	
	int spacepos = web_interface->answer4M105.indexOf(" ",slashpos+1);
	if(slashpos!=-1 && spacepos!=-1 )
			{
			temperature = (int)atof(web_interface->answer4M105.substring(Tpos+2,slashpos).c_str());
			target = (int)atof(web_interface->answer4M105.substring(slashpos+2,spacepos).c_str());
			Tpos = web_interface->answer4M105.indexOf("T1:");
			if (Tpos>-1) description = "Extruder 1: ";
			else description = "Extruder : ";
			description += String(temperature);
			description += "/";
			if (target >0) description += String(target);
			else description += "off";
			TEMP_SVG(temperature,target,description)
			}
	//check for second extruder
	Tpos = web_interface->answer4M105.indexOf("T1:");
	if (Tpos>-1)
		{
		slashpos = web_interface->answer4M105.indexOf(" /",Tpos);
		spacepos = web_interface->answer4M105.indexOf(" ",slashpos+1);
		if(slashpos!=-1 && spacepos!=-1 )
			{
			temperature = (int)atof(web_interface->answer4M105.substring(Tpos+3,slashpos).c_str());
			target = (int)atof(web_interface->answer4M105.substring(slashpos+2,spacepos).c_str());
			description = "Extruder 2: ";
			description += String(temperature);
			description += "/";
			if (target >0) description += String(target);
			else description += "off";
			buffer2send+=(PROGMEM2CHAR(BR));
			TEMP_SVG(temperature,target,description)
			}
		}
	//check for bed
	Tpos = web_interface->answer4M105.indexOf("B:");
	if (Tpos>-1)
		{
		slashpos = web_interface->answer4M105.indexOf(" /",Tpos);	
		spacepos = web_interface->answer4M105.indexOf(" ",slashpos+1);
		if(slashpos!=-1 && spacepos!=-1 )
			{
			temperature = (int)atof(web_interface->answer4M105.substring(Tpos+2,slashpos).c_str());
			target = (int)atof(web_interface->answer4M105.substring(slashpos+2,spacepos).c_str());
			description = "Bed: ";
			description += String(temperature);
			description += "/";
			if (target >0) description += String(target);
			else description += "off";
			buffer2send+=(PROGMEM2CHAR(BR));
			TEMP_SVG(temperature*2,target*2,description)
			}
		}
	buffer2send+=(PROGMEM2CHAR(DIV_E));
	buffer2send+=(PROGMEM2CHAR(DIV_POSITION));
	int tagpos = web_interface->answer4M114.indexOf("X:");
	int tagpos2 = web_interface->answer4M114.indexOf(" ",tagpos);
	description=web_interface->answer4M114.substring(tagpos+2,tagpos2);
	buffer2send+="<LABEL>X:</LABEL><LABEL class=\"text-info\">";
	buffer2send+=description;
	buffer2send+="</LABEL>&nbsp;&nbsp;<LABEL>Y:</LABEL><LABEL class=\"text-info\">";
	tagpos = web_interface->answer4M114.indexOf("Y:");
	tagpos2 = web_interface->answer4M114.indexOf(" ",tagpos);
	description=web_interface->answer4M114.substring(tagpos+2,tagpos2);
	buffer2send+=description;
	buffer2send+="</LABEL>&nbsp;&nbsp;<LABEL>Z:</LABEL><LABEL class=\"text-info\">";
	tagpos = web_interface->answer4M114.indexOf("Z:");
	tagpos2 = web_interface->answer4M114.indexOf(" ",tagpos);
	description=web_interface->answer4M114.substring(tagpos+2,tagpos2);
	buffer2send+=description;
	buffer2send+="</LABEL>&nbsp;&nbsp;";
	buffer2send+=(PROGMEM2CHAR(DIV_E));
	buffer2send+=(PROGMEM2CHAR(DIV_SPEED));
	buffer2send+=web_interface->answer4M220;
	buffer2send+=(PROGMEM2CHAR(DIV_E));
	buffer2send+=(PROGMEM2CHAR(DIV_FLOW));
	buffer2send+=web_interface->answer4M221;
	buffer2send+=(PROGMEM2CHAR(DIV_E));
	buffer2send+=(PROGMEM2CHAR(DIV_STATUS));
	STATUS_SVG(status_color)
	buffer2send+=(PROGMEM2CHAR(DIV_E));
	int p=1;
	buffer2send+=(PROGMEM2CHAR(DIV_INFOMSG));
	if (web_interface->info_msg.get_used_max_index()>-1)
	for (int i=web_interface->info_msg.get_used_max_index();i>=0;i--)
		{
			buffer2send+=String(p);
			buffer2send+="-";
			p++;
			buffer2send+=web_interface->info_msg.get_index_at(i);
			buffer2send+=(PROGMEM2CHAR(BR));
		}
	buffer2send+=(PROGMEM2CHAR(DIV_E));
	p=1;
	buffer2send+=(PROGMEM2CHAR(DIV_ERRORMSG));
	if (web_interface->error_msg.get_used_max_index()>-1)
	for (int i=web_interface->error_msg.get_used_max_index();i>=0;i--)
		{
			buffer2send+=String(p);
			buffer2send+="-";
			p++;
			buffer2send+=web_interface->error_msg.get_index_at(i);
			buffer2send+=(PROGMEM2CHAR(BR));
		}
	buffer2send+=(PROGMEM2CHAR(DIV_E));
	p=1;
	buffer2send+=(PROGMEM2CHAR(DIV_STATUSMSG));
	if (web_interface->status_msg.get_used_max_index()>-1)
	for (int i=web_interface->status_msg.get_used_max_index();i>=0;i--)
		{	buffer2send+=String(p);
			buffer2send+="-";
			p++;
			buffer2send+=web_interface->status_msg.get_index_at(i);
			buffer2send+=(PROGMEM2CHAR(BR));
		}
	buffer2send+=(PROGMEM2CHAR(DIV_E));
	buffer2send+=(PROGMEM2CHAR(DATA_E));
	web_interface->WebServer.send(200, "text/html", buffer2send);
	
}

void handle_restart()
{
	web_interface->WebServer.send(200,"text/html",PROGMEM2CHAR(RESTARTINGMSG));
    web_interface->restartmodule=true;
}

void handle_web_command()
{
if (web_interface->WebServer.hasArg(PROGMEM2CHAR(COMMAND_ID)))
  {  String cmd = web_interface->WebServer.arg(PROGMEM2CHAR(COMMAND_ID));
	 String param = web_interface->WebServer.arg(PROGMEM2CHAR(PARAM_ID));
	  if (cmd=="M112")
				{	
					Serial.println(cmd.c_str());
				}
  }
}

void handle_css()
{
	web_interface->WebServer.send_P(200, TEXT_HTML, CSS);
}

 #ifdef SSDP_FEATURE
 void handle_SSDP(){
      SSDP.schema(web_interface->WebServer.client());
    }
 #endif

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


//constructor
WEBINTERFACE_CLASS::WEBINTERFACE_CLASS (int port):WebServer(port)
{
  //init what will handle "/"
  WebServer.on("/",HTTP_ANY, handle_web_interface_root);
  WebServer.on("/CONFIGSYS",HTTP_ANY, handle_web_interface_configSys);
  WebServer.on("/CONFIGAP",HTTP_ANY, handle_web_interface_configAP);
  WebServer.on("/CONFIGSTA",HTTP_ANY, handle_web_interface_configSTA);
  WebServer.on("/STATUS",HTTP_ANY, handle_web_interface_status);
  WebServer.on("/PRINTER",HTTP_ANY, handle_web_interface_printer);
  WebServer.on("/CMD",HTTP_ANY, handle_web_command);
  WebServer.on("/mincss.css",HTTP_GET, handle_css);
  WebServer.on("/RESTART",HTTP_GET, handle_restart);
  #ifdef SSDP_FEATURE
  WebServer.on("/description.xml", HTTP_GET, handle_SSDP);
  #endif
  WebServer.onNotFound( handle_not_found);
  answer4M105="T:0 /0 ";
  answer4M114="X:0.0 Y:0.0 Z:0.000";
  answer4M220="100";
  answer4M221="100";
  last_temp=system_get_time();
  restartmodule=false;
}

WEBINTERFACE_CLASS * web_interface;

