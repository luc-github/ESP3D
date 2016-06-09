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
#include <WiFiUdp.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
extern "C" {
#include "user_interface.h"
}
#include <FS.h>
#include "LinkedList.h"
#include "storestrings.h"
#include "command.h"

#ifdef SSDP_FEATURE
#include <ESP8266SSDP.h>
#endif

#define MAX_AUTH_IP 10
#define UPLOAD_STATUS_NONE	0
#define UPLOAD_STATUS_FAILED 1
#define UPLOAD_STATUS_CANCELLED 2
#define UPLOAD_STATUS_SUCCESSFUL 3
#define UPLOAD_STATUS_ONGOING 4

const char PAGE_404 [] PROGMEM ="<HTML>\n<HEAD>\n<title>Redirecting...</title> \n</HEAD>\n<BODY>\n<CENTER>Unknown page - you will be redirected...\n<BR><BR>\nif not redirected, <a href='http://$WEB_ADDRESS$'>click here</a>\n<BR><BR>\n<PROGRESS name='prg' id='prg'></PROGRESS>\n\n<script>\nvar i = 0; \nvar x = document.getElementById(\"prg\"); \nx.max=5; \nvar interval=setInterval(function(){\ni=i+1; \nvar x = document.getElementById(\"prg\"); \nx.value=i; \nif (i>5) \n{\nclearInterval(interval);\nwindow.location.href='/';\n}\n},1000);\n</script>\n</CENTER>\n</BODY>\n</HTML>\n\n";
const char PAGE_RESTART [] PROGMEM ="<HTML>\n<HEAD>\n<title>Restarting...</title> \n</HEAD>\n<BODY>\n<CENTER>Restarting, please wait....\n<BR>\n<PROGRESS name='prg' id='prg'></PROGRESS>\n</CENTER>\n<script>\nvar i = 0;\nvar interval; \nvar x = document.getElementById(\"prg\"); \nx.max=40; \ninterval = setInterval(function(){\ni=i+1; \nvar x = document.getElementById(\"prg\"); \nx.value=i; \nif (i>40) \n{\nclearInterval(interval);\nwindow.location.href='/';\n}\n},1000);\n</script>\n</BODY>\n</HTML>\n";
const char RESTARTCMD [] PROGMEM ="<script>setTimeout(function(){window.location.href='/RESTART'},3000);</script>";
const char VALUE_11B[] PROGMEM = "11b";
const char VALUE_11N[] PROGMEM = "11n";
const char VALUE_11G[] PROGMEM = "11g";
const char VALUE_NONE[] PROGMEM = "None";
const char VALUE_LIGHT[] PROGMEM = "Light";
const char VALUE_MODEM[] PROGMEM = "Modem";
const char VALUE_ENABLED[] PROGMEM = "Enabled";
const char VALUE_DISABLED[] PROGMEM = "Disabled";
const char VALUE_WEP[] PROGMEM = "WEP";
const char VALUE_WPA[] PROGMEM = "WPA";
const char VALUE_WPA2[] PROGMEM = "WPA2";
const char VALUE_WPAWPA2[] PROGMEM = "WPA/WPA2";
const char VALUE_STARTED[] PROGMEM = "Started";
const char VALUE_STOPPED[] PROGMEM = "Stopped";
const char VALUE_NO[] PROGMEM = "No";
const char VALUE_YES[] PROGMEM = "Yes";
const char VALUE_CONNECTED[] PROGMEM = "Connected";
const char VALUE_NO_SSID[] PROGMEM = "SSID not Available!";
const char VALUE_CONNECTION_FAILED[] PROGMEM = "Connection failed!";
const char VALUE_CONNECTION_FAILED2[] PROGMEM = "Connection failed! (Wrong Password)";
const char VALUE_IDLE[] PROGMEM = "Idle";
const char VALUE_DISCONNECTED[] PROGMEM = "Disconnected";
const char VALUE_ITEM_VISIBLE[] PROGMEM = "visibility:visible;";
const char VALUE_ITEM_HIDDEN[] PROGMEM ="visibility:hidden;height:0px;width:0px;padding:0px";
const char KEY_IP[] PROGMEM ="$IP$";
const char KEY_WEB_ADDRESS[] PROGMEM ="$WEB_ADDRESS$";
const char KEY_HOSTNAME[] PROGMEM ="$HOSTNAME$";
const char KEY_HOSTNAME_VISIBLE[] PROGMEM ="$HOSTNAME_VISIBLE$";
const char KEY_NOT_APPLICABLE_4_AP[] PROGMEM ="Not applicable for Access Point";
const char KEY_PAGE_TITLE[] PROGMEM ="$PAGE_TITLE$";
const char VALUE_HOME[] PROGMEM = "Home";
const char VALUE_FW_VERSION[] PROGMEM = FW_VERSION;
const char VALUE_NOT_AVAILABLE [] PROGMEM ="Not available";
const char VALUE_NO_IP [] PROGMEM = "0.0.0.0";
const char KEY_FILE_NAME [] PROGMEM ="$FILE_NAME$";
const char KEY_SHORT_FILE_NAME [] PROGMEM ="$SHORT_FILE_NAME$";
const char KEY_MENU_HOME [] PROGMEM ="$MENU_HOME$";
const char KEY_MENU_SYSTEM [] PROGMEM ="$MENU_SYSTEM$";
const char KEY_MENU_AP [] PROGMEM ="$MENU_AP$";
const char KEY_MENU_STA [] PROGMEM ="$MENU_STA$";
const char KEY_MENU_PRINTER [] PROGMEM ="$MENU_PRINTER$";
const char KEY_MENU_SETTINGS [] PROGMEM ="$MENU_SETTINGS$";
const char KEY_MENU_ADMIN [] PROGMEM ="$MENU_ADMIN$";
const char KEY_FW_VER [] PROGMEM ="$FW_VER$";
const char KEY_CHIP_ID [] PROGMEM ="$CHIP_ID$";
const char KEY_CPU_FREQ [] PROGMEM ="$CPU_FREQ$";
const char KEY_SDK_VER [] PROGMEM ="$SDK_VER$";
const char KEY_MDNS_NAME [] PROGMEM = "$MDNS_NAME$";
const char KEY_MDNS_VISIBLE [] PROGMEM ="$MDNS_VISIBLE$";
const char KEY_CONNECTED_STATIONS_NB_ITEMS [] PROGMEM = "$CONNECTED_STATIONS_NB_ITEMS$";
const char KEY_CAPTIVE_PORTAL_STATUS [] PROGMEM = "$CAPTIVE_PORTAL_STATUS$";
const char KEY_CAPTIVE_PORTAL_VISIBLE [] PROGMEM = "$CAPTIVE_PORTAL_VISIBLE$";
const char KEY_SSDP_STATUS [] PROGMEM ="$SSDP_STATUS$";
const char KEY_SSDP_VISIBLE [] PROGMEM ="$SSDP_VISIBLE$";
const char KEY_NET_PHY [] PROGMEM ="$NET_PHY$";
const char KEY_SLEEP_MODE [] PROGMEM = "$SLEEP_MODE$";
const char KEY_BOOT_VER [] PROGMEM = "$BOOT_VER$";
const char KEY_WEB_PORT [] PROGMEM = "$WEB_PORT$";
const char KEY_DATA_PORT[] PROGMEM = "$DATA_PORT$";
const char KEY_BAUD_RATE [] PROGMEM = "$BAUD_RATE$";
const char KEY_REFRESH_PAGE [] PROGMEM = "$REFRESH_PAGE$";
const char KEY_AP_STATUS_ENABLED [] PROGMEM = "$AP_STATUS_ENABLED$";
const char KEY_AP_VISIBILITY [] PROGMEM = "$AP_VISIBILITY$";
const char KEY_AP_MAC [] PROGMEM = "$AP_MAC$";
const char KEY_AP_SSID [] PROGMEM = "$AP_SSID$";
const char KEY_AP_IS_VISIBLE [] PROGMEM = "$AP_IS_VISIBLE$";
const char KEY_AP_CHANNEL [] PROGMEM = "$AP_CHANNEL$";
const char KEY_AP_AUTH [] PROGMEM = "$AP_AUTH$";
const char KEY_AP_MAX_CON [] PROGMEM = "$AP_MAX_CON$";
const char KEY_AP_DHCP_STATUS [] PROGMEM = "$AP_DHCP_STATUS$";
const char KEY_AP_IP [] PROGMEM = "$AP_IP$";
const char KEY_AP_GW [] PROGMEM = "$AP_GW$";;
const char KEY_AP_SUBNET [] PROGMEM = "$AP_SUBNET$";;
const char KEY_STA_STATUS_ENABLED [] PROGMEM = "$STA_STATUS_ENABLED$";
const char KEY_STA_VISIBILITY [] PROGMEM = "$STA_VISIBILITY$";
const char VALUE_ACTIVE [] PROGMEM = "active";
const char KEY_STA_DHCP_STATUS [] PROGMEM = "$STA_DHCP_STATUS$";
const char KEY_STA_IP [] PROGMEM = "$STA_IP$";
const char KEY_STA_GW [] PROGMEM = "$STA_GW$";
const char KEY_STA_SUBNET [] PROGMEM = "$STA_SUBNET$";
const char KEY_STA_MAC [] PROGMEM = "$STA_MAC$";
const char KEY_STA_SSID [] PROGMEM = "$STA_SSID$";
const char KEY_STA_CHANNEL [] PROGMEM = "$STA_CHANNEL$";
const char KEY_STA_STATUS [] PROGMEM = "$STA_STATUS$";
const char KEY_FREE_MEM [] PROGMEM = "$FREE_MEM$";
const char KEY_SERVICE_PAGE [] PROGMEM = "$SERVICE_PAGE$";
const char ARG_SUBMIT [] PROGMEM = "SUBMIT";
const char KEY_MODE  [] PROGMEM = "$MODE$";
const char VALUE_AP [] PROGMEM = "AP";
const char VALUE_STA [] PROGMEM = "STA";
const char VALUE_AP_STA [] PROGMEM = "AP_STA";
const char KEY_BAUD_RATE_OPTIONS_LIST [] PROGMEM ="$BAUD_RATE_OPTIONS_LIST$";
const char KEY_SLEEP_MODE_OPTIONS_LIST [] PROGMEM ="$SLEEP_MODE_OPTIONS_LIST$";
const char KEY_ERROR_MSG [] PROGMEM ="$ERROR_MSG$";
const char KEY_SUCCESS_MSG [] PROGMEM ="$SUCCESS_MSG$";
const char KEY_ERROR_MSG_VISIBILITY [] PROGMEM ="$ERROR_MSG_VISIBILITY$";
const char KEY_SUCCESS_MSG_VISIBILITY[] PROGMEM  ="$SUCCESS_MSG_VISIBILITY$";
const char KEY_SUBMIT_BUTTON_VISIBILITY [] PROGMEM ="$SUBMIT_BUTTON_VISIBILITY$";
const char VALUE_CONFIG_AP [] PROGMEM = "Configuration Access Point";
const char VALUE_CONFIG_STA [] PROGMEM = "Configuration Station Client";
const char VALUE_PRINTER [] PROGMEM = "Printer Interface";
const char VALUE_HAS_ERROR [] PROGMEM = "has-error";
const char VALUE_HAS_SUCCESS [] PROGMEM = "has-success";
const char KEY_BAUD_RATE_STATUS [] PROGMEM = "$BAUD_RATE_STATUS$";
const char KEY_SLEEP_MODE_STATUS [] PROGMEM ="$SLEEP_MODE_STATUS$";
const char KEY_WEB_PORT_STATUS [] PROGMEM = "$WEB_PORT_STATUS$";
const char KEY_DATA_PORT_STATUS [] PROGMEM ="$DATA_PORT_STATUS$";
const char KEY_AP_SSID_STATUS [] PROGMEM = "$AP_SSID_STATUS$";
const char KEY_AP_PASSWORD_STATUS [] PROGMEM = "$AP_PASSWORD_STATUS$";
const char KEY_NETWORK_OPTION_LIST_STATUS [] PROGMEM = "$NETWORK_OPTION_LIST_STATUS$";
const char KEY_NETWORK_OPTION_LIST [] PROGMEM = "$NETWORK_OPTION_LIST$";
const char KEY_CHANNEL_OPTION_LIST_STATUS [] PROGMEM = "$CHANNEL_OPTION_LIST_STATUS$";
const char KEY_CHANNEL_OPTION_LIST [] PROGMEM = "$CHANNEL_OPTION_LIST$";
const char KEY_AUTH_OPTION_LIST_STATUS [] PROGMEM = "$AUTH_OPTION_LIST_STATUS$";
const char KEY_AUTH_OPTION_LIST [] PROGMEM = "$AUTH_OPTION_LIST$";
const char KEY_AP_IP_STATUS [] PROGMEM = "$AP_IP_STATUS$";
const char KEY_AP_GW_STATUS [] PROGMEM = "$AP_GW_STATUS$";
const char KEY_AP_SUBNET_STATUS [] PROGMEM = "$AP_SUBNET_STATUS$";
const char KEY_AP_PASSWORD [] PROGMEM = "$AP_PASSWORD$";
const char KEY_IS_SSID_VISIBLE_STATUS [] PROGMEM = "$IS_SSID_VISIBLE_STATUS$";
const char KEY_IS_SSID_VISIBLE [] PROGMEM = "$IS_SSID_VISIBLE$";
const char VALUE_CHECKED [] PROGMEM = "checked";
const char VALUE_SELECTED [] PROGMEM ="selected";
const char KEY_IS_STATIC_IP [] PROGMEM = "$IS_STATIC_IP$";
const char KEY_AP_STATIC_IP_STATUS [] PROGMEM = "$AP_STATIC_IP_STATUS$";
const char KEY_STA_STATIC_IP_STATUS [] PROGMEM = "$STA_STATIC_IP_STATUS$";
const char KEY_STA_SSID_STATUS [] PROGMEM = "$STA_SSID_STATUS$";
const char KEY_STA_PASSWORD_STATUS [] PROGMEM = "$STA_PASSWORD_STATUS$";
const char KEY_STA_IP_STATUS [] PROGMEM = "$STA_IP_STATUS$";
const char KEY_STA_GW_STATUS [] PROGMEM = "$STA_GW_STATUS$";
const char KEY_STA_SUBNET_STATUS [] PROGMEM = "$STA_SUBNET_STATUS$";
const char KEY_STA_PASSWORD [] PROGMEM = "$STA_PASSWORD$";
const char KEY_AVAILABLE_AP_NB_ITEMS [] PROGMEM = "$AVAILABLE_AP_NB_ITEMS$";
const char KEY_AP_SCAN_VISIBILITY [] PROGMEM = "$AP_SCAN_VISIBILITY$";
const char KEY_HOSTNAME_STATUS [] PROGMEM = "$HOSTNAME_STATUS$";
const char KEY_XY_FEEDRATE [] PROGMEM = "$XY_FEEDRATE$";
const char KEY_Z_FEEDRATE [] PROGMEM = "$Z_FEEDRATE$";
const char KEY_E_FEEDRATE [] PROGMEM = "$E_FEEDRATE$";
const char KEY_XY_FEEDRATE_STATUS [] PROGMEM = "$XY_FEEDRATE_STATUS$";
const char KEY_Z_FEEDRATE_STATUS [] PROGMEM = "$Z_FEEDRATE_STATUS$";
const char KEY_E_FEEDRATE_STATUS [] PROGMEM = "$E_FEEDRATE_STATUS$";
const char VALUE_SETTINGS [] PROGMEM = "Extra Settings";
const char KEY_REFRESH_PAGE_STATUS [] PROGMEM = "$REFRESH_PAGE_STATUS$";
const char KEY_DISCONNECT_VISIBILITY [] PROGMEM = "$DISCONNECT_VISIBILITY$";
const char VALUE_LOGIN [] PROGMEM = "Login page";
const char KEY_USER_STATUS [] PROGMEM = "$USER_STATUS$";
const char KEY_USER_PASSWORD_STATUS [] PROGMEM = "$USER_PASSWORD_STATUS$";
const char KEY_USER_PASSWORD_STATUS2 [] PROGMEM = "$USER_PASSWORD_STATUS2$";
const char KEY_USER [] PROGMEM = "$USER$";
const char KEY_USER_PASSWORD [] PROGMEM = "$USER_PASSWORD$";
const char KEY_USER_PASSWORD2 [] PROGMEM = "$USER_PASSWORD2$";
const char KEY_RETURN [] PROGMEM = "$RETURN$";
const char VALUE_CHANGE_PASSWORD [] PROGMEM = "Change Password";
const char MISSING_DATA [] PROGMEM = "Error: Missing data";
const char EEPROM_NOWRITE [] PROGMEM = "Error: Cannot write to EEPROM";
const char KEY_WEB_UPDATE [] PROGMEM = "$WEB_UPDATE_VISIBILITY$";
const char KEY_STA_SIGNAL [] PROGMEM = "$STA_SIGNAL$";
const char KEY_DATA_PORT_VISIBILITY [] PROGMEM = "$DATA_PORT_VISIBILITY$";

