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
SSD1306  esp3d_screen(DISPLAY_I2C_ADDR, DISPLAY_I2C_PIN_SDA, DISPLAY_I2C_PIN_SCL);
#include "OLED_SSD1306.h"
#endif //DISPLAY_DEVICE == OLED_I2C_SSD1306
#if DISPLAY_DEVICE == OLED_I2C_SSDSH1106
#include <SH1106.h>
SH1106  esp3d_screen(DISPLAY_I2C_ADDR, (DISPLAY_I2C_PIN_SDA==-1)?SDA:DISPLAY_I2C_PIN_SDA, (DISPLAY_I2C_PIN_SCL==-1)?SCL:DISPLAY_I2C_PIN_SCL);
#include "OLED_SSDSH1106.h"
#endif //DISPLAY_DEVICE == OLED_I2C_SSDSH1106
#endif //DISPLAY_DEVICE == OLED_I2C_SSD1306 || DISPLAY_DEVICE == OLED_I2C_SSDSH1106
#if DISPLAY_DEVICE == TFT_SPI_ILI9341_320X240
#include <TFT_eSPI.h>
TFT_eSPI esp3d_screen = TFT_eSPI();
#include "esp3d_logob.h"
#include "TFT_ILI9341_240X320.h"
#endif //TFT_SPI_ILI9341_240X320
#if defined (WIFI_FEATURE)
#include "../wifi/wificonfig.h"
#endif // WIFI_FEATURE
#if defined (ETH_FEATURE)
#include "../ethernet/ethconfig.h"
#endif //ETH_FEATURE
#if defined (BLUETOOTH_FEATURE)
#include "../bluetooth/BT_service.h"
#endif //BLUETOOTH_FEATURE
#if defined (WIFI_FEATURE) || defined (ETH_FEATURE) || defined (BLUETOOTH_FEATURE)
#include "../network/netconfig.h"
#endif //WIFI_FEATURE || ETH_FEATURE) ||BLUETOOTH_FEATURE
#define DISPLAY_REFRESH_TIME 1000

Display esp3d_display;

bool Display::splash()
{
	log_esp3d("Splash");
    if ( !ESP3DOutput::isOutput(ESP_SCREEN_CLIENT)) {
        return false;
    }
    if (!_splash_displayed) {
        drawXbm((_screenwidth-ESP3D_Logo_width)/2, (_screenheight-ESP3D_Logo_height)/2, ESP3D_Logo_width, ESP3D_Logo_height, SPLASH_FG, SPLASH_BG ,ESP3D_Logo);
        log_esp3d("Display Splash");
        _splash_displayed = true;
        return true;
    }
    return false;
}


void Display::showStatus()
{
		//Display Status
		String status = _status;
		setTextFont(FONTSTATUS);
		uint16_t size = sizetoFitSpace(status.c_str(), STATUS_AREA_W);
		//check the need for resize 
		if (size < status.length()){
			static int status_shift = -1;
			status+=" ";
			log_esp3d("current %s", status.c_str());
			if (status_shift > status.length()){
				status_shift = -1;
			}
			log_esp3d("shift %d", status_shift);
			if (status_shift > 0){
				status.remove(0,status_shift);
			}
			log_esp3d("shifted %s", status.c_str());
			size = sizetoFitSpace(status.c_str(), STATUS_AREA_W);
			log_esp3d("size available %d existing %d",size, status.length());
			if (size < status.length()){
				//cut
				status = status.substring(0,size);
				status_shift++;
			} else{
				status_shift = -1;
			}
			log_esp3d("sized %s", status.c_str());
		}
		//clear area
		fillRect(STATUS_AREA_X, STATUS_AREA_Y, STATUS_AREA_W, STATUS_AREA_H, SCREEN_BG);
		drawString(status.c_str(), STATUS_AREA_X, STATUS_AREA_Y, STATUS_FG);
}

