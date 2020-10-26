/*
  hal.cpp - ESP3D hal class

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

#include "../include/esp3d_config.h"

#if defined(ARDUINO_ARCH_ESP8266)
#include "ESP8266WiFi.h"
#endif //ARDUINO_ARCH_ESP8266
#if defined(ARDUINO_ARCH_ESP32)
#include <soc/soc.h>
#include <soc/rtc_cntl_reg.h>
#include "WiFi.h"
#ifdef __cplusplus
extern "C" {
#endif //__cplusplus
esp_err_t esp_task_wdt_reset();
#ifdef __cplusplus
}
#endif //__cplusplus
#endif //ARDUINO_ARCH_ESP32

#include "esp3doutput.h"

#if defined(ARDUINO_ARCH_ESP32)
int ChannelAttached2Pin[16]= {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
#endif //ARDUINO_ARCH_ESP32

uint32_t Hal::_analogWriteRange = 255;
uint32_t Hal::_analogWriteFreq = 1000;

void Hal::clearAnalogChannels()
{
#ifdef ARDUINO_ARCH_ESP32
    for (uint8_t p = 0; p < 16; p++) {
        if(ChannelAttached2Pin[p] != -1) {
            ledcDetachPin(ChannelAttached2Pin[p]);
            ChannelAttached2Pin[p] = -1;
        }
    }
#endif //ARDUINO_ARCH_ESP32
}

void Hal::pinMode(uint8_t pin, uint8_t mode)
{
#if defined (ARDUINO_ARCH_ESP8266)
    if ((pin == 16) && (mode == INPUT_PULLUP)) {
        pinMode(pin, INPUT_PULLDOWN_16);
        return;
    }
#endif
    ::pinMode(pin, mode);
}

void Hal::toneESP(uint8_t pin, unsigned int frequency, unsigned int duration, bool sync)
{
#if defined(ARDUINO_ARCH_ESP8266)
    (void) sync; //useless for esp8266
    tone(pin, frequency, duration);
#endif //ARDUINO_ARCH_ESP8266
#if defined(ARDUINO_ARCH_ESP32)
    int channel = getAnalogWriteChannel(pin);
    if (channel  != -1) {
        ledcAttachPin(pin, channel);
        ledcWriteTone(channel,frequency);
        if (sync) {
            wait(duration);
            ledcWriteTone(pin,0);
        }
    }

#endif //ARDUINO_ARCH_ESP32
}
void Hal::no_tone(uint8_t pin)
{
#if defined(ARDUINO_ARCH_ESP8266)
    tone(pin, 0, 0);
#endif //ARDUINO_ARCH_ESP8266
#if defined(ARDUINO_ARCH_ESP32)
    int channel = getAnalogWriteChannel(pin);
    if (channel  != -1) {
        ledcWrite(channel, 0);
    }
#endif //ARDUINO_ARCH_ESP32
}

int Hal::analogRead(uint8_t pin)
{
#ifdef ARDUINO_ARCH_ESP8266 //only one ADC on ESP8266 A0
    (void)pin;
    return analogRead (A0);
#else
    return analogRead (pin);
#endif
}

#if defined(ARDUINO_ARCH_ESP32)
int Hal::getAnalogWriteChannel(uint8_t pin)
{
    for (uint8_t p = 0; p < 16; p++) {
        if(ChannelAttached2Pin[p] == pin) {
            return p;
        }
    }
    for (uint8_t p = 0; p < 16; p++) {
        if(ChannelAttached2Pin[p] == -1) {
            ChannelAttached2Pin[p] = pin;
            return p;
        }
    }

    return -1;
}
#endif //ARDUINO_ARCH_ESP32

bool Hal::analogWrite(uint8_t pin, uint value)
{
    if (value > (_analogWriteRange-1)) {
        return false;
    }
#ifdef ARDUINO_ARCH_ESP8266
    analogWrite(pin, value);
#endif //ARDUINO_ARCH_ESP8266
#ifdef ARDUINO_ARCH_ESP32
    int channel  = getAnalogWriteChannel(pin);
    if (channel==-1) {
        return false;
    }
    uint8_t resolution = 0;
    switch(_analogWriteRange) {
    case 8191:
        resolution=13;
        break;
    case 1024:
        resolution=10;
        break;
    case 2047:
        resolution=11;
        break;
    case 4095:
        resolution=12;
        break;
    default:
        resolution=8;
        _analogWriteRange = 255;
        break;
    }
    ledcSetup(channel, _analogWriteFreq, resolution);
    ledcAttachPin(pin, channel);
    ledcWrite(channel, value);
#endif //ARDUINO_ARCH_ESP32
    return true;
}
void Hal::analogWriteFreq(uint32_t freq)
{
    _analogWriteFreq = freq;
#ifdef ARDUINO_ARCH_ESP8266
    analogWriteFreq(_analogWriteFreq);
#endif //ARDUINO_ARCH_ESP8266
}
void Hal::analogWriteRange(uint32_t range)
{
    _analogWriteRange = range;
#ifdef ARDUINO_ARCH_ESP8266
    analogWriteRange(_analogWriteRange);
#endif //ARDUINO_ARCH_ESP8266
}

//Setup
bool Hal::begin()
{
#if defined(ARDUINO_ARCH_ESP32) && defined(CAMERA_DEVICE)
    log_esp3d("Disable brown out");
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
#endif //ARDUINO_ARCH_ESP32 && CAMERA_DEVICE
    //Clear all wifi state
    WiFi.persistent(false);
    WiFi.disconnect(true);
    WiFi.enableSTA (false);
    WiFi.enableAP (false);
    WiFi.mode (WIFI_OFF);
    return true;
}

//End ESP3D
void Hal::end()
{
#if defined(ARDUINO_ARCH_ESP32)
    clearAnalogChannels();
#endif //ARDUINO_ARCH_ESP32
}

//Watchdog feeder
void Hal::wdtFeed()
{
#ifdef ARDUINO_ARCH_ESP8266
    ESP.wdtFeed();
#endif //ARDUINO_ARCH_ESP8266
#ifdef ARDUINO_ARCH_ESP32
    void esp_task_wdt_feed();
#endif //ARDUINO_ARCH_ESP32
}

//wait function
void Hal::wait (uint32_t milliseconds)
{
#if defined(ASYNCWEBSERVER)
    uint32_t timeout = millis();
    while ( (millis() - timeout) < milliseconds) {
        wdtFeed();
    }
#else // !(ASYNCWEBSERVER 
    wdtFeed();
    delay(milliseconds);
#endif // !ASYNCWEBSERVER
}

uint16_t Hal::getChipID()
{
#ifdef ARDUINO_ARCH_ESP8266
    return ESP.getChipId();
#endif //ARDUINO_ARCH_ESP8266
#ifdef ARDUINO_ARCH_ESP32
    return (uint16_t) (ESP.getEfuseMac() >> 32);
#endif //ARDUINO_ARCH_ESP32
}

bool Hal::has_temperature_sensor()
{
#ifdef ARDUINO_ARCH_ESP8266
    return false;
#endif //ARDUINO_ARCH_ESP8266
#ifdef ARDUINO_ARCH_ESP32
    return true;
#endif //ARDUINO_ARCH_ESP32
}

float Hal::temperature()
{
#ifdef ARDUINO_ARCH_ESP8266
    return 0.0;
#endif //ARDUINO_ARCH_ESP8266
#ifdef ARDUINO_ARCH_ESP32
    return temperatureRead();
#endif //ARDUINO_ARCH_ESP32
}

bool Hal::is_pin_usable(uint pin)
{
#ifdef ARDUINO_ARCH_ESP8266
    if  ((pin <= 5) || ((pin >= 12) && (pin <= 16))) {
        return true;
    } else {
        return false;
    }
#endif //ARDUINO_ARCH_ESP8266
#ifdef ARDUINO_ARCH_ESP32
    if  ((pin <= 5) || ((pin >= 12) && (pin <= 39))) {
        return true;
    } else {
        return false;
    }
#endif //ARDUINO_ARCH_ESP32
}