bool WEBINTERFACE_CLASS::isHostnameValid(const char * hostname)
{
    //limited size
    char c;
    if (strlen(hostname)>MAX_HOSTNAME_LENGTH || strlen(hostname) < 1) {
        return false;
    }
    //only letter and digit
    for (int i=0; i < strlen(hostname); i++) {
        c = hostname[i];
        if (!(isdigit(c) || isalpha(c) || c=='_')) {
            return false;
        }
        if (c==' ') {
            return false;
        }
    }
    return true;
}

bool WEBINTERFACE_CLASS::isSSIDValid(const char * ssid)
{
    //limited size
    char c;
    if (strlen(ssid)>MAX_SSID_LENGTH || strlen(ssid)<MIN_SSID_LENGTH) {
        return false;
    }
    //only letter and digit
    for (int i=0; i < strlen(ssid); i++) {
        c = ssid[i];
        //if (!(isdigit(c) || isalpha(c))) return false;
        if (c==' ') {
            return false;
        }
    }
    return true;
}

bool WEBINTERFACE_CLASS::isPasswordValid(const char * password)
{
    //limited size
    if ((strlen(password)>MAX_PASSWORD_LENGTH)||  (strlen(password)<MIN_PASSWORD_LENGTH)) {
        return false;
    }
    //no space allowed
    for (int i=0; i < strlen(password); i++)
        if (password[i] == ' ') {
            return false;
        }

    return true;
}

bool WEBINTERFACE_CLASS::isAdminPasswordValid(const char * password)
{
    char c;
    //limited size
    if ((strlen(password)>MAX_ADMIN_PASSWORD_LENGTH)||  (strlen(password)<MIN_ADMIN_PASSWORD_LENGTH)) {
        return false;
    }
    //no space allowed
    for (int i=0; i < strlen(password); i++) {
        c= password[i];
        if (c==' ') {
            return false;
        }
    }
    return true;
}

bool WEBINTERFACE_CLASS::isIPValid(const char * IP)
{
    //limited size
    int internalcount=0;
    int dotcount = 0;
    bool previouswasdot=false;
    char c;

    if (strlen(IP)>15 || strlen(IP)==0) {
        return false;
    }
    //cannot start with .
    if (IP[0]=='.') {
        return false;
    }
    //only letter and digit
    for (int i=0; i < strlen(IP); i++) {
        c = IP[i];
        if (isdigit(c)) {
            //only 3 digit at once
            internalcount++;
            previouswasdot=false;
            if (internalcount>3) {
                return false;
            }
        } else if(c=='.') {
            //cannot have 2 dots side by side
            if (previouswasdot) {
                return false;
            }
            previouswasdot=true;
            internalcount=0;
            dotcount++;
        }//if not a dot neither a digit it is wrong
        else {
            return false;
        }
    }
    //if not 3 dots then it is wrong
    if (dotcount!=3) {
        return false;
    }
    //cannot have the last dot as last char
    if (IP[strlen(IP)-1]=='.') {
        return false;
    }
    return true;
}

//TODO should be in some tool class
char * intTostr(int value)
{
    static char result [12];
    sprintf(result,"%d",value);
    return result;
}

//TODO: should be in webserver class
bool processTemplate(const char  * filename, STORESTRINGS_CLASS & KeysList ,  STORESTRINGS_CLASS & ValuesList )
{
    if(KeysList.size() != ValuesList.size()) { //Sanity check
		Serial.print("Error");
        return false;
    }
    
    LinkedList<File> myFileList  = LinkedList<File>();
    String  buffer2send;
    String bufferheader(F("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: "));
    int size_content=0;
    bool header_sent=false;

    //one loop to calculate size + one loop to really send
    //size_content is a mandatory element to avoid memory leak
    for(int processing_step=0; processing_step<2; processing_step++) {
        buffer2send="";
        //open template file
        File currentfile = SPIFFS.open(filename, "r");
        //if error display error on web page
        if (!currentfile) {
            buffer2send = String(F("Error opening: ")) + filename;
            web_interface->WebServer.send(200,"text/plain",buffer2send);
            return false;
        } else { //template is open
            int b ;
            String sLine;
            bool bprocessing=true;

            while (bprocessing) { //read all bytes
                b = currentfile.read(); //from current open file
                if (b!=-1) { //if not EOF
                    sLine+=char(b); //add to current line
                    if (b=='\n') { //end of line is reached
                        //change all variables by their values
                        for (int k=0; k<KeysList.size(); k++) {
                            sLine.replace(KeysList.get(k),ValuesList.get(k));
                        }
                        //is line an Include line ? no others command will be displayed
                        //but they can be used to build file name like
                        //$INCLUDE[$SHORT_FILENAME$-$MODE$.inc]$
                        int pos_tag=sLine.indexOf("$INCLUDE[");
                        if (pos_tag!=-1) { //if yes
                            //extract include file name
                            int pos_tag_end = sLine.indexOf("]$");
                            String includefilename = "/"+sLine.substring( pos_tag+strlen("$INCLUDE["),pos_tag_end);
                            //try to open include file
                            File includefile = SPIFFS.open(includefilename, "r");
                            if (!includefile) { //if error display it on web page
                                buffer2send+= String("Error opening: ") + includefilename;
                            } else { //if include is open lets read it, current open file is now include file
                                myFileList.add(currentfile);
                                currentfile=includefile;
                            }
                        } else { //if it is not include file
                            //check if there is a table to display
                            int pos_tag=sLine.indexOf("$CONNECTED_STATIONS[");
                            if (pos_tag!=-1) { //if yes
                                //extract line
                                int pos_tag_end = sLine.indexOf("]$",pos_tag);
                                int nb = -1;
                                int ipos = -1;
                                //part before repetitive section
                                String beforelinetoprocess=sLine.substring( 0,pos_tag);
                                //part after repetitive section
                                String afterlinetoprocess=sLine.substring( pos_tag_end+2);
                                //repetitive section itself
                                String linetoprocess =sLine.substring( pos_tag+strlen("$CONNECTED_STATIONS["),pos_tag_end);
                                String tablepart;
                                //get how many items
                                ipos=KeysList.get_index("$CONNECTED_STATIONS_NB_ITEMS$");
                                if (ipos >-1) {
                                    //get value
                                    nb=atoi(ValuesList.get(ipos));
                                    ipos=ipos-(nb*3);
                                }
                                //do a sanity check data are there
                                String Last_IP_Key = "$IP_CONNECTED["+String(nb-1)+"]$";
                                if (nb>0 && (KeysList.get_index("$ROW_NUMBER[0]$")==ipos) &&(Last_IP_Key==KeysList.get(ipos-1+(nb*3)))) {
                                    for (int j=0; j<nb; j++) {
                                        String tmppart=linetoprocess + "\n";
                                        if (ipos+j>-1) {
                                            tmppart.replace("$ROW_NUMBER$",ValuesList.get(ipos+0+(j*3)));
                                            tmppart.replace("$MAC_CONNECTED$",ValuesList.get(ipos+1+(j*3)));
                                            tmppart.replace("$IP_CONNECTED$",ValuesList.get(ipos+2+(j*3)));
                                        }
                                        tablepart +=tmppart;
                                    }
                                }
                                //now build back
                                sLine = beforelinetoprocess + tablepart + afterlinetoprocess;
                            }

                            pos_tag=sLine.indexOf("$AVAILABLE_AP[");
                            if (pos_tag!=-1) { //if yes
                                //extract line
                                int pos_tag_end = sLine.indexOf("]$",pos_tag);
                                int nb = -1;
                                int ipos = -1;
                                //part before repetitive section
                                String beforelinetoprocess=sLine.substring( 0,pos_tag);
                                //part after repetitive section
                                String afterlinetoprocess=sLine.substring( pos_tag_end+2);
                                //repetitive section itself
                                String linetoprocess =sLine.substring( pos_tag+strlen("$AVAILABLE_AP["),pos_tag_end);
                                String tablepart;
                                //get how many items
                                ipos=KeysList.get_index("$AVAILABLE_AP_NB_ITEMS$");
                                if (ipos >-1) {
                                    //get value
                                    nb=atoi(ValuesList.get(ipos));
                                    ipos=ipos-(nb*4);
                                }
                                //do a sanity check data are there
                                String Last_IP_Key = "$IS_PROTECTED["+String(nb-1)+"]$";
                                if (nb>0 && (KeysList.get_index("$ROW_NUMBER[0]$")==ipos) &&(Last_IP_Key==KeysList.get(ipos-1+(nb*4)))) {
                                    for (int j=0; j<nb; j++) {
                                        String tmppart=linetoprocess + "\n";
                                        if (ipos+j>-1) {
                                            tmppart.replace("$ROW_NUMBER$",ValuesList.get(ipos+0+(j*4)));
                                            tmppart.replace("$AP_SSID$",ValuesList.get(ipos+1+(j*4)));
                                            tmppart.replace("$AP_SIGNAL$",ValuesList.get(ipos+2+(j*4)));
                                            tmppart.replace("$IS_PROTECTED$",ValuesList.get(ipos+3+(j*4)));
                                        }
                                        tablepart +=tmppart;
                                    }
                                }
                                //now build back
                                sLine = beforelinetoprocess + tablepart + afterlinetoprocess;
                            }

                            //add current line to buffer
                            buffer2send+=sLine;
                            //if buffer limit is reached
                            if (buffer2send.length()>1200) {
                                //if we are just doing size calculation
                                if (processing_step==0) {
                                    //add buffer size
                                    size_content+=buffer2send.length();
                                } else { //if no size calculation, send data
                                    //if header is not send yet
                                    if (!header_sent) {
                                        //send header with calculated size
                                        header_sent=true;
                                        web_interface->WebServer.sendContent(bufferheader);
                                        
                                    }
                                    //send data
                                    web_interface->WebServer.sendContent(buffer2send);
                                }
                                //reset buffer
                                buffer2send="";
                            }
                        }
                        //reset line
                        sLine="";
                        //add a delay for safety for WDT
                        delay(1);
                    }
                } else { //EOF is reached
                    //close current file
                    currentfile.close();
                    //if current file is not template file but included one
                    if (myFileList.size()>0) {
                        //get level +1 file description and continue
                        currentfile = myFileList.pop();
                    } else {
                        //we have done template parsing, let's stop reading
                        bprocessing=false;
                    }
                }
            }
        }
        //check if something is still in buffer and need to be send
        if (buffer2send!="") {
            //if we are doing size calculation
            if (processing_step==0) {
                //add buffer size
                size_content+=buffer2send.length();
            } else { //if no size calculation, send data
                //if header is not send yet
                if (!header_sent) {
                    //send header with calculated size
                    web_interface->WebServer.sendContent(bufferheader);
                }
                //send data
                web_interface->WebServer.sendContent(buffer2send);
                
            }
        }
        //if we end size calculation loop
        if (processing_step==0) {
            //let's build the header with correct size'
            bufferheader.concat(size_content);
            bufferheader.concat(F("\r\nConnection: close\r\nAccess-Control-Allow-Origin: *\r\n\r\n"));
        }
    }
    return true;
}
// -----------------------------------------------------------------------------
// Helper for FreeMem and Firmware
// -----------------------------------------------------------------------------
void GetFreeMem(STORESTRINGS_CLASS & KeysList, STORESTRINGS_CLASS & ValuesList)
{
    //FreeMem
    KeysList.add(FPSTR(KEY_FREE_MEM));
    ValuesList.add(intTostr(system_get_free_heap_size()));
    //FW Version
    KeysList.add(FPSTR(KEY_FW_VER));
    ValuesList.add(FPSTR(VALUE_FW_VERSION));
}
// -----------------------------------------------------------------------------
// Helper for IP+Web address
// -----------------------------------------------------------------------------
void GetIpWeb(STORESTRINGS_CLASS & KeysList, STORESTRINGS_CLASS & ValuesList)
{
    String stmp;

    KeysList.add(FPSTR(KEY_IP));
    if (wifi_get_opmode() == WIFI_STA ) {
        stmp = WiFi.localIP().toString();
    } else {
        stmp = WiFi.softAPIP().toString();
    }
    ValuesList.add(stmp);

    //Web address = ip + port
    KeysList.add(FPSTR(KEY_WEB_ADDRESS));
    if (wifi_config.iweb_port != 80) {
        stmp.concat(":");
        stmp.concat(wifi_config.iweb_port);
    }
    ValuesList.add(stmp);
}
// -----------------------------------------------------------------------------
// Helper for Wifi Mode
// -----------------------------------------------------------------------------
void GetMode(STORESTRINGS_CLASS & KeysList, STORESTRINGS_CLASS & ValuesList)
{
    if (wifi_get_opmode() == WIFI_STA ) {
        KeysList.add(FPSTR(KEY_MODE));
        ValuesList.add(FPSTR(VALUE_STA));
    } else {
        if (wifi_get_opmode() == WIFI_AP ) {
            KeysList.add(FPSTR(KEY_MODE));
            ValuesList.add(FPSTR(VALUE_AP));
        } else {
            KeysList.add(FPSTR(KEY_MODE));
            ValuesList.add(FPSTR(VALUE_AP_STA));
        }
    }
}
// -----------------------------------------------------------------------------
// Helper for Web ports
// -----------------------------------------------------------------------------
void GetPorts(STORESTRINGS_CLASS & KeysList, STORESTRINGS_CLASS & ValuesList)
{
    //Web port
    KeysList.add(FPSTR(KEY_WEB_PORT));
    ValuesList.add(intTostr(wifi_config.iweb_port));
    //Data port
    KeysList.add(FPSTR(KEY_DATA_PORT));
    KeysList.add(FPSTR(KEY_DATA_PORT_VISIBILITY));
#ifdef TCP_IP_DATA_FEATURE
    ValuesList.add(intTostr(wifi_config.idata_port));
    ValuesList.add(FPSTR(VALUE_ITEM_VISIBLE));
#else
    ValuesList.add(FPSTR(VALUE_NONE));
    ValuesList.add(FPSTR(VALUE_ITEM_HIDDEN));
#endif
}
// -----------------------------------------------------------------------------
// Helper for Page properties
// -----------------------------------------------------------------------------
void SetPageProp(STORESTRINGS_CLASS & KeysList, STORESTRINGS_CLASS & ValuesList,
                 const __FlashStringHelper *title, const __FlashStringHelper *filename)
{
    String fullFilename(filename);
    fullFilename.concat(".tpl");

    //page title
    KeysList.add(FPSTR(KEY_PAGE_TITLE));
    ValuesList.add(title);
    //tpl file name with extension
    KeysList.add(FPSTR(KEY_FILE_NAME));
    ValuesList.add(fullFilename);
    //tpl file name without extension
    KeysList.add(FPSTR(KEY_SHORT_FILE_NAME));
    ValuesList.add(filename);
}

// -----------------------------------------------------------------------------
// Helper for DHCP (APP/STA)tus
// -----------------------------------------------------------------------------
void GetDHCPStatus(STORESTRINGS_CLASS & KeysList, STORESTRINGS_CLASS & ValuesList)
{
    KeysList.add(FPSTR(KEY_AP_DHCP_STATUS));
    if (wifi_softap_dhcps_status() == DHCP_STARTED) {
        ValuesList.add(FPSTR(VALUE_STARTED));
    } else {
        ValuesList.add(FPSTR(VALUE_STOPPED));
    }

    KeysList.add(FPSTR(KEY_STA_DHCP_STATUS));
    if (wifi_station_dhcpc_status()==DHCP_STARTED) {
        ValuesList.add(FPSTR(VALUE_STARTED));
    } else {
        ValuesList.add(FPSTR(VALUE_STOPPED));
    }
}

// -----------------------------------------------------------------------------
// Helper for Error Msg processing
// -----------------------------------------------------------------------------
void ProcessAlertError(STORESTRINGS_CLASS & KeysList, STORESTRINGS_CLASS & ValuesList, String & smsg)
{
    KeysList.add(FPSTR(KEY_ERROR_MSG));
    ValuesList.add(smsg);
    KeysList.add(FPSTR(KEY_SUCCESS_MSG));
    ValuesList.add("");
    KeysList.add(FPSTR(KEY_ERROR_MSG_VISIBILITY ));
    ValuesList.add(FPSTR(VALUE_ITEM_VISIBLE));
    KeysList.add(FPSTR(KEY_SUCCESS_MSG_VISIBILITY));
    ValuesList.add(FPSTR(VALUE_ITEM_HIDDEN));
    KeysList.add(FPSTR(KEY_SUBMIT_BUTTON_VISIBILITY));
    ValuesList.add(FPSTR(VALUE_ITEM_VISIBLE));
    KeysList.add(FPSTR(KEY_SERVICE_PAGE));
    ValuesList.add("");
}

