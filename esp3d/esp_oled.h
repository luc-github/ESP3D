/*
  esp_oled.h - ESP3D oled class

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

#ifndef ESP_OLED_H
#define ESP_OLED_H
#include "config.h"
#ifdef ESP_OLED_FEATURE
class OLED_DISPLAY
{
public:
    static void begin();
    static void setCursor(int col, int row = -1);
    static void print(String & s);
    static void print(const char * s);
    static void display_signal( int value, int x=86, int y=0);
    static void display_text(const char * txt, int x=0, int y=48, int max=128);
    static void display_progress(int value, int x=0, int y=48);
    static void display_mini_progress(int value, int x = 14, int y=61, int w=100);
    static void update_lcd();
    static void clear_lcd();
    static void splash();
    static String L0;
    static String L1;
    static String L2;
    static String L3;
    static int L0_size;
    static int L1_size;
    static int L2_size;
    static int L3_size;
private:
    static int col;
    static int row;


};
#endif
#endif
