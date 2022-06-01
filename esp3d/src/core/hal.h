/*
  hal.h - esp3d hal class

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

#ifndef _ESP3D_HAL_H
#define _ESP3D_HAL_H
//be sure correct IDE and settings are used for ESP8266 or ESP32
#if !(defined( ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32))
#error Oops!  Make sure you have 'ESP8266 or ESP32' compatible board selected from the 'Tools -> Boards' menu.
#endif // ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32
#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>
#endif //ARDUINO_ARCH_ESP8266
#if defined(ARDUINO_ARCH_ESP32)
#include <WiFi.h>
#include <esp_task_wdt.h>
#endif //ARDUINO_ARCH_ESP32
#include <Arduino.h>

class Hal
{
public:
    static bool begin();
    static void end();
    static void wait (uint32_t milliseconds);
    static uint16_t getChipID();
    static bool has_temperature_sensor();
    static float temperature();
    static bool is_pin_usable(uint pin);
    static void clearAnalogChannels();
    static void pinMode(uint8_t pin, uint8_t mode);
    static int analogRead(uint8_t pin);
    static bool analogWrite(uint8_t pin, uint value);
    static void analogWriteFreq(uint32_t freq);
    static void analogRange(uint32_t range);
#if defined(ARDUINO_ARCH_ESP32)
    static TaskHandle_t xHandle;
#endif //ARDUINO_ARCH_ESP32
private:
    static void wdtFeed();
    static uint32_t _analogRange;
    static uint32_t _analogWriteFreq;
};
#endif //_ESP3D_HAL_H