bool Display::main_screen()
{
	log_esp3d("Mainscreen");
	//Signal
	int sig = -1;
	
#if defined (WIFI_FEATURE)
	if (WiFiConfig::started()){
		String ssid;
		if (WiFi.getMode() == WIFI_AP) {
			sig = 100;
			ssid = WiFiConfig::AP_SSID();
		} else {
			if (WiFi.isConnected()) {
               sig = WiFiConfig::getSignal (WiFi.RSSI ());
               ssid = WiFi.SSID();
            }
		}
		
		//Display SSID
		setTextFont(FONTSSID);
		uint16_t size = sizetoFitSpace(ssid.c_str(), SSID_AREA_W);
		//check the need for resize 
		if (size < ssid.length()){
			static int ssid_shift = -1;
			ssid+=" ";
			log_esp3d("current %s", ssid.c_str());
			if (ssid_shift > ssid.length()){
				ssid_shift = -1;
			}
			log_esp3d("shift %d", ssid_shift);
			if (ssid_shift > 0){
				ssid.remove(0,ssid_shift);
			}
			log_esp3d("shifted %s", ssid.c_str());
			size = sizetoFitSpace(ssid.c_str(), SSID_AREA_W);
			log_esp3d("size available %d existing %d",size, ssid.length());
			if (size < ssid.length()){
				//cut
				ssid = ssid.substring(0,size);
				ssid_shift++;
			} else{
				ssid_shift = -1;
			}
			log_esp3d("sized %s", ssid.c_str());
		}
		//clear area
		fillRect(SSID_AREA_X, SSID_AREA_Y, SSID_AREA_W, SSID_AREA_H, SCREEN_BG);
		drawString(ssid.c_str(), SSID_AREA_X, SSID_AREA_Y, SSID_FG);
	}
#endif // WIFI_FEATURE
#if defined (ETH_FEATURE)
	if (EthConfig::started()){
		sig = -2;
		setTextFont(FONTSSID);
		//clear area
		fillRect(SSID_AREA_X, SSID_AREA_Y, SSID_AREA_W, SSID_AREA_H, SCREEN_BG);
		//display connection speed
		if(ETH.linkUp()){
			ssid = ETH.linkSpeed();
			ssid+= "Mbps";	
			drawString(ssid.c_str(), SSID_AREA_X, SSID_AREA_Y, SSID_FG);
		}
	}
#endif //ETH_FEATURE
#if defined (BLUETOOTH_FEATURE)
	if (bt_service.started()){
		sig = -3;
		//Display hostname
		String hostname = bt_service.hostname();
		setTextFont(FONTSSID);
		uint16_t size = sizetoFitSpace(hostname.c_str(), SSID_AREA_W);
		//check the need for resize 
		if (size < hostname.length()){
			static int hostname_shift = -1;
			hostname+=" ";
			log_esp3d("current %s", hostname.c_str());
			if (hostname_shift > hostname.length()){
				hostname_shift = -1;
			}
			log_esp3d("shift %d", hostname_shift);
			if (hostname_shift > 0){
				hostname.remove(0,hostname_shift);
			}
			log_esp3d("shifted %s", hostname.c_str());
			size = sizetoFitSpace(hostname.c_str(), SSID_AREA_W);
			log_esp3d("size available %d existing %d",size, hostname.length());
			if (size < hostname.length()){
				//cut
				hostname = hostname.substring(0,size);
				hostname_shift++;
			} else{
				hostname_shift = -1;
			}
			log_esp3d("sized %s", hostname.c_str());
		}
		//clear area
		fillRect(SSID_AREA_X, SSID_AREA_Y, SSID_AREA_W, SSID_AREA_H, SCREEN_BG);
		if (hostname.length()>0){
			drawString(hostname.c_str(), SSID_AREA_X, SSID_AREA_Y, SSID_FG);
		}
	}
#endif //BLUETOOTH_FEATURE
	//set current font size
	setTextFont(FONTSIGNAL);
	fillRect(SIGNAL_X, SIGNAL_Y, SIGNAL_W, SIGNAL_H,SCREEN_BG);
	fillRect(SIGNAL_ICON_X, SIGNAL_ICON_Y, SIGNAL_ICON_W, SIGNAL_ICON_H,SCREEN_BG);
	String s;
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
	
	//IP
	setTextFont(FONTIP);
	fillRect(IP_AREA_X, IP_AREA_Y, IP_AREA_W, IP_AREA_H, SCREEN_BG);
#if defined (WIFI_FEATURE) || defined (ETH_FEATURE) || defined (BLUETOOTH_FEATURE)
	if (NetConfig::started()){
		switch(NetConfig::getMode()){
#if defined (WIFI_FEATURE)
			case ESP_WIFI_STA:
				s = WiFi.localIP().toString();
				break;
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
				break;
		}
		if (s.length()>0){
			drawString(s.c_str(), IP_AREA_X, IP_AREA_Y, IP_FG);
		}
	}
#endif //WIFI_FEATURE || ETH_FEATURE || BLUETOOTH_FEATURE
	//status
	showStatus();
	return true;
}

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

