/*
  display.cpp -  display functions class

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

#include "../../../include/esp3d_config.h"
#if defined (DISPLAY_DEVICE) && DISPLAY_DEVICE == TFT_SPI_ST7789_135X240

#include "../display.h"
#include "../../../core/settings_esp3d.h"
#include "../../../core/esp3doutput.h"
#include "Wire.h"
#include "esp3d_logo.h"
#include <TFT_eSPI.h>
#include "settings.h"

#if defined (WIFI_FEATURE)
#include "../../wifi/wificonfig.h"
#endif // WIFI_FEATURE
#if defined (ETH_FEATURE)
#include "../../ethernet/ethconfig.h"
#endif //ETH_FEATURE
#if defined (BLUETOOTH_FEATURE)
#include "../../bluetooth/BT_service.h"
#endif //BLUETOOTH_FEATURE
#if defined (WIFI_FEATURE) || defined (ETH_FEATURE) || defined (BLUETOOTH_FEATURE)
#include "../../network/netconfig.h"
#endif //WIFI_FEATURE || ETH_FEATURE) ||BLUETOOTH_FEATURE
#define DISPLAY_REFRESH_TIME 1000

TFT_eSPI esp3d_screen = TFT_eSPI();
Display esp3d_display;

#if defined(DISPLAY_TOUCH_DRIVER)
bool Display::startCalibration()
{
#error "DISPLAY_TOUCH_DRIVER not supported with OLED_I2C_SSD1306_128X64"
}
#endif //DISPLAY_TOUCH_DRIVER

/**
 * The constructor for the Display class.
 */
Display::Display()
{
    _started = false;
    _screenID = SPLASH_SCREEN;
    _splashDone=false;
    _screenWidth = SCREEN_WIDTH;
    _screenHeight = SCREEN_HEIGHT;
}

/**
 * The destructor for the Display class.
 */
Display::~Display()
{
    end();
}

/**
 * If the display is started, set the status to blank, set the started flag to false, set the screen ID
 * to the splash screen, set the splash displayed flag to false, and clear the screen
 *
 */
void Display::end()
{
    if(!_started) {
        return;
    }
    _status ="";
    _started = false;
    _screenID = SPLASH_SCREEN;
    _splashDone=false;
    clearScreen();
}

/**
 * It initializes the display.
 *
 * @return true if success.
 */
bool Display::begin()
{
    bool res = true;
    _started = false;
    log_esp3d("Init Display");

    esp3d_screen.init();
    clearScreen();
    setTextFont(2);
#if defined(DISPLAY_FLIP_VERTICALY)
    esp3d_screen.setRotation(3);
#else
    esp3d_screen.setRotation(1);
#endif
    showScreenID(SPLASH_SCREEN);
    updateScreen(true);
#if defined(DISPLAY_TOUCH_DRIVER)
    if(Settings_ESP3D::read_byte(ESP_CALIBRATION)==1) {
        uint16_t calibrationData[5];
        for (uint8_t i = 0; i < 5; i++) {
            calibrationData[i] = Settings_ESP3D::read_uint32 (ESP_CALIBRATION_1+(4*i));
        }
        esp3d_screen.setTouch(calibrationData);
    }
#endif //DISPLAY_TOUCH_DRIVER
    res = true;
    if (!res) {
        end();
    }
    _started = res;
    return _started;
}

void Display::handle()
{
    if ( !ESP3DOutput::isOutput(ESP_SCREEN_CLIENT)) {
        return;
    }
    if (_started) {
        updateScreen();
    }
}

const char * Display::getModelString()
{
    return "TFT_SPI_ST7789_135X240";
}

uint8_t Display::getModelID()
{
    return DISPLAY_DEVICE;
}

/**
 * Display the screen with the given screenID
 *
 * @param screenID The screen ID to show.
 */
void Display::showScreenID(uint8_t screenID)
{
    /* Calling the function updateScreen. */
    _screenID = screenID;
}

/**
 * The function takes a string as an argument and sets the value of the private member variable _status
 * to the value of the argument
 *
 * @param status The status to set.
 */
