/*
  bridge.h - esp3d bridge serial/tcp class

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

#ifndef BRIDGE_H
#define BRIDGE_H
#include <WiFiServer.h>
#include "config.h"
#ifdef TCP_IP_DATA_FEATURE
extern WiFiServer * data_server;
#endif

class AsyncResponseStream;

class BRIDGE
{
public:
    static bool processFromSerial (bool async = false);
    static void print (const __FlashStringHelper *data, tpipe output, AsyncResponseStream  *asyncresponse = NULL);
    static void print (String & data, tpipe output, AsyncResponseStream  *asyncresponse = NULL);
    static void print (const char * data, tpipe output, AsyncResponseStream  *asyncresponse = NULL);
    static void println (const __FlashStringHelper *data, tpipe output, AsyncResponseStream  *asyncresponse = NULL);
    static void println (String & data, tpipe output, AsyncResponseStream  *asyncresponse = NULL);
    static void println (const char * data, tpipe output, AsyncResponseStream  *asyncresponse = NULL);
#ifdef TCP_IP_DATA_FEATURE
    static void processFromTCP2Serial();
    static void send2TCP (const __FlashStringHelper *data, bool async = false);
    static void send2TCP (String data, bool async = false);
    static void send2TCP (const char * data, bool async = false);
#endif
};
#endif
