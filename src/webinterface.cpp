/*
  webinterface.cpp - ESP3D configuration class

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
#include "wificonf.h"
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <WiFiUdp.h>
#include <StreamString.h>
#ifndef FS_NO_GLOBALS
#define FS_NO_GLOBALS
#endif
#include <FS.h>
#include <ESPAsyncWebServer.h>
#ifdef ARDUINO_ARCH_ESP8266
#include "ESP8266WiFi.h"
#include <ESPAsyncTCP.h>
#else //ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#include "SPIFFS.h"
#include "Update.h"
#endif

#include "GenLinkedList.h"
#include "storestrings.h"
#include "command.h"
#include "espcom.h"

#ifdef SSDP_FEATURE
    #ifdef ARDUINO_ARCH_ESP32
    #include <ESP32SSDP.h>
    #else
    #include <ESP8266SSDP.h>
    #endif
#endif

//embedded response file if no files on SPIFFS
#include "nofile.h"

#define MAX_AUTH_IP 10
#define HIDDEN_PASSWORD "********"

 static long id_connection = 0;

typedef enum {
    UPLOAD_STATUS_NONE = 0,
    UPLOAD_STATUS_FAILED = 1,
    UPLOAD_STATUS_CANCELLED = 2,
    UPLOAD_STATUS_SUCCESSFUL = 3,
    UPLOAD_STATUS_ONGOING  = 4
} upload_status_type;

const uint8_t PAGE_404 [] PROGMEM = "<HTML>\n<HEAD>\n<title>Redirecting...</title> \n</HEAD>\n<BODY>\n<CENTER>Unknown page : $QUERY$- you will be redirected...\n<BR><BR>\nif not redirected, <a href='http://$WEB_ADDRESS$'>click here</a>\n<BR><BR>\n<PROGRESS name='prg' id='prg'></PROGRESS>\n\n<script>\nvar i = 0; \nvar x = document.getElementById(\"prg\"); \nx.max=5; \nvar interval=setInterval(function(){\ni=i+1; \nvar x = document.getElementById(\"prg\"); \nx.value=i; \nif (i>5) \n{\nclearInterval(interval);\nwindow.location.href='/';\n}\n},1000);\n</script>\n</CENTER>\n</BODY>\n</HTML>\n\n";
const uint8_t PAGE_CAPTIVE [] PROGMEM = "<HTML>\n<HEAD>\n<title>Captive Portal</title> \n</HEAD>\n<BODY>\n<CENTER>Captive Portal page : $QUERY$- you will be redirected...\n<BR><BR>\nif not redirected, <a href='http://$WEB_ADDRESS$'>click here</a>\n<BR><BR>\n<PROGRESS name='prg' id='prg'></PROGRESS>\n\n<script>\nvar i = 0; \nvar x = document.getElementById(\"prg\"); \nx.max=5; \nvar interval=setInterval(function(){\ni=i+1; \nvar x = document.getElementById(\"prg\"); \nx.value=i; \nif (i>5) \n{\nclearInterval(interval);\nwindow.location.href='/';\n}\n},1000);\n</script>\n</CENTER>\n</BODY>\n</HTML>\n\n";
#define  CONTENT_TYPE_HTML "text/html"

#ifdef AUTHENTICATION_FEATURE
/*
void handle_login()
{
    String smsg;
    String sUser,sPassword;
    String auths;
    int code = 200;
    bool msg_alert_error=false;
    //disconnect can be done anytime no need to check credential
    if (web_interface->web_server.hasArg("DISCONNECT")) {
        String cookie = web_interface->web_server.header("Cookie");
        int pos = cookie.indexOf("ESPSESSIONID=");
        String sessionID;
        if (pos!= -1) {
            int pos2 = cookie.indexOf(";",pos);
            sessionID = cookie.substring(pos+strlen("ESPSESSIONID="),pos2);
            }
        web_interface->ClearAuthIP(web_interface->web_server.client().remoteIP(), sessionID.c_str());
        web_interface->web_server.sendHeader("Set-Cookie","ESPSESSIONID=0");
        web_interface->web_server.sendHeader("Cache-Control","no-cache");
        String buffer2send = "{\"status\":\"Ok\",\"authentication_lvl\":\"guest\"}";
        web_interface->web_server.send(code, "application/json", buffer2send);
        //web_interface->web_server.client().stop();
        return;
    }

    level_authenticate_type auth_level= web_interface->is_authenticated();
   if (auth_level == LEVEL_GUEST) auths = F("guest");
    else if (auth_level == LEVEL_USER) auths = F("user");
    else if (auth_level == LEVEL_ADMIN) auths = F("admin");
    else auths = F("???");

    //check is it is a submission or a query
    if (web_interface->web_server.hasArg("SUBMIT")) {
        //is there a correct list of query?
        if ( web_interface->web_server.hasArg("PASSWORD")&& web_interface->web_server.hasArg("USER")) {
            //USER
            sUser = web_interface->web_server.arg("USER");
            if ( !((sUser==FPSTR(DEFAULT_ADMIN_LOGIN)) || (sUser==FPSTR(DEFAULT_USER_LOGIN)))) {
                msg_alert_error=true;
                smsg=F("Error : Incorrect User");
                code=401;
            }
            if (msg_alert_error == false) {
                //Password
                sPassword = web_interface->web_server.arg("PASSWORD");
                String sadminPassword;

                if (!CONFIG::read_string(EP_ADMIN_PWD, sadminPassword, MAX_LOCAL_PASSWORD_LENGTH)) {
                    sadminPassword=FPSTR(DEFAULT_ADMIN_PWD);
                }

                String suserPassword;

                if (!CONFIG::read_string(EP_USER_PWD, suserPassword, MAX_LOCAL_PASSWORD_LENGTH)) {
                    suserPassword=FPSTR(DEFAULT_USER_PWD);
                }

                if(!(((sUser==FPSTR(DEFAULT_ADMIN_LOGIN)) && (strcmp(sPassword.c_str(),sadminPassword.c_str())==0)) ||
                        ((sUser==FPSTR(DEFAULT_USER_LOGIN)) && (strcmp(sPassword.c_str(),suserPassword.c_str()) == 0)))) {
                    msg_alert_error=true;
                    smsg=F("Error: Incorrect password");
                    code = 401;
                }
            }
        } else {
            msg_alert_error=true;
            smsg = F("Error: Missing data");
            code = 500;
        }
        //change password
        if ( web_interface->web_server.hasArg("PASSWORD")&& web_interface->web_server.hasArg("USER") && web_interface->web_server.hasArg("NEWPASSWORD") && (msg_alert_error==false) ) {
            String newpassword =  web_interface->web_server.arg("NEWPASSWORD");
            if (CONFIG::isLocalPasswordValid(newpassword.c_str())) {
                int pos=0;
                if(sUser==FPSTR(DEFAULT_ADMIN_LOGIN)) pos = EP_ADMIN_PWD;
                else pos = EP_USER_PWD;
                if (!CONFIG::write_string(pos,newpassword.c_str())){
                     msg_alert_error=true;
                     smsg = F("Error: Cannot apply changes");
                     code = 500;
                }
            } else {
                msg_alert_error=true;
                smsg = F("Error: Incorrect password");
                code = 500;
            }
        }
   if ((code == 200) || (code == 500)) {
       level_authenticate_type current_auth_level;
      if(sUser == FPSTR(DEFAULT_ADMIN_LOGIN)) {
            current_auth_level = LEVEL_ADMIN;
        } else  if(sUser == FPSTR(DEFAULT_USER_LOGIN)){
            current_auth_level = LEVEL_USER;
        } else {
            current_auth_level = LEVEL_GUEST;
        }
        //create Session
        if ((current_auth_level != auth_level) || (auth_level== LEVEL_GUEST)) {
            auth_ip * current_auth = new auth_ip;
            current_auth->level = current_auth_level;
            current_auth->ip=web_interface->web_server.client().remoteIP();
            strcpy(current_auth->sessionID,web_interface->create_session_ID());
            strcpy(current_auth->userID,sUser.c_str());
            current_auth->last_time=millis();
            if (web_interface->AddAuthIP(current_auth)) {
                String tmps ="ESPSESSIONID=";
                tmps+=current_auth->sessionID;
                web_interface->web_server.sendHeader("Set-Cookie",tmps);
                web_interface->web_server.sendHeader("Cache-Control","no-cache");
                switch(current_auth->level) {
                    case LEVEL_ADMIN:
                        auths = "admin";
                        break;
                     case LEVEL_USER:
                        auths = "user";
                        break;
                    default:
                        auths = "guest";
                    }
            } else {
                delete current_auth;
                msg_alert_error=true;
                code = 500;
                smsg = F("Error: Too many connections");
            }
        }
   }
    if (code == 200) smsg = F("Ok");

    //build  JSON
    String buffer2send = "{\"status\":\"" + smsg + "\",\"authentication_lvl\":\"";
    buffer2send += auths;
    buffer2send += "\"}";
    web_interface->web_server.send(code, "application/json", buffer2send);
    } else {
    if (auth_level != LEVEL_GUEST) {
        String cookie = web_interface->web_server.header("Cookie");
        int pos = cookie.indexOf("ESPSESSIONID=");
        String sessionID;
        if (pos!= -1) {
            int pos2 = cookie.indexOf(";",pos);
            sessionID = cookie.substring(pos+strlen("ESPSESSIONID="),pos2);
            auth_ip * current_auth_info = web_interface->GetAuth(web_interface->web_server.client().remoteIP(), sessionID.c_str());
            if (current_auth_info != NULL){
                    sUser = current_auth_info->userID;
                }
        }
    }
    String buffer2send = "{\"status\":\"200\",\"authentication_lvl\":\"";
    buffer2send += auths;
    buffer2send += "\",\"user\":\"";
    buffer2send += sUser;
    buffer2send +="\"}";
    web_interface->web_server.send(code, "application/json", buffer2send);
    }
}
*/
#endif