void Display::setStatus(const char * status)
{
    _status= status;
}

/**
 * The function clears the screen by calling the clear() function of the esp3d_screen object
 */
void Display::clearScreen()
{
    log_esp3d("clear screen");
    esp3d_screen.fillScreen(SCREEN_BG);
}

void Display::updateScreen(bool force)
{
    static uint8_t lastScreenID = 255;
    log_esp3d("update screen");
    if ( !ESP3DOutput::isOutput(ESP_SCREEN_CLIENT)) {
        return;
    }
    static uint32_t last_update = millis();
    bool need_update = force;
    if (((millis()- last_update) > DISPLAY_REFRESH_TIME) ) {
        if (lastScreenID != _screenID) {
            lastScreenID = _screenID;
            need_update = true;
            clearScreen();
        }
        last_update = millis();
        switch(_screenID) {
        case SPLASH_SCREEN:
            if (force) {
                _splashDone=false;
            }
            need_update = splash();
            break;
        case MAIN_SCREEN:
            need_update =  mainScreenHandler(force);
            break;
        default:
            break;
        }

    }
}


/**
 * It returns the width of the string in pixels.
 *
 * @param text The text to be displayed
 *
 * @return The width of the string in pixels.
 */
uint16_t Display::getStringWidth(const char* text)
{
    return  esp3d_screen.textWidth(text, _font);
}

/**
 * It takes a string and a maximum width, and returns the number of characters that will fit in that
 * width
 *
 * @param string The string to be measured
 * @param maxwidth The maximum width of the string in pixels.
 *
 * @return The length of the string.
 */
uint16_t Display::sizetoFitSpace(const char * string, uint16_t maxwidth)
{
    String s = string;
    while (getStringWidth(s.c_str()) > maxwidth) {
        if (s.length() > 0) {
            s.remove(s.length()-1);
        } else {
            return 0;
        }
    }
    return s.length();
}

void Display::setTextFont(uint8_t font)
{
    log_esp3d("setTextFont size %d", font);
    _font=font;
}

void Display::drawString(const char *string, int32_t poX, int32_t poY, int16_t color)
{
    log_esp3d("drawString %s at %d,%d", string, poX, poY);
    if ( !ESP3DOutput::isOutput(ESP_SCREEN_CLIENT)) {
        return;
    }
    esp3d_screen.setTextColor(color);
    esp3d_screen.drawString(string,poX, poY, _font);
}

/**
 * > Draw a line from point (x0,y0) to point (x1,y1) using the specified color
 *
 * @param x0 The x coordinate of the first point
 * @param y0 The y coordinate of the first point
 * @param x1 The x coordinate of the second point.
 * @param y1 The y coordinate of the second point.
 * @param color The color of the line.
 *
 * @return Nothing.
 */
void Display::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t color)
{
    if ( !ESP3DOutput::isOutput(ESP_SCREEN_CLIENT)) {
        return;
    }
    esp3d_screen.drawLine(x0, y0, x1, y1,color);
}


/**
 * > Draw a rectangle on the screen
 *
 * @param x The x coordinate of the upper left corner of the rectangle.
 * @param y The y coordinate of the top left corner of the rectangle.
 * @param width The width of the rectangle in pixels.
 * @param height The height of the rectangle.
 * @param color the color of the rectangle
 *
 * @return Nothing.
 */
void Display::drawRect(int16_t x, int16_t y, int16_t width, int16_t height, int16_t color)
{
    if ( !ESP3DOutput::isOutput(ESP_SCREEN_CLIENT)) {
        return;
    }
    esp3d_screen.drawRect(x, y, width, height, color);
}

/**
 * > If the screen is connected, set the color and fill the rectangle
 *
 * @param x The x coordinate of the upper-left corner of the rectangle to fill.
 * @param y The y coordinate of the upper left corner of the rectangle.
 * @param width The width of the rectangle in pixels.
 * @param height The height of the rectangle, in pixels.
 * @param color the color of the rectangle
 *
 * @return Nothing.
 */
