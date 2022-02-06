/*
  advanceddisplay.cpp -  display functions class

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

#include "../../include/esp3d_config.h"
#if defined (DISPLAY_DEVICE) && (DISPLAY_UI_TYPE == UI_TYPE_ADVANCED)

//to get bmp file use ImageMagick
//Under Windows:
//ImageMagick-7.0.8-Q16>magick.exe -size 480x320 -depth 8 bgra:snapshot.bin snap.bmp
//On others systems:
//convert -size 480x320 -depth 8 bgra:snapshot.bin snap.bmp
#define SNAPFILENAME "/snapshot.bin"

#include "advanceddisplay.h"
#include "../../core/settings_esp3d.h"
#include "../../core/esp3doutput.h"
#include "../filesystem/esp_filesystem.h"
#include <lvgl.h>
#include <Ticker.h>
#include "lv_flash_drv.h"
//screen  object
lv_obj_t * esp_lv_screen;
lv_obj_t * esp_lv_bar_progression;
lv_obj_t * esp_lv_status_label;
lv_obj_t * esp_lv_IP_label;
lv_obj_t * esp_lv_network_label;

#if defined(DISPLAY_SNAPSHOT_FEATURE)
static ESP_File fsSnapFile;
static bool bSnapshot;
#endif //DISPLAY_SNAPSHOT_FEATURE

#if DISPLAY_DEVICE == OLED_I2C_SSD1306 || DISPLAY_DEVICE == OLED_I2C_SSDSH1106
#include "Wire.h"
#if DISPLAY_DEVICE == OLED_I2C_SSD1306
#include <SSD1306Wire.h>
SSD1306Wire  esp3d_screen(DISPLAY_I2C_ADDR, ESP_SDA_PIN, ESP_SCL_PIN);
#include "advanced_SSD1306.h"
#endif //DISPLAY_DEVICE == OLED_I2C_SSD1306
#if DISPLAY_DEVICE == OLED_I2C_SSDSH1106
#include <SH1106Wire.h>
SH1106Wire  esp3d_screen(DISPLAY_I2C_ADDR, ESP_SDA_PIN, ESP_SCL_PIN);
#include "advanced_SSDSH1106.h"
#endif //DISPLAY_DEVICE == OLED_I2C_SSDSH1106
#endif //DISPLAY_DEVICE == OLED_I2C_SSD1306 || DISPLAY_DEVICE == OLED_I2C_SSDSH1106
#if (DISPLAY_DEVICE == TFT_SPI_ILI9341_320X240) || (DISPLAY_DEVICE == TFT_SPI_ILI9488_480X320)
#include <TFT_eSPI.h>
TFT_eSPI esp3d_screen = TFT_eSPI();
#if (DISPLAY_DEVICE == TFT_SPI_ILI9341_320X240)
#include "advanced_ILI9341_320X240.h"
#endif //(DISPLAY_DEVICE == TFT_SPI_ILI9341_320X240)
#if (DISPLAY_DEVICE == TFT_SPI_ILI9488_480X320)
#include "advanced_ILI9488_480X320.h"
#endif //(DISPLAY_DEVICE == TFT_SPI_ILI9488_480X320)
#endif //(DISPLAY_DEVICE == TFT_SPI_ILI9341_320X240) || (DISPLAY_DEVICE == TFT_SPI_ILI9488_480X320)
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


Display esp3d_display;

static lv_disp_buf_t esp_lv_disp_buf;
static lv_color_t lv_buf1[LV_HOR_RES_MAX * 10];
static lv_color_t lv_buf2[LV_HOR_RES_MAX * 10];
#if defined(DISPLAY_SNAPSHOT_FEATURE)
static uint8_t error_snapshot = 0;
#endif //DISPLAY_SNAPSHOT_FEATURE
Ticker esp_lv_tick; /* timer for interrupt handler */
#define LVGL_TICK_PERIOD 10
#define ESP_FLASH_LETTER_DRIVE 'F'
//#define ESP_SD_LETTER_DRIVE 'S'

//Logo
LV_IMG_DECLARE(esplogo)

