/*
  espcom.cpp - esp3d communication serial/tcp/etc.. class

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
#include "espcom.h"
#include "command.h"
#include "webinterface.h"
#if defined (ASYNCWEBSERVER)
#include "asyncwebserver.h"
#else
#include "syncwebserver.h"
#endif

#ifdef ESP_OLED_FEATURE
#include "esp_oled.h"
bool  ESPCOM::block_2_oled = false;
#endif

uint8_t  ESPCOM::current_socket_id=0;

#ifdef TCP_IP_DATA_FEATURE
WiFiServer * data_server;
WiFiClient serverClients[MAX_SRV_CLIENTS];
#endif

bool ESPCOM::block_2_printer = false;

void ESPCOM::bridge(bool async)
{
#if defined (ASYNCWEBSERVER)
    if(can_process_serial) {
#endif
//be sure wifi is on to proceed wifi function
        if ((WiFi.getMode() != WIFI_OFF) || wifi_config.WiFi_on) {
//read tcp port input
#ifdef TCP_IP_DATA_FEATURE
            ESPCOM::processFromTCP2Serial();
#endif
        }
//read serial input
        ESPCOM::processFromSerial();
#if defined (ASYNCWEBSERVER)
    }
#endif
}

long ESPCOM::readBytes (tpipe output, uint8_t * sbuf, size_t len)
{
    switch (output) {
#ifdef USE_SERIAL_0
    case SERIAL_PIPE:{
        long l = Serial.readBytes(sbuf,len);
#ifdef DEBUG_OUTPUT_SOCKET
        if(socket_server){
            socket_server->sendBIN(ESPCOM::current_socket_id,sbuf,l);
            }
#endif
        return l;
    }
        break;
#endif
#ifdef USE_SERIAL_1
    case SERIAL_PIPE:
        return Serial1.readBytes(sbuf,len);
        break;
#endif
#ifdef USE_SERIAL_2
    case SERIAL_PIPE:
        return Serial2.readBytes(sbuf,len);
        break;
#endif
    default:
        return 0;
        break;
    }
}
long ESPCOM::baudRate(tpipe output)
{
    long br = 0;
    switch (output) {
#ifdef USE_SERIAL_0
    case SERIAL_PIPE:
        br = Serial.baudRate();
        break;
#endif
#ifdef USE_SERIAL_1
    case SERIAL_PIPE:
        br = Serial1.baudRate();
        break;
#endif
#ifdef USE_SERIAL_2
    case SERIAL_PIPE:
        br = Serial2.baudRate();
        break;
#endif
    default:
        return 0;
        break;
    }
#ifdef ARDUINO_ARCH_ESP32
    //workaround for ESP32
    if (br == 115201) {
        br = 115200;
    }
    if (br == 230423) {
        br = 230400;
    }
#endif
    return br;
}
size_t ESPCOM::available(tpipe output)
{
    switch (output) {
#ifdef USE_SERIAL_0
    case SERIAL_PIPE:
        return Serial.available();
        break;
#endif
#ifdef USE_SERIAL_1
    case SERIAL_PIPE:
        return Serial1.available();
        break;
#endif
#ifdef USE_SERIAL_2
    case SERIAL_PIPE:
        return Serial2.available();
        break;
#endif
    default:
        return 0;
        break;
    }
}
size_t   ESPCOM::write(tpipe output, uint8_t d)
{
    if ((DEFAULT_PRINTER_PIPE == output) && (block_2_printer || CONFIG::is_locked(FLAG_BLOCK_SERIAL))) {
        return 0;
    }
    if ((SERIAL_PIPE == output) && CONFIG::is_locked(FLAG_BLOCK_SERIAL)) {
        return 0;
    }
    switch (output) {
#ifdef USE_SERIAL_0
    case SERIAL_PIPE:
        return Serial.write(d);
        break;
#endif
#ifdef USE_SERIAL_1
    case SERIAL_PIPE:
        return Serial1.write(d);
        break;
#endif
#ifdef USE_SERIAL_2
    case SERIAL_PIPE:
        return Serial2.write(d);
        break;
#endif
    default:
        return 0;
        break;
    }
}
void ESPCOM::flush (tpipe output, ESPResponseStream  *espresponse)
{
    switch (output) {
#ifdef USE_SERIAL_0
    case SERIAL_PIPE:
        Serial.flush();
        break;
#endif
#ifdef USE_SERIAL_1
    case SERIAL_PIPE:
        Serial1.flush();
        break;
#endif
#ifdef USE_SERIAL_2
    case SERIAL_PIPE:
        Serial2.flush();
        break;
#endif
#if !defined (ASYNCWEBSERVER)
    case WEB_PIPE:
        if (espresponse) {
            if(espresponse->header_sent) {
                //send data
                web_interface->web_server.sendContent(espresponse->buffer_web);
                //close line
                web_interface->web_server.sendContent("");
            }
            espresponse->header_sent = false;
            espresponse->buffer_web = String();
        }
        break;
#endif
    default:
        break;

    }
}

void ESPCOM::print (const __FlashStringHelper *data, tpipe output, ESPResponseStream  *espresponse)
{
    String tmp = data;
    ESPCOM::print (tmp.c_str(), output, espresponse);
}
void ESPCOM::print (String & data, tpipe output, ESPResponseStream  *espresponse)
{
    ESPCOM::print (data.c_str(), output, espresponse);
}
void ESPCOM::print (const char * data, tpipe output, ESPResponseStream  *espresponse)
{
    if ((DEFAULT_PRINTER_PIPE == output) && ( block_2_printer || CONFIG::is_locked(FLAG_BLOCK_SERIAL))) {
        return;
    }
    if ((SERIAL_PIPE == output) && CONFIG::is_locked(FLAG_BLOCK_SERIAL)) {
        return;
    }
#ifdef TCP_IP_DATA_FEATURE
    if ((TCP_PIPE == output) && CONFIG::is_locked(FLAG_BLOCK_TCP)) {
        return;
    }
#endif
#ifdef WS_DATA_FEATURE
    if ((WS_PIPE == output) && CONFIG::is_locked(FLAG_BLOCK_WSOCKET)) {
        return;
    }
#endif
#ifdef ESP_OLED_FEATURE
    if ((OLED_PIPE == output) && CONFIG::is_locked(FLAG_BLOCK_OLED)) {
        return;
    }
#endif
    switch (output) {
#ifdef USE_SERIAL_0
    case SERIAL_PIPE:
#ifdef DEBUG_OUTPUT_SOCKET
        if(socket_server){
            socket_server->sendBIN(ESPCOM::current_socket_id,(const uint8_t*)data,strlen(data));
        }
#endif
        Serial.print (data);
        break;
#endif
#ifdef USE_SERIAL_1
    case SERIAL_PIPE:
        Serial1.print (data);
        break;
#endif
#ifdef USE_SERIAL_2
    case SERIAL_PIPE:
        Serial2.print (data);
        break;
#endif
#ifdef TCP_IP_DATA_FEATURE
    case TCP_PIPE:
        ESPCOM::send2TCP (data);
        break;
#endif
    case WEB_PIPE:
        if (espresponse != NULL) {
#if defined(ASYNCWEBSERVER)
            espresponse->print (data);
#else
            if (!espresponse->header_sent) {
                web_interface->web_server.setContentLength(CONTENT_LENGTH_UNKNOWN);
                web_interface->web_server.sendHeader("Content-Type","text/html");
                web_interface->web_server.sendHeader("Cache-Control","no-cache");
                web_interface->web_server.send(200);
                espresponse->header_sent = true;
            }
            espresponse->buffer_web+=data;
            if (espresponse->buffer_web.length() > 1200) {
                //send data
                web_interface->web_server.sendContent(espresponse->buffer_web);
                //reset buffer
                espresponse->buffer_web="";
            }
#endif
        }
        break;
#ifdef WS_DATA_FEATURE
    case WS_PIPE: {
#if defined(ASYNCWEBSERVER)
//Todo
#else
        if(socket_server){
            socket_server->sendBIN(current_socket_id,(const uint8_t *)data,strlen(data));
        }
#endif
    }
    break;
#endif

#ifdef ESP_OLED_FEATURE
    case OLED_PIPE: {
        if (!ESPCOM::block_2_oled) {
            if(!(!strcmp(data,"\n")||!strcmp(data,"\r")||!strcmp(data,"\r\n"))) {
                OLED_DISPLAY::print(data);
                OLED_DISPLAY::update_lcd();
            }
        }
    }
    break;
#endif
    case PRINTER_PIPE: {
#ifdef ESP_OLED_FEATURE
        OLED_DISPLAY::setCursor(0, 48);
        if(!(!strcmp(data,"\n")||!strcmp(data,"\r")||!strcmp(data,"\r\n"))) {
            ESPCOM::print(data, OLED_PIPE);
        }
#endif
        if (!CONFIG::is_locked(FLAG_BLOCK_M117)) {
            if(!(!strcmp(data,"\n")||!strcmp(data,"\r")||!strcmp(data,"\r\n"))) {
                ESPCOM::print ("M117 ", DEFAULT_PRINTER_PIPE);
            }
            ESPCOM::print (data, DEFAULT_PRINTER_PIPE);
        }
    }
    break;
    default:
        break;
    }
}
void ESPCOM::println (const __FlashStringHelper *data, tpipe output, ESPResponseStream  *espresponse)
{
    ESPCOM::print (data, output, espresponse);
#ifdef TCP_IP_DATA_FEATURE
    ESPCOM::print ("\r", output, espresponse);
#endif
    ESPCOM::print ("\n", output, espresponse);
}
void ESPCOM::println (String & data, tpipe output, ESPResponseStream  *espresponse)
{
    ESPCOM::print (data, output, espresponse);
#ifdef TCP_IP_DATA_FEATURE
    ESPCOM::print ("\r", output, espresponse);
#endif
    ESPCOM::print ("\n", output, espresponse);
}
void ESPCOM::println (const char * data, tpipe output, ESPResponseStream  *espresponse)
{
    ESPCOM::print (data, output, espresponse);
#ifdef TCP_IP_DATA_FEATURE
    ESPCOM::print ("\r", output, espresponse);
#endif
    ESPCOM::print ("\n", output, espresponse);
}


#ifdef TCP_IP_DATA_FEATURE
void ESPCOM::send2TCP (const __FlashStringHelper *data, bool async)
{
    String tmp = data;
    ESPCOM::send2TCP (tmp.c_str(), async);
}
void ESPCOM::send2TCP (String data, bool async)
{
    ESPCOM::send2TCP (data.c_str(), async);
}
void ESPCOM::send2TCP (const char * data, bool async)
{
    if (!async) {
        for (uint8_t i = 0; i < MAX_SRV_CLIENTS; i++) {
            if (serverClients[i] && serverClients[i].connected() ) {
                serverClients[i].write (data, strlen (data) );
                delay (0);
            }
        }
    }
}
#endif

bool ESPCOM::processFromSerial (bool async)
{
    uint8_t i;
    //check UART for data
    if (ESPCOM::available(DEFAULT_PRINTER_PIPE)) {
        size_t len = ESPCOM::available(DEFAULT_PRINTER_PIPE);
        uint8_t * sbuf = (uint8_t *)malloc(len+1);
        if(!sbuf) {
            return false;
        }
        sbuf[len] = '\0';
        ESPCOM::readBytes (DEFAULT_PRINTER_PIPE, sbuf, len);
#ifdef TCP_IP_DATA_FEATURE
        if (!async &&  !CONFIG::is_locked(FLAG_BLOCK_TCP)) {
            if ((WiFi.getMode() != WIFI_OFF)  || !wifi_config.WiFi_on) {
                //push UART data to all connected tcp clients
                for (i = 0; i < MAX_SRV_CLIENTS; i++) {
                    if (serverClients[i] && serverClients[i].connected() ) {
                        serverClients[i].write (sbuf, len);
                        delay (0);
                    }
                }
            }
        }
#endif
#ifdef WS_DATA_FEATURE

#if defined (ASYNCWEBSERVER)
        if (!CONFIG::is_locked(FLAG_BLOCK_WSOCKET)) {
            web_interface->web_socket.textAll(sbuf, len);
        }
#else
        if (!CONFIG::is_locked(FLAG_BLOCK_WSOCKET) && socket_server) {
#ifndef DEBUG_OUTPUT_SOCKET
            if(socket_server){
                socket_server->sendBIN(current_socket_id,sbuf,len);
                }
#endif
        }
#endif

#endif
        //process data if any
        COMMAND::read_buffer_serial (sbuf, len);
        free(sbuf);
        return true;
    } else {
        return false;
    }
}
#ifdef TCP_IP_DATA_FEATURE
void ESPCOM::processFromTCP2Serial()
{
    uint8_t i, data;
    //check if there are any new clients
    if (data_server->hasClient() ) {
        for (i = 0; i < MAX_SRV_CLIENTS; i++) {
            //find free/disconnected spot
            if (!serverClients[i] || !serverClients[i].connected() ) {
                if (serverClients[i]) {
                    serverClients[i].stop();
                }
                serverClients[i] = data_server->available();
                continue;
            }
        }
        //no free/disconnected spot so reject
        WiFiClient serverClient = data_server->available();
        serverClient.stop();
    }
    //check clients for data
    //to avoid any pollution if Uploading file to SDCard
    if (!((web_interface->blockserial)  || CONFIG::is_locked(FLAG_BLOCK_TCP) || CONFIG::is_locked(FLAG_BLOCK_SERIAL))) {
        for (i = 0; i < MAX_SRV_CLIENTS; i++) {
            if (serverClients[i] && serverClients[i].connected() ) {
                if (serverClients[i].available() ) {
                    //get data from the tcp client and push it to the UART
                    while (serverClients[i].available() ) {
                        data = serverClients[i].read();
                        ESPCOM::write(DEFAULT_PRINTER_PIPE, data);
                        COMMAND::read_buffer_tcp (data);
                    }
                }
            }
        }
    }
}
#endif
