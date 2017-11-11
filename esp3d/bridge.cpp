/*
  bridge.cpp - esp3d bridge serial/tcp class

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
#include "bridge.h"
#include "command.h"
#include "webinterface.h"

#ifdef TCP_IP_DATA_FEATURE
WiFiServer * data_server;
WiFiClient serverClients[MAX_SRV_CLIENTS];
#endif

void BRIDGE::print (const __FlashStringHelper *data, tpipe output, AsyncResponseStream  *asyncresponse)
{
    String tmp = data;
    BRIDGE::print (tmp.c_str(), output, asyncresponse);
}
void BRIDGE::print (String & data, tpipe output, AsyncResponseStream  *asyncresponse)
{
    BRIDGE::print (data.c_str(), output, asyncresponse);
}
void BRIDGE::print (const char * data, tpipe output, AsyncResponseStream  *asyncresponse)
{
    switch (output) {
    case SERIAL_PIPE:
        ESP_SERIAL_OUT.print (data);
        break;
#ifdef TCP_IP_DATA_FEATURE
    case TCP_PIPE:
        BRIDGE::send2TCP (data);
        break;
#endif
    case WEB_PIPE:
        if (asyncresponse != NULL) {
            asyncresponse->print (data);
        }
        break;
    default:
        break;
    }
}
void BRIDGE::println (const __FlashStringHelper *data, tpipe output, AsyncResponseStream  *asyncresponse)
{
    BRIDGE::print (data, output, asyncresponse);
#ifdef TCP_IP_DATA_FEATURE
    BRIDGE::print ("\r", output, asyncresponse);
#endif
    BRIDGE::print ("\n", output, asyncresponse);
}
void BRIDGE::println (String & data, tpipe output, AsyncResponseStream  *asyncresponse)
{
    BRIDGE::print (data, output, asyncresponse);
#ifdef TCP_IP_DATA_FEATURE
    BRIDGE::print ("\r", output, asyncresponse);
#endif
    BRIDGE::print ("\n", output, asyncresponse);
}
void BRIDGE::println (const char * data, tpipe output, AsyncResponseStream  *asyncresponse)
{
    BRIDGE::print (data, output, asyncresponse);
#ifdef TCP_IP_DATA_FEATURE
    BRIDGE::print ("\r", output, asyncresponse);
#endif
    BRIDGE::print ("\n", output, asyncresponse);
}


#ifdef TCP_IP_DATA_FEATURE
void BRIDGE::send2TCP (const __FlashStringHelper *data, bool async)
{
    String tmp = data;
    BRIDGE::send2TCP (tmp.c_str(), async);
}
void BRIDGE::send2TCP (String data, bool async)
{
    BRIDGE::send2TCP (data.c_str(), async);
}
void BRIDGE::send2TCP (const char * data, bool async)
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

bool BRIDGE::processFromSerial (bool async)
{
    uint8_t i;
    //check UART for data
    if (ESP_SERIAL_OUT.available() ) {
        size_t len = ESP_SERIAL_OUT.available();
        uint8_t sbuf[len];
        ESP_SERIAL_OUT.readBytes (sbuf, len);
#ifdef TCP_IP_DATA_FEATURE
        if (!async) {
            if (WiFi.getMode() != WIFI_OFF ) {
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
        //process data if any
        COMMAND::read_buffer_serial (sbuf, len);
        return true;
    } else {
        return false;
    }
}
#ifdef TCP_IP_DATA_FEATURE
void BRIDGE::processFromTCP2Serial()
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
    if ( (web_interface->blockserial) == false) {
        for (i = 0; i < MAX_SRV_CLIENTS; i++) {
            if (serverClients[i] && serverClients[i].connected() ) {
                if (serverClients[i].available() ) {
                    //get data from the tcp client and push it to the UART
                    while (serverClients[i].available() ) {
                        data = serverClients[i].read();
                        ESP_SERIAL_OUT.write (data);
                        COMMAND::read_buffer_tcp (data);
                    }
                }
            }
        }
    }
}
#endif
