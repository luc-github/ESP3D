/*
  display.h -  display functions class

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

#define SPLASH_SCREEN       0
#define MAIN_SCREEN         1
#define CALIBRATION_SCREEN  2

#ifndef _DISPLAY_CLASS_H
#define _DISPLAY_CLASS_H

class Display
{
public:
    Display();
    ~Display();
    bool begin();
    void end();
    void handle();
    void showScreenID(uint8_t screenID);
    void updateScreen(bool force=false);
    void clearScreen();
    void progress(uint8_t v);
    void setStatus(const char * status);
    void updateIP();
    const char * getModelString();
    uint8_t getModelID();
#if defined(DISPLAY_TOUCH_DRIVER)
    bool startCalibration();
#endif //DISPLAY_TOUCH_DRIVER
private:
    bool mainScreenHandler(bool force = false);
    bool displaySignal(bool force = false);
    bool displayIP(bool force = false);
    bool splash();
    bool showStatus(bool force = false);
    bool _started;
    uint8_t _screenID;
    bool _splashDone;
    uint _screenWidth;
    uint _screenHeight;
    uint16_t sizetoFitSpace(const char * string, uint16_t maxwidth);
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t color);
    void drawRect(int16_t x, int16_t y, int16_t width, int16_t height, int16_t color);
    void fillRect(int16_t x, int16_t y, int16_t width, int16_t height, int16_t color);
    void setTextFont(uint8_t font);
    void drawString(const char *string, int32_t poX, int32_t poY, int16_t color);
    void drawXbm(int16_t x, int16_t y, int16_t width, int16_t height, int16_t color, const uint8_t *xbm);
    void drawXbm(int16_t x, int16_t y, int16_t width, int16_t height, uint16_t fgcolor, uint16_t bgcolor, const uint8_t *xbm);
    uint16_t getStringWidth(const char* text);
    uint8_t _font;
    String _status;
};

extern Display esp3d_display;

#endif //_DISPLAY_CLASS_H
