/*
  config.h - ESP3D configuration file

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

#ifndef _ESP3D_CONFIG_H
#define _ESP3D_CONFIG_H
#include <Arduino.h>
#include "../include/defines.h"
#include "../include/pins.h"
#include "../../configuration.h"
#include "../include/sanity_esp3d.h"
#include "../core/hal.h"
#include "../core/debug_esp3d.h"
#include "../include/version.h"

/************************************
 *
 * Additional Flags
 *
 * **********************************/

//Make Flag more generic
#if defined(PIN_RESET_FEATURE) || defined(SD_RECOVERY_FEATURE)
#define RECOVERY_FEATURE
#endif //PIN_RESET_FEATURE || SD_RECOVERY_FEATURE

#if defined(DISPLAY_DEVICE) || defined(DHT_DEVICE) || defined(RECOVERY_FEATURE) || defined(BUZZER_DEVICE) || defined(CAMERA_DEVICE)  || defined(SD_DEVICE)
#define CONNECTED_DEVICES_FEATURE
#endif //DISPLAY_DEVICE || DHT_DEVICE , etc...

#endif //_ESP3D_CONFIG_H