void Display::fillRect(int16_t x, int16_t y, int16_t width, int16_t height, int16_t color)
{
    if ( !ESP3DOutput::isOutput(ESP_SCREEN_CLIENT)) {
        return;
    }
    esp3d_screen.fillRect(x, y, width, height, color);
}

/**
 * It draws an XBM image on the screen.
 *
 * @param x x coordinate of the top left corner of the image
 * @param y y coordinate of the top left corner of the image
 * @param width The width of the image in pixels.
 * @param height the height of the image in pixels
 * @param color the color of the image.
 * @param xbm the xbm image data
 *
 * @return Nothing.
 */
void Display::drawXbm(int16_t x, int16_t y, int16_t width, int16_t height, int16_t color, const uint8_t *xbm)
{
    if ( !ESP3DOutput::isOutput(ESP_SCREEN_CLIENT)) {
        return;
    }
    esp3d_screen.drawXBitmap(x, y, xbm, width, height,color);
}

/**
 * It draws an XBM image on the screen.
 *
 * @param x x coordinate of the top left corner of the image
 * @param y y coordinate of the top left corner of the image
 * @param width The width of the image in pixels.
 * @param height the height of the image in pixels
 * @param fgcolor foreground color
 * @param bgcolor background color
 * @param xbm the xbm image data
 *
 */
void Display::drawXbm(int16_t x, int16_t y, int16_t width, int16_t height, uint16_t fgcolor, uint16_t bgcolor, const uint8_t *xbm)
{
    if ( !ESP3DOutput::isOutput(ESP_SCREEN_CLIENT)) {
        return;
    }
    (void)fgcolor;
    (void)bgcolor;
    esp3d_screen.drawXBitmap(x, y, xbm, width, height,fgcolor,bgcolor);
}

/**
 * The function is called with a value between 0 and 100. The function draws a rectangle on the screen
 * with a width that is proportional to the value
 *
 * @param v The percentage of the progress bar to fill.
 *
 * @return the value of the variable v.
 */
void Display::progress(uint8_t v)
{
    if ( !ESP3DOutput::isOutput(ESP_SCREEN_CLIENT)) {
        return;
    }
    static uint8_t previous = 0;
    if (previous > v) {
        //clear
        fillRect(10, _screenHeight-4, _screenWidth-20, 4, SCREEN_BG);
    }
    log_esp3d("%d", v);
    previous = v>100?100:v;
    //display bar
    fillRect(10, _screenHeight-4, ((_screenWidth-20) * v)/100, 4, PROGRESS_FG);
    //update screen
    updateScreen(true);
}

/**
 * If the display is not busy, then check if the signal strength has changed, if so, then display it.
 * If the IP address has changed, then display it. If the status has changed, then display it
 *
 * @param force boolean, if true, the screen will be updated regardless of whether or not it needs to
 * be.
 *
 * @return A boolean value.
 */
bool Display::mainScreenHandler(bool force)
{
    bool res = false;
    if (displaySignal(force)) {
        res = true;
    }
    if (displayIP(force)) {
        res = true;
    }
    if (showStatus(force)) {
        res = true;
    }
    return res;
}

/**
 * If the screen is connected, and the splash screen has not been displayed, then display the splash
 * screen
 *
 * @return a boolean value.
 */
bool Display::splash()
{
    log_esp3d("Splash");
    if ( !ESP3DOutput::isOutput(ESP_SCREEN_CLIENT)) {
        return false;
    }
    if (!_splashDone) {
        drawXbm((_screenWidth-ESP3D_Logo_width)/2, (_screenHeight-ESP3D_Logo_height)/2, ESP3D_Logo_width, ESP3D_Logo_height, SPLASH_FG, SPLASH_BG,ESP3D_Logo);
        log_esp3d("Display Splash");
        _splashDone = true;
        return true;
    }
    return false;
}

void Display::updateIP()
{
    if ( !ESP3DOutput::isOutput(ESP_SCREEN_CLIENT)) {
        return;
    }
    updateScreen(true);
}

