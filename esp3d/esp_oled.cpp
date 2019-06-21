/*
  esp_oled.cpp - ESP3D oled class

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
#include "config.h"
#ifdef ESP_OLED_FEATURE
#include "esp_oled.h"
#include "Wire.h"


// Initialize the OLED display using I2C
#ifdef OLED_DISPLAY_SSD1306
#include "SSD1306.h"   // alias for `#include "SSD1306Wire.h"`
#elif defined OLED_DISPLAY_SH1106
#include "SH1106.h"  // alias for `#include "SH1106Wire.h"`
#endif


#ifdef OLED_DISPLAY_SSD1306
SSD1306  esp_display(OLED_ADDR, OLED_PIN_SDA, OLED_PIN_SCL);
#elif defined OLED_DISPLAY_SH1106
SH1106 esp_display(OLED_ADDR, OLED_PIN_SDA, OLED_PIN_SCL);
#endif


#define ESP3D_Logo_width 62
#define ESP3D_Logo_height 45
const char  ESP3D_Logo[]  = {
    0x00, 0x00, 0x00, 0xFE, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xFF,
    0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE0, 0xFF, 0xFF, 0x01, 0x00, 0x00,
    0x00, 0x00, 0xF8, 0xFF, 0xFF, 0x07, 0x00, 0x00, 0x00, 0x00, 0xFE, 0xFF,
    0xFF, 0x1F, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x3F, 0x00, 0x00,
    0x00, 0x80, 0xFF, 0xFF, 0xFF, 0x7F, 0x00, 0x00, 0x00, 0xC0, 0xFF, 0xFF,
    0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x3F, 0x00, 0xC0, 0xFF, 0xFF, 0xFF,
    0xFF, 0x00, 0xE0, 0x00, 0xE0, 0xFF, 0xFF, 0xFF, 0x3F, 0x00, 0x80, 0x01,
    0xF0, 0xFF, 0xFF, 0xFF, 0x0F, 0x00, 0x00, 0x03, 0xF8, 0xFF, 0xFF, 0xFF,
    0x07, 0x00, 0x00, 0x06, 0xFC, 0xFF, 0xFF, 0xFF, 0x03, 0x00, 0x00, 0x0C,
    0x0C, 0x7C, 0x78, 0xC0, 0xC1, 0xC1, 0x0F, 0x18, 0x06, 0x38, 0x60, 0x80,
    0xE1, 0xC3, 0x3F, 0x10, 0xC6, 0x19, 0x67, 0x1C, 0xF1, 0xC7, 0x7F, 0x10,
    0xC6, 0x1F, 0x7F, 0x3C, 0x31, 0xCF, 0xF1, 0x10, 0xC7, 0x1F, 0x7F, 0x3C,
    0x01, 0xCE, 0xE1, 0x20, 0xC7, 0x3F, 0x7E, 0x1C, 0x01, 0xC7, 0xC1, 0x21,
    0x07, 0x3C, 0x78, 0x80, 0xE1, 0xC3, 0xC1, 0x21, 0x07, 0xFC, 0x70, 0xC0,
    0xE1, 0xC7, 0xC1, 0x21, 0xC7, 0xFF, 0x63, 0xFC, 0x01, 0xCF, 0xC1, 0x21,
    0xC7, 0xFF, 0x63, 0xFC, 0x01, 0xCE, 0xC1, 0x21, 0xC7, 0xDF, 0x63, 0xFC,
    0x01, 0xCE, 0xE1, 0x20, 0xC6, 0x99, 0x73, 0xFC, 0x31, 0xCF, 0x7F, 0x10,
    0x06, 0x18, 0x70, 0xFC, 0xF0, 0xC7, 0x3F, 0x10, 0x0E, 0x78, 0x78, 0xFC,
    0xE0, 0xC3, 0x0F, 0x10, 0xFC, 0xFF, 0xFF, 0x7F, 0x00, 0x00, 0x00, 0x18,
    0xFC, 0xFF, 0xFF, 0x7F, 0x00, 0x00, 0x00, 0x0C, 0xF8, 0xFF, 0xFF, 0x3F,
    0x00, 0x00, 0x00, 0x06, 0xF0, 0xFF, 0xFF, 0x1F, 0x00, 0x00, 0x00, 0x03,
    0xE0, 0xFF, 0xFF, 0x0F, 0x00, 0x00, 0x80, 0x01, 0xC0, 0xFF, 0xFF, 0x03,
    0x00, 0x00, 0xE0, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x3F, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0xFF, 0xFF,
    0xFF, 0xFF, 0x00, 0x00, 0x00, 0x80, 0xFF, 0xFF, 0xFF, 0x7F, 0x00, 0x00,
    0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x3F, 0x00, 0x00, 0x00, 0x00, 0xFE, 0xFF,
    0xFF, 0x1F, 0x00, 0x00, 0x00, 0x00, 0xFC, 0xFF, 0xFF, 0x07, 0x00, 0x00,
    0x00, 0x00, 0xF0, 0xFF, 0xFF, 0x03, 0x00, 0x00, 0x00, 0x00, 0xC0, 0xFF,
    0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFE, 0x1F, 0x00, 0x00, 0x00
};

void OLED_DISPLAY::splash()
{
    if ( CONFIG::is_locked(FLAG_BLOCK_OLED)) {
        return;
    }
    esp_display.drawXbm(33, 10, ESP3D_Logo_width, ESP3D_Logo_height, ESP3D_Logo);
    update_lcd();
}

void OLED_DISPLAY::begin()
{
    //For Embeded OLED on Wifi kit 32
#if HELTEC_EMBEDDED_PIN > 0
    pinMode(HELTEC_EMBEDDED_PIN,OUTPUT);
    digitalWrite(HELTEC_EMBEDDED_PIN, LOW);   // turn the LED on (HIGH is the voltage level)
    delay(100);                       // wait for a second
    digitalWrite(HELTEC_EMBEDDED_PIN, HIGH);    // turn the LED off by making the voltage LOW
#endif
    esp_display.init();
    esp_display.clear();
#if OLED_FLIP_VERTICALY
    esp_display.flipScreenVertically();
#endif
}

void OLED_DISPLAY::setCursor(int col, int row)
{
    if (col!=-1) {
        OLED_DISPLAY::col = col;
    }
    if (row!=-1) {
        OLED_DISPLAY::row = row;
    }
}
void OLED_DISPLAY::print(String & s)
{
    OLED_DISPLAY::print(s.c_str());
}
void OLED_DISPLAY::print(const char * s)
{
    display_text(s, col, row);
}

void OLED_DISPLAY::display_signal(int value, int x, int y)
{
    if(CONFIG::is_locked(FLAG_BLOCK_OLED)) {
        return;
    }
    //clear area only
    esp_display.setColor(BLACK);
    esp_display.fillRect(x, y, x + 46, 16);
    esp_display.setColor(WHITE);
    if (value == -1) {

        esp_display.setFont(ArialMT_Plain_10);
        esp_display.drawString(x+20, y, "X");

    } else {
        if (value > 0) {
            esp_display.fillRect(x, y + 6, 3, 4);
        } else {
            esp_display.drawRect(x, y + 6, 3, 4);
        }

        if (value >= 25) {
            esp_display.fillRect(x + 4, y + 4, 3, 6);
        } else {
            esp_display.drawRect(x + 4, y + 4, 3, 6);
        }

        if (value >= 50) {
            esp_display.fillRect(x + 8, y + 2, 3, 8);
        } else {
            esp_display.drawRect(x + 8, y + 2, 3, 8);
        }

        if (value >= 75) {
            esp_display.fillRect(x + 12, y, 3, 10);
        } else {
            esp_display.drawRect(x + 12, y, 3, 10);
        }

        String s = CONFIG::intTostr (value);
        s+="%";
        //set current font size
        esp_display.setFont(ArialMT_Plain_10);
        esp_display.drawString(x+16, y, s.c_str());
    }
}


void OLED_DISPLAY::display_progress(int value, int x, int y)
{
    if ( CONFIG::is_locked(FLAG_BLOCK_OLED)) {
        return;
    }
    esp_display.setFont(ArialMT_Plain_10);
    esp_display.setColor(BLACK);
    esp_display.fillRect(x, y, x + 128, 16);
    esp_display.setColor(WHITE);
    esp_display.drawProgressBar(x, y, 100, 10, value);
    String p = String(value) + "%";
    esp_display.drawString(x+102, y - 1, p.c_str());
}

void OLED_DISPLAY::display_mini_progress(int value, int x, int y, int w)
{
    if ( CONFIG::is_locked(FLAG_BLOCK_OLED)) {
        return;
    }
    esp_display.setColor(BLACK);
    esp_display.fillRect(x, y, x + w, 2);
    esp_display.setColor(WHITE);
    esp_display.drawRect(x, y, value, 2);
}

//max is 128 by default but can be 85 for first line

void OLED_DISPLAY::display_text(const char * txt, int x, int y, int max)
{
    static int shift_pos[4] = {-1, -1, -1, -1};
    static int t[4] = {0, 0, 0, 0};

    int p = 0;
    if (y==16) {
        OLED_DISPLAY::L1 = txt;
        OLED_DISPLAY::L1_size = esp_display.getStringWidth( txt);
        p=1;
    } else if (y==32) {
        OLED_DISPLAY::L2 = txt;
        OLED_DISPLAY::L2_size = esp_display.getStringWidth( txt);
        p=2;
    } else if (y==0) {
        max = 85;
        OLED_DISPLAY::L0 = txt;
        OLED_DISPLAY::L0_size = esp_display.getStringWidth( txt);
        p=0;
    } else {
        OLED_DISPLAY::L3 = txt;
        OLED_DISPLAY::L3_size = esp_display.getStringWidth( txt);
        p=3;
    }
    esp_display.setFont(ArialMT_Plain_10);
    //clear area only
    esp_display.setColor(BLACK);
    esp_display.fillRect(x, y, max, 16);
    esp_display.setColor(WHITE);
    String Stxt = txt;
    Stxt += " ";
    (t[p])++;
    if ((esp_display.getStringWidth( Stxt) > max) && (t[p] > 1)) {
        (shift_pos[p]) ++;
        String s2 = Stxt.substring(shift_pos[p]);
        Stxt = s2;
        if (esp_display.getStringWidth( s2) < max) {
            //reset for next time
            shift_pos[p] = -1;
            t[p] = 0;
        }
    }
    //be sure we stay in boundaries
    while (esp_display.getStringWidth(Stxt) > max) {
        Stxt.remove(Stxt.length()-1, 1);
    }
    esp_display.drawString(x, y, Stxt.c_str());
}

void OLED_DISPLAY::update_lcd()
{
    esp_display.display();
}
void OLED_DISPLAY::clear_lcd()
{
    esp_display.clear();
}
int OLED_DISPLAY::col = 0;
int OLED_DISPLAY::row = 0;
String OLED_DISPLAY::L0 = "";
String OLED_DISPLAY::L1 = "";
String OLED_DISPLAY::L2 = "";
String OLED_DISPLAY::L3 = "";
int OLED_DISPLAY::L0_size = 0;
int OLED_DISPLAY::L1_size = 0;
int OLED_DISPLAY::L2_size = 0;
int OLED_DISPLAY::L3_size = 0;
#endif