// -----------------------------------------------------------------------------
// Helper for Success Msg processing
// -----------------------------------------------------------------------------
void ProcessAlertSuccess(STORESTRINGS_CLASS & KeysList, STORESTRINGS_CLASS & ValuesList, String & smsg)
{
    KeysList.add(FPSTR(KEY_ERROR_MSG));
    ValuesList.add("");
    KeysList.add(FPSTR(KEY_SUCCESS_MSG));
    ValuesList.add(smsg);
    KeysList.add(FPSTR(KEY_ERROR_MSG_VISIBILITY ));
    ValuesList.add(FPSTR(VALUE_ITEM_HIDDEN));
    KeysList.add(FPSTR(KEY_SUCCESS_MSG_VISIBILITY));
    ValuesList.add(FPSTR(VALUE_ITEM_VISIBLE));
    KeysList.add(FPSTR(KEY_SUBMIT_BUTTON_VISIBILITY));
    ValuesList.add(FPSTR(VALUE_ITEM_HIDDEN));
}

// -----------------------------------------------------------------------------
// Helper for No Msg processing
// -----------------------------------------------------------------------------
void ProcessNoAlert(STORESTRINGS_CLASS & KeysList, STORESTRINGS_CLASS & ValuesList)
{
    KeysList.add(FPSTR(KEY_ERROR_MSG));
    ValuesList.add("");
    KeysList.add(FPSTR(KEY_SUCCESS_MSG));
    ValuesList.add("");
    KeysList.add(FPSTR(KEY_ERROR_MSG_VISIBILITY ));
    ValuesList.add(FPSTR(VALUE_ITEM_HIDDEN));
    KeysList.add(FPSTR(KEY_SUCCESS_MSG_VISIBILITY));
    ValuesList.add(FPSTR(VALUE_ITEM_HIDDEN));
    KeysList.add(FPSTR(KEY_SUBMIT_BUTTON_VISIBILITY));
    ValuesList.add(FPSTR(VALUE_ITEM_VISIBLE));
    KeysList.add(FPSTR(KEY_SERVICE_PAGE));
    ValuesList.add("");
}

//root insterface
void handle_web_interface_root()
{
	static const char HOME_PAGE [] PROGMEM = "HTTP/1.1 301 OK\r\nLocation: /HOME\r\nCache-Control: no-cache\r\n\r\n";
	web_interface->WebServer.sendContent_P(HOME_PAGE);
}

//root insterface
void handle_web_interface_home()
{
    String stmp;
    long lstatus;
    int istatus;
    byte bbuf;
    STORESTRINGS_CLASS KeysList ;
    STORESTRINGS_CLASS ValuesList ;
    struct softap_config apconfig;
    struct ip_info info;
    uint8_t mac [WL_MAC_ADDR_LENGTH];

    KeysList.add(FPSTR(KEY_DISCONNECT_VISIBILITY));
    if (web_interface->is_authenticated()) {
        ValuesList.add(FPSTR(VALUE_ITEM_VISIBLE));
    } else {
        ValuesList.add(FPSTR(VALUE_ITEM_HIDDEN));
    }

    //IP+Web
    GetIpWeb(KeysList, ValuesList);

    //Hostname
    if (wifi_get_opmode()==WIFI_STA ) {
        KeysList.add(FPSTR(KEY_MODE));
        ValuesList.add(FPSTR(VALUE_STA));
        KeysList.add(FPSTR(KEY_HOSTNAME));
        ValuesList.add(wifi_config.get_hostname());
        KeysList.add(FPSTR(KEY_HOSTNAME_VISIBLE));
        ValuesList.add(FPSTR(VALUE_ITEM_VISIBLE));
    } else {
        KeysList.add(FPSTR(KEY_HOSTNAME));
        ValuesList.add(FPSTR(KEY_NOT_APPLICABLE_4_AP));
        KeysList.add(FPSTR(KEY_HOSTNAME_VISIBLE));
        ValuesList.add(FPSTR(VALUE_ITEM_HIDDEN));
        if (wifi_get_opmode()==WIFI_AP ) {
            KeysList.add(FPSTR(KEY_MODE));
            ValuesList.add(FPSTR(VALUE_AP));
        } else {
            KeysList.add(FPSTR(KEY_MODE));
            ValuesList.add(FPSTR(VALUE_AP_STA));
        }
    }

    //page title and filenames
    SetPageProp(KeysList,ValuesList,FPSTR(VALUE_HOME),F("home"));
    //menu item
    KeysList.add(FPSTR(KEY_MENU_HOME));
    ValuesList.add(FPSTR(VALUE_ACTIVE));

    //Chip ID
    KeysList.add(FPSTR(KEY_CHIP_ID));
    ValuesList.add(intTostr(system_get_chip_id()));
    //CPU Freq
    KeysList.add(FPSTR(KEY_CPU_FREQ));
    ValuesList.add(intTostr(system_get_cpu_freq()));
    //SDK Version
    KeysList.add(FPSTR(KEY_SDK_VER));
    ValuesList.add(system_get_sdk_version());

    //MDNS Feature
#ifdef MDNS_FEATURE
    KeysList.add(FPSTR(KEY_MDNS_NAME));
    stmp="http://";
    stmp+=wifi_config.get_hostname();
    stmp+=".local";
    ValuesList.add(stmp);
    KeysList.add(FPSTR(KEY_MDNS_VISIBLE));
    ValuesList.add(FPSTR(VALUE_ITEM_VISIBLE));
#else
    KeysList.add(FPSTR(KEY_MDNS_NAME));
    ValuesList.add(FPSTR(VALUE_DISABLED));
    KeysList.add(FPSTR(KEY_MDNS_VISIBLE));
    ValuesList.add(FPSTR(VALUE_ITEM_HIDDEN));
#endif

    //SSDP Feature
#ifdef SSDP_FEATURE
    KeysList.add(FPSTR(KEY_SSDP_STATUS));
    ValuesList.add(FPSTR(VALUE_ENABLED));
    KeysList.add(FPSTR(KEY_SSDP_VISIBLE));
    ValuesList.add(FPSTR(VALUE_ITEM_VISIBLE));
#else
    KeysList.add(FPSTR(KEY_SSDP_STATUS));
    ValuesList.add(FPSTR(VALUE_DISABLED));
    KeysList.add(FPSTR(KEY_SSDP_VISIBLE));
    ValuesList.add(FPSTR(VALUE_ITEM_HIDDEN));
#endif

    //Captive portal Feature
#ifdef CAPTIVE_PORTAL_FEATURE
    if (wifi_get_opmode()==WIFI_AP) {
        KeysList.add(FPSTR(KEY_CAPTIVE_PORTAL_STATUS));
        ValuesList.add(FPSTR(VALUE_ENABLED));
        KeysList.add(FPSTR(KEY_CAPTIVE_PORTAL_VISIBLE));
        ValuesList.add(FPSTR(VALUE_ITEM_VISIBLE));
    } else {
        KeysList.add(FPSTR(KEY_CAPTIVE_PORTAL_STATUS));
        ValuesList.add(FPSTR(VALUE_DISABLED));
        KeysList.add(FPSTR(KEY_CAPTIVE_PORTAL_VISIBLE));
        ValuesList.add(FPSTR(VALUE_ITEM_HIDDEN));
    }
#else
    KeysList.add(FPSTR(KEY_CAPTIVE_PORTAL_STATUS));
    ValuesList.add(FPSTR(VALUE_DISABLED));
    KeysList.add(FPSTR(KEY_CAPTIVE_PORTAL_VISIBLE));
    ValuesList.add(FPSTR(VALUE_ITEM_HIDDEN));
#endif

    //network
    KeysList.add(FPSTR(KEY_NET_PHY));
    if (WiFi.getPhyMode()==WIFI_PHY_MODE_11B) {
        ValuesList.add(FPSTR(VALUE_11B));
    } else if (WiFi.getPhyMode()==WIFI_PHY_MODE_11G) {
        ValuesList.add(FPSTR(VALUE_11G));
    } else {
        ValuesList.add(FPSTR(VALUE_11N));
    }
    //sleep mode
    KeysList.add(FPSTR(KEY_SLEEP_MODE));
    if (WiFi.getSleepMode()==WIFI_NONE_SLEEP) {
        ValuesList.add(FPSTR(VALUE_NONE));
    } else if (WiFi.getSleepMode()==WIFI_LIGHT_SLEEP) {
        ValuesList.add(FPSTR(VALUE_LIGHT));
    } else {
        ValuesList.add(FPSTR(VALUE_MODEM));
    }
    //Boot version
    KeysList.add(FPSTR(KEY_BOOT_VER));
    ValuesList.add(intTostr(system_get_boot_version()));
    //Baud rate
    KeysList.add(FPSTR(KEY_BAUD_RATE));
    ValuesList.add(intTostr(wifi_config.baud_rate));
    // Web and Data ports
    GetPorts(KeysList, ValuesList);

    //AP part
    if (wifi_get_opmode()==WIFI_AP ||  wifi_get_opmode()==WIFI_AP_STA) {
        int client_counter=0;
        struct station_info * station;
        //AP is enabled
        KeysList.add(FPSTR(KEY_AP_STATUS_ENABLED));
        ValuesList.add(FPSTR(VALUE_ENABLED));
        //set visible
        KeysList.add(FPSTR(KEY_AP_VISIBILITY));
        ValuesList.add(FPSTR(VALUE_ITEM_VISIBLE));
        //list of connected clients
        station = wifi_softap_get_station_info();
        while(station) {
            //Row number
            stmp = "$ROW_NUMBER["+String(client_counter)+"]$";
            KeysList.add(stmp.c_str());
            stmp=String(client_counter+1);
            ValuesList.add(stmp);
            //BSSID
            stmp = "$MAC_CONNECTED["+String(client_counter)+"]$";
            KeysList.add(stmp.c_str());
            ValuesList.add(wifi_config.mac2str(station->bssid));
            //IP
            stmp = "$IP_CONNECTED["+String(client_counter)+"]$";
            KeysList.add(stmp.c_str());
            ValuesList.add(IPAddress((const uint8_t *)&station->ip).toString().c_str());
            //increment counter
            client_counter++;
            //go next record
            station = STAILQ_NEXT(station, next);
        }
        wifi_softap_free_station_info();
        //Connected clients
        KeysList.add(FPSTR(KEY_CONNECTED_STATIONS_NB_ITEMS));
        ValuesList.add(intTostr(client_counter));
    } else {
        //AP is disabled
        KeysList.add(FPSTR(KEY_AP_STATUS_ENABLED));
        ValuesList.add(FPSTR(VALUE_DISABLED));
        //set hide
        KeysList.add(FPSTR(KEY_AP_VISIBILITY));
        ValuesList.add(FPSTR(VALUE_ITEM_HIDDEN));
        //Connected clients
        KeysList.add(FPSTR(KEY_CONNECTED_STATIONS_NB_ITEMS));
        ValuesList.add("0");
    }
    //AP mac address
    KeysList.add(FPSTR(KEY_AP_MAC));
    ValuesList.add(wifi_config.mac2str(WiFi.softAPmacAddress(mac)));
    //AP configuration
    if (wifi_softap_get_config(&apconfig)) {
        //SSID
        KeysList.add(FPSTR(KEY_AP_SSID));
        ValuesList.add((char *)(apconfig.ssid));
        //AP visible or hidden
        KeysList.add(FPSTR(KEY_AP_IS_VISIBLE));
        if(apconfig.ssid_hidden==1) {
            ValuesList.add(FPSTR(VALUE_NO));
        } else {
            ValuesList.add(FPSTR(VALUE_YES));
        }
        //Channel
        KeysList.add(FPSTR(KEY_AP_CHANNEL));
        ValuesList.add(intTostr(apconfig.channel));
        //Authentification mode
        KeysList.add(FPSTR(KEY_AP_AUTH));
        if (apconfig.authmode==AUTH_OPEN) {
            ValuesList.add(FPSTR(VALUE_NONE));
        } else if (apconfig.authmode==AUTH_WEP) {
            ValuesList.add(FPSTR(VALUE_WEP));
        } else if (apconfig.authmode==AUTH_WPA_PSK) {
            ValuesList.add(FPSTR(VALUE_WPA));
        } else if (apconfig.authmode==AUTH_WPA2_PSK) {
            ValuesList.add(FPSTR(VALUE_WPA2));
        } else {
            ValuesList.add(FPSTR(VALUE_WPAWPA2));
        }

        //Max connections
        KeysList.add(FPSTR(KEY_AP_MAX_CON));
        ValuesList.add(intTostr(apconfig.max_connection));
    } else {
        //SSID
        KeysList.add(FPSTR(KEY_AP_SSID));
        ValuesList.add(FPSTR(VALUE_NOT_AVAILABLE));
        //AP visible or hidden
        KeysList.add(FPSTR(KEY_AP_IS_VISIBLE));
        ValuesList.add(FPSTR(VALUE_NOT_AVAILABLE));
        //Channel
        KeysList.add(FPSTR(KEY_AP_CHANNEL));
        ValuesList.add(FPSTR(VALUE_NOT_AVAILABLE));
        //Authentification mode
        KeysList.add(FPSTR(KEY_AP_AUTH));
        ValuesList.add(FPSTR(VALUE_NOT_AVAILABLE));
        //Max connections
        KeysList.add(FPSTR(KEY_AP_MAX_CON));
        ValuesList.add(FPSTR(VALUE_NOT_AVAILABLE));
    }
    //DHCP Status
    GetDHCPStatus(KeysList, ValuesList);
    //IP/GW/MASK
    if (wifi_get_ip_info(SOFTAP_IF,&info)) {
        //IP address
        KeysList.add(FPSTR(KEY_AP_IP));
        ValuesList.add(IPAddress((const uint8_t *)&(info.ip.addr)).toString().c_str());
        //GW address
        KeysList.add(FPSTR(KEY_AP_GW));
        ValuesList.add(IPAddress((const uint8_t *)&(info.gw.addr)).toString().c_str());
        //Sub Net Mask
        KeysList.add(FPSTR(KEY_AP_SUBNET));
        ValuesList.add(IPAddress((const uint8_t *)&(info.netmask.addr)).toString().c_str());
    } else {
        //IP address
        KeysList.add(FPSTR(KEY_AP_IP));
        ValuesList.add(FPSTR(VALUE_NO_IP));
        //GW address
        KeysList.add(FPSTR(KEY_AP_GW));
        ValuesList.add(FPSTR(VALUE_NO_IP));
        //Sub Net Mask
        KeysList.add(FPSTR(KEY_AP_SUBNET));
        ValuesList.add(FPSTR(VALUE_NO_IP));
    }
    //STA part
    if (wifi_get_opmode()==WIFI_STA ||  wifi_get_opmode()==WIFI_AP_STA) {
        //STA is enabled
        KeysList.add(FPSTR(KEY_STA_STATUS_ENABLED));
        ValuesList.add(FPSTR(VALUE_ENABLED));
        //set visible
        KeysList.add(FPSTR(KEY_STA_VISIBILITY));
        ValuesList.add(FPSTR(VALUE_ITEM_VISIBLE));
    } else {
        //STA is disabled
        KeysList.add(FPSTR(KEY_STA_STATUS_ENABLED));
        ValuesList.add(FPSTR(VALUE_DISABLED));
        //set hide
        KeysList.add(FPSTR(KEY_STA_VISIBILITY));
        ValuesList.add(FPSTR(VALUE_ITEM_HIDDEN));
    }
    //STA mac address
    KeysList.add(FPSTR(KEY_STA_MAC));
    ValuesList.add(wifi_config.mac2str(WiFi.macAddress(mac)));
    //SSID used by STA
    KeysList.add(FPSTR(KEY_STA_SSID));
    if (WiFi.SSID().length()==0) {
        ValuesList.add(FPSTR(VALUE_NOT_AVAILABLE));
    } else {
        ValuesList.add(WiFi.SSID().c_str());
    }
    //Channel
    KeysList.add(FPSTR(KEY_STA_CHANNEL));
    ValuesList.add(intTostr (wifi_get_channel()));
    //Connection status
    istatus = wifi_station_get_connect_status();
    KeysList.add(FPSTR(KEY_STA_STATUS));
    if (istatus==STATION_GOT_IP) {
        ValuesList.add(FPSTR(VALUE_CONNECTED));
    } else if  (istatus==STATION_NO_AP_FOUND) {
        ValuesList.add(FPSTR(VALUE_NO_SSID));
    } else if  (istatus==STATION_CONNECT_FAIL) {
        ValuesList.add(FPSTR(VALUE_CONNECTION_FAILED));
    } else if  (istatus==STATION_WRONG_PASSWORD) {
        ValuesList.add(FPSTR(VALUE_CONNECTION_FAILED2));
    } else if  (istatus==STATION_IDLE) {
        ValuesList.add(FPSTR(VALUE_IDLE));    //should not happen
    } else {
        ValuesList.add(FPSTR(VALUE_DISCONNECTED));
    }
    //Signal strength
    KeysList.add(FPSTR(KEY_STA_SIGNAL));
    ValuesList.add(intTostr(100+WiFi.RSSI()));
    //DHCP Client status
    GetDHCPStatus(KeysList, ValuesList);
    //IP address
    KeysList.add(FPSTR(KEY_STA_IP));
    ValuesList.add(WiFi.localIP().toString().c_str());
    //GW address
    KeysList.add(FPSTR(KEY_STA_GW));
    ValuesList.add(WiFi.gatewayIP().toString().c_str());
    //Sub Net Mask
    KeysList.add(FPSTR(KEY_STA_SUBNET));
    ValuesList.add(WiFi.subnetMask().toString().c_str());
    //Service page / no need refresh on this page
    KeysList.add(FPSTR(KEY_SERVICE_PAGE));
    ValuesList.add("");
    //Firmware & Free Mem, at the end to reflect situation
    GetFreeMem(KeysList, ValuesList);
    //process the template file and provide list of variables
    processTemplate("/home.tpl", KeysList , ValuesList);
    //need to clean to speed up memory recovery
    KeysList.clear();
    ValuesList.clear();
}