/**
 * It takes a string, and if it's too long to fit in the status area, it chops it up into pieces and
 * displays them one at a time
 *
 * @param force if true, the status will be redrawn regardless of whether it's changed or not.
 *
 * @return a boolean value.
 */
bool Display::showStatus(bool force)
{
    //Display Status
    bool refresh_status = force;
    static String status;
    if (status != _status) {
        status = _status;
        refresh_status = true;
    }

    setTextFont(FONTSTATUS);
    uint16_t size = sizetoFitSpace(status.c_str(), STATUS_AREA_W);
    //check the need for resize
    if (size < status.length()) {
        static int status_shift = -1;
        refresh_status = true;
        status+=" ";
        log_esp3d("current %s", status.c_str());
        if (status_shift != -1) {
            if( (uint16_t)(status_shift)> status.length()) {
                status_shift = -1;
            }
        }
        log_esp3d("shift %d", status_shift);
        if (status_shift > 0) {
            status.remove(0,status_shift);
        }
        log_esp3d("shifted %s", status.c_str());
        size = sizetoFitSpace(status.c_str(), STATUS_AREA_W);
        log_esp3d("size available %d existing %d",size, status.length());
        if (size < status.length()) {
            //cut
            status = status.substring(0,size);
            status_shift++;
        } else {
            status_shift = -1;
        }
        log_esp3d("sized %s", status.c_str());
    }
    if (refresh_status) {
        //clear area
        fillRect(STATUS_AREA_X, STATUS_AREA_Y, STATUS_AREA_W, STATUS_AREA_H, SCREEN_BG);
        drawString(status.c_str(), STATUS_AREA_X, STATUS_AREA_Y, STATUS_FG);
    }
    return refresh_status;
}