void Display::show_screenID(uint8_t screenID)
{
	clear_screen();
    _screenID = screenID;
}

Display::Display()
{
    _started = false;
    _screenID = SPLASH_SCREEN;
    _splash_displayed=false;
    _screenwidth = SCREEN_WIDTH;
    _screenheight = SCREEN_HEIGHT;
}
Display::~Display()
{
    end();
}

bool Display::begin()
{
    bool res = true;
    _started = false;
    log_esp3d("Init Display");
#if DISPLAY_DEVICE == OLED_I2C_SSD1306 || DISPLAY_DEVICE == OLED_I2C_SSDSH1106
#if defined(DISPLAY_I2C_PIN_RST)
    pinMode(DISPLAY_I2C_PIN_RST,OUTPUT);
    digitalWrite(DISPLAY_I2C_PIN_RST, LOW);   // turn the LED on (HIGH is the voltage level)
    delay(10);                       // wait for a second
    digitalWrite(DISPLAY_I2C_PIN_RST, HIGH);    // turn the LED off by making the voltage LOW
#endif //DISPLAY_I2C_PIN_RST
    esp3d_screen.init();
    esp3d_screen.clear();
#if defined(DISPLAY_FLIP_VERTICALY)
    esp3d_screen.flipScreenVertically();
#endif
#endif //DISPLAY_DEVICE == OLED_I2C_SSD1306 || DISPLAY_DEVICE == OLED_I2C_SSDSH1106
#if DISPLAY_DEVICE == TFT_SPI_ILI9341_320X240
    esp3d_screen.begin();               // Initialise the display
#if defined(DISPLAY_FLIP_VERTICALY)
    esp3d_screen.setRotation(3);
#else
    esp3d_screen.setRotation(1);
#endif
    esp3d_screen.fillScreen(SCREEN_BG); // Black screen fill
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
    _status ="";
    _started = false;
    _screenID = SPLASH_SCREEN;
    _splash_displayed=false;
    clear_screen();
}

void Display::SetStatus(const char * status)
{
	_status= status;
}

void Display::clear_screen()
{
	log_esp3d("clear screen");
#if DISPLAY_DEVICE == OLED_I2C_SSD1306 || DISPLAY_DEVICE == OLED_I2C_SSDSH1106
    esp3d_screen.clear();
#endif //#if DISPLAY_DEVICE == OLED_I2C_SSD1306 || DISPLAY_DEVICE == OLED_I2C_SSDSH1106
#if DISPLAY_DEVICE == TFT_SPI_ILI9341_320X240
	log_esp3d("Fill black");
    esp3d_screen.fillScreen(SCREEN_BG); // Black screen fill
#endif //TFT_SPI_ILI9341_240X320
}

void Display::update_screen(bool force)
{
    if ( !ESP3DOutput::isOutput(ESP_SCREEN_CLIENT)) {
        return;
    }
    bool need_update = force;
    switch(_screenID) {
    case SPLASH_SCREEN:
		if (!_splash_displayed) {
			need_update = splash();
		}
        break;
    case MAIN_SCREEN:
        need_update =  main_screen();
        break;
    default:
        break;
    }
    if (need_update) {
#if DISPLAY_DEVICE == OLED_I2C_SSD1306 || DISPLAY_DEVICE == OLED_I2C_SSDSH1106
        esp3d_screen.display();
        log_esp3d("Update display");
#endif //DISPLAY_DEVICE == OLED_I2C_SSD1306 || DISPLAY_DEVICE == OLED_I2C_SSDSH1106
    Hal::wait(0);
    }
}