/* Display flushing */
void esp_lv_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
    //TODO: need to write version for oled with big resolution
    uint16_t c;
    esp3d_screen.startWrite(); /* Start new TFT transaction */
    esp3d_screen.setAddrWindow(area->x1, area->y1, (area->x2 - area->x1 + 1), (area->y2 - area->y1 + 1)); /* set the working window */
    for (int y = area->y1; y <= area->y2; y++) {
        for (int x = area->x1; x <= area->x2; x++) {
            c = color_p->full;
            esp3d_screen.writeColor(c, 1);
#if defined(DISPLAY_SNAPSHOT_FEATURE)
            if(bSnapshot) {
                uint32_t data = lv_color_to32(*color_p);
                //to handle any write issue
                if (fsSnapFile.write((const uint8_t *)(&data), sizeof(uint32_t)) !=  sizeof(uint32_t)) {
                    //if error we stop to dump
                    bSnapshot = false;
                    //raise error
                    error_snapshot = 1;
                }
            }
#endif //DISPLAY_SNAPSHOT_FEATURE
            color_p++;
        }
    }
    esp3d_screen.endWrite(); /* terminate TFT transaction */
    lv_disp_flush_ready(disp); /* tell lvgl that flushing is done */
}

/* Interrupt driven periodic handler */
static void esp_lv_tick_handler(void)
{
    lv_tick_inc(LVGL_TICK_PERIOD);
}

#if USE_LV_LOG != 0
/* Serial debugging */
void esp_lv_print(lv_log_level_t level, const char * file, uint32_t line, const char * dsc)
{

    Serial.printf("%s@%d->%s\r\n", file, line, dsc);
    delay(100);
}
#endif

#if defined(DISPLAY_TOUCH_DRIVER)
bool esp_lv_touch_read(lv_indev_drv_t * indev, lv_indev_data_t * data)
{
// Use TFT_eSPI for touch events
    uint8_t bPressed = 0;
    uint16_t nX=0;
    uint16_t nY=0;
    static lv_coord_t last_x = 0;
    static lv_coord_t last_y = 0;
    if (esp3d_screen.getTouch(&nX,&nY) > 0) {
        last_x = nX;
        last_y = nY;
        data->state = LV_INDEV_STATE_PR;
    } else {
        data->state = LV_INDEV_STATE_REL;
    }
    data->point.x = last_x;
    data->point.y = last_y;
    return false;
}
#endif //DISPLAY_TOUCH_DRIVER

bool Display::startCalibration()
{
    //TODO add better calibration page with sound and contextual text
    bool res = false;
#if defined(DISPLAY_TOUCH_DRIVER)
#if DISPLAY_TOUCH_DRIVER == XPT2046_SPI
    uint16_t calibrationData[5];
    show_screenID(CALIBRATION_SCREEN);
    update_screen(true);
    esp3d_screen.calibrateTouch(calibrationData, CALIBRATION_CORNER, CALIBRATION_BG, 20);
    res = true;
    for (uint8_t i = 0; i < 5; i++) {
        if(!Settings_ESP3D::write_uint32 (ESP_CALIBRATION_1+(4*i), calibrationData[i])) {
            res= false;
        }
    }
    if (!Settings_ESP3D::write_byte (ESP_CALIBRATION, 1)) {
        res= false;
    }
    if(res) {
        SetStatus("Calibration done");
    } else {
        SetStatus("Calibration error");
    }
    show_screenID(MAIN_SCREEN);
#endif //XPT2046_SPI 

#endif //DISPLAY_TOUCH_DRIVER
    return res;
}


bool Display::display_network_status(bool force)
{
    if (esp_lv_network_label == nullptr) {
        return false;
    }
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
                if (sig != WiFiConfig::getSignal (WiFi.RSSI ())) {
                    sig = WiFiConfig::getSignal (WiFi.RSSI ());
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
        if ( force || refresh_signal || refresh_label) {
            String s ;
            if (sig == -1) {
                s = LV_SYMBOL_CLOSE;
            } else {
                s = LV_SYMBOL_WIFI;
                s+= String(sig) + "%";
            }
            lv_label_set_text(esp_lv_network_label, s.c_str());
            lv_coord_t w = lv_obj_get_width(esp_lv_network_label);
            lv_obj_set_pos(esp_lv_network_label, SCREEN_WIDTH-w-5,+15);
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
            String tmp = String(ETH.linkSpeed());
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
            lv_label_set_text(esp_lv_network_label, label.c_str());
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
        if( refresh_label || force) {
            if (label.length()>0) {
                lv_label_set_text(esp_lv_network_label, label.c_str());
            }
        }
    }
#endif //BLUETOOTH_FEATURE

    return true;
}

bool Display::display_IP(bool force)
{
    if (esp_lv_IP_label != nullptr) {
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
                break;
            }
            if (s!= label) {
                label = s;
                refresh_label = true;
            }
            if (refresh_label) {
                if (label.length()>0) {
                    lv_label_set_text(esp_lv_IP_label, label.c_str());
                }
            }
        } else {
            if (label != "") {
                lv_label_set_text(esp_lv_IP_label, "");
            }
        }
#endif //WIFI_FEATURE || ETH_FEATURE || BLUETOOTH_FEATURE
        return refresh_label;
    }
    return false;
}