void handle_web_interface_configSys()
{
    static const char NOT_AUTH_CS [] PROGMEM = "HTTP/1.1 301 OK\r\nLocation: /LOGIN?return=CONFIGSYS\r\nCache-Control: no-cache\r\n\r\n";

    String stmp,smsg;
    long lstatus;
    int istatus;
    byte bbuf;
    long ibaud=DEFAULT_BAUD_RATE;
    int iweb_port =DEFAULT_WEB_PORT;
    int idata_port =DEFAULT_DATA_PORT;
    byte bsleepmode=DEFAULT_SLEEP_MODE;
    bool msg_alert_error=false;
    bool msg_alert_success=false;
    long lbaudlist[] = {9600 ,19200,38400,57600,115200,230400,250000,-1};
    int bmodemvaluelist[] = {WIFI_NONE_SLEEP,WIFI_LIGHT_SLEEP,WIFI_MODEM_SLEEP, -1};
    const __FlashStringHelper  *smodemdisplaylist[]= {FPSTR(VALUE_NONE),FPSTR(VALUE_LIGHT),FPSTR(VALUE_MODEM),FPSTR(VALUE_MODEM)};
    STORESTRINGS_CLASS KeysList ;
    STORESTRINGS_CLASS ValuesList ;

    if (!web_interface->is_authenticated()) {
        web_interface->WebServer.sendContent_P(NOT_AUTH_CS);
        return;
    }

    //IP+Web
    GetIpWeb(KeysList, ValuesList);
    //mode
    GetMode(KeysList, ValuesList);
    //page title and filenames
    SetPageProp(KeysList,ValuesList,FPSTR(VALUE_HOME),F("system"));
    //menu item
    KeysList.add(FPSTR(KEY_MENU_SYSTEM));
    ValuesList.add(FPSTR(VALUE_ACTIVE));

    //check is it is a submission or a display
    if (web_interface->WebServer.hasArg("SUBMIT")) {
        //is there a correct list of values?
        if (web_interface->WebServer.hasArg("BAUD_RATE") 
        && web_interface->WebServer.hasArg("SLEEP_MODE")
 #ifdef TCP_IP_DATA_FEATURE
        && web_interface->WebServer.hasArg("DATAPORT")
 #endif
        && web_interface->WebServer.hasArg("WEBPORT")) {
            //is each value correct ?
            ibaud  = web_interface->WebServer.arg("BAUD_RATE").toInt();
            iweb_port  = web_interface->WebServer.arg("WEBPORT").toInt();
#ifdef TCP_IP_DATA_FEATURE
            idata_port  = web_interface->WebServer.arg("DATAPORT").toInt();
#endif
            bsleepmode = web_interface->WebServer.arg("SLEEP_MODE").toInt();

            if (!(iweb_port>0 && iweb_port<65001)) {
                msg_alert_error=true;
                smsg.concat(F("Error: invalid port value for web port<BR>"));
                KeysList.add(FPSTR(KEY_WEB_PORT_STATUS));
                ValuesList.add(FPSTR(VALUE_HAS_ERROR));
            }
#ifdef TCP_IP_DATA_FEATURE
            if (!(idata_port>0 && idata_port<65001)) {
                msg_alert_error=true;
                smsg.concat("Error: invalid port value for data port<BR>");
                KeysList.add(FPSTR(KEY_DATA_PORT_STATUS));
                ValuesList.add(FPSTR(VALUE_HAS_ERROR));
            }
#endif
            if (iweb_port== idata_port) {
                msg_alert_error=true;
                smsg.concat("Error: web port and data port cannot be identical<BR>");
                KeysList.add(FPSTR(KEY_WEB_PORT_STATUS));
                ValuesList.add(FPSTR(VALUE_HAS_ERROR));
                KeysList.add(FPSTR(KEY_DATA_PORT_STATUS));
                ValuesList.add(FPSTR(VALUE_HAS_ERROR));
            }
            if (!(ibaud==9600 || ibaud==19200|| ibaud==38400|| ibaud==57600|| ibaud==115200|| ibaud==230400 || ibaud==250000)) {
                msg_alert_error=true;
                smsg.concat(F("Error: value for baud rate is not correct<BR>"));
                KeysList.add(FPSTR(KEY_BAUD_RATE_STATUS));
                ValuesList.add(FPSTR(VALUE_HAS_ERROR));
            }
            if (!(bsleepmode==WIFI_NONE_SLEEP ||bsleepmode==WIFI_LIGHT_SLEEP ||bsleepmode==WIFI_MODEM_SLEEP )) {
                msg_alert_error=true;
                smsg.concat(F("Error: value for sleeping mode is not correct<BR>"));
                KeysList.add(FPSTR(KEY_SLEEP_MODE_STATUS));
                ValuesList.add(FPSTR(VALUE_HAS_ERROR));
            }
        } else {
            msg_alert_error=true;
            smsg = FPSTR(MISSING_DATA);
        }
        //if no error apply the changes
        if (msg_alert_error!=true) {
            if(!CONFIG::write_buffer(EP_BAUD_RATE,(const byte *)&ibaud,INTEGER_LENGTH)
            ||!CONFIG::write_buffer(EP_WEB_PORT,(const byte *)&iweb_port,INTEGER_LENGTH)
#ifdef TCP_IP_DATA_FEATURE 
            ||!CONFIG::write_buffer(EP_DATA_PORT,(const byte *)&idata_port,INTEGER_LENGTH)
#endif
            ||!CONFIG::write_byte(EP_SLEEP_MODE,bsleepmode)) {
                msg_alert_error=true;
                smsg = FPSTR(EEPROM_NOWRITE);
            } else {
                msg_alert_success=true;
#ifdef TCP_IP_DATA_FEATURE
                wifi_config.iweb_port=iweb_port;
#endif
                wifi_config.idata_port=idata_port;
                smsg = F("Changes saved to EEPROM, restarting....");
            }
        }
    } else { //no submit need to get data from EEPROM
        if (!CONFIG::read_buffer(EP_BAUD_RATE,  (byte *)&ibaud , INTEGER_LENGTH)) {
            ibaud=DEFAULT_BAUD_RATE;
        }
        if (!CONFIG::read_byte(EP_SLEEP_MODE, &bsleepmode )) {
            bsleepmode=DEFAULT_SLEEP_MODE;
        }
        if (!CONFIG::read_buffer(EP_WEB_PORT,  (byte *)&iweb_port , INTEGER_LENGTH)) {
            iweb_port=DEFAULT_WEB_PORT;
        }
        wifi_config.iweb_port=iweb_port;
        if (!CONFIG::read_buffer(EP_DATA_PORT,  (byte *)&idata_port , INTEGER_LENGTH)) {
            idata_port=DEFAULT_DATA_PORT;
        }
        wifi_config.idata_port=idata_port;
    };
    //Baud rate list
    istatus = 0;
    stmp="";
    while (lbaudlist[istatus]>-1) {
        stmp+="<OPTION VALUE=\"";
        stmp+= intTostr(lbaudlist[istatus]);
        stmp+="\" ";
        if (lbaudlist[istatus]==ibaud) {
            stmp+=FPSTR(VALUE_SELECTED);
        }
        stmp+=">" ;
        stmp+=intTostr(lbaudlist[istatus]);
        stmp+= "</OPTION>\n";
        istatus++;
    }
    KeysList.add(FPSTR(KEY_BAUD_RATE_OPTIONS_LIST));
    ValuesList.add(stmp);
    //Sleep Mode
    istatus = 0;
    stmp="";
    while (bmodemvaluelist[istatus]>-1) {
        stmp+="<OPTION VALUE=\"";
        stmp+= intTostr(bmodemvaluelist[istatus]);
        stmp+="\" ";
        if (bmodemvaluelist[istatus]==bsleepmode) {
            stmp+=FPSTR(VALUE_SELECTED);
        }
        stmp+=">" ;
        stmp+=smodemdisplaylist[istatus];
        stmp+= "</OPTION>\n";
        istatus++;
    }
    KeysList.add(FPSTR(KEY_SLEEP_MODE_OPTIONS_LIST));
    ValuesList.add(stmp);

    // Web and Data ports
    GetPorts(KeysList, ValuesList);

    if (msg_alert_error) {
        ProcessAlertError(KeysList, ValuesList, smsg);
    } else if (msg_alert_success) {
        ProcessAlertSuccess(KeysList, ValuesList, smsg);
        KeysList.add(FPSTR(KEY_SERVICE_PAGE));
        ValuesList.add(FPSTR(RESTARTCMD));
        KeysList.add(FPSTR(KEY_BAUD_RATE_STATUS));
        ValuesList.add(FPSTR(VALUE_HAS_SUCCESS));
        KeysList.add(FPSTR(KEY_SLEEP_MODE_STATUS));
        ValuesList.add(FPSTR(VALUE_HAS_SUCCESS));
        KeysList.add(FPSTR(KEY_WEB_PORT_STATUS));
        ValuesList.add(FPSTR(VALUE_HAS_SUCCESS));
        KeysList.add(FPSTR(KEY_DATA_PORT_STATUS));
        ValuesList.add(FPSTR(VALUE_HAS_SUCCESS));
    }

    else

    {
        ProcessNoAlert(KeysList, ValuesList);
    }
    KeysList.add(FPSTR(KEY_WEB_UPDATE));
#ifdef WEB_UPDATE_FEATURE
    ValuesList.add(FPSTR(VALUE_ITEM_VISIBLE));
#else
    ValuesList.add(FPSTR(VALUE_ITEM_HIDDEN));
#endif
    //Firmware and Free Mem, at the end to reflect situation
    GetFreeMem(KeysList, ValuesList);

    //process the template file and provide list of variables
    processTemplate("/system.tpl", KeysList , ValuesList);
    //need to clean to speed up memory recovery
    KeysList.clear();
    ValuesList.clear();
}

void handle_password()
{
    static const char NOT_AUTH_PW [] PROGMEM = "HTTP/1.1 301 OK\r\nLocation: /LOGIN?return=PASSWORD\r\nCache-Control: no-cache\r\n\r\n";

    String smsg;
    String sPassword,sPassword2;
    bool msg_alert_error=false;
    bool msg_alert_success=false;
    int ipos;
    STORESTRINGS_CLASS KeysList ;
    STORESTRINGS_CLASS ValuesList ;

    if (!web_interface->is_authenticated()) {
        web_interface->WebServer.sendContent_P(NOT_AUTH_PW);
        return;
    }

    //IP+Web
    GetIpWeb(KeysList, ValuesList);
    //mode
    GetMode(KeysList, ValuesList);
    //page title and filenames
    SetPageProp(KeysList,ValuesList,FPSTR(VALUE_CHANGE_PASSWORD),F("password"));
    //menu item
    KeysList.add(FPSTR(KEY_MENU_ADMIN));
    ValuesList.add(FPSTR(VALUE_ACTIVE));

    //check if it is a submission or a display
    smsg="";
    if (web_interface->WebServer.hasArg("SUBMIT")) {
        //is there a correct list of values?
        if (web_interface->WebServer.hasArg("PASSWORD") && web_interface->WebServer.hasArg("PASSWORD2")) {
            //Password
            web_interface->urldecode(sPassword,web_interface->WebServer.arg("PASSWORD").c_str());
            web_interface->urldecode(sPassword2,web_interface->WebServer.arg("PASSWORD2").c_str());
            if (!web_interface->isAdminPasswordValid(sPassword.c_str()) ) {
                msg_alert_error=true;
                smsg.concat(F("Error: Incorrect password<BR>"));
                KeysList.add(FPSTR(KEY_USER_PASSWORD_STATUS));
                ValuesList.add(FPSTR(VALUE_HAS_ERROR));
            }
            if (sPassword!=sPassword2) {
                msg_alert_error=true;
                smsg.concat(F("Error: Passwords do not match<BR>"));
                KeysList.add(FPSTR(KEY_USER_PASSWORD_STATUS2));
                ValuesList.add(FPSTR(VALUE_HAS_ERROR));
            }
        } else {
            msg_alert_error=true;
            smsg = FPSTR(MISSING_DATA);
        }

        //if no error apply the change
        if (msg_alert_error==false) {
            //save
            if(!CONFIG::write_string(EP_ADMIN_PWD,sPassword.c_str())) {
                msg_alert_error=true;
                smsg = FPSTR(EEPROM_NOWRITE);
            } else {
                msg_alert_success=true;
                smsg = F("Changes saved to EEPROM");
            }
        }
    }

    else { //no submit, need to get data from EEPROM
        //password
        sPassword="";
        sPassword2="";
    }
    //Display values
    //password
    KeysList.add(FPSTR(KEY_USER_PASSWORD));
    ValuesList.add(sPassword);
    KeysList.add(FPSTR(KEY_USER_PASSWORD2));
    ValuesList.add(sPassword2);

    if (msg_alert_error) {
        ProcessAlertError(KeysList, ValuesList, smsg);
    } else if (msg_alert_success) {
        ProcessAlertSuccess(KeysList, ValuesList, smsg);
        KeysList.add(FPSTR(KEY_SERVICE_PAGE));
        ValuesList.add("");
        //Add all green
        KeysList.add(FPSTR(KEY_USER_PASSWORD_STATUS));
        ValuesList.add(FPSTR(VALUE_HAS_SUCCESS));
        KeysList.add(FPSTR(KEY_USER_PASSWORD_STATUS2));
        ValuesList.add(FPSTR(VALUE_HAS_SUCCESS));
    }

    else

    {
        ProcessNoAlert(KeysList,ValuesList);
    }

    //Firmware and Free Mem, at the end to reflect situation
    GetFreeMem(KeysList, ValuesList);

    //process the template file and provide list of variables
    processTemplate("/password.tpl", KeysList , ValuesList);
    //need to clean to speed up memory recovery
    KeysList.clear();
    ValuesList.clear();
}