void Display::handle()
{
    static uint32_t last_update = millis();
    if ( !ESP3DOutput::isOutput(ESP_SCREEN_CLIENT)) {
        return;
    }
    if (_started) {
        if ((millis()- last_update) > DISPLAY_REFRESH_TIME) {
            last_update = millis();
            update_screen();
        }
    }
}

// Draw a line from position 0 to position 1
void Display::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t color)
{
    if ( !ESP3DOutput::isOutput(ESP_SCREEN_CLIENT)) {
        return;
    }
#if DISPLAY_DEVICE == OLED_I2C_SSD1306 || DISPLAY_DEVICE == OLED_I2C_SSDSH1106
    esp3d_screen.setColor((color == TFT_BLACK)?BLACK:WHITE);
    esp3d_screen.drawLine(x0, y0, x1, y1);
#endif //#if DISPLAY_DEVICE == OLED_I2C_SSD1306 || DISPLAY_DEVICE == OLED_I2C_SSDSH1106
#if DISPLAY_DEVICE == TFT_SPI_ILI9341_320X240
    esp3d_screen.drawLine(x0, y0, x1, y1, color);
#endif //TFT_SPI_ILI9341_240X320
}

// Draw the border of a rectangle at the given location
void Display::drawRect(int16_t x, int16_t y, int16_t width, int16_t height, int16_t color)
{
    if ( !ESP3DOutput::isOutput(ESP_SCREEN_CLIENT)) {
        return;
    }
#if DISPLAY_DEVICE == OLED_I2C_SSD1306 || DISPLAY_DEVICE == OLED_I2C_SSDSH1106
    esp3d_screen.setColor((color == TFT_BLACK)?BLACK:WHITE);
    esp3d_screen.drawRect(x, y, width, height);
#endif //#if DISPLAY_DEVICE == OLED_I2C_SSD1306 || DISPLAY_DEVICE == OLED_I2C_SSDSH1106
#if DISPLAY_DEVICE == TFT_SPI_ILI9341_320X240
    esp3d_screen.drawRect(x, y, width, height, color);
#endif //TFT_SPI_ILI9341_240X320
}

// Fill the rectangle
void Display::fillRect(int16_t x, int16_t y, int16_t width, int16_t height, int16_t color)
{
    if ( !ESP3DOutput::isOutput(ESP_SCREEN_CLIENT)) {
        return;
    }
#if DISPLAY_DEVICE == OLED_I2C_SSD1306 || DISPLAY_DEVICE == OLED_I2C_SSDSH1106
    esp3d_screen.setColor((color == TFT_BLACK)?BLACK:WHITE);
    esp3d_screen.fillRect(x, y, width, height);
#endif //#if DISPLAY_DEVICE == OLED_I2C_SSD1306 || DISPLAY_DEVICE == OLED_I2C_SSDSH1106
#if DISPLAY_DEVICE == TFT_SPI_ILI9341_320X240
    esp3d_screen.fillRect(x, y, width, height, color);
#endif //TFT_SPI_ILI9341_240X320
}
void Display::setTextFont(uint8_t font)
{
#if DISPLAY_DEVICE == OLED_I2C_SSD1306 || DISPLAY_DEVICE == OLED_I2C_SSDSH1106
    switch(font) {
	case 3:
		esp3d_screen.setFont(ArialMT_Plain_16);
		break;
    case 2:
    default:
        esp3d_screen.setFont(ArialMT_Plain_10);
    }
#endif //#if DISPLAY_DEVICE == OLED_I2C_SSD1306 || DISPLAY_DEVICE == OLED_I2C_SSDSH1106
#if DISPLAY_DEVICE == TFT_SPI_ILI9341_320X240
	esp3d_screen.setTextFont(font);
#endif //TFT_SPI_ILI9341_240X320
}
void Display::drawString(const char *string, int32_t poX, int32_t poY, int16_t color)
{
    if ( !ESP3DOutput::isOutput(ESP_SCREEN_CLIENT)) {
        return;
    }
#if DISPLAY_DEVICE == OLED_I2C_SSD1306 || DISPLAY_DEVICE == OLED_I2C_SSDSH1106
    esp3d_screen.setColor((color == TFT_BLACK)?BLACK:WHITE);
    esp3d_screen.drawString(poX, poY, string);
#endif //#if DISPLAY_DEVICE == OLED_I2C_SSD1306 || DISPLAY_DEVICE == OLED_I2C_SSDSH1106
#if DISPLAY_DEVICE == TFT_SPI_ILI9341_320X240
	esp3d_screen.setTextColor(color);
    esp3d_screen.drawString(string, poX, poY);
#endif //TFT_SPI_ILI9341_240X320
}

