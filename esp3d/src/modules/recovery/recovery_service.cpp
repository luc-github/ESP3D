/*
  recovery.cpp -  recovery functions class

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

#include "../../include/esp3d_config.h"
#if defined(RECOVERY_FEATURE)
#include "recovery_service.h"
#include "../../core/settings_esp3d.h"
#include "../../core/esp3doutput.h"
RecoveryService recovery_service;

#ifdef PIN_RESET_FEATURE
#include "../../core/esp3d.h"
volatile bool interruptswitch =false;
#ifdef ARDUINO_ARCH_ESP32
portMUX_TYPE espmux = portMUX_INITIALIZER_UNLOCKED;
#endif //ARDUINO_ARCH_ESP32

#ifdef ARDUINO_ARCH_ESP8266
void handlePinResetInterrupt()
{
#endif //ARDUINO_ARCH_ESP8266
#ifdef ARDUINO_ARCH_ESP32
    void IRAM_ATTR handlePinResetInterrupt() {
        portENTER_CRITICAL_ISR(&espmux);
#endif //ARDUINO_ARCH_ESP32
        interruptswitch = true;
#ifdef ARDUINO_ARCH_ESP32
        portEXIT_CRITICAL_ISR(&espmux);
#endif //ARDUINO_ARCH_ESP32
    }
#endif //PIN_RESET_FEATURE

    RecoveryService::RecoveryService() {
        _started = false;
    }
    RecoveryService::~RecoveryService() {
        end();
    }

    bool RecoveryService::begin() {
        bool res = true;
        end();
#if defined (PIN_RESET_FEATURE)
#if defined(ESP3D_RESET_PIN) &&  ESP3D_RESET_PIN !=-1
        pinMode(ESP3D_RESET_PIN, INPUT_PULLUP);
        attachInterrupt(digitalPinToInterrupt(ESP3D_RESET_PIN), handlePinResetInterrupt, FALLING);
#endif //ESP3D_RESET_PIN
#endif //PIN_RESET_FEATURE
        if (!res) {
            end();
        }
        _started = res;
        return _started;
    }


    void RecoveryService::end() {
        if(!_started) {
            return;
        }
#ifdef PIN_RESET_FEATURE
        detachInterrupt(digitalPinToInterrupt(ESP3D_RESET_PIN));
#endif //PIN_RESET_FEATURE
        _started = false;
    }


    bool RecoveryService::started() {
        return _started;
    }

    void RecoveryService::handle() {
        if (_started) {
#ifdef PIN_RESET_FEATURE
            if (interruptswitch) {
                static uint32_t lastreset = 0;
                interruptswitch = false;
                if ((millis() - lastreset) > 1000) {
                    lastreset = millis();
                    ESP3DOutput  output(ESP_ALL_CLIENTS);
                    output.printMSG("Reset requested");
                    Esp3D::reset();
                }
            }
#endif //PIN_RESET_FEATURE
        }
    }

#endif //RECOVERY_FEATURE
