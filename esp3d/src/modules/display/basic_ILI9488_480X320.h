/*
  TFT_ILI9488_480X320.h - ESP3D display data file

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
#include "esp3d_logob.h"

//Screen size
#define SCREEN_WIDTH    480
#define SCREEN_HEIGHT   320

//Colors
#define SPLASH_FG   TFT_BLACK
#define SPLASH_BG   TFT_WHITE
#define SCREEN_BG   TFT_BLACK
#define PROGRESS_FG TFT_WHITE
#define SIGNAL_FG   TFT_WHITE
#define SSID_FG TFT_WHITE
#define IP_FG   TFT_WHITE
#define STATUS_FG   TFT_WHITE
#define CALIBRATION_BG TFT_BLACK
#define CALIBRATION_FG TFT_GREEN
#define CALIBRATION_CORNER TFT_RED

//Fonts
#define FONTSIGNAL 2
#define FONTSSID     2
#define FONTIP   2
#define FONTSTATUS   2
#define FONTCALIBRATION 2

//Positions
#define SIGNAL_X SCREEN_WIDTH-34
#define SIGNAL_Y 0
#define SIGNAL_W 34
#define SIGNAL_H 16

#define SIGNAL_ICON_X SCREEN_WIDTH-50
#define SIGNAL_ICON_Y 2
#define SIGNAL_ICON_W 15
#define SIGNAL_ICON_H 10
#define SIGNAL_ICON_W_BAR 3
#define SIGNAL_ICON_SPACER_X 1

#define SSID_AREA_X 0
#define SSID_AREA_Y 0
#define SSID_AREA_W SCREEN_WIDTH -51 -120
#define SSID_AREA_H 16

#define IP_AREA_X SCREEN_WIDTH -51 -120
#define IP_AREA_Y 0
#define IP_AREA_W 100
#define IP_AREA_H 16

#define STATUS_AREA_X 0
#define STATUS_AREA_Y SCREEN_HEIGHT - 17
#define STATUS_AREA_W SCREEN_WIDTH
#define STATUS_AREA_H 16
