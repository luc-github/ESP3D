/*
  boot_delay.cpp -  boot delay functions class

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

#include "boot_delay.h"

#include "../../core/esp3d_message.h"
#include "../../core/esp3d_settings.h"
#include "../../include/esp3d_config.h"

#if defined(RECOVERY_FEATURE)
#include "../recovery/recovery_service.h"
#endif  // RECOVERY_FEATURE

BootDelay::BootDelay() {
  _started = false;
  _startdelay = 0;
  _totalduration = 0;
}
BootDelay::~BootDelay() { end(); }

bool BootDelay::started() { return _started; }

bool BootDelay::begin() {
  _totalduration = Settings_ESP3D::read_uint32(ESP_BOOT_DELAY);
  esp3d_log("Boot delay %d", _totalduration);
  if (!Settings_ESP3D::isValidIntegerSetting(_totalduration, ESP_BOOT_DELAY)) {
    _totalduration = Settings_ESP3D::getDefaultIntegerSetting(ESP_BOOT_DELAY);
    esp3d_log("Boot delay modified %d", _totalduration);
  }
  _started = true;
  ESP3D_Message::toScreen(ESP_OUTPUT_PROGRESS, "0");
  if (_totalduration > 0) {
    _startdelay = millis();
    handle();
  }
  ESP3D_Message::toScreen(ESP_OUTPUT_PROGRESS, "100");
  esp3d_log("Boot delay done");
  return _started;
}
void BootDelay::end() {}

void BootDelay::handle() {
  uint8_t lastpercent = 0;
  uint32_t lastSent = millis();
  while ((millis() - _startdelay) < _totalduration) {
#if defined(RECOVERY_FEATURE)
    recovery_service.handle();
#endif  // RECOVERY_FEATURE
        // to avoid overfload 2x/sec is enough for progression
    if ((millis() - lastSent) > 500) {
      lastSent = millis();
      uint8_t p = (100 * (millis() - _startdelay)) / _totalduration;
      if (p != lastpercent) {
        lastpercent = p;
        ESP3D_Message::toScreen(ESP_OUTPUT_PROGRESS, String(p).c_str());
      }
    }
    Hal::wait(10);
  }
}
