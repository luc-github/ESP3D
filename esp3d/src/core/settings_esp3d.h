
/*
  settings_esp3d.h -  settings esp3d functions class

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

class Settings_ESP3D {
 public:
  static bool begin();

  /*static uint8_t get_max_string_size(int ps);   // TODO:change to new API
  static uint8_t get_min_string_size(int pos);   // TODO:change to new API
  static uint32_t get_max_int32_value(int pos);  // TODO:change to new API
  static uint32_t get_min_int32_value(int pos);  // TODO:change to new API
  static uint8_t get_max_byte(int pos);          // TODO:change to new API
  static int8_t get_min_byte(int pos);           // TODO:change to new API*/
  static uint8_t read_byte(int pos, bool *haserror = NULL);
  static uint32_t read_uint32(int pos, bool *haserror = NULL);
  static uint32_t read_IP(int pos, bool *haserror = NULL);
  static String read_IP_String(int pos, bool *haserror = NULL);
  static const char *read_string(int pos, bool *haserror = NULL);
  static bool write_byte(int pos, const uint8_t value);
  static bool write_string(int pos, const char *byte_buffer);
  static bool write_uint32(int pos, const uint32_t value);
  static bool write_IP(int pos, const uint32_t value);
  static bool write_IP_String(int pos, const char *value);
  static bool reset(bool networkonly = false);
  static int8_t GetSettingsVersion();
  static uint8_t GetFirmwareTarget(bool fromsettings = false);
  static bool isVerboseBoot(bool fromsettings = false);
  static uint8_t GetSDDevice();
  static const char *GetFirmwareTargetShortName();
  static String IPtoString(uint32_t ip_int);  // TODO:move to private
  static uint32_t StringtoIP(const char *s);  // TODO:move to private
  static const char *TargetBoard();
  static bool isLocalPasswordValid(
      const char *password);  // TODO:change to new API

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
  static bool is_string(const char *s, uint len);
  static uint8_t _FirmwareTarget;
  static bool _isverboseboot;
};

#endif  //_SETTINGS_ESP3D_H