bool Display::displaySignal(bool force)
{
    static int sig = -1;
    bool refresh_signal = false;
    bool refresh_label = false;
    static String label;
#if defined (WIFI_FEATURE)
    if (WiFiConfig::started()) {

        if (WiFi.getMode() == WIFI_AP) {
            if (sig != 100) {
                sig = 100;
                refresh_signal = true;
            }
            if (label != WiFiConfig::AP_SSID()) {
                label = WiFiConfig::AP_SSID();
                refresh_label = true;
            }
        } else {
            if (WiFi.isConnected()) {
                int sigtmp =   WiFiConfig::getSignal (WiFi.RSSI (),false);
                if (sig != sigtmp) {
                    sig =sigtmp;
                    refresh_signal = true;
                }
                if (label != WiFi.SSID()) {
                    label = WiFi.SSID();
                    refresh_label = true;
                }
            } else {
                if (sig != -1) {
                    sig = -1;
                    refresh_signal = true;
                }
                if (label != "") {
                    label = "";
                    refresh_label = true;
                }
            }
        }
        //Display SSID
        setTextFont(FONTSSID);
        uint16_t size = sizetoFitSpace(label.c_str(), SSID_AREA_W);
        //check the need for resize
        if (size < label.length()) {
            refresh_label = true;
            static int label_shift = -1;
            label+=" ";
            //log_esp3d("current %s", label.c_str());
            if (label_shift != -1) {
                if((uint16_t)(label_shift)> label.length()) {
                    label_shift = -1;
                }
            }
            //log_esp3d("shift %d", label_shift);
            if (label_shift > 0) {
                label.remove(0,label_shift);
            }
            //log_esp3d("shifted %s", label.c_str());
            size = sizetoFitSpace(label.c_str(), SSID_AREA_W);
            //log_esp3d("size available %d existing %d",size, label.length());
            if (size < label.length()) {
                //cut
                label = label.substring(0,size);
                label_shift++;
            } else {
                label_shift = -1;
            }
            //log_esp3d("sized %s", label.c_str());
        }
        if (refresh_label || force) {
            //clear area
            fillRect(SSID_AREA_X, SSID_AREA_Y, SSID_AREA_W, SSID_AREA_H, SCREEN_BG);
            drawString(label.c_str(), SSID_AREA_X, SSID_AREA_Y, SSID_FG);
        }
    }
#endif // WIFI_FEATURE

#if defined (ETH_FEATURE)
    if (EthConfig::started()) {
        if (sig != -2) {
            sig = -2;
            refresh_signal = true;
        }
        //display connection speed
        if(ETH.linkUp()) {
            String tmp = ETH.linkSpeed();
            tmp+= "Mbps";
            if (label != tmp) {
                label = tmp;
                refresh_label = true;
            }
        } else {
            if (label !="") {
                label ="";
                refresh_label = true;
            }
        }
        if (refresh_label || force) {
            setTextFont(FONTSSID);
            //clear area
            fillRect(SSID_AREA_X, SSID_AREA_Y, SSID_AREA_W, SSID_AREA_H, SCREEN_BG);
            drawString(label.c_str(), SSID_AREA_X, SSID_AREA_Y, SSID_FG);
        }
    }
#endif //ETH_FEATURE

#if defined (BLUETOOTH_FEATURE)
    if (bt_service.started()) {
        if (sig!=-3) {
            sig = -3;
            refresh_signal = true;
        }

        //Display hostname
        if (label != bt_service.hostname()) {
            refresh_label = true;
            label = bt_service.hostname();
        }
        setTextFont(FONTSSID);
        uint16_t size = sizetoFitSpace(label.c_str(), SSID_AREA_W);
        //check the need for resize
        if (size < label.length()) {
            refresh_label = true;
            static int label_shift = -1;
            label+=" ";
            //log_esp3d("current %s", hostname.c_str());
            if (label_shift > label.length()) {
                label_shift = -1;
            }
            //log_esp3d("shift %d", label_shift);
            if (label_shift > 0) {
                label.remove(0,label_shift);
            }
            //log_esp3d("shifted %s", hostname.c_str());
            size = sizetoFitSpace(label.c_str(), SSID_AREA_W);
            //log_esp3d("size available %d existing %d",size, hostname.length());
            if (size < label.length()) {
                //cut
                label = label.substring(0,size);
                label_shift++;
            } else {
                label_shift = -1;
            }
            //log_esp3d("sized %s", hostname.c_str());
        }
        if( refresh_label || force) {
            //clear area
            fillRect(SSID_AREA_X, SSID_AREA_Y, SSID_AREA_W, SSID_AREA_H, SCREEN_BG);
            if (label.length()>0) {
                drawString(label.c_str(), SSID_AREA_X, SSID_AREA_Y, SSID_FG);
            }
        }
    }
#endif //BLUETOOTH_FEATURE
    String s;
    if (refresh_signal || force) {

        //set current font size
        setTextFont(FONTSIGNAL);
        fillRect(SIGNAL_X, SIGNAL_Y, SIGNAL_W, SIGNAL_H,SCREEN_BG);
        fillRect(SIGNAL_ICON_X, SIGNAL_ICON_Y, SIGNAL_ICON_W, SIGNAL_ICON_H,SCREEN_BG);
        if (sig > 0) {
            //Signal %
            s = String(sig);
            s+="%";
            //Signal Icon according %
            if (sig > 0) {
                fillRect(SIGNAL_ICON_X, SIGNAL_ICON_Y + (SIGNAL_ICON_H * 0.6), SIGNAL_ICON_W_BAR, SIGNAL_ICON_H * 0.4, SIGNAL_FG);
            } else {
                drawRect(SIGNAL_ICON_X, SIGNAL_ICON_Y + (SIGNAL_ICON_H * 0.6), SIGNAL_ICON_W_BAR, SIGNAL_ICON_H * 0.4, SIGNAL_FG);
            }

            if (sig >= 25) {
                fillRect(SIGNAL_ICON_X + SIGNAL_ICON_SPACER_X +SIGNAL_ICON_W_BAR, SIGNAL_ICON_Y + (SIGNAL_ICON_H * 0.4), SIGNAL_ICON_W_BAR, (SIGNAL_ICON_H * 0.6), SIGNAL_FG);
            } else {
                drawRect(SIGNAL_ICON_X + SIGNAL_ICON_SPACER_X + SIGNAL_ICON_W_BAR, SIGNAL_ICON_Y + (SIGNAL_ICON_H * 0.4), SIGNAL_ICON_W_BAR, (SIGNAL_ICON_H * 0.6), SIGNAL_FG);
            }

            if (sig >= 50) {
                fillRect(SIGNAL_ICON_X + (2*(SIGNAL_ICON_SPACER_X + SIGNAL_ICON_W_BAR)), SIGNAL_ICON_Y + (SIGNAL_ICON_H * 0.2), SIGNAL_ICON_W_BAR, (SIGNAL_ICON_H * 0.8), SIGNAL_FG);
            } else {
                drawRect(SIGNAL_ICON_X + (2*(SIGNAL_ICON_SPACER_X + SIGNAL_ICON_W_BAR)), SIGNAL_ICON_Y + (SIGNAL_ICON_H * 0.2), SIGNAL_ICON_W_BAR, (SIGNAL_ICON_H * 0.8), SIGNAL_FG);
            }

            if (sig >= 75) {
                fillRect(SIGNAL_ICON_X + (3*(SIGNAL_ICON_SPACER_X + SIGNAL_ICON_W_BAR)), SIGNAL_ICON_Y, SIGNAL_ICON_W_BAR, (SIGNAL_ICON_H), SIGNAL_FG);
            } else {
                drawRect(SIGNAL_ICON_X + (3*(SIGNAL_ICON_SPACER_X + SIGNAL_ICON_W_BAR)), SIGNAL_ICON_Y, SIGNAL_ICON_W_BAR, (SIGNAL_ICON_H), SIGNAL_FG);
            }

        }
        //No signal / no connection
        if (sig == -1) {
            s = " X";
        }
        //Ethernet is connected
        if (sig == -2) {
            s = "Eth";
        }
        //BT is active
        if (sig == -3) {
            s = "BT";
        }
        //Show Connection type
        drawString(s.c_str(), SIGNAL_X, SIGNAL_Y, SIGNAL_FG);
    }
    if (refresh_signal || refresh_label || force) {
        return true;
    } else {
        return false;
    }
}