#ifdef SSDP_FEATURE
void handle_SSDP (AsyncWebServerRequest *request)
{
    StreamString sschema ;
    if (sschema.reserve (1024) ) {
        String templ =  "<?xml version=\"1.0\"?>"
                        "<root xmlns=\"urn:schemas-upnp-org:device-1-0\">"
                        "<specVersion>"
                        "<major>1</major>"
                        "<minor>0</minor>"
                        "</specVersion>"
                        "<URLBase>http://%s:%u/</URLBase>"
                        "<device>"
                        "<deviceType>upnp:rootdevice</deviceType>"
                        "<friendlyName>%s</friendlyName>"
                        "<presentationURL>/</presentationURL>"
                        "<serialNumber>%s</serialNumber>"
#ifdef ARDUINO_ARCH_ESP32
                        "<modelName>ESP32</modelName>"
#else
                        "<modelName>ESP8266</modelName>"
#endif
                        "<modelNumber>ESP3D</modelNumber>"
#ifdef ARDUINO_ARCH_ESP32
                        "<modelURL>http://espressif.com/en/products/hardware/esp-wroom-32/overview</modelURL>"
#else
                        "<modelURL>http://espressif.com/en/products/esp8266</modelURL>"
#endif
                        "<manufacturer>Espressif Systems</manufacturer>"
                        "<manufacturerURL>http://espressif.com</manufacturerURL>"
                        "<UDN>uuid:%s</UDN>"
                        "</device>"
                        "</root>\r\n"
                        "\r\n";
        char uuid[37];
        String sip = WiFi.localIP().toString();
#ifdef ARDUINO_ARCH_ESP8266
        uint32_t chipId = ESP.getChipId();
#else
        uint32_t chipId = (uint16_t) (ESP.getEfuseMac() >> 32);
#endif
        sprintf (uuid, "38323636-4558-4dda-9188-cda0e6%02x%02x%02x",
                 (uint16_t) ( (chipId >> 16) & 0xff),
                 (uint16_t) ( (chipId >>  8) & 0xff),
                 (uint16_t)   chipId        & 0xff  );
        String friendlyName;
        if (!CONFIG::read_string (EP_HOSTNAME, friendlyName, MAX_HOSTNAME_LENGTH) ) {
            friendlyName = wifi_config.get_default_hostname();
        }
        String serialNumber = String (chipId);
        sschema.printf (templ.c_str(),
                        sip.c_str(),
                        wifi_config.iweb_port,
                        friendlyName.c_str(),
                        serialNumber.c_str(),
                        uuid);
        request->send (200, "text/xml", (String) sschema);
    } else {
        request->send (500);
    }
}
#endif

//Not found Page
void handle_not_found (AsyncWebServerRequest *request)
{
    //if we are here it means no index.html
    if (request->url() == "/") {
        AsyncWebServerResponse * response = request->beginResponse_P (200, CONTENT_TYPE_HTML, PAGE_NOFILES, PAGE_NOFILES_SIZE);
        response->addHeader ("Content-Encoding", "gzip");
        request->send (response);
    } else {
        String path = F ("/404.htm");
        String pathWithGz =  path + F (".gz");
        if (SPIFFS.exists (pathWithGz) || SPIFFS.exists (path) ) {
            request->send (SPIFFS, path);
        } else {
            //if not template use default page
            String contentpage = FPSTR (PAGE_404);
            String stmp;
            if (WiFi.getMode() == WIFI_STA ) {
                stmp = WiFi.localIP().toString();
            } else {
                stmp = WiFi.softAPIP().toString();
            }
            //Web address = ip + port
            String KEY_IP = F ("$WEB_ADDRESS$");
            String KEY_QUERY = F ("$QUERY$");
            if (wifi_config.iweb_port != 80) {
                stmp += ":";
                stmp += CONFIG::intTostr (wifi_config.iweb_port);
            }
            contentpage.replace (KEY_IP, stmp);
            contentpage.replace (KEY_QUERY, request->url() );
            request->send (404, CONTENT_TYPE_HTML, contentpage.c_str() );
        }
    }
}

//filter to intercept command line on root
bool filterOnRoot (AsyncWebServerRequest *request)
{
    if (request->hasArg ("forcefallback") ) {
        String stmp = request->arg ("forcefallback");
        //to use all case
        stmp.toLowerCase();
        if ( stmp == "yes") {
            return false;
        }
    }
    return true;
}

