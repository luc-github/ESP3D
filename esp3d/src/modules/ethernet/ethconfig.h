/*
  ethconfig.h -  ethernet functions class

  Copyright (c) 2018 Luc Lebosse. All rights reserved.

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



#ifndef _ETH_CONFIG_H
#define _ETH_CONFIG_H
#include <Arduino.h>
#ifdef ARDUINO_ARCH_ESP32
#include "ETH.h"
#endif //ARDUINO_ARCH_ESP32
#ifdef ARDUINO_ARCH_ESP8266
#endif //ARDUINO_ARCH_ESP8266

class EthConfig
{
public:
    static bool begin(int8_t & espMode);
    static bool StartSTA();
    //static bool StartSRV();
    static void end();
    static void handle();
    static bool started();
private :
    static bool _started;

};

#endif //_ETH_CONFIG_H
