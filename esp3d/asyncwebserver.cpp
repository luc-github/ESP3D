/*
  asyncwebserver.cpp - ESP3D sync functions class

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
#if defined(ASYNCWEBSERVER)

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
bool can_process_serial = true;

extern bool  deleteRecursive(String path);
extern bool sendLine2Serial (String &  line, int32_t linenb,  int32_t * newlinenb);
extern void CloseSerialUpload (bool iserror, String & filename, int32_t linenb);
extern bool purge_serial();
extern long id_connection;

const uint8_t PAGE_404 [] PROGMEM = "<HTML>\n<HEAD>\n<title>Redirecting...</title> \n</HEAD>\n<BODY>\n<CENTER>Unknown page : $QUERY$- you will be redirected...\n<BR><BR>\nif not redirected, <a href='http://$WEB_ADDRESS$'>click here</a>\n<BR><BR>\n<PROGRESS name='prg' id='prg'></PROGRESS>\n\n<script>\nvar i = 0; \nvar x = document.getElementById(\"prg\"); \nx.max=5; \nvar interval=setInterval(function(){\ni=i+1; \nvar x = document.getElementById(\"prg\"); \nx.value=i; \nif (i>5) \n{\nclearInterval(interval);\nwindow.location.href='/';\n}\n},1000);\n</script>\n</CENTER>\n</BODY>\n</HTML>\n\n";
const uint8_t PAGE_CAPTIVE [] PROGMEM = "<HTML>\n<HEAD>\n<title>Captive Portal</title> \n</HEAD>\n<BODY>\n<CENTER>Captive Portal page : $QUERY$- you will be redirected...\n<BR><BR>\nif not redirected, <a href='http://$WEB_ADDRESS$'>click here</a>\n<BR><BR>\n<PROGRESS name='prg' id='prg'></PROGRESS>\n\n<script>\nvar i = 0; \nvar x = document.getElementById(\"prg\"); \nx.max=5; \nvar interval=setInterval(function(){\ni=i+1; \nvar x = document.getElementById(\"prg\"); \nx.value=i; \nif (i>5) \n{\nclearInterval(interval);\nwindow.location.href='/';\n}\n},1000);\n</script>\n</CENTER>\n</BODY>\n</HTML>\n\n";
#define  CONTENT_TYPE_HTML "text/html"

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

void handle_login(AsyncWebServerRequest *request)
{
#ifdef AUTHENTICATION_FEATURE
#else
    AsyncWebServerResponse * response = request->beginResponse (200, "application/json", "{\"status\":\"Ok\",\"authentication_lvl\":\"admin\"}");
    response->addHeader("Cache-Control","no-cache");
    request->send(response);
#endif
}


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
    if ( (web_interface->_upload_status == UPLOAD_STATUS_FAILED) || (web_interface->_upload_status == UPLOAD_STATUS_FAILED) ) {
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
    //get authentication status
    level_authenticate_type auth_level= web_interface->is_authenticated();
    //Guest cannot upload - only admin
    if (auth_level == LEVEL_GUEST) {
        web_interface->_upload_status = UPLOAD_STATUS_FAILED;
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
    if(auth_level != LEVEL_ADMIN) {
        String filename = upload_filename;
        upload_filename = "/user" + filename;
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
        web_interface->_upload_status = UPLOAD_STATUS_FAILED;
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
            web_interface->_upload_status = UPLOAD_STATUS_FAILED;
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
            web_interface->_upload_status = UPLOAD_STATUS_FAILED;
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
            //Update is done
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

//Not found Page handler //////////////////////////////////////////////////////////
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

//Handle web command query and send answer//////////////////////////////
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
            can_process_serial = false;
            request->onDisconnect([request]() {
                can_process_serial = true;
            });
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

//Handle web command query and sent ack or fail instead of answer///////
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

//serial SD files list//////////////////////////////////////////////////
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
    if ( (web_interface->_upload_status == UPLOAD_STATUS_FAILED) || (web_interface->_upload_status == UPLOAD_STATUS_FAILED) ) {
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

//SD file upload by serial
#define NB_RETRY 5
#define MAX_RESEND_BUFFER 128
#define SERIAL_CHECK_TIMEOUT 2000
void SDFile_serial_upload (AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
{
    LOG ("Uploading: ")
    LOG (filename)
    LOG ("\n")
    static int32_t lineNb =-1;
    static String current_line;
    static bool is_comment = false;
    static String current_filename;
    String response;
    //Guest cannot upload - only admin and user
    if (web_interface->is_authenticated() == LEVEL_GUEST) {
        web_interface->_upload_status = UPLOAD_STATUS_FAILED;
        ESPCOM::println (F ("SD upload rejected"), PRINTER_PIPE);
        LOG ("SD upload rejected\r\n");
        request->client()->abort();
        return;
    }
    //Upload start
    //**************
    if (!index) {
        LOG("Upload Start\r\n")
        String command = "M29";
        String resetcmd = "M110 N0";
        if (CONFIG::GetFirmwareTarget() == SMOOTHIEWARE) {
            resetcmd = "N0 M110";
        }
        lineNb=1;
        //close any ongoing upload and get current line number
        if(!sendLine2Serial (command,1, &lineNb)) {
            //it can failed for repetier
            if ( ( CONFIG::GetFirmwareTarget() == REPETIER4DV) || (CONFIG::GetFirmwareTarget() == REPETIER) ) {
                if(!sendLine2Serial (command,-1, NULL)) {
                    LOG("Start Upload failed")
                    web_interface->_upload_status= UPLOAD_STATUS_FAILED;
                    return;
                }
            } else {
                LOG("Start Upload failed")
                web_interface->_upload_status= UPLOAD_STATUS_FAILED;
                return;
            }
        }
        //Mount SD card
        command = "M21";
        if(!sendLine2Serial (command,-1, NULL)) {
            LOG("Mounting SD failed")
            web_interface->_upload_status= UPLOAD_STATUS_FAILED;
            return;
        }
        //Reset line numbering
        if(!sendLine2Serial (resetcmd,-1, NULL)) {
            LOG("Reset Numbering failed")
            web_interface->_upload_status= UPLOAD_STATUS_FAILED;
            return;
        }
        lineNb=1;
        //need to lock serial out to avoid garbage in file
        (web_interface->blockserial) = true;
        current_line ="";
        current_filename = filename;
        is_comment = false;
        String response;
        ESPCOM::println (F ("Uploading..."), PRINTER_PIPE);
        //Clear all serial
        ESPCOM::flush (DEFAULT_PRINTER_PIPE);
        purge_serial();
        //besure nothing left again
        purge_serial();
        command = "M28 " + current_filename;
        //send start upload
        //no correction allowed because it means reset numbering was failed
        if (sendLine2Serial(command, lineNb, NULL)) {
            CONFIG::wait(1200);
            //additional purge, in case it is slow to answer
            purge_serial();
            web_interface->_upload_status= UPLOAD_STATUS_ONGOING;
            LOG("Creation Ok\r\n")

        } else  {
            web_interface->_upload_status= UPLOAD_STATUS_FAILED;
            LOG("Creation failed\r\n");
        }
    }
    //Upload write
    //**************
    if ( ( web_interface->_upload_status = UPLOAD_STATUS_ONGOING) && len) {
        LOG ("Writing to serial\n")
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
                is_comment = false;
                //does line fit the buffer ?
                if (current_line.length() < 126) {
                    //do we have something in buffer ?
                    if (current_line.length() > 0 ) {
                        lineNb++;
                        if (!sendLine2Serial (current_line, lineNb, NULL) ) {
                            LOG ("Error sending line\n")
                            CloseSerialUpload (true, current_filename,lineNb);
                            request->client()->abort();
                            return;
                        }
                        //reset line
                        current_line = "";
                        //if comment line then reset
                    } else {
                        LOG ("Empy line\n")
                    }
                } else {
                    //error buffer overload
                    LOG ("Error over buffer\n")
                    lineNb++;
                    CloseSerialUpload (true, current_filename, lineNb);
                    request->client()->abort();
                    return;
                }
            } else if (!is_comment) {
                if (current_line.length() < 126) {
                    current_line += char (data[pos]);  //copy current char to buffer to send/resend
                } else {
                    LOG ("Error over buffer\n")
                    lineNb++;
                    CloseSerialUpload (true, current_filename, lineNb);
                    request->client()->abort();
                    return;
                }
            }
        }
        LOG ("Parsing Done\n")
    } else {
        LOG ("Nothing to write\n")
    }

    //Upload end
    //**************
    if (final) {
        LOG ("Final is reached\n")
        //if last part does not have '\n'
        if (current_line.length()  > 0) {
            lineNb++;
            if (!sendLine2Serial (current_line, lineNb, NULL) ) {
                LOG ("Error sending buffer\n")
                lineNb++;
                CloseSerialUpload (true, current_filename, lineNb);
                request->client()->abort();
                return;
            }
        }
        LOG ("Upload finished ");
        lineNb++;
        CloseSerialUpload (false, current_filename, lineNb);
    }
    LOG ("Exit fn\n")
}


//on event connect function
void handle_onevent_connect(AsyncEventSourceClient *client)
{
    if (!client->lastId()) {
        //Init active ID
        id_connection++;
        client->send(String(id_connection).c_str(), "InitID", id_connection, 1000);
        //Dispatch who is active ID
        web_interface->web_events.send( String(id_connection).c_str(),"ActiveID");
    }
}

void handle_Websocket_Event(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len)
{
    //Handle WebSocket event
}

#endif
