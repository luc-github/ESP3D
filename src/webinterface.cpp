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
#if defined (ASYNCWEBSERVER)
#include <ESPAsyncWebServer.h>
#endif
#ifdef ARDUINO_ARCH_ESP8266
#include "ESP8266WiFi.h"
#if defined (ASYNCWEBSERVER)
#include <ESPAsyncTCP.h>
#else
#include <ESP8266WebServer.h>
#endif
#else //ESP32
#include <WiFi.h>
#if defined (ASYNCWEBSERVER)
#include <AsyncTCP.h>
#else
#include <WebServer.h>
#endif
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

#if defined(ASYNCWEBSERVER)
#include "asyncwebserver.h"
#else
#include "syncwebserver.h"
#endif

#define MAX_AUTH_IP 10

long id_connection = 0;



//function to send line to serial///////////////////////////////////////
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
                uint8_t * sbuf = (uint8_t *)malloc(len+1);
					if(!sbuf){
					return false;
					}
                //read buffer
                ESPCOM::readBytes (DEFAULT_PRINTER_PIPE, sbuf, len);
                //convert buffer to zero end array
                sbuf[len] = '\0';
                //use string because easier to handle
                String response = (const char*) sbuf;
                if ( (response.indexOf ("ok") > -1)  || (response.indexOf ("wait") > -1) ) {
					free(sbuf);
                    return true;
                }
                if (response.indexOf ("Resend") > -1) {
                    count++;
                    if (count > 5) {
						free(sbuf);
                        return false;
                    }
                    LOG ("resend\r\n")
                    ESPCOM::println (line, DEFAULT_PRINTER_PIPE);
                    ESPCOM::flush (DEFAULT_PRINTER_PIPE);
                    CONFIG::wait (5);
                    timeout = millis();
                }
                free(sbuf);
            }
            //no answer so exit: no news = good news
            if ( millis() - timeout > 500) {
                done = true;
            }
        }
    }
    return true;
}

//send M29 / M30 command to close file on SD////////////////////////////

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


//constructor
WEBINTERFACE_CLASS::WEBINTERFACE_CLASS (int port) : web_server (port) 
#if defined(ASYNCWEBSERVER)
													, web_events("/events")
#ifdef WS_DATA_FEATURE
													, web_socket("/ws")
#endif
#endif
{
    //that handle "/" and default index.html.gz
#if defined(ASYNCWEBSERVER)
	//trick to catch command line on "/" before file being processed
    web_server.serveStatic ("/", SPIFFS, "/").setDefaultFile ("index.html").setFilter (filterOnRoot);
    web_server.serveStatic ("/", SPIFFS, "/Nowhere");
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
#else 
	web_server.on("/",HTTP_ANY, handle_web_interface_root);
#endif
	//need to be there even no authentication to say to UI no authentication
	web_server.on("/login", HTTP_ANY, handle_login);
#ifdef SSDP_FEATURE
    web_server.on ("/description.xml", HTTP_GET, handle_SSDP);
#endif
#ifdef CAPTIVE_PORTAL_FEATURE
	#if defined(ASYNCWEBSERVER)
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
    #else
    web_server.on("/generate_204",HTTP_ANY, handle_web_interface_root);
    web_server.on("/gconnectivitycheck.gstatic.com",HTTP_ANY, handle_web_interface_root);
    //do not forget the / at the end 
    web_server.on("/fwlink/",HTTP_ANY, handle_web_interface_root);
    #endif
#endif
    //SPIFFS
    web_server.on ("/files", HTTP_ANY, handleFileList, SPIFFSFileupload);
#ifdef WEB_UPDATE_FEATURE
    web_server.on ("/updatefw", HTTP_ANY, handleUpdate, WebUpdateUpload);
#endif
    //Page not found handler
    web_server.onNotFound ( handle_not_found);
    //web commands
    web_server.on ("/command", HTTP_ANY, handle_web_command);
	web_server.on ("/command_silent", HTTP_ANY, handle_web_command_silent);
	//Serial SD management
	web_server.on ("/upload_serial", HTTP_ANY, handle_serial_SDFileList, SDFile_serial_upload);

    blockserial = false;
    restartmodule = false;
    _head = NULL;
    _nb_ip = 0;
    _upload_status = UPLOAD_STATUS_NONE;
}
//Destructor
WEBINTERFACE_CLASS::~WEBINTERFACE_CLASS()
{
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
