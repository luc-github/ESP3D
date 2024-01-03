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

#define MKS_FRAME_SIZE 1024
#define MKS_FRAME_DATA_MAX_SIZE (MKS_FRAME_SIZE - 5 - 4)
#include "../../core/esp3d_message.h"
class MKSService {
 public:
  static bool begin();
  static bool sendNetworkFrame();
  static bool dispatch(ESP3DMessage* message);
  static bool sendGcodeFrame(const char* cmd);
  static void handle();
  static void handleFrame(const uint8_t type, const uint8_t* dataFrame,
                          const size_t dataSize);
  static void end();
  static bool started() { return _started; }
  static bool isHead(const char c);
  static bool isTail(const char c);
  static bool isFrame(const char c);
  static bool isCommand(const char c);
  static bool sendFirstFragment(const char* filename, size_t filesize);
  static bool sendFragment(const uint8_t* dataFrame, const size_t dataSize,
                           uint fragmentID);
  static uint getFragmentID(uint32_t fragmentNumber, bool isLast = false);
  static void commandMode(bool fromSettings = false);
  static void uploadMode();

 private:
  static uint8_t _uploadStatus;
  static long _commandBaudRate;
  static void sendWifiHotspots();
  static void messageWiFiControl(const uint8_t* dataFrame,
                                 const size_t dataSize);
  static void messageException(const uint8_t* dataFrame, const size_t dataSize);
  static void messageWiFiConfig(const uint8_t* dataFrame,
                                const size_t dataSize);
  static void clearFrame(uint start = 0);
  static bool canSendFrame();
  static void sendFrameDone();
  static bool _started;
  static uint8_t _frame[MKS_FRAME_SIZE];
  static char _moduleId[22];
  static bool _uploadMode;
};

#endif  //_SERIAL_SERVICES_H
