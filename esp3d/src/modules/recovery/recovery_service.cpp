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
#include "../../core/esp3d_commands.h"
#include "../../core/esp3d_settings.h"
#include "recovery_service.h"

RecoveryService recovery_service;

#if defined(PIN_RESET_FEATURE) && defined(ESP3D_RESET_PIN) && \
    ESP3D_RESET_PIN != -1
#include "../../core/esp3d.h"
bool interruptswitch = false;

RecoveryService::RecoveryService() { _started = false; }
RecoveryService::~RecoveryService() { end(); }

bool RecoveryService::begin() {
  bool res = true;
  end();

#if defined(PIN_RESET_FEATURE) && defined(ESP3D_RESET_PIN) && \
    ESP3D_RESET_PIN != -1
  pinMode(ESP3D_RESET_PIN, INPUT_PULLUP);
  // attach interrupt to pin is conflicting with camera device because it
  // already attach interrupt to pin
#endif  // PIN_RESET_FEATURE
  if (!res) {
    end();
  }
  _started = res;
  _servicetimeout = millis();
  return _started;
}

void RecoveryService::end() {
  if (!_started) {
    return;
  }
  _started = false;
}

bool RecoveryService::started() { return _started; }

void RecoveryService::handle() {
  if (_started) {
    // Stop service 1000 ms after started to be sure pin can be used for
    // something else and avoid unwished reset
    if (millis() - _servicetimeout > 5000) {
      _started = false;
      return;
    }
#if defined(PIN_RESET_FEATURE) && defined(ESP3D_RESET_PIN) && \
    ESP3D_RESET_PIN != -1
    // attach interrupt to pin is conflicting with camera device because it
    // already attach interrupt to pin so use digitalread to check pin state
    interruptswitch = !digitalRead(ESP3D_RESET_PIN);
#endif  // PIN_RESET_FEATURE
    if (interruptswitch) {
      static uint32_t lastreset = 0;
      interruptswitch = false;
      if ((millis() - lastreset) > 1000) {
        lastreset = millis();
        esp3d_commands.dispatch("Reset requested", ESP3DClientType::all_clients,
                                no_id, ESP3DMessageType::unique,
                                ESP3DClientType::system,
                                ESP3DAuthenticationLevel::admin);
        Esp3D::reset();
        ESP3DHal::wait(100);
        Esp3D::restart_esp();
      }
    }
#endif  // PIN_RESET_FEATURE
  }
}

#endif  // RECOVERY_FEATURE