void Display::show_screenID(uint8_t screenID)
{
    if (_screenID != screenID) {
#if defined(AUTO_SNAPSHOT_FEATURE)
        if (_screenID != -1) {
            String s = "/snap" + String(_screenID);
            s+=".bin";
            snapshot((char *)s.c_str());
        }
#endif //AUTO_SNAPSHOT_FEATURE
        _screenID = screenID;
        clear_screen();
        switch (screenID) {
        case SPLASH_SCREEN: {
            lv_obj_t * img_splash = lv_img_create(esp_lv_screen, NULL);
            lv_img_set_src(img_splash, &esplogo);
            lv_obj_align(img_splash, NULL, LV_ALIGN_CENTER, 0, -20);
            esp_lv_bar_progression = lv_bar_create(esp_lv_screen, NULL);
            lv_obj_set_size(esp_lv_bar_progression, 200, 30);
            lv_obj_align(esp_lv_bar_progression, NULL, LV_ALIGN_CENTER, 0, 80);
            lv_bar_set_range(esp_lv_bar_progression, 0, 90);
            lv_obj_t * labelsplash = lv_label_create(esp_lv_screen, NULL);
            lv_label_set_text(labelsplash, "Please wait...");
            lv_obj_align(labelsplash, NULL, LV_ALIGN_CENTER, 0, 120);
        }
        break;
        case MAIN_SCREEN: {
            lv_obj_t * esp_lv_top_container;
            lv_obj_t * esp_lv_main_container;
            lv_obj_t * esp_lv_bottom_container;
            //top
            esp_lv_top_container = lv_obj_create(esp_lv_screen, NULL);
            lv_obj_set_pos(esp_lv_top_container, 0,-10);
            lv_obj_set_size(esp_lv_top_container, SCREEN_WIDTH, 40);
            lv_obj_set_style(esp_lv_top_container, &lv_style_pretty_color);

            esp_lv_IP_label = lv_label_create(esp_lv_top_container, NULL);
            lv_label_set_text(esp_lv_IP_label, "0.0.0.0");
            lv_obj_align(esp_lv_IP_label, NULL, LV_ALIGN_IN_LEFT_MID, 10,+5);
            display_IP(true);

            esp_lv_network_label = lv_label_create(esp_lv_top_container, NULL);
            lv_label_set_text(esp_lv_network_label, LV_SYMBOL_CLOSE);
            lv_coord_t w = lv_obj_get_width(esp_lv_network_label);
            lv_obj_set_pos(esp_lv_network_label, SCREEN_WIDTH-w-5,+15);
            display_network_status(true);

            //main window
            esp_lv_main_container = lv_obj_create(esp_lv_screen, NULL);
            lv_obj_set_pos(esp_lv_main_container, 0,30);
            lv_obj_set_size(esp_lv_main_container, SCREEN_WIDTH, SCREEN_HEIGHT - (2*30));
            lv_obj_set_style(esp_lv_main_container, &lv_style_scr);

            //bottom
            esp_lv_bottom_container = lv_obj_create(esp_lv_screen, NULL);
            lv_obj_set_pos(esp_lv_bottom_container, 0,SCREEN_HEIGHT-30);
            lv_obj_set_size(esp_lv_bottom_container, SCREEN_WIDTH, 40);
            lv_obj_set_style(esp_lv_bottom_container, &lv_style_pretty_color);

            //status label
            esp_lv_status_label = lv_label_create(esp_lv_bottom_container, NULL);
            lv_label_set_text(esp_lv_status_label, _status.c_str());
            lv_obj_align(esp_lv_status_label, NULL, LV_ALIGN_IN_LEFT_MID, 10,-5);

        }
        break;
        case CALIBRATION_SCREEN: {
            lv_obj_t * labeltouch = lv_label_create(esp_lv_screen, NULL);
            lv_label_set_text(labeltouch, "Touch corners when requested.");
            lv_obj_align(labeltouch, NULL, LV_ALIGN_CENTER, 0, 0);
        }
        break;
        default:
            break;
        }
    } else {
    }

    update_screen(true);
}

