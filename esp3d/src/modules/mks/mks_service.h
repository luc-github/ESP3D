/*
  mks_service.cpp -  mks communication service functions class

  Copyright (c) 2014 Luc Lebosse. All rights reserved.

  This code is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This code is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with This code; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef _MKS_SERVICES_H
#define _MKS_SERVICES_H

#define MKS_FRAME_SIZE  1024

class MKSService
{
public:
    static bool begin();
    static bool sendNetworkFrame();
    static bool sendGcodeFrame(const char* cmd);
    static void handle();
    static void handleFrame(const uint8_t type, const uint8_t * dataFrame, const size_t dataSize );
    static void end();
    static bool started()
    {
        return _started;
    }
    static bool isHead(const char c);
    static bool isTail(const char c);
    static bool isFrame(const char c);
    static bool isCommand(const char c);
private:
    static uint8_t _uploadStatus;
    static void sendWifiHotspots();
    static void messageWiFiControl(const uint8_t * dataFrame, const size_t dataSize);
    static void messageException(const uint8_t * dataFrame, const size_t dataSize);
    static void messageWiFiConfig(const uint8_t * dataFrame, const size_t dataSize);
    static void clearFrame();
    static bool canSendFrame();
    static void sendFrameDone();
    static bool _started;
    static char _frame[MKS_FRAME_SIZE];
    static char _moduleId[22];
};


#endif //_SERIAL_SERVICES_H