void handle_web_interface_configAP()
{
    static const char NOT_AUTH_AP [] PROGMEM = "HTTP/1.1 301 OK\r\nLocation: /LOGIN?return=CONFIGAP\r\nCache-Control: no-cache\r\n\r\n";

    String stmp,smsg;
    String sSSID,sPassword,sIP,sGW,sMask;
    bool msg_alert_error=false;
    bool msg_alert_success=false;
    byte visible_buf;
    byte static_ip_buf;
    byte auth_buf;
    byte channel_buf;
    byte phy_mode_buf;
    byte ip_sav[4];
    byte gw_sav[4];
    byte msk_sav[4];
    int ipos;
    int inetworkvaluelist []= {WIFI_PHY_MODE_11B,WIFI_PHY_MODE_11G,-1};
    const __FlashStringHelper  * inetworkdisplaylist []= {FPSTR(VALUE_11B),FPSTR(VALUE_11G),FPSTR(VALUE_11B)};
    int iauthvaluelist[] = {AUTH_OPEN,AUTH_WPA_PSK,AUTH_WPA2_PSK,AUTH_WPA_WPA2_PSK,-1};
    const __FlashStringHelper  * iauthdisplaylist[] = {FPSTR(VALUE_NONE),FPSTR(VALUE_WPA),FPSTR(VALUE_WPA2),FPSTR(VALUE_WPAWPA2)};
    STORESTRINGS_CLASS KeysList ;
    STORESTRINGS_CLASS ValuesList ;

    if (!web_interface->is_authenticated()) {
        web_interface->WebServer.sendContent_P(NOT_AUTH_AP);
        return;
    }

    //IP+Web
    GetIpWeb(KeysList, ValuesList);
    //mode
    GetMode(KeysList, ValuesList);
    //page title and filenames
    SetPageProp(KeysList,ValuesList,FPSTR(VALUE_CONFIG_AP),F("config_ap"));
    //menu item
    KeysList.add(FPSTR(KEY_MENU_AP));
    ValuesList.add(FPSTR(VALUE_ACTIVE));

    //check is it is a submission or a display
    smsg="";
    if (web_interface->WebServer.hasArg("SUBMIT")) {
        //is there a correct list of values?
        if (web_interface->WebServer.hasArg("SSID") && web_interface->WebServer.hasArg("PASSWORD")&& web_interface->WebServer.hasArg("NETWORK")
                && web_interface->WebServer.hasArg("AUTHENTIFICATION")&& web_interface->WebServer.hasArg("IP")
                && web_interface->WebServer.hasArg("GATEWAY")&& web_interface->WebServer.hasArg("SUBNET")
                && web_interface->WebServer.hasArg("CHANNEL")) {
            //SSID
            web_interface->urldecode(sSSID,web_interface->WebServer.arg("SSID").c_str());
            if (!web_interface->isSSIDValid(sSSID.c_str())) {
                msg_alert_error=true;
                smsg.concat(F("Error: Incorrect SSID<BR>"));
                KeysList.add(FPSTR(KEY_AP_SSID_STATUS));
                ValuesList.add(FPSTR(VALUE_HAS_ERROR));
            }
            //Password
            web_interface->urldecode(sPassword,web_interface->WebServer.arg("PASSWORD").c_str());
            if (!web_interface->isPasswordValid(sPassword.c_str())) {
                msg_alert_error=true;
                smsg.concat(F("Error: Incorrect password<BR>"));
                KeysList.add(FPSTR(KEY_AP_PASSWORD_STATUS));
                ValuesList.add(FPSTR(VALUE_HAS_ERROR));
            }
            //ssid visible ?
            if (web_interface->WebServer.hasArg("SSID_VISIBLE")) {
                visible_buf=1;
            } else {
                visible_buf=0;
            }
            //phy mode
            phy_mode_buf  = byte(web_interface->WebServer.arg("NETWORK").toInt());
            if (!(phy_mode_buf==WIFI_PHY_MODE_11B||phy_mode_buf==WIFI_PHY_MODE_11G) ) {
                msg_alert_error=true;
                smsg.concat(F("Error: Incorrect network<BR>"));
                KeysList.add(FPSTR(KEY_NETWORK_OPTION_LIST_STATUS));
                ValuesList.add(FPSTR(VALUE_HAS_ERROR));
            }
            //channel
            channel_buf  = byte (web_interface->WebServer.arg("CHANNEL").toInt());
            if (channel_buf< 1|| channel_buf>11) {
                msg_alert_error=true;
                smsg.concat("Error: Incorrect channel<BR>");
                KeysList.add(FPSTR(KEY_CHANNEL_OPTION_LIST_STATUS));
                ValuesList.add(FPSTR(VALUE_HAS_ERROR));
            }
            //authentification
            auth_buf  = byte(web_interface->WebServer.arg("AUTHENTIFICATION").toInt());
            if (!(auth_buf == AUTH_OPEN || auth_buf == AUTH_WEP || auth_buf == AUTH_WPA_PSK ||
                    auth_buf == AUTH_WPA2_PSK || auth_buf == AUTH_WPA_WPA2_PSK)) {
                msg_alert_error=true;
                smsg.concat(F("Error: Incorrect authentification method<BR>"));
                KeysList.add(FPSTR(KEY_AUTH_OPTION_LIST_STATUS));
                ValuesList.add(FPSTR(VALUE_HAS_ERROR));
            }
            //Static IP ?
            if (web_interface->WebServer.hasArg("STATIC_IP") ) {
                static_ip_buf=STATIC_IP_MODE;
            } else {
                static_ip_buf=DHCP_MODE;
            }

            //IP
            web_interface->urldecode(sIP,web_interface->WebServer.arg("IP").c_str());
            if (!web_interface->isIPValid(sIP.c_str())) {
                msg_alert_error=true;
                smsg.concat(F("Error: Incorrect IP fortmat<BR>"));
                KeysList.add(FPSTR(KEY_AP_IP_STATUS));
                ValuesList.add(FPSTR(VALUE_HAS_ERROR));
            }

            //Gateway
            web_interface->urldecode(sGW,web_interface->WebServer.arg("GATEWAY").c_str());
            if (!web_interface->isIPValid(sGW.c_str())) {
                msg_alert_error=true;
                smsg.concat(F("Error: Incorrect gateway<BR>"));
                KeysList.add(FPSTR(KEY_AP_GW_STATUS));
                ValuesList.add(FPSTR(VALUE_HAS_ERROR));
            }
            //subnet
            web_interface->urldecode(sMask,web_interface->WebServer.arg("SUBNET").c_str());
            if (!web_interface->isIPValid(sMask.c_str())) {
                msg_alert_error=true;
                smsg.concat(F("Error: Incorrect subnet<BR>"));
                KeysList.add(FPSTR(KEY_AP_SUBNET_STATUS));
                ValuesList.add(FPSTR(VALUE_HAS_ERROR));
            }
        } else {
            msg_alert_error=true;
            smsg = FPSTR(MISSING_DATA);
        }

        //if no error apply the change
        if (msg_alert_error==false) {
            //save
            wifi_config.split_ip(sIP.c_str(),ip_sav);
            wifi_config.split_ip(sGW.c_str(),gw_sav);
            wifi_config.split_ip(sMask.c_str(),msk_sav);
            if((!CONFIG::write_byte(EP_WIFI_MODE,AP_MODE))||
                    (!CONFIG::write_string(EP_SSID,sSSID.c_str()))||
                    (!CONFIG::write_string(EP_PASSWORD,sPassword.c_str()))||
                    (!CONFIG::write_byte(EP_SSID_VISIBLE,visible_buf))||
                    (!CONFIG::write_byte(EP_PHY_MODE,phy_mode_buf))||
                    (!CONFIG::write_byte(EP_CHANNEL,channel_buf)) ||
                    (!CONFIG::write_byte(EP_AUTH_TYPE,auth_buf)) ||
                    (!CONFIG::write_byte(EP_IP_MODE,static_ip_buf)) ||
                    (!CONFIG::write_buffer(EP_IP_VALUE,ip_sav,IP_LENGTH))||
                    (!CONFIG::write_buffer(EP_GATEWAY_VALUE,gw_sav,IP_LENGTH))||
                    (!CONFIG::write_buffer(EP_MASK_VALUE,msk_sav,IP_LENGTH))) {
                msg_alert_error=true;
                smsg = FPSTR(EEPROM_NOWRITE);
            } else {
                msg_alert_success=true;
                smsg = F("Changes saved to EEPROM, restarting...");
            }
        }
    }

    else { //no submit need to get data from EEPROM
        //ssid
        if (!CONFIG::read_string(EP_SSID, sSSID , MAX_SSID_LENGTH) ) {
            sSSID=FPSTR(DEFAULT_SSID);
        }
        //password
        if (!CONFIG::read_string(EP_PASSWORD, sPassword , MAX_PASSWORD_LENGTH) ) {
            sPassword=FPSTR(DEFAULT_PASSWORD);
        }
        //ssid visible ?
        if (!CONFIG::read_byte(EP_SSID_VISIBLE, &visible_buf )) {
            visible_buf=DEFAULT_SSID_VISIBLE;
        }
        //phy mode
        if (!CONFIG::read_byte(EP_PHY_MODE, &phy_mode_buf )) {
            phy_mode_buf=DEFAULT_PHY_MODE;
        }
        //authentification
        if (!CONFIG::read_byte(EP_AUTH_TYPE, &auth_buf )) {
            auth_buf=DEFAULT_AUTH_TYPE;
        }
        //channel
        if (!CONFIG::read_byte(EP_CHANNEL, &channel_buf )) {
            channel_buf=DEFAULT_CHANNEL;
        }
        //static IP ?
        if (!CONFIG::read_byte(EP_IP_MODE, &static_ip_buf )) {
            static_ip_buf=DEFAULT_IP_MODE;
        }
        //IP for static IP
        if (!CONFIG::read_buffer(EP_IP_VALUE,ip_sav , IP_LENGTH) ) {
            sIP=IPAddress((const uint8_t *)DEFAULT_IP_VALUE).toString();
            
        } else {
            sIP=IPAddress((const uint8_t *)ip_sav).toString();
        }
        //GW for static IP
        if (!CONFIG::read_buffer(EP_GATEWAY_VALUE,gw_sav , IP_LENGTH) ) {
            sGW=IPAddress((const uint8_t *)DEFAULT_GATEWAY_VALUE).toString();
        } else {
            sGW=IPAddress((const uint8_t *)gw_sav).toString();
        }

        //Subnet for static IP
        if (!CONFIG::read_buffer(EP_MASK_VALUE,msk_sav , IP_LENGTH) ) {
            sMask=IPAddress((const uint8_t *)DEFAULT_MASK_VALUE).toString();
        } else {
            sMask=IPAddress((const uint8_t *)msk_sav).toString();
        }
    }

    //Display values

    //ssid
    KeysList.add(FPSTR(KEY_AP_SSID));
    ValuesList.add(sSSID);

    //password
    KeysList.add(FPSTR(KEY_AP_PASSWORD));
    ValuesList.add(sPassword);

    //ssid visible ?
    KeysList.add(FPSTR(KEY_IS_SSID_VISIBLE));
    if (visible_buf==1) {
        ValuesList.add(FPSTR(VALUE_CHECKED));
    } else {
        ValuesList.add("");
    }

    //network
    ipos = 0;
    stmp="";
    while (inetworkvaluelist[ipos]>-1) {
        stmp+="<OPTION VALUE=\"";
        stmp+= intTostr(inetworkvaluelist[ipos]);
        stmp+="\" ";
        if (inetworkvaluelist[ipos]==phy_mode_buf) {
            stmp+=FPSTR(VALUE_SELECTED);
        }
        stmp+=">" ;
        stmp+=inetworkdisplaylist[ipos];
        stmp+= "</OPTION>\n";
        ipos++;
    }
    KeysList.add(FPSTR(KEY_NETWORK_OPTION_LIST));
    ValuesList.add(stmp);

    //channel
    stmp ="";
    for (int c=1; c < 12; c++) {
        stmp+="<OPTION VALUE=\"";
        stmp+=intTostr(c);
        stmp+="\" ";
        if (channel_buf==c) {
            stmp += FPSTR(VALUE_SELECTED);
        } else {
            stmp+="";
        }
        stmp+=" >";
        stmp+=intTostr(c);
        stmp+= "</OPTION>\n";
    }

    KeysList.add(FPSTR(KEY_CHANNEL_OPTION_LIST));
    ValuesList.add(stmp);
    //auth
    ipos = 0;
    stmp="";
    while (iauthvaluelist[ipos]>-1) {
        stmp+="<OPTION VALUE=\"";
        stmp+= intTostr(iauthvaluelist[ipos]);
        stmp+="\" ";
        if (iauthvaluelist[ipos]==auth_buf) {
            stmp+=FPSTR(VALUE_SELECTED);
        }
        stmp+=">" ;
        stmp+=iauthdisplaylist[ipos];
        stmp+= "</OPTION>\n";
        ipos++;
    }
    KeysList.add(FPSTR(KEY_AUTH_OPTION_LIST));
    ValuesList.add(stmp);

    //static IP ?
    KeysList.add(FPSTR(KEY_IS_STATIC_IP));
    if (static_ip_buf==STATIC_IP_MODE) {
        ValuesList.add(FPSTR(VALUE_CHECKED));
    } else {
        ValuesList.add("");
    }

    //IP for static IP
    KeysList.add(FPSTR(KEY_AP_IP));
    ValuesList.add(sIP);

    //Gateway for static IP
    KeysList.add(FPSTR(KEY_AP_GW));
    ValuesList.add(sGW);

    //Mask for static IP
    KeysList.add(FPSTR(KEY_AP_SUBNET));
    ValuesList.add(sMask);

    if (msg_alert_error) {
        ProcessAlertError(KeysList, ValuesList, smsg);
    } else if (msg_alert_success) {
        ProcessAlertSuccess(KeysList, ValuesList, smsg);
        KeysList.add(FPSTR(KEY_SERVICE_PAGE));
        ValuesList.add(FPSTR(RESTARTCMD));
        //Add all green
        KeysList.add(FPSTR(KEY_AP_SSID_STATUS));
        ValuesList.add(FPSTR(VALUE_HAS_SUCCESS));
        KeysList.add(FPSTR(KEY_AP_PASSWORD_STATUS));
        ValuesList.add(FPSTR(VALUE_HAS_SUCCESS));
        KeysList.add(FPSTR(KEY_IS_SSID_VISIBLE_STATUS));
        ValuesList.add(FPSTR(VALUE_HAS_SUCCESS));
        KeysList.add(FPSTR(KEY_NETWORK_OPTION_LIST_STATUS));
        ValuesList.add(FPSTR(VALUE_HAS_SUCCESS));
        KeysList.add(FPSTR(KEY_CHANNEL_OPTION_LIST_STATUS));
        ValuesList.add(FPSTR(VALUE_HAS_SUCCESS));
        KeysList.add(FPSTR(KEY_AUTH_OPTION_LIST_STATUS));
        ValuesList.add(FPSTR(VALUE_HAS_SUCCESS));
        KeysList.add(FPSTR(KEY_AP_STATIC_IP_STATUS));
        ValuesList.add(FPSTR(VALUE_HAS_SUCCESS));
        KeysList.add(FPSTR(KEY_AP_IP_STATUS));
        ValuesList.add(FPSTR(VALUE_HAS_SUCCESS));
        KeysList.add(FPSTR(KEY_AP_GW_STATUS));
        ValuesList.add(FPSTR(VALUE_HAS_SUCCESS));
        KeysList.add(FPSTR(KEY_AP_SUBNET_STATUS));
        ValuesList.add(FPSTR(VALUE_HAS_SUCCESS));
    }

    else

    {
        ProcessNoAlert(KeysList,ValuesList);
    }

    //Firmware and Free Mem, at the end to reflect situation
    GetFreeMem(KeysList, ValuesList);
    //process the template file and provide list of variables
    processTemplate("/config_ap.tpl", KeysList , ValuesList);
    //need to clean to speed up memory recovery
    KeysList.clear();
    ValuesList.clear();
}

