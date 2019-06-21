/*
  esp3d.h - esp3d class

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

#ifndef ESP3D_H
#define ESP3D_H
//be sure correct IDE and settings are used for ESP8266 or ESP32
#if !(defined( ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32))
#error Oops!  Make sure you have 'ESP8266 or ESP32' compatible board selected from the 'Tools -> Boards' menu.
#endif

#include "Arduino.h"
class Esp3D
{
public:
    Esp3D();
    void begin(uint16_t startdelayms = 8000, uint16_t recoverydelayms = 8000);
    void process();
};
#endif