bool Display::displayIP(bool force)
{
    bool refresh_label = force;
    static String label;
#if defined (WIFI_FEATURE) || defined (ETH_FEATURE) || defined (BLUETOOTH_FEATURE)
    if (NetConfig::started()) {
        String s;
        switch(NetConfig::getMode()) {
#if defined (WIFI_FEATURE)
        case ESP_WIFI_STA:
            s = WiFi.localIP().toString();
            break;
        case ESP_AP_SETUP:
        case ESP_WIFI_AP:
            s = WiFi.softAPIP().toString();
            break;
#endif //WIFI_FEATURE   
#if defined (ETH_FEATURE)
        case ESP_ETH_STA:
            s = ETH.localIP().toString();
            break;
#endif //ETH_FEATURE
#if defined (BLUETOOTH_FEATURE)
        case ESP_BT:
            s = bt_service.isConnected()?"Connected":"";
            break;
#endif //BLUETOOTH_FEATURE
        default:
            s="";
            log_esp3d("Unknown mode %d", NetConfig::getMode());
            break;
        }
        if (s!= label) {
            label = s;
            refresh_label = true;
        }
        if (refresh_label) {
            if (label.length()>0) {
                setTextFont(FONTIP);
                fillRect(IP_AREA_X, IP_AREA_Y, IP_AREA_W, IP_AREA_H, SCREEN_BG);
                drawString(label.c_str(), IP_AREA_X, IP_AREA_Y, IP_FG);
            }

        }
    } else {
        if (label != "") {
            label = "";
            refresh_label = true;
            fillRect(IP_AREA_X, IP_AREA_Y, IP_AREA_W, IP_AREA_H, SCREEN_BG);
        }
    }
#endif //WIFI_FEATURE || ETH_FEATURE || BLUETOOTH_FEATURE
    return refresh_label;
}

#endif //DISPLAY_DEVICE && DISPLAY_DEVICE == OLED_I2C_SSD1306_128X64