void handle_web_interface_configSTA()
{
    static const char NOT_AUTH_STA [] PROGMEM = "HTTP/1.1 301 OK\r\nLocation: /LOGIN?return=CONFIGSTA\r\nCache-Control: no-cache\r\n\r\n";

    String stmp,smsg;
    String sSSID,sPassword,sIP,sGW,sMask,sHostname;
    bool msg_alert_error=false;
    bool msg_alert_success=false;
    byte static_ip_buf;
    byte phy_mode_buf;
    byte ip_sav[4];
    byte gw_sav[4];
    byte msk_sav[4];
    bool revertSTA=false;
    int ipos;
    int inetworkvaluelist []= {WIFI_PHY_MODE_11B,WIFI_PHY_MODE_11G,WIFI_PHY_MODE_11N,-1};
    const __FlashStringHelper  * inetworkdisplaylist []= {FPSTR(VALUE_11B),FPSTR(VALUE_11G),FPSTR(VALUE_11N),FPSTR(VALUE_11B)};
    STORESTRINGS_CLASS KeysList ;
    STORESTRINGS_CLASS ValuesList ;

    if (!web_interface->is_authenticated()) {
        web_interface->WebServer.sendContent_P(NOT_AUTH_STA);
        return;
    }

    //IP+Web
    GetIpWeb(KeysList, ValuesList);
    //mode
    GetMode(KeysList, ValuesList);
    //page title and filenames
    SetPageProp(KeysList,ValuesList,FPSTR(VALUE_CONFIG_STA),F("config_sta"));
    //menu item
    KeysList.add(FPSTR(KEY_MENU_STA));
    ValuesList.add(FPSTR(VALUE_ACTIVE));

    smsg="";
    if (web_interface->WebServer.hasArg("SUBMIT")) {
        //is there a correct list of values?
        if (web_interface->WebServer.hasArg("SSID") && web_interface->WebServer.hasArg("PASSWORD")&& web_interface->WebServer.hasArg("NETWORK")
                && web_interface->WebServer.hasArg("IP") && web_interface->WebServer.hasArg("GATEWAY")&& web_interface->WebServer.hasArg("SUBNET")
                && web_interface->WebServer.hasArg("HOSTNAME")) {
            //SSID
            web_interface->urldecode(sSSID,web_interface->WebServer.arg("SSID").c_str());
            if (!web_interface->isSSIDValid(sSSID.c_str())) {
                msg_alert_error=true;
                smsg.concat(F("Error: Incorrect SSID<BR>"));
                KeysList.add(FPSTR(KEY_STA_SSID_STATUS));
                ValuesList.add(FPSTR(VALUE_HAS_ERROR));
            }

            //Password
            web_interface->urldecode(sPassword,web_interface->WebServer.arg("PASSWORD").c_str());
            if (!web_interface->isPasswordValid(sPassword.c_str())) {
                msg_alert_error=true;
                smsg.concat(F("Error: Incorrect password<BR>"));
                KeysList.add(FPSTR(KEY_STA_PASSWORD_STATUS));
                ValuesList.add(FPSTR(VALUE_HAS_ERROR));
            }

            //Hostname
            web_interface->urldecode(sHostname,web_interface->WebServer.arg("HOSTNAME").c_str());
            if (!web_interface->isHostnameValid(sHostname.c_str())) {
                msg_alert_error=true;
                smsg.concat(F("Error: Incorrect hostname<BR>"));
                KeysList.add(FPSTR(KEY_HOSTNAME_STATUS));
                ValuesList.add(FPSTR(VALUE_HAS_ERROR));
            }

            //phy mode
            phy_mode_buf  = byte(web_interface->WebServer.arg("NETWORK").toInt());
            if (!(phy_mode_buf==WIFI_PHY_MODE_11B||phy_mode_buf==WIFI_PHY_MODE_11G||phy_mode_buf==WIFI_PHY_MODE_11N) ) {
                msg_alert_error=true;
                smsg.concat(F("Error: Incorrect network<BR>"));
                KeysList.add(FPSTR(KEY_NETWORK_OPTION_LIST_STATUS));
                ValuesList.add(FPSTR(VALUE_HAS_ERROR));
            }

            //Static IP ?
            if (web_interface->WebServer.hasArg("STATIC_IP") ) {
                static_ip_buf=STATIC_IP_MODE;
            } else {
                static_ip_buf=DHCP_MODE;
            }

            //IP
            web_interface->urldecode(sIP,web_interface->WebServer.arg("IP").c_str());
            if (!web_interface->isIPValid(sIP.c_str())) {
                msg_alert_error=true;
                smsg.concat(F("Error: Incorrect IP format<BR>"));
                KeysList.add(FPSTR(KEY_STA_IP_STATUS));
                ValuesList.add(FPSTR(VALUE_HAS_ERROR));
            }

            //Gateway
            web_interface->urldecode(sGW,web_interface->WebServer.arg("GATEWAY").c_str());
            if (!web_interface->isIPValid(sGW.c_str())) {
                msg_alert_error=true;
                smsg.concat(F("Error: Incorrect gateway<BR>"));
                KeysList.add(FPSTR(KEY_STA_GW_STATUS));
                ValuesList.add(FPSTR(VALUE_HAS_ERROR));
            }
            //subnet
            web_interface->urldecode(sMask,web_interface->WebServer.arg("SUBNET").c_str());
            if (!web_interface->isIPValid(sMask.c_str())) {
                msg_alert_error=true;
                smsg.concat(F("Error: Incorrect subnet<BR>"));
                KeysList.add(FPSTR(KEY_STA_SUBNET_STATUS));
                ValuesList.add(FPSTR(VALUE_HAS_ERROR));
            }
        } else {
            msg_alert_error=true;
            smsg = FPSTR(MISSING_DATA);
        }

        //no error ? then save
        if (msg_alert_error==false) {
            //save
            wifi_config.split_ip(sIP.c_str(),ip_sav);
            wifi_config.split_ip(sGW.c_str(),gw_sav);
            wifi_config.split_ip(sMask.c_str(),msk_sav);
            if((!CONFIG::write_byte(EP_WIFI_MODE,CLIENT_MODE))||
                    (!CONFIG::write_string(EP_SSID,sSSID.c_str()))||
                    (!CONFIG::write_string(EP_PASSWORD,sPassword.c_str()))||
                    (!CONFIG::write_string(EP_HOSTNAME,sHostname.c_str()))||
                    (!CONFIG::write_byte(EP_PHY_MODE,phy_mode_buf))||
                    (!CONFIG::write_byte(EP_IP_MODE,static_ip_buf)) ||
                    (!CONFIG::write_buffer(EP_IP_VALUE,ip_sav,IP_LENGTH))||
                    (!CONFIG::write_buffer(EP_GATEWAY_VALUE,gw_sav,IP_LENGTH))||
                    (!CONFIG::write_buffer(EP_MASK_VALUE,msk_sav,IP_LENGTH))) {
                msg_alert_error=true;
                smsg = FPSTR(EEPROM_NOWRITE);
            } else {
                msg_alert_success=true;
                smsg = F("Changes saved to EEPROM, restarting...");
            }
        }
    } else { //no submit, need to get data from EEPROM
        //ssid
        if (!CONFIG::read_string(EP_SSID, sSSID , MAX_SSID_LENGTH) ) {
            sSSID=FPSTR(DEFAULT_SSID);
        }
        //password
        if (!CONFIG::read_string(EP_PASSWORD, sPassword , MAX_PASSWORD_LENGTH) ) {
            sPassword=FPSTR(DEFAULT_PASSWORD);
        }
        //hostname
        if (!CONFIG::read_string(EP_HOSTNAME, sHostname , MAX_HOSTNAME_LENGTH) ) {
            sHostname=wifi_config.get_default_hostname();
        }
        //phy mode
        if (!CONFIG::read_byte(EP_PHY_MODE, &phy_mode_buf )) {
            phy_mode_buf=DEFAULT_PHY_MODE;
        }
        //static IP ?
        if (!CONFIG::read_byte(EP_IP_MODE, &static_ip_buf )) {
            static_ip_buf=DEFAULT_IP_MODE;
        }
        //IP for static IP
        if (!CONFIG::read_buffer(EP_IP_VALUE,ip_sav , IP_LENGTH) ) {
            sIP=IPAddress((const uint8_t *)DEFAULT_IP_VALUE).toString();
        } else {
            sIP=IPAddress((const uint8_t *)ip_sav).toString();
        }
        //GW for static IP
        if (!CONFIG::read_buffer(EP_GATEWAY_VALUE,gw_sav , IP_LENGTH) ) {
            sGW=IPAddress((const uint8_t *)DEFAULT_GATEWAY_VALUE).toString();
        } else {
            sGW=IPAddress((const uint8_t *)gw_sav).toString();
        }
        //Subnet for static IP
        if (!CONFIG::read_buffer(EP_MASK_VALUE,msk_sav , IP_LENGTH) ) {
            sMask=IPAddress((const uint8_t *)DEFAULT_MASK_VALUE).toString();
        } else {
            sMask=IPAddress((const uint8_t *)msk_sav).toString();
        }
    }
    //Display values
    //ssid
    KeysList.add(FPSTR(KEY_STA_SSID));
    ValuesList.add(sSSID);

    //password
    KeysList.add(FPSTR(KEY_STA_PASSWORD));
    ValuesList.add(sPassword);

    //hostname
    KeysList.add(FPSTR(KEY_HOSTNAME));
    ValuesList.add(sHostname);

    //network
    ipos = 0;
    stmp="";
    while (inetworkvaluelist[ipos]>-1) {
        stmp+="<OPTION VALUE=\"";
        stmp+= intTostr(inetworkvaluelist[ipos]);
        stmp+="\" ";
        if (inetworkvaluelist[ipos]==phy_mode_buf) {
            stmp+=FPSTR(VALUE_SELECTED);
        }
        stmp+=">" ;
        stmp+=inetworkdisplaylist[ipos];
        stmp+= "</OPTION>\n";
        ipos++;
    }
    KeysList.add(FPSTR(KEY_NETWORK_OPTION_LIST));
    ValuesList.add(stmp);

    //static IP ?
    KeysList.add(FPSTR(KEY_IS_STATIC_IP));
    if (static_ip_buf==STATIC_IP_MODE) {
        ValuesList.add(FPSTR(VALUE_CHECKED));
    } else {
        ValuesList.add("");
    }

    //IP for static IP
    KeysList.add(FPSTR(KEY_STA_IP));
    ValuesList.add(sIP);

    //Gateway for static IP
    KeysList.add(FPSTR(KEY_STA_GW));
    ValuesList.add(sGW);

    //Mask for static IP
    KeysList.add(FPSTR(KEY_STA_SUBNET));
    ValuesList.add(sMask);

    //do we need to do a scan and display it ?
    if (!msg_alert_success) {
        //if in AP mode switch to mixed mode to be able to scan
        if (wifi_get_opmode()!=WIFI_STA ) {
            WiFi.mode(WIFI_AP_STA);
            revertSTA=true;
        }

        int n = WiFi.scanNetworks();
        for (int i = 0; i < n; ++i) {
            //row number
            stmp = "$ROW_NUMBER["+String(i)+"]$";
            KeysList.add(stmp);
            stmp=String(i+1);
            ValuesList.add(stmp);
            //SSID
            stmp = "$AP_SSID["+String(i)+"]$";
            KeysList.add(stmp);
            ValuesList.add(WiFi.SSID(i).c_str());
            //signal strength
            stmp = "$AP_SIGNAL["+String(i)+"]$";
            KeysList.add(stmp);
            stmp = intTostr(100+WiFi.RSSI(i)) ;
            stmp += "%";
            ValuesList.add(stmp);
            //is protected
            stmp = "$IS_PROTECTED["+String(i)+"]$";
            KeysList.add(stmp);
            if (WiFi.encryptionType(i) == ENC_TYPE_NONE) {
                ValuesList.add(FPSTR(VALUE_NO));
            } else {
                ValuesList.add(FPSTR(VALUE_YES));
            }
        }
        WiFi.scanDelete();
        KeysList.add(FPSTR(KEY_AVAILABLE_AP_NB_ITEMS));
        ValuesList.add(intTostr(n));

        //revert to pure softAP
        if (revertSTA) {
            WiFi.mode(WIFI_AP);
        }
        KeysList.add(FPSTR(KEY_AP_SCAN_VISIBILITY));
        ValuesList.add(FPSTR(VALUE_ITEM_VISIBLE));
    } else {
        //no need to do a scan if we are going to restart
        KeysList.add(FPSTR(KEY_AP_SCAN_VISIBILITY));
        ValuesList.add(FPSTR(VALUE_ITEM_HIDDEN));
        KeysList.add(FPSTR(KEY_AVAILABLE_AP_NB_ITEMS));
        ValuesList.add(intTostr(0));
    }

    if (msg_alert_error) {
        ProcessAlertError(KeysList, ValuesList, smsg);
    } else if (msg_alert_success) {
        ProcessAlertSuccess(KeysList, ValuesList, smsg);
        KeysList.add(FPSTR(KEY_SERVICE_PAGE));
        ValuesList.add(FPSTR(RESTARTCMD));
        //Add all green
        KeysList.add(FPSTR(KEY_STA_SSID_STATUS));
        ValuesList.add(FPSTR(VALUE_HAS_SUCCESS));
        KeysList.add(FPSTR(KEY_STA_PASSWORD_STATUS));
        ValuesList.add(FPSTR(VALUE_HAS_SUCCESS));
        KeysList.add(FPSTR(KEY_NETWORK_OPTION_LIST_STATUS));
        ValuesList.add(FPSTR(VALUE_HAS_SUCCESS));
        KeysList.add(FPSTR(KEY_STA_STATIC_IP_STATUS));
        ValuesList.add(FPSTR(VALUE_HAS_SUCCESS));
        KeysList.add(FPSTR(KEY_STA_IP_STATUS));
        ValuesList.add(FPSTR(VALUE_HAS_SUCCESS));
        KeysList.add(FPSTR(KEY_STA_GW_STATUS));
        ValuesList.add(FPSTR(VALUE_HAS_SUCCESS));
        KeysList.add(FPSTR(KEY_STA_SUBNET_STATUS));
        ValuesList.add(FPSTR(VALUE_HAS_SUCCESS));
        KeysList.add(FPSTR(KEY_HOSTNAME_STATUS));
        ValuesList.add(FPSTR(VALUE_HAS_SUCCESS));
    }

    else

    {
        ProcessNoAlert(KeysList,ValuesList);
    }

    //Firmware and Free Mem, at the end to reflect situation
    GetFreeMem(KeysList, ValuesList);
    //process the template file and provide list of variables
    processTemplate("/config_sta.tpl", KeysList , ValuesList);
    //need to clean to speed up memory recovery
    KeysList.clear();
    ValuesList.clear();
}

void handle_web_interface_printer()
{
    static const char NOT_AUTH_PRT [] PROGMEM = "HTTP/1.1 301 OK\r\nLocation: /LOGIN?return=PRINTER\r\nCache-Control: no-cache\r\n\r\n";

    bool msg_alert_error=false;
    bool msg_alert_success=false;
    STORESTRINGS_CLASS KeysList ;
    STORESTRINGS_CLASS ValuesList ;

    if (!web_interface->is_authenticated()) {
        web_interface->WebServer.sendContent_P(NOT_AUTH_PRT);
        return;
    }

    //IP+Web
    GetIpWeb(KeysList, ValuesList);
    //mode
    GetMode(KeysList, ValuesList);
    //page title and filenames
    SetPageProp(KeysList,ValuesList,FPSTR(VALUE_PRINTER),F("printer"));
    //menu item
    KeysList.add(FPSTR(KEY_MENU_PRINTER));
    ValuesList.add(FPSTR(VALUE_ACTIVE));

    //Refresh page time
    KeysList.add(FPSTR(KEY_REFRESH_PAGE));
    byte bflag;
    if (!CONFIG::read_byte(EP_REFRESH_PAGE_TIME, &bflag )) {
        ValuesList.add(intTostr(DEFAULT_REFRESH_PAGE_TIME*1000));
    } else {
        ValuesList.add(intTostr(1000*bflag));
    }
    int istatus;
    //xy feedrate
    KeysList.add(FPSTR(KEY_XY_FEEDRATE));
    if (!CONFIG::read_buffer(EP_XY_FEEDRATE,  (byte *)&istatus , INTEGER_LENGTH)) {
        istatus=DEFAULT_XY_FEEDRATE;
    }
    ValuesList.add(intTostr(istatus));
    //Z feedrate
    KeysList.add(FPSTR(KEY_Z_FEEDRATE));
    if (!CONFIG::read_buffer(EP_Z_FEEDRATE,  (byte *)&istatus , INTEGER_LENGTH)) {
        istatus=DEFAULT_Z_FEEDRATE;
    }
    ValuesList.add(intTostr(istatus));
    //E feedrate
    KeysList.add(FPSTR(KEY_E_FEEDRATE));
    if (!CONFIG::read_buffer(EP_E_FEEDRATE,  (byte *)&istatus , INTEGER_LENGTH)) {
        istatus=DEFAULT_E_FEEDRATE;
    }
    ValuesList.add(intTostr(istatus));

    //Serial.println("M114");
    Serial.println(F("M220"));
    Serial.println(F("M221"));
    KeysList.add(FPSTR(KEY_SERVICE_PAGE));
    ValuesList.add("");

    //Firmware and Free Mem, at the end to reflect situation
    GetFreeMem(KeysList, ValuesList);

    processTemplate("/printer.tpl", KeysList , ValuesList);
    //need to clean to speed up memory recovery
    KeysList.clear();
    ValuesList.clear();
}

