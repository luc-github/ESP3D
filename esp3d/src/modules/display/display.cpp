/*
  display.cpp -  display functions class

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

#include "../../include/esp3d_config.h"
#if defined (DISPLAY_DEVICE)
#include "display.h"
#include "../../core/settings_esp3d.h"
#include "../../core/esp3doutput.h"

#if DISPLAY_DEVICE == OLED_I2C_SSD1306 || DISPLAY_DEVICE == OLED_I2C_SSDSH1106
#include "Wire.h"
#include "esp3d_logo.h"
#if DISPLAY_DEVICE == OLED_I2C_SSD1306
#include <SSD1306.h>
SSD1306  esp_display(DISPLAY_I2C_ADDR, DISPLAY_I2C_PIN_SDA, DISPLAY_I2C_PIN_SCL);
#endif //DISPLAY_DEVICE == OLED_I2C_SSD1306
#if DISPLAY_DEVICE == OLED_I2C_SSDSH1106
#include <SH1106.h>
SH1106  esp_display(DISPLAY_I2C_ADDR, (DISPLAY_I2C_PIN_SDA==-1)?SDA:DISPLAY_I2C_PIN_SDA, (DISPLAY_I2C_PIN_SCL==-1)?SCL:DISPLAY_I2C_PIN_SCL);
#endif //DISPLAY_DEVICE == OLED_I2C_SSDSH1106
#endif //DISPLAY_DEVICE == OLED_I2C_SSD1306 || DISPLAY_DEVICE == OLED_I2C_SSDSH1106
#if DISPLAY_DEVICE == TFT_SPI_ILI9341_320X240
#include <TFT_eSPI.h>
TFT_eSPI esp_display = TFT_eSPI();
#include "esp3d_logob.h"
#endif //TFT_SPI_ILI9341_240X320



#define DISPLAY_REFRESH_TIME 1000

Display esp3d_display;

bool Display::splash()
{
    if (!_splash_displayed) {
#if DISPLAY_DEVICE == OLED_I2C_SSD1306 || DISPLAY_DEVICE == OLED_I2C_SSDSH1106
        esp_display.drawXbm((_screenwidth-ESP3D_Logo_width)/2, (_screenheight-ESP3D_Logo_height)/2, ESP3D_Logo_width, ESP3D_Logo_height, ESP3D_Logo);
#endif //DISPLAY_DEVICE == OLED_I2C_SSD1306 || DISPLAY_DEVICE == OLED_I2C_SSDSH1106
#if DISPLAY_DEVICE == TFT_SPI_ILI9341_320X240
        esp_display.drawXBitmap((_screenwidth-ESP3D_Logo_width)/2, (_screenheight-ESP3D_Logo_height)/2, ESP3D_Logo, ESP3D_Logo_width, ESP3D_Logo_height, TFT_WHITE);
#endif //TFT_SPI_ILI9341_240X320
        log_esp3d("Splash");
        _splash_displayed = true;
        return true;
    }
    return false;
}

void Display::show_screenID(uint8_t screenID)
{
    _screenID = screenID;
}

Display::Display()
{
    _started = false;
    _screenID = SPLASH_SCREEN;
    _splash_displayed=false;
#if DISPLAY_DEVICE == OLED_I2C_SSD1306
    _screenwidth = 128;
    _screenheight = 64;
#endif //OLED_I2C_SSD1306
#if DISPLAY_DEVICE == OLED_I2C_SSDSH1106
    _screenwidth = 132;
    _screenheight = 64;
#endif //OLED_I2C_SSDSH1106
#if DISPLAY_DEVICE == TFT_SPI_ILI9341_320X240
    _screenwidth = 320;
    _screenheight = 240;
#endif //TFT_SPI_ILI9341_240X320
}
Display::~Display()
{
    end();
}

bool Display::begin()
{
    bool res = true;
    _started = false;
#if DISPLAY_DEVICE == OLED_I2C_SSD1306 || DISPLAY_DEVICE == OLED_I2C_SSDSH1106
#if defined(DISPLAY_I2C_PIN_RST)
    pinMode(DISPLAY_I2C_PIN_RST,OUTPUT);
    digitalWrite(DISPLAY_I2C_PIN_RST, LOW);   // turn the LED on (HIGH is the voltage level)
    delay(10);                       // wait for a second
    digitalWrite(DISPLAY_I2C_PIN_RST, HIGH);    // turn the LED off by making the voltage LOW
#endif //DISPLAY_I2C_PIN_RST
    log_esp3d("Init Display");
    esp_display.init();
    esp_display.clear();
#if defined(DISPLAY_FLIP_VERTICALY)
    esp_display.flipScreenVertically();
#endif
#endif //DISPLAY_DEVICE == OLED_I2C_SSD1306 || DISPLAY_DEVICE == OLED_I2C_SSDSH1106
#if DISPLAY_DEVICE == TFT_SPI_ILI9341_320X240
    esp_display.begin();               // Initialise the display
    esp_display.setRotation(3);
    esp_display.fillScreen(TFT_BLACK); // Black screen fill
#endif //TFT_SPI_ILI9341_320X240 
    show_screenID(SPLASH_SCREEN);
    update_screen();
    res = true;
    if (!res) {
        end();
    }
    _started = res;
    return _started;
}
void Display::end()
{
    if(!_started) {
        return;
    }
    _started = false;
    _screenID = SPLASH_SCREEN;
    _splash_displayed=false;
    clear_screen();
}

void Display::clear_screen()
{
#if DISPLAY_DEVICE == OLED_I2C_SSD1306 || DISPLAY_DEVICE == OLED_I2C_SSDSH1106
    esp_display.clear();
#endif //#if DISPLAY_DEVICE == OLED_I2C_SSD1306 || DISPLAY_DEVICE == OLED_I2C_SSDSH1106
#if DISPLAY_DEVICE == TFT_SPI_ILI9341_320X240
    esp_display.fillScreen(TFT_BLACK); // Black screen fill
#endif //TFT_SPI_ILI9341_240X320
}

void Display::update_screen()
{
    bool need_update = false;
    switch(_screenID) {
    case SPLASH_SCREEN:
        need_update = splash();
        break;
    case MAIN_SCREEN:
        log_esp3d("Mainscreen");
        clear_screen();
        need_update =  true;
        break;
    default:
        break;
    }
    if (need_update) {
#if DISPLAY_DEVICE == OLED_I2C_SSD1306 || DISPLAY_DEVICE == OLED_I2C_SSDSH1106
        esp_display.display();
#endif //DISPLAY_DEVICE == OLED_I2C_SSD1306 || DISPLAY_DEVICE == OLED_I2C_SSDSH1106
    }
}

void Display::handle()
{
    static uint32_t last_update = millis();

    if (_started) {
        if ((millis()- last_update) > DISPLAY_REFRESH_TIME) {
            last_update = millis();
            update_screen();
        }
    }
}

#endif //DISPLAY_DEVICE
