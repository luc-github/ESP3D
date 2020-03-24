/*
  advanceddisplay.h -  display functions class

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

#define SPLASH_SCREEN        0
#define CALIBRATION_SCREEN   1
#define MAIN_SCREEN          2

#ifndef _ADVANCEDDISPLAY_H
#define _ADVANCEDDISPLAY_H


class Display
{
public:
    Display();
    ~Display();
    bool begin();
    void end();
    void handle();
    bool display_IP(bool force = false);
    bool display_network_status(bool force = false);
    void show_screenID(uint8_t screenID);
    void update_screen(bool force=false);
    void clear_screen(bool force=false);
    void progress(uint8_t v);
    void SetStatus(const char * status);
    bool startCalibration();
    bool snapshot(char * filename = nullptr);
private:
    bool _started;
    uint8_t _screenID;
    uint _screenwidth;
    uint _screenheight;
    String _status;
};

extern Display esp3d_display;

#endif //_ADVANCEDDISPLAY_H