void handle_web_settings()
{
    static const char NOT_AUTH_SET [] PROGMEM = "HTTP/1.1 301 OK\r\nLocation: /LOGIN?return=SETTINGS\r\nCache-Control: no-cache\r\n\r\n";

    String smsg;
    int istatus;
    byte bbuf;
    bool msg_alert_error=false;
    bool msg_alert_success=false;
    byte irefresh_page;
    int ixy_feedrate,iz_feedrate,ie_feedrate;
    STORESTRINGS_CLASS KeysList ;
    STORESTRINGS_CLASS ValuesList ;

    if (!web_interface->is_authenticated()) {
        web_interface->WebServer.sendContent_P(NOT_AUTH_SET);
        return;
    }
	web_interface->blockserial = false;
    //IP+Web
    GetIpWeb(KeysList, ValuesList);
    //mode
    GetMode(KeysList, ValuesList);
    //page title and filenames
    SetPageProp(KeysList,ValuesList,FPSTR(VALUE_SETTINGS),F("settings"));
    //menu item
    KeysList.add(FPSTR(KEY_MENU_SETTINGS));
    ValuesList.add(FPSTR(VALUE_ACTIVE));

    //check is it is a submission or a display
    if (web_interface->WebServer.hasArg("SUBMIT")) {
        //is there a correct list of values?
        if (web_interface->WebServer.hasArg("REFRESH_PAGE") && web_interface->WebServer.hasArg("XY_FEEDRATE")&& web_interface->WebServer.hasArg("Z_FEEDRATE")&& web_interface->WebServer.hasArg("E_FEEDRATE")) {
            //is each value correct ?
            irefresh_page  = web_interface->WebServer.arg("REFRESH_PAGE").toInt();
            ixy_feedrate  = web_interface->WebServer.arg("XY_FEEDRATE").toInt();
            iz_feedrate  = web_interface->WebServer.arg("Z_FEEDRATE").toInt();
            ie_feedrate = web_interface->WebServer.arg("E_FEEDRATE").toInt();

            if (!(irefresh_page>1 && irefresh_page<120)) {
                msg_alert_error=true;
                smsg.concat(F("Error: invalid value for refresh time<BR>"));
                KeysList.add(FPSTR(KEY_REFRESH_PAGE_STATUS));
                ValuesList.add(FPSTR(VALUE_HAS_ERROR));
            }
            if (!(ixy_feedrate>0 && ixy_feedrate<9999)) {
                msg_alert_error=true;
                smsg.concat(F("Error: invalid value for XY axis feedrate<BR>"));
                KeysList.add(FPSTR(KEY_XY_FEEDRATE_STATUS));
                ValuesList.add(FPSTR(VALUE_HAS_ERROR));
            }
            if (!(iz_feedrate>0 && iz_feedrate<9999)) {
                msg_alert_error=true;
                smsg.concat(F("Error: invalid value for Z axis feedrate<BR>"));
                KeysList.add(FPSTR(KEY_Z_FEEDRATE_STATUS));
                ValuesList.add(FPSTR(VALUE_HAS_ERROR));
            }
            if (!(ie_feedrate>0 && ie_feedrate<9999)) {
                msg_alert_error=true;
                smsg.concat(F("Error: invalid value for Extruder feedrate<BR>"));
                KeysList.add(FPSTR(KEY_XY_FEEDRATE_STATUS));
                ValuesList.add(FPSTR(VALUE_HAS_ERROR));
            }
        } else {
            msg_alert_error=true;
            smsg = FPSTR(MISSING_DATA);
        }
        //if no error apply the changes
        if (msg_alert_error!=true) {
            if(!CONFIG::write_buffer(EP_XY_FEEDRATE,(const byte *)&ixy_feedrate,INTEGER_LENGTH)||!CONFIG::write_buffer(EP_Z_FEEDRATE,(const byte *)&iz_feedrate,INTEGER_LENGTH)||!CONFIG::write_buffer(EP_E_FEEDRATE,(const byte *)&ie_feedrate,INTEGER_LENGTH)||!CONFIG::write_byte(EP_REFRESH_PAGE_TIME,irefresh_page)) {
                msg_alert_error=true;
                smsg = FPSTR(EEPROM_NOWRITE);
            } else {
                msg_alert_success=true;
                smsg = F("Changes saved to EEPROM");
            }
        }
    } else { //no submit need to get data from EEPROM
        if (!CONFIG::read_buffer(EP_XY_FEEDRATE,  (byte *)&ixy_feedrate , INTEGER_LENGTH)) {
            ixy_feedrate=DEFAULT_XY_FEEDRATE;
        }
        if (!CONFIG::read_byte(EP_REFRESH_PAGE_TIME, &irefresh_page )) {
            irefresh_page=DEFAULT_REFRESH_PAGE_TIME;
        }
        if (!CONFIG::read_buffer(EP_Z_FEEDRATE,  (byte *)&iz_feedrate , INTEGER_LENGTH)) {
            iz_feedrate=DEFAULT_Z_FEEDRATE;
        }
        if (!CONFIG::read_buffer(EP_E_FEEDRATE,  (byte *)&ie_feedrate , INTEGER_LENGTH)) {
            ie_feedrate=DEFAULT_E_FEEDRATE;
        }
    }
    //fill the variables
    //refresh page
    KeysList.add(FPSTR(KEY_REFRESH_PAGE));
    ValuesList.add(intTostr(irefresh_page));
    //xy feedrate
    KeysList.add(FPSTR(KEY_XY_FEEDRATE));
    ValuesList.add(intTostr(ixy_feedrate));
    //Z feedrate
    KeysList.add(FPSTR(KEY_Z_FEEDRATE));
    ValuesList.add(intTostr(iz_feedrate));
    //E feedrate
    KeysList.add(FPSTR(KEY_E_FEEDRATE));
    ValuesList.add(intTostr(ie_feedrate));

    if (msg_alert_error) {
        ProcessAlertError(KeysList, ValuesList, smsg);
    } else if (msg_alert_success) {
        ProcessAlertSuccess(KeysList, ValuesList, smsg);
        KeysList.add(FPSTR(KEY_SERVICE_PAGE));
        ValuesList.add("");
        KeysList.add(FPSTR(KEY_REFRESH_PAGE_STATUS));
        ValuesList.add(FPSTR(VALUE_HAS_SUCCESS));
        KeysList.add(FPSTR(KEY_XY_FEEDRATE_STATUS));
        ValuesList.add(FPSTR(VALUE_HAS_SUCCESS));
        KeysList.add(FPSTR(KEY_Z_FEEDRATE_STATUS));
        ValuesList.add(FPSTR(VALUE_HAS_SUCCESS));
        KeysList.add(FPSTR(KEY_E_FEEDRATE_STATUS));
        ValuesList.add(FPSTR(VALUE_HAS_SUCCESS));
        KeysList.add(FPSTR(KEY_SERVICE_PAGE));
        ValuesList.add("");
    }

    else {
        ProcessNoAlert(KeysList,ValuesList);
    }

    //Firmware and Free Mem, at the end to reflect situation
    GetFreeMem(KeysList, ValuesList);

    //process the template file and provide list of variables
    processTemplate("/settings.tpl", KeysList , ValuesList);
    //need to clean to speed up memory recovery
    KeysList.clear();
    ValuesList.clear();
}

void handle_web_interface_status()
{
    static const char NO_TEMP_LINE[] PROGMEM = "\"temperature\":\"0\",\"target\":\"0\",\"active\":\"0\"";
    web_interface->is_authenticated();
    Serial.println(F("M114"));
    int tagpos,tagpos2;
    String buffer2send;
    String value;
    int temperature,target;

    //request temperature only if no feedback
    if ((system_get_time()-web_interface->last_temp)>2000000) {
        Serial.println(F("M105"));
    }

    if ((system_get_time()-web_interface->last_temp)<3200000) {
        value="Connected";
    } else if ((system_get_time()-web_interface->last_temp)<32000000) {
        value="Busy";
    } else {
        value="Offline";
    }

    //start JSON answer
    buffer2send="{";
    //status color
    buffer2send+="\"status\":\""+value +"\",";
    //speed
    buffer2send+="\"speed\":\""+web_interface->answer4M220 +"\",";
    //flow
    buffer2send+="\"flow\":\""+web_interface->answer4M221 +"\",";
    //X position
    tagpos = web_interface->answer4M114.indexOf("X:");
    tagpos2 = web_interface->answer4M114.indexOf(" ",tagpos);
    value=web_interface->answer4M114.substring(tagpos+2,tagpos2);
    buffer2send+="\"Xpos\":\""+value +"\",";
    //Y position
    tagpos = web_interface->answer4M114.indexOf("Y:");
    tagpos2 = web_interface->answer4M114.indexOf(" ",tagpos);
    value=web_interface->answer4M114.substring(tagpos+2,tagpos2);
    buffer2send+="\"Ypos\":\""+value +"\",";
    //Z position
    tagpos = web_interface->answer4M114.indexOf("Z:");
    tagpos2 = web_interface->answer4M114.indexOf(" ",tagpos);
    value=web_interface->answer4M114.substring(tagpos+2,tagpos2);
    buffer2send+="\"Zpos\":\""+value +"\",";
    //heater
    buffer2send.concat(F("\"heater\":["));
    //Extruder 1
    buffer2send.concat(F("{\"name\":\"Extruder 1\","));
    int Tpos = web_interface->answer4M105.indexOf("T0:");
    byte bshift=1;
    if (Tpos==-1) {
        Tpos = web_interface->answer4M105.indexOf("T:");
        bshift=0;
    }
    int slashpos = web_interface->answer4M105.indexOf(" /",Tpos);
    int spacepos = web_interface->answer4M105.indexOf(" ",slashpos+2);
    //have Extruder 1 ?
    if(slashpos!=-1 && spacepos!=-1 ) {
        buffer2send += "\"temperature\":\""+web_interface->answer4M105.substring(Tpos+2+bshift,slashpos)+
                       "\",\"target\":\""+web_interface->answer4M105.substring(slashpos+2,spacepos)+"\",\"active\":\"1\"";
    } else { //no extruder temperature
        buffer2send.concat(FPSTR(NO_TEMP_LINE));
    }

    buffer2send+="},";
    //Extruder 2
    buffer2send.concat(F("{\"name\":\"Extruder 2\","));
    Tpos = web_interface->answer4M105.indexOf("T1:");
    if (Tpos>-1) { //have extruder 2 ?
        slashpos = web_interface->answer4M105.indexOf(" /",Tpos);
        spacepos = web_interface->answer4M105.indexOf(" ",slashpos+2);
        if(slashpos!=-1 && spacepos!=-1 ) {
            buffer2send += "\"temperature\":\""+web_interface->answer4M105.substring(Tpos+3,slashpos)+
                           "\",\"target\":\""+web_interface->answer4M105.substring(slashpos+2,spacepos)+"\",\"active\":\"1\"";
        } else { //no extruder temperature
            buffer2send.concat(FPSTR(NO_TEMP_LINE));
        }
    } else { //no extruder temperature
        buffer2send.concat(FPSTR(NO_TEMP_LINE));
    }
    buffer2send+="},";

    //Bed
    buffer2send.concat(F("{\"name\":\"Bed\","));
    Tpos = web_interface->answer4M105.indexOf("B:");
    if (Tpos>-1) {
        slashpos = web_interface->answer4M105.indexOf(" /",Tpos);
        spacepos = web_interface->answer4M105.indexOf(" ",slashpos+2);
        if(slashpos!=-1 && spacepos!=-1 ) {
            temperature = (int)atof(web_interface->answer4M105.substring(Tpos+2,slashpos).c_str());
            target = (int)atof(web_interface->answer4M105.substring(slashpos+2,spacepos).c_str());
            buffer2send += "\"temperature\":\""+web_interface->answer4M105.substring(Tpos+2,slashpos)+
                           "\",\"target\":\""+web_interface->answer4M105.substring(slashpos+2,spacepos)+"\",\"active\":\"1\"";
        } else { //no extruder temperature
            buffer2send.concat(FPSTR(NO_TEMP_LINE));
        }
    } else { //no extruder temperature
        buffer2send.concat(FPSTR(NO_TEMP_LINE));
    }
    buffer2send+="}";
    buffer2send+="],";

    //information
    buffer2send.concat(F("\"InformationMsg\":["));
    for (int i=0; i<web_interface->info_msg.size(); i++) {
        if (i>0) {
            buffer2send+=",";
        }
        buffer2send+="{\"line\":\"";
        buffer2send+=web_interface->info_msg.get(i);
        buffer2send+="\"}";
    }
    buffer2send+="],";
    //Error
    buffer2send.concat(F("\"ErrorMsg\":["));
    for (int i=0; i<web_interface->error_msg.size(); i++) {
        if (i>0) {
            buffer2send+=",";
        }
        buffer2send+="{\"line\":\"";
        buffer2send+=web_interface->error_msg.get(i);
        buffer2send+="\"}";
    }
    buffer2send+="],";
    //Status
    buffer2send.concat(F("\"StatusMsg\":["));
    for (int i=0; i<web_interface->status_msg.size(); i++) {
        if (i>0) {
            buffer2send+=",";
        }
        buffer2send+="{\"line\":\"";
        buffer2send+=web_interface->status_msg.get(i);
        buffer2send+="\"}";
    }
    buffer2send+="]";
    buffer2send+="}";
    web_interface->WebServer.sendHeader("Cache-Control", "no-cache");
    web_interface->WebServer.send(200, "application/json",buffer2send);
}

String formatBytes(size_t bytes)
{
    if (bytes < 1024) {
        return String(bytes)+" B";
    } else if(bytes < (1024 * 1024)) {
        return String(bytes/1024.0)+" KB";
    } else if(bytes < (1024 * 1024 * 1024)) {
        return String(bytes/1024.0/1024.0)+" MB";
    } else {
        return String(bytes/1024.0/1024.0/1024.0)+" GB";
    }
}

String getContentType(String filename)
{
    if(filename.endsWith(".htm")) {
        return "text/html";
    } else if(filename.endsWith(".html")) {
        return "text/html";
    } else if(filename.endsWith(".css")) {
        return "text/css";
    } else if(filename.endsWith(".js")) {
        return "application/javascript";
    } else if(filename.endsWith(".png")) {
        return "image/png";
    } else if(filename.endsWith(".gif")) {
        return "image/gif";
    } else if(filename.endsWith(".jpg")) {
        return "image/jpeg";
    } else if(filename.endsWith(".ico")) {
        return "image/x-icon";
    } else if(filename.endsWith(".xml")) {
        return "text/xml";
    } else if(filename.endsWith(".pdf")) {
        return "application/x-pdf";
    } else if(filename.endsWith(".zip")) {
        return "application/x-zip";
    } else if(filename.endsWith(".gz")) {
        return "application/x-gzip";
    } else if(filename.endsWith(".tpl")) {
        return "text/plain";
    } else if(filename.endsWith(".inc")) {
        return "text/plain";
    } else if(filename.endsWith(".txt")) {
        return "text/plain";
    }
    return "application/octet-stream";
}

void SPIFFSFileupload()
{
    HTTPUpload& upload = (web_interface->WebServer).upload();
    if(upload.status == UPLOAD_FILE_START) {
        String filename = upload.filename;
        Serial.println("M117 Start ESP upload");
        web_interface->fsUploadFile = SPIFFS.open(filename, "w");
        filename = String();
        web_interface->_upload_status= UPLOAD_STATUS_ONGOING;
    } else if(upload.status == UPLOAD_FILE_WRITE) {
		web_interface->_upload_status= UPLOAD_STATUS_ONGOING;
        if(web_interface->fsUploadFile) {
            web_interface->fsUploadFile.write(upload.buf, upload.currentSize);
        }
    } else if(upload.status == UPLOAD_FILE_END) {
		web_interface->_upload_status=UPLOAD_STATUS_SUCCESSFUL;
		Serial.println("M117 End ESP upload");
        if(web_interface->fsUploadFile) {
            web_interface->fsUploadFile.close();
        }
    } else {
		web_interface->_upload_status=UPLOAD_STATUS_CANCELLED;
        Serial.println("M117 Error ESP upload");
    }
    delay(0);
}

void SDFileupload()
{
	static bool linewrote = false; 
    HTTPUpload& upload = (web_interface->WebServer).upload();
    if(upload.status == UPLOAD_FILE_START) {
		(web_interface->blockserial) = true;
		linewrote = false;
		web_interface->_upload_status= UPLOAD_STATUS_ONGOING;
		Serial.println("M117 Start SD upload");
        String filename = "M28 " + upload.filename;
        Serial.println(filename);
        filename = String();
    } else if(upload.status == UPLOAD_FILE_WRITE) {
			web_interface->_upload_status= UPLOAD_STATUS_ONGOING;
			if (linewrote == false){
            Serial.write("M117 one line yes\n");
            Serial.flush();
            linewrote = true;
            }
            
            //Serial.write(upload.buf, upload.currentSize);
    } else if(upload.status == UPLOAD_FILE_END) {
            web_interface->blockserial = false;
            linewrote = false;
            web_interface->_upload_status=UPLOAD_STATUS_SUCCESSFUL;
			Serial.println("M29\n");
			delay (500);
			Serial.println("M117 SD upload done\n");
    } else {
		web_interface->_upload_status=UPLOAD_STATUS_CANCELLED;
        Serial.println("M29\n");
        delay (500);
        Serial.println("M117 SD upload failed");
    }
    delay(0);
}

#ifdef WEB_UPDATE_FEATURE
void WebUpdateUpload()
{
    HTTPUpload& upload = (web_interface->WebServer).upload();
    if(upload.status == UPLOAD_FILE_START) {
        Serial.println(F("M117 Update Firmware"));
        web_interface->_upload_status= UPLOAD_STATUS_ONGOING;
        WiFiUDP::stopAll();
        uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
        if(!Update.begin(maxSketchSpace)) { //start with max available size
        }
    } else if(upload.status == UPLOAD_FILE_WRITE) {
        web_interface->_upload_status= UPLOAD_STATUS_ONGOING;
        if(Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        }
    } else if(upload.status == UPLOAD_FILE_END) {
        if(Update.end(true)) { //true to set the size to the current progress
            //Now Reboot
            web_interface->_upload_status=UPLOAD_STATUS_SUCCESSFUL;
        }
    } else if(upload.status == UPLOAD_FILE_ABORTED) {
        Update.end();
        web_interface->_upload_status=UPLOAD_STATUS_CANCELLED;
    }
    delay(0);
}

void handleUpdate()
{
    web_interface->is_authenticated();
    String jsonfile = "{\"status\":\"" ;
    jsonfile+=intTostr(web_interface->_upload_status);
    jsonfile+="\"}";
    //send status
    web_interface->WebServer.sendHeader("Cache-Control", "no-cache");
    web_interface->WebServer.send(200, "application/json", jsonfile);
    //if success restart
    if (web_interface->_upload_status==UPLOAD_STATUS_SUCCESSFUL) {
        web_interface->restartmodule=true;
    }
}
#endif

void handleFileList()
{
    if (!web_interface->is_authenticated()) {
        return;
    }
    String path = "/";
    String status="Ok";

    if(web_interface->WebServer.hasArg("action")) {
        if(web_interface->WebServer.arg("action")=="delete" && web_interface->WebServer.hasArg("filename")) {
            String filename;
            web_interface->urldecode(filename,web_interface->WebServer.arg("filename").c_str());
            if(!SPIFFS.exists(filename)) {
                status="Cannot delete, file not found!";
            } else {
                SPIFFS.remove(filename);
            }
        }
    }
    String jsonfile = "{\"path\":\"" + path + "\",";
    Dir dir = SPIFFS.openDir(path);
    jsonfile+="\"files\":[";
    bool firstentry=true;
    while (dir.next()) {
        if (!firstentry) {
            jsonfile+=",";
        } else {
            firstentry=false;
        }
        jsonfile+="{";
        jsonfile+="\"name\":\"";
        jsonfile+=dir.fileName();
        jsonfile+="\",\"size\":\"";
        File f = dir.openFile("r");
        jsonfile+=formatBytes(f.size());
        jsonfile+="\"";
        jsonfile+="}";
        f.close();
    }
    jsonfile+="],";
    jsonfile+="\"status\":\"" + status + "\",";
    FSInfo info;
    SPIFFS.info(info);
    jsonfile+="\"total\":\"" + formatBytes(info.totalBytes) + "\",";
    jsonfile+="\"used\":\"" + formatBytes(info.usedBytes) + "\",";
    jsonfile.concat(F("\"occupation\":\""));
    jsonfile+= intTostr(100*info.usedBytes/info.totalBytes);
    jsonfile+="\"";
    jsonfile+="}";
    path = "";
    web_interface->WebServer.sendHeader("Cache-Control", "no-cache");
    web_interface->WebServer.send(200, "application/json", jsonfile);
}