Display::Display()
{
    _started = false;
    _screenID = -1;
    _screenwidth = SCREEN_WIDTH;
    _screenheight = SCREEN_HEIGHT;
    esp_lv_screen = nullptr;
    esp_lv_status_label = nullptr;
    esp_lv_bar_progression = nullptr;
    esp_lv_IP_label = nullptr;
    esp_lv_network_label = nullptr;
    bSnapshot = false;
}

Display::~Display()
{
    end();
}

bool Display::begin()
{
    bool res = true;
    _started = false;
    //log_esp3d("Init Display");
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
#if (DISPLAY_DEVICE == TFT_SPI_ILI9341_320X240) || (DISPLAY_DEVICE == TFT_SPI_ILI9488_480X320)
#if defined (DISPLAY_LED_PIN) && (DISPLAY_LED_PIN!=-1)
    pinMode(DISPLAY_LED_PIN, OUTPUT);    // sets the digital pin 13 as output
    digitalWrite(DISPLAY_LED_PIN, HIGH);
#endif //DISPLAY_LED_PIN
    //init lvg
    lv_init();
#if USE_LV_LOG != 0
    lv_log_register_print(esp_lv_print); /* register print function for debugging */
#endif
    esp3d_screen.begin();               // Initialise the tft display
#if defined(DISPLAY_FLIP_VERTICALY)
    esp3d_screen.setRotation(3);
#else
    esp3d_screen.setRotation(1);
#endif
    //init lvg related functions
    //double buffer
    lv_disp_buf_init(&esp_lv_disp_buf, lv_buf1, lv_buf2, LV_HOR_RES_MAX * 10);

    /*Initialize the display*/
    lv_disp_drv_t esp_lv_disp_drv;
    lv_disp_drv_init(&esp_lv_disp_drv);
    //resolution
    esp_lv_disp_drv.hor_res = SCREEN_WIDTH;
    esp_lv_disp_drv.ver_res = SCREEN_HEIGHT;
    esp_lv_disp_drv.flush_cb = esp_lv_disp_flush;
    esp_lv_disp_drv.buffer = &esp_lv_disp_buf;
    lv_disp_drv_register(&esp_lv_disp_drv);

    //Register Flash driver for images
    lv_fs_drv_t esp_lv_flash_drv;                         /*A driver descriptor*/
    memset(&esp_lv_flash_drv, 0, sizeof(lv_fs_drv_t));    /*Initialization*/
    esp_lv_flash_drv.file_size = sizeof(file_t);       /*Set up fields...*/
    esp_lv_flash_drv.letter = ESP_FLASH_LETTER_DRIVE;
    esp_lv_flash_drv.open_cb = esp_flash_open;
    esp_lv_flash_drv.close_cb = esp_flash_close;
    esp_lv_flash_drv.read_cb = esp_flash_read;
    esp_lv_flash_drv.seek_cb = esp_flash_seek;
    esp_lv_flash_drv.tell_cb = esp_flash_tell;
    lv_fs_drv_register(&esp_lv_flash_drv);

    //TODO: Register SD card driver for images

#if defined(DISPLAY_LED_PIN) && (DISPLAY_LED_PIN != -1)
    pinMode(DISPLAY_LED_PIN, OUTPUT);    // sets the digital pin as output
    digitalWrite(DISPLAY_LED_PIN, HIGH); //switch on the led
#endif //DISPLAY_LED_PIN
#endif //(DISPLAY_DEVICE == TFT_SPI_ILI9341_320X240) || (DISPLAY_DEVICE == TFT_SPI_ILI9488_480X320) 
#if defined(DISPLAY_TOUCH_DRIVER)
    if(Settings_ESP3D::read_byte(ESP_CALIBRATION)==1) {
        uint16_t calibrationData[5];
        for (uint8_t i = 0; i < 5; i++) {
            calibrationData[i] = Settings_ESP3D::read_uint32 (ESP_CALIBRATION_1+(4*i));
        }
        esp3d_screen.setTouch(calibrationData);
    }

    /*Register the touch pad*/
    lv_indev_drv_t esp_lv_indev_drv;
    lv_indev_drv_init(&esp_lv_indev_drv);
    esp_lv_indev_drv.type = LV_INDEV_TYPE_POINTER;//LV_INDEV_TYPE_ENCODER;
    esp_lv_indev_drv.read_cb = esp_lv_touch_read;
    lv_indev_drv_register(&esp_lv_indev_drv);
#endif //DISPLAY_TOUCH_DRIVER

#if DISPLAY_UI_COLOR == UI_MONOCHROME
    lv_theme_t *th = lv_theme_mono_init(0, NULL);
    lv_theme_set_current(th);
#endif //DISPLAY_UI_COLOR == UI_MONOCHROME

    /*Initialize the graphics library's tick*/
    esp_lv_tick.attach_ms(LVGL_TICK_PERIOD, esp_lv_tick_handler);
    show_screenID(SPLASH_SCREEN);
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
    _screenID = -1;
    clear_screen();
}