//SPIFFS
//SPIFFS files list and file commands
void handleFileList (AsyncWebServerRequest *request)
{
    level_authenticate_type auth_level = web_interface->is_authenticated();
    if (auth_level == LEVEL_GUEST) {
        web_interface->_upload_status = UPLOAD_STATUS_NONE;
        request->send (401, "text/plain", "Authentication failed!\n");
        return;
    }
    String path ;
    String status = "Ok";
    if ( (web_interface->_upload_status == UPLOAD_STATUS_FAILED) || (web_interface->_upload_status == UPLOAD_STATUS_CANCELLED) ) {
        status = "Upload failed";
    }
    //be sure root is correct according authentication
    if (auth_level == LEVEL_ADMIN) {
        path = "/";
    } else {
        path = "/user";
    }
    //get current path
    if (request->hasArg ("path") ) {
        path += request->arg ("path") ;
    }
    //to have a clean path
    path.trim();
    path.replace ("//", "/");
    if (path[path.length() - 1] != '/') {
        path += "/";
    }
    //check if query need some action
    if (request->hasArg ("action") ) {
        //delete a file
        if (request->arg ("action") == "delete" && request->hasArg ("filename") ) {
            String filename;
            String shortname = request->arg ("filename");
            shortname.replace ("/", "");
            filename = path + request->arg ("filename");
            filename.replace ("//", "/");
            if (!SPIFFS.exists (filename) ) {
                status = shortname + F (" does not exists!");
            } else {
                if (SPIFFS.remove (filename) ) {
                    status = shortname + F (" deleted");
                    //what happen if no "/." and no other subfiles ?
#ifdef ARDUINO_ARCH_ESP8266
                    FS_DIR dir = SPIFFS.openDir (path);
                    if (!dir.next() ) {
#else
                    String ptmp = path;
                    if ( (path != "/") && (path[path.length() - 1] = '/') ) {
                        ptmp = path.substring (0, path.length() - 1);
                    }
                    FS_FILE dir = SPIFFS.open (ptmp);
                    FS_FILE dircontent = dir.openNextFile();
                    if (!dircontent) {
#endif
                        //keep directory alive even empty
                        FS_FILE r = SPIFFS.open (path + "/.", SPIFFS_FILE_WRITE);
                        if (r) {
                            r.close();
                        }
                    }
                } else {
                    status = F ("Cannot deleted ") ;
                    status += shortname ;
                }
            }
        }
        //delete a directory
        if (request->arg ("action") == "deletedir" && request->hasArg ("filename") ) {
            String filename;
            String shortname = request->arg ("filename");
            shortname.replace ("/", "");
            filename = path + request->arg ("filename");
            filename += "/";
            filename.replace ("//", "/");
            if (filename != "/") {
                bool delete_error = false;
#ifdef ARDUINO_ARCH_ESP8266
                FS_DIR dir = SPIFFS.openDir (path + shortname);
                {
                    while (dir.next() ) {
#else
                FS_FILE dir = SPIFFS.open (path + shortname);
                {
                    FS_FILE file2deleted = dir.openNextFile();
                    while (file2deleted) {
#endif
#ifdef ARDUINO_ARCH_ESP8266
                        String fullpath = dir.fileName();
#else
                        String fullpath = file2deleted.name();
#endif
                        if (!SPIFFS.remove (fullpath) ) {
                            delete_error = true;
                            status = F ("Cannot deleted ") ;
                            status += fullpath;
                        }
#ifdef ARDUINO_ARCH_ESP32
                        file2deleted = dir.openNextFile();
#endif
                    }
                }
                if (!delete_error) {
                    status = shortname ;
                    status += " deleted";
                }
            }
        }
        //create a directory
        if (request->arg ("action") == "createdir" && request->hasArg ("filename") ) {
            String filename;
            filename = path + request->arg ("filename") + "/.";
            String shortname = request->arg ("filename");
            shortname.replace ("/", "");
            filename.replace ("//", "/");
            if (SPIFFS.exists (filename) ) {
                status = shortname + F (" already exists!");
            } else {
                FS_FILE r = SPIFFS.open (filename, SPIFFS_FILE_WRITE);
                if (!r) {
                    status = F ("Cannot create ");
                    status += shortname ;
                } else {
                    r.close();
                    status = shortname + F (" created");
                }
            }
        }
    }
    String jsonfile = "{";
#ifdef ARDUINO_ARCH_ESP8266
    FS_DIR dir = SPIFFS.openDir (path);
#else
    String ptmp = path;
    if ( (path != "/") && (path[path.length() - 1] = '/') ) {
        ptmp = path.substring (0, path.length() - 1);
    }
    FS_FILE dir = SPIFFS.open (ptmp);
#endif
    jsonfile += "\"files\":[";
    bool firstentry = true;
    String subdirlist = "";
#ifdef ARDUINO_ARCH_ESP8266
    while (dir.next() ) {
        String filename = dir.fileName();
#else
    File fileparsed = dir.openNextFile();
    while (fileparsed) {
        String filename = fileparsed.name();
#endif
        String size = "";
        bool addtolist = true;
        //remove path from name
        filename = filename.substring (path.length(), filename.length() );
        //check if file or subfile
        if (filename.indexOf ("/") > -1) {
            //Do not rely on "/." to define directory as SPIFFS upload won't create it but directly files
            //and no need to overload SPIFFS if not necessary to create "/." if no need
            //it will reduce SPIFFS available space so limit it to creation
            filename = filename.substring (0, filename.indexOf ("/") );
            String tag = "*";
            tag += filename + "*";
            if (subdirlist.indexOf (tag) > -1 || filename.length() == 0) { //already in list
                addtolist = false; //no need to add
            } else {
                size = "-1"; //it is subfile so display only directory, size will be -1 to describe it is directory
                if (subdirlist.length() == 0) {
                    subdirlist += "*";
                }
                subdirlist += filename + "*"; //add to list
            }
        } else {
            //do not add "." file
            if (! ( (filename == ".") || (filename == "") ) ) {
#ifdef ARDUINO_ARCH_ESP8266
                size = CONFIG::formatBytes (dir.fileSize() );
#else
                size = CONFIG::formatBytes (fileparsed.size() );
#endif

            } else {
                addtolist = false;
            }
        }
        if (addtolist) {
            if (!firstentry) {
                jsonfile += ",";
            } else {
                firstentry = false;
            }
            jsonfile += "{";
            jsonfile += "\"name\":\"";
            jsonfile += filename;
            jsonfile += "\",\"size\":\"";
            jsonfile += size;
            jsonfile += "\"";
            jsonfile += "}";
        }
#ifdef ARDUINO_ARCH_ESP32
        fileparsed = dir.openNextFile();
#endif
    }
    jsonfile += "],";
    jsonfile += "\"path\":\"" + path + "\",";
    jsonfile += "\"status\":\"" + status + "\",";
    size_t totalBytes;
    size_t usedBytes;
#ifdef ARDUINO_ARCH_ESP8266
    fs::FSInfo info;
    SPIFFS.info (info);
    totalBytes = info.totalBytes;
    usedBytes = info.usedBytes;
#else
    totalBytes = SPIFFS.totalBytes();
    usedBytes = SPIFFS.usedBytes();
#endif
    jsonfile += "\"total\":\"" + CONFIG::formatBytes (totalBytes) + "\",";
    jsonfile += "\"used\":\"" + CONFIG::formatBytes (usedBytes) + "\",";
    jsonfile.concat (F ("\"occupation\":\"") );
    jsonfile += CONFIG::intTostr (100 * usedBytes / totalBytes);
    jsonfile += "\"";
    jsonfile += "}";
    path = "";
    AsyncResponseStream  *response = request->beginResponseStream ("application/json");
    response->addHeader ("Cache-Control", "no-cache");
    response->print (jsonfile.c_str() );
    request->send (response);
    web_interface->_upload_status = UPLOAD_STATUS_NONE;
}

//SPIFFS files uploader handle
void SPIFFSFileupload (AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
{
    LOG ("Uploading: ")
    LOG (filename)
    LOG ("\n")
    //Guest cannot upload - only admin
    if (web_interface->is_authenticated() != LEVEL_ADMIN) {
        web_interface->_upload_status = UPLOAD_STATUS_CANCELLED;
        ESPCOM::println (F ("Upload rejected"), PRINTER_PIPE);
        LOG ("Upload rejected\r\n");
        request->client()->abort();
        return;
    }
    String upload_filename = filename;
    if (upload_filename[0] != '/') {
        upload_filename = "/" + filename;
        LOG ("Fix :")
        LOG (upload_filename)
        LOG ("\n")
    }
    //Upload start
    //**************
    if (!index) {
        LOG ("Upload start: ")
        LOG ("filename")
        LOG ("\n")
        LOG (upload_filename)
        if (SPIFFS.exists (upload_filename) ) {
            SPIFFS.remove (upload_filename);
        }
        if (request->_tempFile) {
            request->_tempFile.close();
        }
        request->_tempFile = SPIFFS.open (upload_filename, SPIFFS_FILE_WRITE);
        if (!request->_tempFile) {
            LOG ("Error")
            request->client()->abort();
            web_interface->_upload_status = UPLOAD_STATUS_FAILED;
        } else {
            web_interface->_upload_status = UPLOAD_STATUS_ONGOING;
        }

    }
    //Upload write
    //**************
    if ( request->_tempFile) {
        if ( ( web_interface->_upload_status = UPLOAD_STATUS_ONGOING) && len) {
            request->_tempFile.write (data, len);
            LOG ("Write file\n")
        }
    }
    //Upload end
    //**************
    if (final) {
        request->_tempFile.flush();
        request->_tempFile.close();
        request->_tempFile = SPIFFS.open (upload_filename, SPIFFS_FILE_READ);
        uint32_t filesize = request->_tempFile.size();
        request->_tempFile.close();
        String  sizeargname  = filename + "S";
        if (request->hasArg (sizeargname.c_str()) ) {
            if (request->arg (sizeargname.c_str()) != String(filesize)) {
                web_interface->_upload_status = UPLOAD_STATUS_FAILED;
                SPIFFS.remove (upload_filename);
                }
            } 
        LOG ("Close file\n")
        if (web_interface->_upload_status == UPLOAD_STATUS_ONGOING) {
            web_interface->_upload_status = UPLOAD_STATUS_SUCCESSFUL;
        }
    }
}

//FW update using Web interface
#ifdef WEB_UPDATE_FEATURE
void handleUpdate (AsyncWebServerRequest *request)
{
    level_authenticate_type auth_level = web_interface->is_authenticated();
    if (auth_level != LEVEL_ADMIN) {
        web_interface->_upload_status = UPLOAD_STATUS_NONE;
        request->send (403, "text/plain", "Not allowed, log in first!\n");
        return;
    }
    String jsonfile = "{\"status\":\"" ;
    jsonfile += CONFIG::intTostr (web_interface->_upload_status);
    jsonfile += "\"}";
    //send status
    AsyncResponseStream  *response = request->beginResponseStream ("application/json");
    response->addHeader ("Cache-Control", "no-cache");
    response->print (jsonfile.c_str() );
    request->send (response);
    //if success restart
    if (web_interface->_upload_status == UPLOAD_STATUS_SUCCESSFUL) {
        web_interface->restartmodule = true;
    } else {
        web_interface->_upload_status = UPLOAD_STATUS_NONE;
    }
}
void WebUpdateUpload (AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
{
    static size_t last_upload_update;
    static size_t totalSize;
    static uint32_t maxSketchSpace ;
    //only admin can update FW
    if (web_interface->is_authenticated() != LEVEL_ADMIN) {
        web_interface->_upload_status = UPLOAD_STATUS_CANCELLED;
        request->client()->abort();
        ESPCOM::println (F ("Update rejected"), PRINTER_PIPE);
        LOG ("Update failed\r\n");
        return;
    }
    //Upload start
    //**************
    if (!index) {
        ESPCOM::println (F ("Update Firmware"), PRINTER_PIPE);
        web_interface->_upload_status = UPLOAD_STATUS_ONGOING;
#ifdef ARDUINO_ARCH_ESP8266
        Update.runAsync (true);
        WiFiUDP::stopAll();
#endif
#ifdef ARDUINO_ARCH_ESP8266
        maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
#else
//Not sure can do OTA on 2Mb board
        maxSketchSpace = (ESP.getFlashChipSize() > 0x20000) ? 0x140000 : 0x140000 / 2;
#endif
        last_upload_update = 0;
        totalSize = 0;
        if (!Update.begin (maxSketchSpace) ) { //start with max available size
            web_interface->_upload_status = UPLOAD_STATUS_CANCELLED;
            ESPCOM::println (F ("Update Cancelled"), PRINTER_PIPE);
            request->client()->abort();
            return;
        } else {
            if ( ( CONFIG::GetFirmwareTarget() == REPETIER4DV) || (CONFIG::GetFirmwareTarget() == REPETIER) ) {
                ESPCOM::println (F ("Update 0%%"), PRINTER_PIPE);
            } else {
                ESPCOM::println (F ("Update 0%"), PRINTER_PIPE);
            }
        }
    }
    //Upload write
    //**************
    if (web_interface->_upload_status == UPLOAD_STATUS_ONGOING) {
        //we do not know the total file size yet but we know the available space so let's use it
        totalSize += len;
        if ( ( (100 * totalSize) / maxSketchSpace) != last_upload_update) {
            last_upload_update = (100 * totalSize) / maxSketchSpace;
            String s = "Update ";
            s+= String(last_upload_update);

            if ( ( CONFIG::GetFirmwareTarget() == REPETIER4DV) || (CONFIG::GetFirmwareTarget() == REPETIER) ) {
                s+="%%";
            } else {
                s+="%";
            }
            ESPCOM::println (s, PRINTER_PIPE);
        }
        if (Update.write (data, len) != len) {
            web_interface->_upload_status = UPLOAD_STATUS_CANCELLED;
            ESPCOM::println(F("Update Error"), PRINTER_PIPE);
            Update.end();
            request->client()->abort();
        }
    }
    //Upload end
    //**************
    if (final) {
        String  sizeargname  = filename + "S";
        if (request->hasArg (sizeargname.c_str()) ) {
            ESPCOM::println (F ("Check integrity"), PRINTER_PIPE);
             if (request->arg (sizeargname.c_str()) != String(totalSize)) {
                 web_interface->_upload_status = UPLOAD_STATUS_FAILED;
                 ESPCOM::println (F ("Update Error"), PRINTER_PIPE);
                 Update.end();
                 request->client()->abort();
             }
        }
        if (Update.end (true) ) { //true to set the size to the current progress
            //Now Reboot
            if ( ( CONFIG::GetFirmwareTarget() == REPETIER4DV) || (CONFIG::GetFirmwareTarget() == REPETIER) ) {
                ESPCOM::println (F ("Update 100%%"), PRINTER_PIPE);
            } else {
                ESPCOM::println (F ("Update 100%"), PRINTER_PIPE);
            }
            web_interface->_upload_status = UPLOAD_STATUS_SUCCESSFUL;
        }
    }
}
#endif

//Handle web command query and send answer
void handle_web_command (AsyncWebServerRequest *request)
{
     //to save time if already disconnected
     if (request->hasArg ("PAGEID") ) {
        if (request->arg ("PAGEID").length() > 0 ) {
           if (request->arg ("PAGEID").toInt() != id_connection) {
           request->send (200, "text/plain", "Invalid command");
           return;
           }
        }
    }
    level_authenticate_type auth_level = web_interface->is_authenticated();
    LOG (" Web command\r\n")
#ifdef DEBUG_ESP3D
    int nb = request->args();
    for (int i = 0 ; i < nb; i++) {
        LOG (request->argName (i) )
        LOG (":")
        LOG (request->arg (i) )
        LOG ("\r\n")
    }
#endif
    String cmd = "";
    if (request->hasArg ("plain") || request->hasArg ("commandText") ) {
        if (request->hasArg ("plain") ) {
            cmd = request->arg ("plain");
        } else {
            cmd = request->arg ("commandText");
        }
        LOG ("Web Command:")
        LOG (cmd)
        LOG ("\r\n")
    } else {
        LOG ("invalid argument\r\n")
        request->send (200, "text/plain", "Invalid command");
        return;
    }
    //if it is for ESP module [ESPXXX]<parameter>
    cmd.trim();
    int ESPpos = cmd.indexOf ("[ESP");
    if (ESPpos > -1) {
        //is there the second part?
        int ESPpos2 = cmd.indexOf ("]", ESPpos);
        if (ESPpos2 > -1) {
            //Split in command and parameters
            String cmd_part1 = cmd.substring (ESPpos + 4, ESPpos2);
            String cmd_part2 = "";
            //only [ESP800] is allowed login free if authentication is enabled
            if ( (auth_level == LEVEL_GUEST)  && (cmd_part1.toInt() != 800) ) {
                request->send (401, "text/plain", "Authentication failed!\n");
                return;
            }
            //is there space for parameters?
            if (ESPpos2 < cmd.length() ) {
                cmd_part2 = cmd.substring (ESPpos2 + 1);
            }
            //if command is a valid number then execute command
            if (cmd_part1.toInt() != 0) {
                AsyncResponseStream  *response = request->beginResponseStream ("text/html");
                response->addHeader ("Cache-Control", "no-cache");
                COMMAND::execute_command (cmd_part1.toInt(), cmd_part2, WEB_PIPE, auth_level, response);
                request->send (response);
            }
            //if not is not a valid [ESPXXX] command
        }
    } else {
        if (auth_level == LEVEL_GUEST) {
            request->send (401, "text/plain", "Authentication failed!\n");
            return;
        }
        //send command to serial as no need to transfer ESP command
        //to avoid any pollution if Uploading file to SDCard
        if ( (web_interface->blockserial) == false) {
            //block every query
            web_interface->blockserial = true;
            LOG ("Block Serial\r\n")
            //empty the serial buffer and incoming data
            LOG ("Start PurgeSerial\r\n")
            ESPCOM::processFromSerial (true);
            LOG ("End PurgeSerial\r\n")
            LOG ("Start PurgeSerial\r\n")
            ESPCOM::processFromSerial (true);
            LOG ("End PurgeSerial\r\n")
            //send command
            LOG ("Send Command\r\n")
            ESPCOM::println (cmd, DEFAULT_PRINTER_PIPE);
            CONFIG::wait (1);
            AsyncWebServerResponse *response = request->beginChunkedResponse ("text/plain", [] (uint8_t *buffer, size_t maxLen, size_t index) -> size_t {
                static bool  finish_check;
                static String current_line;
                static int multiple_counter = 0;
                static uint8_t *active_serial_buffer = NULL;
                static  size_t active_serial_buffer_size = 0;
                uint32_t timeout = millis();
                size_t count = 0;
                LOG ("Entering\n")
                //this is the start
                if (!index)
                {
                    //we are not done
                    finish_check = false;
                    //no error / multiple heat
                    multiple_counter = 0;
                    //if somehow the buffer is not empty - clean it
                    if (active_serial_buffer != NULL) {
                        delete active_serial_buffer;
                        active_serial_buffer = NULL;
                    }
                    active_serial_buffer_size = 0;
                    current_line = "";
                }
                //current_line is not empty because previous buffer was full
                if (current_line.length()  > 0 )
                {
                    //does it have a end already ?
                    if (current_line[current_line.length() - 1] == '\n') {
                        LOG (current_line.c_str() )
                        //if yes fill the buffer
                        String tmp = "";
                        for (int p = 0 ; p < current_line.length() ; p++) {
                            CONFIG::wdtFeed();
                            //Just in case line is too long
                            if (count < maxLen) {
                                buffer[count] = current_line[p];
                                count++;
                            } //too long let's copy to buffer what's left for next time
                            else {
                                tmp += current_line[p];
                            }
                        }
                        //this will be sent next time
                        current_line = tmp;
                    }
                }
                LOG (" Max Len Size ")
                LOG (String (maxLen) )
                LOG (" Index  ")
                LOG (String (index) )
                LOG (" initial count  ")
                LOG (String (count) )
                LOG (" buffer  ")
                LOG (String (active_serial_buffer_size) )
                LOG (" line  ")
                LOG (String (current_line.length() ) )
                LOG ("\n")
                int packet_size = (maxLen >= 1460) ? 1200 : maxLen;
                //now check if serial has data if have space in send buffer
                while (!finish_check && (count < maxLen) )
                {
                    CONFIG::wdtFeed();
                    size_t len = ESPCOM::available(DEFAULT_PRINTER_PIPE);
                    LOG ("Input Size ")
                    LOG (String (len) )
                    LOG ("\n")
                    if (len > 0) {
                        //prepare serial buffer
                        uint8_t * tmp_buf = new  uint8_t[active_serial_buffer_size + len];
                        //copy current buffer in new buffer
                        if ( (active_serial_buffer_size > 0) && (active_serial_buffer != NULL) ) {
                            for (int p = 0; p < active_serial_buffer_size; p++) {
                                tmp_buf[p] = active_serial_buffer[p];
                            }
                            delete active_serial_buffer;
                        }
                        //new sized buffer
                        active_serial_buffer = tmp_buf;
                        //read full buffer instead of read char by char
                        //so it give time to refill when processing
                        ESPCOM::readBytes (DEFAULT_PRINTER_PIPE, &active_serial_buffer[active_serial_buffer_size], len);
                        //new size of current buffer
                        active_serial_buffer_size += len;
                    }
                    LOG ("buffer Size ")
                    LOG (String (active_serial_buffer_size) )
                    LOG ("\n")
                    //if buffer is not empty let's process it
                    if (active_serial_buffer_size > 0) {
                        for (int p = 0; p < active_serial_buffer_size ; p++) {
                            //reset timeout
                            timeout = millis();
                            //feed WDT
                            CONFIG::wdtFeed();
                            //be sure there is some space
                            if (count < maxLen) {
                                //read next char
                                uint8_t c = active_serial_buffer[p];
                                //if it is end line
                                if ( (c == 13) || (c == 10) ) {
                                    //it is an ok or a wait so this is the end
                                    LOG ("line ")
                                    LOG (current_line.c_str() )
                                    if ( (current_line == "ok") || (current_line == "wait") ||   ( ( (CONFIG::GetFirmwareTarget() == REPETIER) || (CONFIG::GetFirmwareTarget() == REPETIER4DV) ) && ( (current_line.indexOf ("busy:") > -1) || (current_line.startsWith ( "ok ") ) ) ) ) {
                                        LOG ("ignore ")
                                        LOG (current_line.c_str() )
                                        current_line = "";
                                        //check we have something before we leave and it is not
                                        //an ack for the command
                                        if ( (index == 0) && (count == 0) ) {
                                            multiple_counter ++ ;
                                        } else {
                                            finish_check = true;
                                        }
                                        //we do not ignore it
                                    } else {
                                        if (current_line.length() > 0) {
                                            current_line += "\n";
                                            //do we add current line to buffer or wait for next callback ?
                                            if ( (count + current_line.length() ) < maxLen) {
                                                LOG ("line added\n")
                                                //have space so copy to buffer
                                                for (int pp = 0 ; pp < current_line.length() ; pp++) {
                                                    buffer[count] = current_line[pp];
                                                    count++;
                                                }
                                                //CONFIG::wait(1);
                                                timeout = millis();
                                                if (COMMAND::check_command (current_line, NO_PIPE, false, false) ) {
                                                    multiple_counter ++ ;
                                                }
                                                //reset line
                                                current_line = "";
                                                //no space return and add next time
                                            } else {
                                                //we should never reach here - but better prevent
                                                if (p < active_serial_buffer_size) {
                                                    uint8_t * tmp_buf = new  uint8_t[active_serial_buffer_size - p];
                                                    //copy old one to new one
                                                    for (int pp = 0; pp < active_serial_buffer_size - p; pp++) {
                                                        tmp_buf[pp] = active_serial_buffer[p + pp];
                                                    }
                                                    //delete old one
                                                    if (active_serial_buffer != NULL) {
                                                        delete active_serial_buffer;
                                                    }
                                                    //now new is the active one
                                                    active_serial_buffer = tmp_buf;
                                                    //reset size
                                                    active_serial_buffer_size = active_serial_buffer_size - p;
                                                }
                                                return count;
                                            }
                                        }
                                    }
                                } else {
                                    current_line += char (c);
                                    //case of long line that won't fit
                                    if (current_line.length() >= maxLen) {
                                        //we push out what we have and what we can
                                        //no need tp check if it is command as it is too long
                                        for (int pp = 0 ; pp < current_line.length() ; pp++) {
                                            CONFIG::wdtFeed();
                                            if (count < maxLen) {
                                                buffer[count] = current_line[pp];
                                                count++;
                                            } else { //put what is left for next time
                                                //remove part already sent
                                                String tmp = current_line.substring (pp);
                                                current_line = tmp;
                                                //save left buffer
                                                if (p < active_serial_buffer_size) {
                                                    uint8_t * tmp_buf = new  uint8_t[active_serial_buffer_size - p];
                                                    //copy old one to new one
                                                    for (int pp = 0; pp < active_serial_buffer_size - p; pp++) {
                                                        tmp_buf[pp] = active_serial_buffer[p + pp];
                                                    }
                                                    //delete old one
                                                    if (active_serial_buffer != NULL) {
                                                        delete active_serial_buffer;
                                                    }
                                                    //now new is the active one
                                                    active_serial_buffer = tmp_buf;
                                                    //reset size
                                                    active_serial_buffer_size = active_serial_buffer_size - p;
                                                }
                                                return count;
                                            }
                                        }
                                    }
                                }
                                //no space return and add next time
                            } else {
                                //we should never reach here - but better prevent
                                //save unprocessed buffer
                                //create a resized buffer
                                uint8_t * tmp_buf = new  uint8_t[active_serial_buffer_size - p];
                                //copy old one to new one
                                for (int pp = 0; pp < active_serial_buffer_size - p; pp++) {
                                    tmp_buf[pp] = active_serial_buffer[p + pp];
                                }
                                //delete old one
                                if (active_serial_buffer != NULL) {
                                    delete active_serial_buffer;
                                }
                                //now new is the active one
                                active_serial_buffer = tmp_buf;
                                //reset size
                                active_serial_buffer_size = active_serial_buffer_size - p;
                                return count;
                            }
                        }//end processing buffer Serial
                        active_serial_buffer_size = 0;
                        if (active_serial_buffer != NULL) {
                            delete active_serial_buffer;
                        }
                        active_serial_buffer = NULL;
                        //we chop to fill packect size not maxLen
                        if (count >= packet_size) {
                            //buffer is empty so time to clean
                            return count;
                        }
                        timeout = millis();
                    } //end of processing serial buffer
                    //we got several ok / wait /busy or temperature, so we should stop to avoid a dead loop
                    if (multiple_counter > 5) {
                        LOG ("Multiple counter reached\n\r")
                        finish_check = true;
                    }
                    //no input during 1 s so consider we are done and we missed the end flag
                    if (millis() - timeout > 1000) {
                        finish_check = true;
                        LOG ("time out\r\n")
                    }
                } // we are done for this call : buffer is full or everything is finished
                //if we are done
                if (finish_check)
                {
                    //do some cleaning if needed
                    active_serial_buffer_size = 0;
                    if (active_serial_buffer != NULL) {
                        delete active_serial_buffer;
                    }
                    active_serial_buffer = NULL;
                }
                return count;
            });
            response->addHeader ("Cache-Control", "no-cache");
            request->send (response);
            LOG ("Start PurgeSerial\r\n")
            ESPCOM::processFromSerial (true);
            LOG ("End PurgeSerial\r\n")
            web_interface->blockserial = false;
            LOG ("Release Serial\r\n")
        } else {
            request->send (409, "text/plain", "Serial is busy, retry later!");
        }
    }
}

//Handle web command query and sent ack or fail instead of answer
void handle_web_command_silent (AsyncWebServerRequest *request)
{
    //to save time if already disconnected
    if (request->hasArg ("PAGEID") ) {
        if (request->arg ("PAGEID").length() > 0 ) {
           if (request->arg ("PAGEID").toInt() != id_connection) {
           request->send (200, "text/plain", "Invalid command");
           return;
           }
        }
    }
    level_authenticate_type auth_level = web_interface->is_authenticated();
    if (auth_level == LEVEL_GUEST) {
        request->send (401, "text/plain", "Authentication failed!\n");
        return;
    }
    LOG (String (request->args() ) )
    LOG (" Web silent command\r\n")
#ifdef DEBUG_ESP3D
    int nb = request->args();
    for (int i = 0 ; i < nb; i++) {
        LOG (request->argName (i) )
        LOG (":")
        LOG (request->arg (i) )
        LOG ("\r\n")
    }
#endif
    String cmd = "";
    //int count ;
    if (request->hasArg ("plain") || request->hasArg ("commandText") ) {
        if (request->hasArg ("plain") ) {
            cmd = request->arg ("plain");
        } else {
            cmd = request->arg ("commandText");
        }
        LOG ("Web Command:")
        LOG (cmd)
        LOG ("\r\n")
    } else {
        LOG ("invalid argument\r\n")
        request->send (200, "text/plain", "Invalid command");
        return;
    }
    //if it is for ESP module [ESPXXX]<parameter>
    cmd.trim();
    int ESPpos = cmd.indexOf ("[ESP");
    if (ESPpos > -1) {
        //is there the second part?
        int ESPpos2 = cmd.indexOf ("]", ESPpos);
        if (ESPpos2 > -1) {
            //Split in command and parameters
            String cmd_part1 = cmd.substring (ESPpos + 4, ESPpos2);
            String cmd_part2 = "";
            //is there space for parameters?
            if (ESPpos2 < cmd.length() ) {
                cmd_part2 = cmd.substring (ESPpos2 + 1);
            }
            //if command is a valid number then execute command
            if (cmd_part1.toInt() != 0) {
                if (COMMAND::execute_command (cmd_part1.toInt(), cmd_part2, NO_PIPE, auth_level) ) {
                    request->send (200, "text/plain", "ok");
                } else {
                    request->send (500, "text/plain", "error");
                }

            }
            //if not is not a valid [ESPXXX] command
        }
    } else {
        //send command to serial as no need to transfer ESP command
        //to avoid any pollution if Uploading file to SDCard
        if ( (web_interface->blockserial) == false) {
            LOG ("Send Command\r\n")
            //send command
            ESPCOM::println (cmd, DEFAULT_PRINTER_PIPE);
            
            request->send (200, "text/plain", "ok");
        } else {
            request->send (200, "text/plain", "Serial is busy, retry later!");
        }
    }
}

//concat several catched informations temperatures/position/status/flow/speed
void handle_web_interface_status (AsyncWebServerRequest *request)
{
    //we do not care if need authentication - just reset counter
    web_interface->is_authenticated();
    String buffer2send;
    String value = "Ok";
    //start JSON answer
    buffer2send = "{";
#ifdef INFO_MSG_FEATURE
    //information
    buffer2send.concat (F ("\"InformationMsg\":[") );

    for (int i = 0; i < web_interface->info_msg.size(); i++) {
        if (i > 0) {
            buffer2send += ",";
        }
        buffer2send += "{\"line\":\"";
        buffer2send += web_interface->info_msg.get (i);
        buffer2send += "\"}";
    }
    buffer2send += "],";
#endif
#ifdef ERROR_MSG_FEATURE
    //Error
    buffer2send.concat (F ("\"ErrorMsg\":[") );
    for (int i = 0; i < web_interface->error_msg.size(); i++) {
        if (i > 0) {
            buffer2send += ",";
        }
        buffer2send += "{\"line\":\"";
        buffer2send += web_interface->error_msg.get (i);
        buffer2send += "\"}";
    }
    buffer2send += "],";
#endif
#ifdef STATUS_MSG_FEATURE
    //Status
    buffer2send.concat (F ("\"StatusMsg\":[") );

    for (int i = 0; i < web_interface->status_msg.size(); i++) {
        if (i > 0) {
            buffer2send += ",";
        }
        buffer2send += "{\"line\":\"";
        buffer2send += web_interface->status_msg.get (i);
        buffer2send += "\"}";
    }
    buffer2send += "],";
#endif
    //status TBD
    buffer2send += "\"status\":\"" + value + "\"";
    buffer2send += "}";
    AsyncResponseStream  *response = request->beginResponseStream ("application/json");
    response->addHeader ("Cache-Control", "no-cache");
    response->print (buffer2send.c_str() );
    request->send (response);
}

//serial SD files list
void handle_serial_SDFileList (AsyncWebServerRequest *request)
{
    //this is only for admin and user
    if (web_interface->is_authenticated() == LEVEL_GUEST) {
        web_interface->_upload_status = UPLOAD_STATUS_NONE;
        request->send (401, "application/json", "{\"status\":\"Authentication failed!\"}");
        return;
    }
    LOG ("serial SD upload done\r\n")
    String sstatus = "Ok";
    if ( (web_interface->_upload_status == UPLOAD_STATUS_FAILED) || (web_interface->_upload_status == UPLOAD_STATUS_CANCELLED) ) {
        sstatus = "Upload failed";
        web_interface->_upload_status = UPLOAD_STATUS_NONE;
    }
    String jsonfile = "{\"status\":\"" + sstatus + "\"}";
    AsyncResponseStream  *response = request->beginResponseStream ("application/json");
    response->addHeader ("Cache-Control", "no-cache");
    response->print (jsonfile.c_str() );
    request->send (response);
    web_interface->blockserial = false;
    web_interface->_upload_status = UPLOAD_STATUS_NONE;
}

//send line to Serial
bool sendLine2Serial (String &  line)
{
    LOG (line)
    ESPCOM::println (line, DEFAULT_PRINTER_PIPE);
    ESPCOM::flush(DEFAULT_PRINTER_PIPE);
#ifdef ARDUINO_ARCH_ESP8266
    CONFIG::wait (5);
#else
    CONFIG::wait (2);
#endif
    if (ESPCOM::available(DEFAULT_PRINTER_PIPE) > 0 ) {
        bool done = false;
        uint32_t timeout = millis();
        uint8_t count = 0;
        while (!done) {
            CONFIG::wdtFeed();
            size_t len = ESPCOM::available(DEFAULT_PRINTER_PIPE);
            //get size of buffer
            if (len > 0) {
                uint8_t sbuf[len + 1];
                //read buffer
                ESPCOM::readBytes (DEFAULT_PRINTER_PIPE, sbuf, len);
                //convert buffer to zero end array
                sbuf[len] = '\0';
                //use string because easier to handle
                String response = (const char*) sbuf;
                if ( (response.indexOf ("ok") > -1)  || (response.indexOf ("wait") > -1) ) {
                    return true;
                }
                if (response.indexOf ("Resend") > -1) {
                    count++;
                    if (count > 5) {
                        return false;
                    }
                    LOG ("resend\r\n")
                    ESPCOM::println (line, DEFAULT_PRINTER_PIPE);
                    ESPCOM::flush (DEFAULT_PRINTER_PIPE);
                    CONFIG::wait (5);
                    timeout = millis();
                }
            }
            //no answer so exit: no news = good news
            if ( millis() - timeout > 500) {
                done = true;
            }
        }
    }
    return true;
}

//send M29 / M30 command to close file on SD
void CloseSerialUpload (bool iserror, String & filename)
{

    ESPCOM::println ("\r\nM29", DEFAULT_PRINTER_PIPE);
    ESPCOM::flush (DEFAULT_PRINTER_PIPE);
    CONFIG::wait (1000);
    ESPCOM::println ("\r\nM29", DEFAULT_PRINTER_PIPE);
    ESPCOM::flush (DEFAULT_PRINTER_PIPE);
    if (iserror) {
        String cmdfilename = "M30 " + filename;
        ESPCOM::println (cmdfilename, DEFAULT_PRINTER_PIPE);
        ESPCOM::println (F ("SD upload failed"), PRINTER_PIPE);
        ESPCOM::flush (DEFAULT_PRINTER_PIPE);
        web_interface->_upload_status = UPLOAD_STATUS_FAILED;
    }  else {
        ESPCOM::println (F ("SD upload done"), PRINTER_PIPE);
        ESPCOM::flush (DEFAULT_PRINTER_PIPE);
        web_interface->_upload_status = UPLOAD_STATUS_SUCCESSFUL;
    }
    //lets give time to FW to proceed
    CONFIG::wait (1000);
    web_interface->blockserial = false;
}

//SD file upload by serial
#define NB_RETRY 5
#define MAX_RESEND_BUFFER 128
#define SERIAL_CHECK_TIMEOUT 2000
void SDFile_serial_upload (AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
{
    LOG ("Uploading: ")
    LOG (filename)
    LOG ("\n")
    static String current_line;
    static bool is_comment = false;
    static String current_filename;
    String response;
    //Guest cannot upload - only admin and user
    if (web_interface->is_authenticated() == LEVEL_GUEST) {
        web_interface->_upload_status = UPLOAD_STATUS_CANCELLED;
        ESPCOM::println (F ("SD upload rejected"), PRINTER_PIPE);
        LOG ("SD upload rejected\r\n");
        request->client()->abort();
        return;
    }
#ifdef DEBUG_PERFORMANCE
    static uint32_t startupload;
    static uint32_t write_time;
    static size_t filesize;
#endif
    //Upload start
    //**************
    if (!index) {
        LOG ("Starting\n")
        //need to lock serial out to avoid garbage in file
        (web_interface->blockserial) = true;
        current_line = "";
        //init flags
        is_comment = false;
        current_filename = filename;
        web_interface->_upload_status = UPLOAD_STATUS_ONGOING;
        ESPCOM::println (F ("Uploading..."), PRINTER_PIPE);
        ESPCOM::flush (DEFAULT_PRINTER_PIPE);
#ifdef DEBUG_PERFORMANCE
        startupload = millis();
        write_time = 0;
        filesize = 0;
#endif
        LOG ("Clear Serial\r\n");
        //get size of buffer
        size_t len = ESPCOM::available(DEFAULT_PRINTER_PIPE);
        if (len > 0) {
            uint8_t sbuf[len + 1];
            //read buffer
            ESPCOM::readBytes (DEFAULT_PRINTER_PIPE, sbuf, len);
            //convert buffer to zero end array
            sbuf[len] = '\0';
            //use string because easier to handle
            response = (const char*) sbuf;
            LOG (response);
            LOG ("\r\n");
        }
        //command to pritnter to start print
        String command = "M28 " + filename;
        LOG (command);
        LOG ("\r\n");
        ESPCOM::println (command, DEFAULT_PRINTER_PIPE);
        ESPCOM::flush (DEFAULT_PRINTER_PIPE);
        CONFIG::wait (500);
        uint32_t timeout = millis();
        bool done = false;
        while (!done) { //time out is  2000ms
            CONFIG::wdtFeed();
            //if there is something in serial buffer
            size_t len = ESPCOM::available(DEFAULT_PRINTER_PIPE);
            //get size of buffer
            if (len > 0) {
                CONFIG::wdtFeed();
                uint8_t sbuf[len + 1];
                //read buffer
                ESPCOM::readBytes (DEFAULT_PRINTER_PIPE, sbuf, len);
                //convert buffer to zero end array
                sbuf[len] = '\0';
                //use string because easier to handle
                response = (const char*) sbuf;
                LOG (response);
                //if there is a wait it means purge is done
                if (response.indexOf ("wait") > -1) {
                    LOG ("Exit start writing\r\n");
                    done = true;
                    break;
                }
                //it is first command if it is failed no need to continue
                //and resend command won't help
                if (response.indexOf ("Resend") > -1 || response.indexOf ("failed") > -1) {
                    web_interface->blockserial = false;
                    LOG ("Error start writing\r\n");
                    web_interface->_upload_status = UPLOAD_STATUS_FAILED;
                    request->client()->abort();
                    return;
                }
            }
            if ( (millis() - timeout) > SERIAL_CHECK_TIMEOUT) {
                done = true;
            }
        }
    }
    //Upload write
    //**************
    if ( ( web_interface->_upload_status = UPLOAD_STATUS_ONGOING) && len) {
        LOG ("Writing to serial\n")
#ifdef DEBUG_PERFORMANCE
        filesize += upload.currentSize;
        uint32_t startwrite = millis();
#endif
        for (int pos = 0; pos < len; pos++) { //parse full post data
            //feed watchdog
            CONFIG::wdtFeed();
            //it is a comment
            if (data[pos] == ';') {
                LOG ("Comment\n")
                is_comment = true;
            }
            //it is an end line
            else  if ( (data[pos] == 13) || (data[pos] == 10) ) {
                //does line fit the buffer ?
                if (current_line.length() < 126) {
                    //do we have something in buffer ?
                    if (current_line.length() > 0 ) {
                        current_line += "\r\n";
                        if (!sendLine2Serial (current_line) ) {
                            LOG ("Error over buffer\n")
                            CloseSerialUpload (true, current_filename);
                            request->client()->abort();
                            return;
                        }
                        //reset line
                        current_line = "";
                        //if comment line then reset
                        is_comment = false;
                    } else {
                        LOG ("Empy line\n")
                    }
                } else {
                    //error buffer overload
                    LOG ("Error over buffer\n")
                    CloseSerialUpload (true, current_filename);
                    request->client()->abort();
                    return;
                }
            } else if (!is_comment) {
                if (current_line.length() < 126) {
                    current_line += char (data[pos]);  //copy current char to buffer to send/resend
                } else {
                    LOG ("Error over buffer\n")
                    CloseSerialUpload (true, current_filename);
                    request->client()->abort();
                    return;
                }
            }
        }
        LOG ("Parsing Done\n")
#ifdef DEBUG_PERFORMANCE
        write_time += (millis() - startwrite);
#endif
    } else {
        LOG ("Nothing to write\n")
    }

    //Upload end
    //**************
    if (final) {
        LOG ("Final is reached\n")
        //if last part does not have '\n'
        if (current_line.length()  > 0) {
            current_line += "\r\n";
            if (!sendLine2Serial (current_line) ) {
                LOG ("Error sending buffer\n")
                CloseSerialUpload (true, current_filename);
                request->client()->abort();
                return;
            }
        }
        LOG ("Upload finished ");
        CloseSerialUpload (false, current_filename);
#ifdef DEBUG_PERFORMANCE
        uint32_t endupload = millis();
        DEBUG_PERF_VARIABLE.add (String (endupload - startupload).c_str() );
        DEBUG_PERF_VARIABLE.add (String (write_time).c_str() );
        DEBUG_PERF_VARIABLE.add (String (filesize).c_str() );
#endif
    }
    LOG ("Exit fn\n")
}

//on event connect function
void handle_onevent_connect(AsyncEventSourceClient *client) 
{       
        if (!client->lastId()){
            //Init active ID
            id_connection++;
            client->send(String(id_connection).c_str(), "InitID", id_connection, 1000);
            //Dispatch who is active ID
            web_interface->web_events.send( String(id_connection).c_str(),"ActiveID");        
            }
}

void handle_Websocket_Event(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
  //Handle WebSocket event
}

//constructor
WEBINTERFACE_CLASS::WEBINTERFACE_CLASS (int port) : web_server (port) 
													, web_events("/events")
#ifdef WS_DATA_FEATURE
													, web_socket("/ws")
#endif
{
    //that handle "/" and default index.html.gz
    //trick to catch command line on "/" before file being processed
    web_server.serveStatic ("/", SPIFFS, "/").setDefaultFile ("index.html").setFilter (filterOnRoot);
    web_server.serveStatic ("/", SPIFFS, "/Nowhere");
    //SPIFFS
    web_server.on ("/files", HTTP_ANY, handleFileList, SPIFFSFileupload);
    web_server.on ("/command", HTTP_ANY, handle_web_command);
    web_server.on ("/command_silent", HTTP_ANY, handle_web_command_silent);
    web_server.on ("/upload_serial", HTTP_ANY, handle_serial_SDFileList, SDFile_serial_upload);
#ifdef WEB_UPDATE_FEATURE
    web_server.on ("/updatefw", HTTP_ANY, handleUpdate, WebUpdateUpload);
#endif
#ifdef AUTHENTICATION_FEATURE
//    web_server.on("/login", HTTP_ANY, handle_login);
#endif
    web_server.on ("/status", HTTP_ANY, handle_web_interface_status);
#ifdef SSDP_FEATURE
    web_server.on ("/description.xml", HTTP_GET, handle_SSDP);
#endif
#ifdef CAPTIVE_PORTAL_FEATURE
    web_server.on ("/generate_204", HTTP_ANY,  [] (AsyncWebServerRequest * request) {
        request->redirect ("/");
    });
    web_server.on ("/gconnectivitycheck.gstatic.com", HTTP_ANY,  [] (AsyncWebServerRequest * request) {
        request->redirect ("/");
    });
    //do not forget the / at the end
    web_server.on ("/fwlink/", HTTP_ANY,  [] (AsyncWebServerRequest * request) {
        request->redirect ("/");
    });
#endif
    //events functions
    web_events.onConnect(handle_onevent_connect);
    //events management
    web_server.addHandler(&web_events);
#ifdef WS_DATA_FEATURE
    //Websocket function
    web_socket.onEvent(handle_Websocket_Event);
    //Websocket management
    web_server.addHandler(&web_socket);
#endif
    //page not found handler
    web_server.onNotFound ( handle_not_found);
    blockserial = false;
    restartmodule = false;
    //rolling list of 4entries with a maximum of 50 char for each entry
#ifdef ERROR_MSG_FEATURE
    error_msg.setsize (4);
    error_msg.setlength (50);
#endif
#ifdef INFO_MSG_FEATURE
    info_msg.setsize (4);
    info_msg.setlength (50);
#endif
#ifdef STATUS_MSG_FEATURE
    status_msg.setsize (4);
    status_msg.setlength (50);
#endif
    fsUploadFile = (FS_FILE) 0;
    _head = NULL;
    _nb_ip = 0;
    _upload_status = UPLOAD_STATUS_NONE;
}
//Destructor
WEBINTERFACE_CLASS::~WEBINTERFACE_CLASS()
{
#ifdef INFO_MSG_FEATURE
    info_msg.clear();
#endif
#ifdef ERROR_MSG_FEATURE
    error_msg.clear();
#endif
#ifdef STATUS_MSG_FEATURE
    status_msg.clear();
#endif
    while (_head) {
        auth_ip * current = _head;
        _head = _head->_next;
        delete current;
    }
    _nb_ip = 0;
}
//check authentification
level_authenticate_type  WEBINTERFACE_CLASS::is_authenticated()
{
#ifdef AUTHENTICATION_FEATURE
    if (web_server.hasHeader ("Cookie") ) {
        String cookie = web_server.header ("Cookie");
        int pos = cookie.indexOf ("ESPSESSIONID=");
        if (pos != -1) {
            int pos2 = cookie.indexOf (";", pos);
            String sessionID = cookie.substring (pos + strlen ("ESPSESSIONID="), pos2);
            IPAddress ip = web_server.client().remoteIP();
            //check if cookie can be reset and clean table in same time
            return ResetAuthIP (ip, sessionID.c_str() );
        }
    }
    return LEVEL_GUEST;
#else
    return LEVEL_ADMIN;
#endif
}

#ifdef AUTHENTICATION_FEATURE
//add the information in the linked list if possible
bool WEBINTERFACE_CLASS::AddAuthIP (auth_ip * item)
{
    if (_nb_ip > MAX_AUTH_IP) {
        return false;
    }
    item->_next = _head;
    _head = item;
    _nb_ip++;
    return true;
}

//Session ID based on IP and time using 16 char
char * WEBINTERFACE_CLASS::create_session_ID()
{
    static char  sessionID[17];
//reset SESSIONID
    for (int i = 0; i < 17; i++) {
        sessionID[i] = '\0';
    }
//get time
    uint32_t now = millis();
//get remote IP
    IPAddress remoteIP = web_server.client().remoteIP();
//generate SESSIONID
    if (0 > sprintf (sessionID, "%02X%02X%02X%02X%02X%02X%02X%02X", remoteIP[0], remoteIP[1], remoteIP[2], remoteIP[3], (uint8_t) ( (now >> 0) & 0xff), (uint8_t) ( (now >> 8) & 0xff), (uint8_t) ( (now >> 16) & 0xff), (uint8_t) ( (now >> 24) & 0xff) ) ) {
        strcpy (sessionID, "NONE");
    }
    return sessionID;
}



bool WEBINTERFACE_CLASS::ClearAuthIP (IPAddress ip, const char * sessionID)
{
    auth_ip * current = _head;
    auth_ip * previous = NULL;
    bool done = false;
    while (current) {
        if ( (ip == current->ip) && (strcmp (sessionID, current->sessionID) == 0) ) {
            //remove
            done = true;
            if (current == _head) {
                _head = current->_next;
                _nb_ip--;
                delete current;
                current = _head;
            } else {
                previous->_next = current->_next;
                _nb_ip--;
                delete current;
                current = previous->_next;
            }
        } else {
            previous = current;
            current = current->_next;
        }
    }
    return done;
}

//Get info
auth_ip * WEBINTERFACE_CLASS::GetAuth (IPAddress ip, const char * sessionID)
{
    auth_ip * current = _head;
    auth_ip * previous = NULL;
    //get time
    //uint32_t now = millis();
    while (current) {
        if (ip == current->ip) {
            if (strcmp (sessionID, current->sessionID) == 0) {
                //found
                return current;
            }
        }
        previous = current;
        current = current->_next;
    }
    return NULL;
}

//Review all IP to reset timers
level_authenticate_type WEBINTERFACE_CLASS::ResetAuthIP (IPAddress ip, const char * sessionID)
{
    auth_ip * current = _head;
    auth_ip * previous = NULL;
    //get time
    //uint32_t now = millis();
    while (current) {
        if ( (millis() - current->last_time) > 180000) {
            //remove
            if (current == _head) {
                _head = current->_next;
                _nb_ip--;
                delete current;
                current = _head;
            } else {
                previous->_next = current->_next;
                _nb_ip--;
                delete current;
                current = previous->_next;
            }
        } else {
            if (ip == current->ip) {
                if (strcmp (sessionID, current->sessionID) == 0) {
                    //reset time
                    current->last_time = millis();
                    return (level_authenticate_type) current->level;
                }
            }
            previous = current;
            current = current->_next;
        }
    }
    return LEVEL_GUEST;
}
#endif

//Check what is the content tye according extension file
String WEBINTERFACE_CLASS::getContentType (String filename)
{
    if (filename.endsWith (".htm") ) {
        return "text/html";
    } else if (filename.endsWith (".html") ) {
        return "text/html";
    } else if (filename.endsWith (".css") ) {
        return "text/css";
    } else if (filename.endsWith (".js") ) {
        return "application/javascript";
    } else if (filename.endsWith (".png") ) {
        return "image/png";
    } else if (filename.endsWith (".gif") ) {
        return "image/gif";
    } else if (filename.endsWith (".jpeg") ) {
        return "image/jpeg";
    } else if (filename.endsWith (".jpg") ) {
        return "image/jpeg";
    } else if (filename.endsWith (".ico") ) {
        return "image/x-icon";
    } else if (filename.endsWith (".xml") ) {
        return "text/xml";
    } else if (filename.endsWith (".pdf") ) {
        return "application/x-pdf";
    } else if (filename.endsWith (".zip") ) {
        return "application/x-zip";
    } else if (filename.endsWith (".gz") ) {
        return "application/x-gzip";
    } else if (filename.endsWith (".txt") ) {
        return "text/plain";
    }
    return "application/octet-stream";
}


WEBINTERFACE_CLASS * web_interface;
