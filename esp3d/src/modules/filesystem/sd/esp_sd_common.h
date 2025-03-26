/*
  esp_sd_common.h - ESP3D SD support class

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

#pragma once
#include "../../../include/esp3d_config.h"

#define ESP_SD_FS_HEADER "/SD"

#define ESP_MAX_SD_OPENHANDLE 4


#if (SD_DEVICE == ESP_SD_NATIVE) && defined(ARDUINO_ARCH_ESP8266)
#define FS_NO_GLOBALS
#define NO_GLOBAL_SD
#include <SD.h>
using ESP3D_File = fs::File;
using ESP3D_SD_Class = SDClass;
#endif // (SD_DEVICE == ESP_SD_NATIVE) && defined(ARDUINO_ARCH_ESP8266

#if ((SD_DEVICE == ESP_SDFAT) || (SD_DEVICE == ESP_SDFAT2)) && defined(ARDUINO_ARCH_ESP8266)
#define FS_NO_GLOBALS
#define NO_GLOBAL_SD
#define DISABLE_FS_H_WARNING 1
#include <SdFat.h>
#if SDFAT_FILE_TYPE == 1
using ESP3D_File = File32;
#elif SDFAT_FILE_TYPE == 2
using ESP3D_File = ExFile;
#elif SDFAT_FILE_TYPE == 3
using ESP3D_File = FsFile;
#else  // SDFAT_FILE_TYPE
#error Invalid SDFAT_FILE_TYPE
#endif  // SDFAT_FILE_TYPE
using ESP3D_SD_Class = SdFat;
#endif // ((SD_DEVICE == ESP_SDFAT) || (SD_DEVICE == ESP_SDFAT2)) && defined(ARDUINO_ARCH_ESP8266

#if ((SD_DEVICE == ESP_SDFAT) || (SD_DEVICE == ESP_SDFAT2)) && defined(ARDUINO_ARCH_ESP32)
#define DISABLE_FS_H_WARNING 1
#include <SdFat.h>
#if SDFAT_FILE_TYPE == 1
using ESP3D_File = File32;
#elif SDFAT_FILE_TYPE == 2
using ESP3D_File = ExFile;
#elif SDFAT_FILE_TYPE == 3
using ESP3D_File = FsFile;
#else  // SDFAT_FILE_TYPE
#error Invalid SDFAT_FILE_TYPE
#endif  // SDFAT_FILE_TYPE
using ESP3D_SD_Class = SdFat;
#endif // ((SD_DEVICE == ESP_SDFAT) || (SD_DEVICE == ESP_SDFAT2)) && defined(ARDUINO_ARCH_ESP32

#if (SD_DEVICE == ESP_SD_NATIVE) && defined(ARDUINO_ARCH_ESP32)
#include <FS.h>
#include <SD.h> 
using ESP3D_File = fs::File;
using ESP3D_SD_Class = fs::SDFS;
#endif // (SD_DEVICE == ESP_SD_NATIVE) && defined(ARDUINO_ARCH_ESP32

#if (SD_DEVICE == ESP_SDIO) && defined(ARDUINO_ARCH_ESP32)
#include <FS.h>
#include <SD_MMC.h>
using ESP3D_File = fs::File;
using ESP3D_SD_Class = fs::SDMMCFS;
#endif // (SD_DEVICE == ESP_SDIO) && defined(ARDUINO_ARCH_ESP32


extern ESP3D_SD_Class ESP3D_SD_Card;
extern ESP3D_File tSDFile_handle[ESP_MAX_SD_OPENHANDLE];