void Display::SetStatus(const char * status)
{
    _status = status;
    if ( !ESP3DOutput::isOutput(ESP_SCREEN_CLIENT)) {
        return;
    }
    if (esp_lv_status_label != nullptr) {
        lv_label_set_text(esp_lv_status_label, status);
        update_screen();
    }
}

void Display::clear_screen(bool force)
{
    //clear all objects on screen
    if(esp_lv_screen != nullptr) {
        lv_obj_del(esp_lv_screen);
    }
    //reset global object
    esp_lv_status_label = nullptr;
    esp_lv_bar_progression = nullptr;
    esp_lv_IP_label = nullptr;
    esp_lv_network_label = nullptr;

    //create enpty screen
    esp_lv_screen = lv_obj_create(lv_scr_act(), NULL);
    lv_obj_set_pos(esp_lv_screen, 0,0);
    lv_obj_set_size(esp_lv_screen, SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_set_style(esp_lv_screen, &lv_style_scr);
    //update screen
    update_screen(force);
}

void Display::update_screen(bool force)
{
    static uint32_t last_update;
    if ( !ESP3DOutput::isOutput(ESP_SCREEN_CLIENT)) {
        return;
    }
    if ((millis() - last_update) > 1000) {
        last_update = millis();
        display_network_status();
    }
    lv_task_handler();
    if(force) {
#if DISPLAY_DEVICE == OLED_I2C_SSD1306 || DISPLAY_DEVICE == OLED_I2C_SSDSH1106
        esp3d_screen.display();
#endif //DISPLAY_DEVICE == OLED_I2C_SSD1306 || DISPLAY_DEVICE == OLED_I2C_SSDSH1106
        delay(100);
        lv_task_handler();
    }
}

void Display::handle()
{
    if (_started) {
        update_screen();
    }
}

void Display::progress(uint8_t v)
{
    if ( !ESP3DOutput::isOutput(ESP_SCREEN_CLIENT)) {
        return;
    }
    if (esp_lv_bar_progression) {
        lv_bar_set_value(esp_lv_bar_progression, v, LV_ANIM_OFF);
        update_screen();
    }
}



bool Display::snapshot(char * filename)
{
    bool res = false;
#if defined(DISPLAY_SNAPSHOT_FEATURE)
    //sanity check to avoid to corrupt FS with capacity overload
    error_snapshot = 0;
    if (ESP_FileSystem::freeBytes() < SNAP_SIZE) {
        return false;
    }
    if(filename) {
        fsSnapFile = ESP_FileSystem::open(filename, ESP_FILE_WRITE);
    } else {
        fsSnapFile = ESP_FileSystem::open(SNAPFILENAME, ESP_FILE_WRITE);
    }
    if (!fsSnapFile) {
        return false;
    }

    bSnapshot = true;
    lv_obj_invalidate(lv_scr_act());
    lv_refr_now(lv_disp_get_default());                    /* Will call our disp_drv.disp_flush function */
    bSnapshot = false;
    fsSnapFile.close();
    //if any snapshot error
    if (error_snapshot == 0) {
        res = true;
    }
#endif //DISPLAY_SNAPSHOT_FEATURE
    return res;
}

#endif //DISPLAY_DEVICE
