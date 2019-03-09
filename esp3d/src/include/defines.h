/*
  defines.h - ESP3D defines file

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

#ifndef _DEFINES_ESP3D_H
#define _DEFINES_ESP3D_H

//Settings
#define SETTINGS_IN_EEPROM 1
#define SETTINGS_IN_PREFERENCES 2

//Debug
#define DEBUG_OUTPUT_SERIAL0 1
#define DEBUG_OUTPUT_SERIAL1 2
#define DEBUG_OUTPUT_SERIAL2 3

//Serial
#define USE_SERIAL_0 1
#define USE_SERIAL_1 2
#define USE_SERIAL_2 3

//Display
#define OLED_I2C_SSD1306    1
#define OLED_I2C_SSDSH1106  2

//File systems
#define ESP_SPIFFS_FILESYSTEM       1
#define ESP_FAT_FILESYSTEM          2
#define ESP_LITTLEFS_FILESYSTEM     3

//Notifications
#define ESP_PUSHOVER_NOTIFICATION	1
#define ESP_EMAIL_NOTIFICATION		2

//DHT
#define DHT11_DEVICE 1
#define DHT22_DEVICE 2
#define USE_CELSIUS		1
#define USE_FAHRENHEIT	2

#endif //_DEFINES_ESP3D_H