// Draw a XBM
void Display::drawXbm(int16_t x, int16_t y, int16_t width, int16_t height, int16_t color, const unsigned char *xbm)
{
    if ( !ESP3DOutput::isOutput(ESP_SCREEN_CLIENT)) {
        return;
    }
#if DISPLAY_DEVICE == OLED_I2C_SSD1306 || DISPLAY_DEVICE == OLED_I2C_SSDSH1106
    (void)color;
    esp3d_screen.drawXbm(x, y, width, height, (const char *)xbm);
#endif //#if DISPLAY_DEVICE == OLED_I2C_SSD1306 || DISPLAY_DEVICE == OLED_I2C_SSDSH1106
#if DISPLAY_DEVICE == TFT_SPI_ILI9341_320X240
    esp3d_screen.drawXBitmap(x, y, xbm, width, height,color);
#endif //TFT_SPI_ILI9341_240X320
}



void Display::drawXbm(int16_t x, int16_t y, int16_t width, int16_t height, uint16_t fgcolor, uint16_t bgcolor, const unsigned char *xbm)
{
    if ( !ESP3DOutput::isOutput(ESP_SCREEN_CLIENT)) {
        return;
    }
#if DISPLAY_DEVICE == OLED_I2C_SSD1306 || DISPLAY_DEVICE == OLED_I2C_SSDSH1106
    (void)fgcolor;
    (void)bgcolor;
    esp3d_screen.drawXbm(x, y, width, height, (const char *)xbm);
#endif //#if DISPLAY_DEVICE == OLED_I2C_SSD1306 || DISPLAY_DEVICE == OLED_I2C_SSDSH1106
#if DISPLAY_DEVICE == TFT_SPI_ILI9341_320X240
    esp3d_screen.drawXBitmap(x, y, xbm, width, height, fgcolor, bgcolor);
#endif //TFT_SPI_ILI9341_240X320
}

uint16_t Display::getStringWidth(const char* text)
{
#if DISPLAY_DEVICE == OLED_I2C_SSD1306 || DISPLAY_DEVICE == OLED_I2C_SSDSH1106
    return  esp3d_screen.getStringWidth(text, strlen(text));
#endif //#if DISPLAY_DEVICE == OLED_I2C_SSD1306 || DISPLAY_DEVICE == OLED_I2C_SSDSH1106
#if DISPLAY_DEVICE == TFT_SPI_ILI9341_320X240
    return esp3d_screen.textWidth(text);
#endif //TFT_SPI_ILI9341_240X320
}

void Display::progress(uint8_t v)
{
    if ( !ESP3DOutput::isOutput(ESP_SCREEN_CLIENT)) {
        return;
    }
    static uint8_t previous = 0;
    if (previous > v) {
		//clear
		fillRect(10, _screenheight-2, _screenwidth-20, 2, SCREEN_BG);
	}
	log_esp3d("%d", v);
	previous = v;
    //display bar
    drawRect(10, _screenheight-2, ((_screenwidth-20) * v)/100, 2, PROGRESS_FG);
    //update screen
    update_screen(true);
}

void display_progress(uint8_t v)
{
    esp3d_display.progress(v);
}

#endif //DISPLAY_DEVICE
