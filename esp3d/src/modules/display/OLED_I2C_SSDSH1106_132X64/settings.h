/*
  settings.h - ESP3D display data file

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

//Screen size
#define SCREEN_WIDTH    132
#define SCREEN_HEIGHT   64
//Colors
#define COLOR_BLACK   BLACK
#define COLOR_WHITE   WHITE
#define SPLASH_FG   COLOR_BLACK
#define SPLASH_BG   COLOR_WHITE
#define SCREEN_BG   COLOR_BLACK
#define PROGRESS_FG COLOR_WHITE
#define SIGNAL_FG   COLOR_WHITE
#define SSID_FG COLOR_WHITE
#define IP_FG   COLOR_WHITE
#define STATUS_FG   COLOR_WHITE

//Fonts
#define FONTSIGNAL 2
#define FONTSSID     2
#define FONTIP   3
#define FONTSTATUS   2

//Positions
#define SIGNAL_X 132-27
#define SIGNAL_Y 0
#define SIGNAL_W 46
#define SIGNAL_H 10

#define SIGNAL_ICON_X 132-43
#define SIGNAL_ICON_Y 2
#define SIGNAL_ICON_W 15
#define SIGNAL_ICON_H 10
#define SIGNAL_ICON_W_BAR 3
#define SIGNAL_ICON_SPACER_X 1

#define SSID_AREA_X 0
#define SSID_AREA_Y 0
#define SSID_AREA_W 85
#define SSID_AREA_H 12

#define IP_AREA_X 0
#define IP_AREA_Y 16
#define IP_AREA_W 132
#define IP_AREA_H 16

#define STATUS_AREA_X 0
#define STATUS_AREA_Y 48
#define STATUS_AREA_W 132
#define STATUS_AREA_H 16