void handleSDFileList()
{
    if (!web_interface->is_authenticated()) {
        return;
    }
     if(web_interface->WebServer.hasArg("action")) {
        if(web_interface->WebServer.arg("action")=="delete" && web_interface->WebServer.hasArg("filename")) {
            String filename;
            web_interface->urldecode(filename,web_interface->WebServer.arg("filename").c_str());
            filename = "M30 " + filename;
            //TODO:need a MACRO or helper for this test
            if((web_interface->blockserial) == false)Serial.println(filename);
        }
    }
    String jsonfile = "[";
    for (int i=0; i<web_interface->fileslist.size(); i++) {
        if (i>0) {
            jsonfile+=",";
        }
        jsonfile+="{\"entry\":\"";
        jsonfile+=web_interface->fileslist.get(i);
        jsonfile+="\"}";
    }
    jsonfile+="]";
    web_interface->WebServer.sendHeader("Cache-Control", "no-cache");
    web_interface->WebServer.send(200, "application/json", jsonfile);
    web_interface->blockserial = false;
}

//do a redirect to avoid to many query
//and handle not registred path
void handle_not_found()
{
    static const char NOT_AUTH_NF [] PROGMEM = "HTTP/1.1 301 OK\r\nLocation: /HOME\r\nCache-Control: no-cache\r\n\r\n";

    if (!web_interface->is_authenticated()) {
        web_interface->WebServer.sendContent_P(NOT_AUTH_NF);
        return;
    }

    String path = web_interface->WebServer.uri();
    String contentType = getContentType(path);
    String pathWithGz = path + ".gz";
    if(SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) {
        if(SPIFFS.exists(pathWithGz)) {
            path = pathWithGz;
        }
        File file = SPIFFS.open(path, "r");
        web_interface->WebServer.streamFile(file, contentType);
        file.close();
    } else {
        if (SPIFFS.exists("/404.tpl")) {
            STORESTRINGS_CLASS KeysList ;
            STORESTRINGS_CLASS ValuesList ;
            String stmp;

            //IP+Web
            GetIpWeb(KeysList, ValuesList);
            //mode
            GetMode(KeysList, ValuesList);
            //page title and filenames
            SetPageProp(KeysList,ValuesList,F("404 Page not found"),F("404"));

            //Firmware and Free Mem, at the end to reflect situation
            GetFreeMem(KeysList, ValuesList);

            //process the template file and provide list of variables
            processTemplate("/404.tpl", KeysList , ValuesList);
            //need to clean to speed up memory recovery
            KeysList.clear();
            ValuesList.clear();
        } else {
            //if not template use default page
            contentType=FPSTR(PAGE_404);
            String stmp;
            if (wifi_get_opmode()==WIFI_STA ) {
                stmp=WiFi.localIP().toString();
            } else {
                stmp=WiFi.softAPIP().toString();
            }
            //Web address = ip + port
            String KEY_IP = FPSTR(KEY_WEB_ADDRESS);
            if (wifi_config.iweb_port!=80) {
                stmp+=":";
                stmp+=intTostr(wifi_config.iweb_port);
            }
            contentType.replace(KEY_IP,stmp);
            web_interface->WebServer.send(200,"text/html",contentType);
        }
    }
}

void handle_login()
{
    static const char NOT_AUTH_LOG [] PROGMEM = "HTTP/1.1 301 OK\r\nSet-Cookie: ESPSESSIONID=0\r\nLocation: /LOGIN\r\nCache-Control: no-cache\r\n\r\n";

    String smsg;
    String sReturn;
    String sUser,sPassword;
    bool msg_alert_error=false;
    bool msg_alert_success=false;
    STORESTRINGS_CLASS KeysList ;
    STORESTRINGS_CLASS ValuesList ;

    if (web_interface->WebServer.hasArg("DISCONNECT")) {
        web_interface->WebServer.sendContent_P(NOT_AUTH_LOG);
        return;
    }

    //check is it is a submission or a display
    smsg="";
    if (web_interface->WebServer.hasArg("return")) {
        web_interface->urldecode(sReturn,web_interface->WebServer.arg("return").c_str());
    }
    if (web_interface->WebServer.hasArg("SUBMIT")) {
        //is there a correct list of values?
        if ( web_interface->WebServer.hasArg("PASSWORD")&& web_interface->WebServer.hasArg("USER")) {
            //USER
            web_interface->urldecode(sUser,web_interface->WebServer.arg("USER").c_str());
#ifdef AUTHENTICATION_FEATURE
            if (sUser!="admin") {
                msg_alert_error=true;
                smsg.concat(F("Error : Incorrect User<BR>"));
                KeysList.add(FPSTR(KEY_USER_STATUS));
                ValuesList.add(FPSTR(VALUE_HAS_ERROR));
            }
            //Password
            web_interface->urldecode(sPassword,web_interface->WebServer.arg("PASSWORD").c_str());
            String scurrentPassword;

            if (!CONFIG::read_string(EP_ADMIN_PWD, scurrentPassword , MAX_ADMIN_PASSWORD_LENGTH)) {
                scurrentPassword=FPSTR(DEFAULT_ADMIN);
            }

            if (strcmp(sPassword.c_str(),scurrentPassword.c_str())!=0) {
                msg_alert_error=true;
                smsg.concat(F("Error: Incorrect password<BR>"));
                KeysList.add(FPSTR(KEY_USER_PASSWORD_STATUS));
                ValuesList.add(FPSTR(VALUE_HAS_ERROR));
            }
#endif
        } else {
            msg_alert_error=true;
            smsg = FPSTR(MISSING_DATA);
        }

        //if no error login is ok
        if (msg_alert_error==false) {
#ifdef AUTHENTICATION_FEATURE
            auth_ip * current_auth = new auth_ip;
            current_auth->ip=web_interface->WebServer.client().remoteIP();
            strcpy(current_auth->sessionID,web_interface->create_session_ID());
            current_auth->last_time=millis();
            if (web_interface->AddAuthIP(current_auth)) {
                String header = F("HTTP/1.1 301 OK\r\nSet-Cookie: ESPSESSIONID=");
                header+=current_auth->sessionID;
                header+="\r\nLocation: /";
                header+=sReturn;
                header.concat(F("\r\nCache-Control: no-cache\r\n\r\n"));
                web_interface->WebServer.sendContent(header);
                return;
            } else {
                delete current_auth;
                msg_alert_error=true;
                smsg = F("Error: Too many connections");
            }
#endif
        }
    }

    else { //no submit need to get data from EEPROM
        sUser=String();
        //password
        sPassword=String();
    }

    //Display values
    KeysList.add(FPSTR(KEY_RETURN));
    ValuesList.add(sReturn);

    KeysList.add(FPSTR(KEY_DISCONNECT_VISIBILITY));
    if (web_interface->is_authenticated()) {
        ValuesList.add(FPSTR(VALUE_ITEM_VISIBLE));
    } else {
        ValuesList.add(FPSTR(VALUE_ITEM_HIDDEN));
    }
    //IP+Web
    GetIpWeb(KeysList, ValuesList);
    //mode
    GetMode(KeysList, ValuesList);

    //page title and filenames
    SetPageProp(KeysList,ValuesList,FPSTR(VALUE_LOGIN),F("login"));
    //User
    KeysList.add(FPSTR(KEY_USER));
    ValuesList.add(sUser);
    //password
    KeysList.add(FPSTR(KEY_USER_PASSWORD));
    ValuesList.add(sPassword);

    if (msg_alert_error) {
        ProcessAlertError(KeysList, ValuesList, smsg);
    } else {
        ProcessNoAlert(KeysList,ValuesList);
    }

    //Firmware and Free Mem, at the end to reflect situation
    GetFreeMem(KeysList, ValuesList);

    //process the template file and provide list of variables
    processTemplate("/login.tpl", KeysList , ValuesList);
    //need to clean to speed up memory recovery
    KeysList.clear();
    ValuesList.clear();
}

void handle_restart()
{
    if (SPIFFS.exists("/restart.tpl")) {
        STORESTRINGS_CLASS KeysList ;
        STORESTRINGS_CLASS ValuesList ;

        //IP+Web
        GetIpWeb(KeysList, ValuesList);
        //mode
        GetMode(KeysList, ValuesList);
        //page title and filenames
        SetPageProp(KeysList,ValuesList,F("Restarting..."),F("restart"));

        //Firmware and Free Mem, at the end to reflect situation
        GetFreeMem(KeysList, ValuesList);

        //process the template file and provide list of variables
        processTemplate("/restart.tpl", KeysList , ValuesList);
        //need to clean to speed up memory recovery
        KeysList.clear();
        ValuesList.clear();
    } else {
        //if not restart template use default
        String contentType=FPSTR(PAGE_RESTART);
        web_interface->WebServer.send(200,"text/html",contentType);
    }
    web_interface->restartmodule=true;
}

void handle_web_command()
{
    if (!web_interface->is_authenticated()) {
        return;
    }
    //check we have proper parameter
    if (web_interface->WebServer.hasArg("COM")) {
        String scmd;
        //decode command
        web_interface->urldecode(scmd,web_interface->WebServer.arg("COM").c_str());
        scmd.trim();
        //give an ack - we need to be polite, right ?
        web_interface->WebServer.send(200,"text/plain","Ok");
        //if it is for ESP module [ESPXXX]<parameter>
        int ESPpos = scmd.indexOf("[ESP");
        if (ESPpos>-1) {
            //is there the second part?
            int ESPpos2 = scmd.indexOf("]",ESPpos);
            if (ESPpos2>-1) {
                //Split in command and parameters
                String cmd_part1=scmd.substring(ESPpos+4,ESPpos2);
                String cmd_part2="";
                //is there space for parameters?
                if (ESPpos2<scmd.length()) {
                    cmd_part2=scmd.substring(ESPpos2+1);
                }
                //if command is a valid number then execute command
                if(cmd_part1.toInt()!=0) {
                    COMMAND::execute_command(cmd_part1.toInt(),cmd_part2);
                }
                //if not is not a valid [ESPXXX] command
            }
        }
        else {
			 //send command to serial as no need to transfer ESP command
			 //to avoid any pollution if Uploading file to SDCard
			 if ((web_interface->blockserial) == false)Serial.println(scmd);
		}
    }
}

#ifdef SSDP_FEATURE
void handle_SSDP()
{
    SSDP.schema(web_interface->WebServer.client());
}
#endif

//URI Decoding function
void   WEBINTERFACE_CLASS::urldecode( String & dst, const char *src)
{
    char a, b,c;
    dst="";
    while (*src) {
        if ((*src == '%') &&
                ((a = src[1]) && (b = src[2])) &&
                (isxdigit(a) && isxdigit(b))) {
            if (a >= 'a') {
                a -= 'a'-'A';
            }
            if (a >= 'A') {
                a -= ('A' - 10);
            } else {
                a -= '0';
            }
            if (b >= 'a') {
                b -= 'a'-'A';
            }
            if (b >= 'A') {
                b -= ('A' - 10);
            } else {
                b -= '0';
            }
            dst+= char(16*a+b);
            src+=3;
        } else {
            c = *src++;
            if(c=='+') {
                c=' ';
            }
            dst+= char(c);
        }
    }
}

//constructor
WEBINTERFACE_CLASS::WEBINTERFACE_CLASS (int port):WebServer(port)
{
    //init what will handle "/"
    WebServer.on("/",HTTP_ANY, handle_web_interface_root);
    WebServer.on("/HOME",HTTP_ANY, handle_web_interface_home);
    WebServer.on("/CONFIGSYS",HTTP_ANY, handle_web_interface_configSys);
    WebServer.on("/CONFIGAP",HTTP_ANY, handle_web_interface_configAP);
    WebServer.on("/CONFIGSTA",HTTP_ANY, handle_web_interface_configSTA);
    WebServer.on("/STATUS",HTTP_ANY, handle_web_interface_status);
    WebServer.on("/SETTINGS",HTTP_ANY, handle_web_settings);
    WebServer.on("/PRINTER",HTTP_ANY, handle_web_interface_printer);
    WebServer.on("/CMD",HTTP_ANY, handle_web_command);
    WebServer.on("/RESTART",HTTP_GET, handle_restart);
#ifdef WEB_UPDATE_FEATURE
    WebServer.on("/UPDATE",HTTP_ANY, handleUpdate,WebUpdateUpload);
#endif
    WebServer.on("/FILES", HTTP_ANY, handleFileList,SPIFFSFileupload);
    WebServer.on("/SDFILES", HTTP_ANY, handleSDFileList,SDFileupload);
    WebServer.on("/LOGIN", HTTP_ANY, handle_login);
    WebServer.on("/PASSWORD", HTTP_ANY, handle_password);
    //Captive portal Feature
#ifdef CAPTIVE_PORTAL_FEATURE
    WebServer.on("/generate_204",HTTP_ANY, handle_web_interface_root);
    WebServer.on("/fwlink",HTTP_ANY, handle_web_interface_root);
#endif
#ifdef SSDP_FEATURE
    WebServer.on("/description.xml", HTTP_GET, handle_SSDP);
#endif
    WebServer.onNotFound( handle_not_found);
    answer4M105="T:0 /0 ";
    answer4M114="X:0.0 Y:0.0 Z:0.000";
    answer4M220="100";
    answer4M221="100";
    blockserial = false;
    last_temp=system_get_time();
    restartmodule=false;
    //rolling list of 4entries with a maximum of 50 char for each entry
    error_msg.setsize(4);
    error_msg.setlength(50);
    info_msg.setsize(4);
    info_msg.setlength(50);
    status_msg.setsize(4);
    status_msg.setlength(50);
    fileslist.setlength(30);//12 for filename + space + size
    fileslist.setsize(70); // 70 files to limite to 2K
    fsUploadFile=(fs::File)0;
    _head=NULL;
    _nb_ip=0;
    _upload_status=UPLOAD_STATUS_NONE;
}
//Destructor
WEBINTERFACE_CLASS::~WEBINTERFACE_CLASS()
{
    info_msg.clear();
    error_msg.clear();
    status_msg.clear();
    fileslist.clear();
    while (_head) {
        auth_ip * current = _head;
        _head=_head->_next;
        delete current;
    }
    _nb_ip=0;
}
//Session ID based on IP and time using 16 char
char * WEBINTERFACE_CLASS::create_session_ID()
{
    static char  sessionID[17];
//reset SESSIONID
    for (int i=0; i<17; i++) {
        sessionID[i]='\0';
    }
//get time
    uint32_t now = millis();
//get remote IP
    IPAddress remoteIP=WebServer.client().remoteIP();
//generate SESSIONID
    if (0>sprintf(sessionID,"%02X%02X%02X%02X%02X%02X%02X%02X",remoteIP[0],remoteIP[1],remoteIP[2],remoteIP[3],(uint8_t) ((now >> 0) & 0xff),(uint8_t) ((now >> 8) & 0xff),(uint8_t) ((now >> 16) & 0xff),(uint8_t) ((now >> 24) & 0xff))) {
        strcpy(sessionID,"NONE");
    }
    return sessionID;
}
//check authentification
bool  WEBINTERFACE_CLASS::is_authenticated()
{
#ifdef AUTHENTICATION_FEATURE
    if (WebServer.hasHeader("Cookie")) {
        String cookie = WebServer.header("Cookie");
        int pos = cookie.indexOf("ESPSESSIONID=");
        if (pos!= -1) {
            int pos2 = cookie.indexOf(";",pos);
            String sessionID = cookie.substring(pos+strlen("ESPSESSIONID="),pos2);
            IPAddress ip = WebServer.client().remoteIP();
            //check if cookie can be reset and clean table in same time
            return ResetAuthIP(ip,sessionID.c_str());
        }
    }
    return false;
#else
    return true;
#endif
}

//add the information in the linked list if possible
bool WEBINTERFACE_CLASS::AddAuthIP(auth_ip * item)
{
    if (_nb_ip>MAX_AUTH_IP) {
        return false;
    }
    item->_next=_head;
    _head=item;
    _nb_ip++;
    return true;
}

bool WEBINTERFACE_CLASS::ResetAuthIP(IPAddress ip,const char * sessionID)
{
    bool done=false;
    auth_ip * current = _head;
    auth_ip * previous = NULL;
    //get time
    uint32_t now = millis();
    while (current) {
        if ((millis()-current->last_time)>400000) {
            //remove
            if (current==_head) {
                _head=current->_next;
                _nb_ip--;
                delete current;
                current=_head;
            } else {
                previous->_next=current->_next;
                _nb_ip--;
                delete current;
                current=previous->_next;
            }
        } else {
            if (ip==current->ip) {
                if (strcmp(sessionID,current->sessionID)==0) {
                    //reset time
                    current->last_time=millis();
                    return true;
                }
            }
            previous = current;
            current=current->_next;
        }
    }
    return done;
}

WEBINTERFACE_CLASS * web_interface;
