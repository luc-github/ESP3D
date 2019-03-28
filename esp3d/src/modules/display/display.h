/*
  display.h -  display functions class

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

#define SPLASH_SCREEN       0
#define MAIN_SCREEN         1

#ifndef _DISPLAY_H
#define _DISPLAY_H


class Display
{
public:
    Display();
    ~Display();
    bool begin();
    void end();
    void handle();
    void show_screenID(uint8_t screenID);
    void update_screen(bool force=false);
    void clear_screen();
    void progress(uint8_t v);
    void SetStatus(const char * status);
private:
    bool main_screen();
    bool splash();
    void showStatus();
    bool _started;
    uint8_t _screenID;
    bool _splash_displayed;
    uint _screenwidth;
    uint _screenheight;
    uint16_t sizetoFitSpace(const char * string, uint16_t maxwidth);
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t color);
    void drawRect(int16_t x, int16_t y, int16_t width, int16_t height, int16_t color);
    void fillRect(int16_t x, int16_t y, int16_t width, int16_t height, int16_t color);
    void setTextFont(uint8_t font);
    void drawString(const char *string, int32_t poX, int32_t poY, int16_t color);
    void drawXbm(int16_t x, int16_t y, int16_t width, int16_t height, int16_t color, const unsigned char *xbm);
    void drawXbm(int16_t x, int16_t y, int16_t width, int16_t height, uint16_t fgcolor, uint16_t bgcolor, const unsigned char *xbm);
    uint16_t getStringWidth(const char* text);
    String _status;
};

extern Display esp3d_display;
extern void display_progress(uint8_t v);

#endif //_DISPLAY_H

