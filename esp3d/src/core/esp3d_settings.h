
/*
  esp3d_settings.h -  settings esp3d functions class

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

#ifndef _SETTINGS_ESP3D_H
#define _SETTINGS_ESP3D_H

#include <Arduino.h>

#include "../include/esp3d_config.h"

enum class ESP3DState : uint8_t {
  off = 0,
  on = 1,
};

enum class ESP3DSettingType : uint8_t {
  byte_t,     // byte
  integer_t,  // 4 bytes
  string_t,   // string
  ip_t,       // 4 bytes
  float_t,    // 4 bytes
  mask,       // x bytes
  bitsfield,  // x bytes
  unknown_t
};

struct ESP3DSettingDescription {
  ESP3DSettingIndex index;
  ESP3DSettingType type;
  uint16_t size;
  const char *default_val;
};
#ifdef SD_DEVICE
extern const uint8_t SupportedSPIDivider[];
extern const uint8_t SupportedSPIDividerSize;
#endif  // SD_DEVICE
#if defined(WIFI_FEATURE)
extern const uint8_t SupportedApChannels[];
extern const uint8_t SupportedApChannelsSize;
#endif  // WIFI_FEATURE

class ESP3DSettings {
 public:
  static bool begin();
  static uint8_t readByte(int pos, bool *haserror = NULL);
  static uint32_t readUint32(int pos, bool *haserror = NULL);
  static uint32_t read_IP(int pos, bool *haserror = NULL);
  static String readIPString(int pos, bool *haserror = NULL);
  static const char *readString(int pos, bool *haserror = NULL);
  static bool writeByte(int pos, const uint8_t value);
  static bool writeString(int pos, const char *byte_buffer);
  static bool writeUint32(int pos, const uint32_t value);
  static bool writeIPString(int pos, const char *value);
  static bool reset(bool networkonly = false);
  static int8_t GetSettingsVersion();
  static uint8_t GetFirmwareTarget(bool fromsettings = false);
  static bool isVerboseBoot(bool fromsettings = false);
  static uint8_t GetSDDevice();
  static const char *GetFirmwareTargetShortName();
  static const char *TargetBoard();

  static bool isValidIPStringSetting(const char *value,
                                     ESP3DSettingIndex settingElement);
  static bool isValidStringSetting(const char *value,
                                   ESP3DSettingIndex settingElement);
  static bool isValidIntegerSetting(uint32_t value,
                                    ESP3DSettingIndex settingElement);
  static bool isValidByteSetting(uint8_t value,
                                 ESP3DSettingIndex settingElement);
  static uint32_t getDefaultIntegerSetting(ESP3DSettingIndex settingElement);
  static const char *getDefaultStringSetting(ESP3DSettingIndex settingElement);
  static uint8_t getDefaultByteSetting(ESP3DSettingIndex settingElement);

  static const ESP3DSettingDescription *getSettingPtr(
      const ESP3DSettingIndex index);

 private:
  static String _IpToString(uint32_t ip_int);
  static uint32_t _stringToIP(const char *s);
  static bool is_string(const char *s, uint len);
  static uint8_t _FirmwareTarget;
  static bool _isverboseboot;
};

#endif  //_SETTINGS_ESP3D_H
