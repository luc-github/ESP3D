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
#include <ESP8266WiFi.h>
#endif  // ARDUINO_ARCH_ESP8266
#if defined(ARDUINO_ARCH_ESP32)
#include <soc/soc.h>
#if CONFIG_IDF_TARGET_ESP32 || CONFIG_IDF_TARGET_ESP32S2
// FIXME : S3 not support it yet
#if __has_include("rtc_wdt.h")
#include <rtc_wdt.h>
#else
#include <soc/rtc_wdt.h>
#endif  // __has_include ("rtc_wdt.h")
#endif  // CONFIG_IDF_TARGET_ESP32 || CONFIG_IDF_TARGET_ESP32S2
#include <WiFi.h>
#include <esp_adc/adc_oneshot.h>
#include <esp_adc/adc_continuous.h>
#include <esp_task_wdt.h>
#include <soc/rtc_cntl_reg.h>

TaskHandle_t ESP3DHal::xHandle = nullptr;
#endif  // ARDUINO_ARCH_ESP32

#include "esp3d_message.h"

uint32_t ESP3DHal::_analogRange = 255;
uint32_t ESP3DHal::_analogWriteFreq = 1000;

void ESP3DHal::pinMode(uint8_t pin, uint8_t mode) {
#if defined(ARDUINO_ARCH_ESP8266)
  if ((pin == 16) && (mode == INPUT_PULLUP)) {
    ::pinMode(pin, INPUT_PULLDOWN_16);
    return;
  }
#endif
  ::pinMode(pin, mode);
}

int ESP3DHal::analogRead(uint8_t pin) {
#ifdef ARDUINO_ARCH_ESP8266  // only one ADC on ESP8266 A0
  (void)pin;
  return ::analogRead(A0);
#else
  return ::analogRead(pin);
#endif
}

const char* ESP3DHal::arduinoVersion() {
  static String version = "";
#ifdef ARDUINO_ARCH_ESP32
  version = ESP_ARDUINO_VERSION_MAJOR;
  version += ".";
  version += ESP_ARDUINO_VERSION_MINOR;
  version += ".";
  version += ESP_ARDUINO_VERSION_PATCH;
#endif  // ARDUINO_ARCH_ESP32

#ifdef ARDUINO_ARCH_ESP8266
#ifdef ARDUINO_ESP8266_RELEASE
  version = ARDUINO_ESP8266_RELEASE;
#else
  version = ESP.getCoreVersion();
#endif  // ARDUINO_ESP8266_RELEASE
#endif  // ARDUINO_ARCH_ESP8266
  return version.c_str();
}

bool ESP3DHal::analogWrite(uint8_t pin, uint value) {
  if (value > (_analogRange - 1)) {
    return false;
  }
#ifdef ARDUINO_ARCH_ESP8266
  ::analogWrite(pin, value);
#endif  // ARDUINO_ARCH_ESP8266
#ifdef ARDUINO_ARCH_ESP32
  ::analogWrite(pin, value);
#endif  // ARDUINO_ARCH_ESP32
  return true;
}
void ESP3DHal::analogWriteFreq(uint32_t freq) {
  _analogWriteFreq = freq;
#ifdef ARDUINO_ARCH_ESP8266
  ::analogWriteFreq(_analogWriteFreq);
#endif  // ARDUINO_ARCH_ESP8266
}
void ESP3DHal::analogRange(uint32_t range) {
  _analogRange = range;
  uint8_t resolution = 0;
  switch (_analogRange) {
    case 8191:
      resolution = 13;
      break;
    case 1024:
      resolution = 10;
      break;
    case 2047:
      resolution = 11;
      break;
    case 4095:
      resolution = 12;
      break;
    default:
      resolution = 8;
      _analogRange = 255;
      break;
  }
#if defined(ARDUINO_ARCH_ESP32)
  analogReadResolution(resolution);
#endif  // ARDUINO_ARCH_ESP32
#ifdef ARDUINO_ARCH_ESP8266
  (void)resolution;
  ::analogWriteRange(_analogRange);
#endif  // ARDUINO_ARCH_ESP8266
}

// Setup
bool ESP3DHal::begin() {
#if defined(ARDUINO_ARCH_ESP32) && defined(CAMERA_DEVICE)
  esp3d_log("Disable brown out");
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);  // disable brownout detector
#endif  // ARDUINO_ARCH_ESP32 && CAMERA_DEVICE
  // Clear all wifi state
  WiFi.persistent(false);
  WiFi.disconnect(true);
  WiFi.enableSTA(false);
  WiFi.enableAP(false);
  WiFi.mode(WIFI_OFF);
#if SD_DEVICE_CONNECTION == ESP_SHARED_SD
#if defined(ESP_SD_DETECT_PIN) && ESP_SD_DETECT_PIN != -1
  pinMode(ESP_SD_DETECT_PIN, INPUT);
#endif
#if defined(ESP_FLAG_SHARED_SD_PIN) && ESP_FLAG_SHARED_SD_PIN != -1
  pinMode(ESP_FLAG_SHARED_SD_PIN, OUTPUT);
  digitalWrite(ESP_FLAG_SHARED_SD_PIN, !ESP_FLAG_SHARED_SD_VALUE);
#endif  // ESP_FLAG_SHARED_SD_PIN
#endif  // SD_DEVICE_CONNECTION == ESP_SHARED_SD
  return true;
}

// End ESP3D
void ESP3DHal::end() {}

// Watchdog feeder
void ESP3DHal::wdtFeed() {
#ifdef ARDUINO_ARCH_ESP8266
  ESP.wdtFeed();
#endif  // ARDUINO_ARCH_ESP8266
#ifdef ARDUINO_ARCH_ESP32
  static uint64_t lastYield = 0;
  uint64_t now = millis();
  if ((now - lastYield) > 2000) {
    lastYield = now;
    vTaskDelay(5);  // delay 1 RTOS tick
  }
#if !defined(DISABLE_WDT_ESP3DLIB_TASK) && !defined(DISABLE_WDT_CORE_0)
#if CONFIG_IDF_TARGET_ESP32
  // FIXME: not implemented
  rtc_wdt_feed();
#endif  // CONFIG_IDF_TARGET_ESP32S2
#endif  //! defined(DISABLE_WDT_ESP3DLIB_TASK) && !defined(DISABLE_WDT_CORE_0)
#ifndef DISABLE_WDT_ESP3DLIB_TASK
  if (xHandle && esp_task_wdt_status(xHandle) == ESP_OK) {
    if (esp_task_wdt_reset() != ESP_OK) {
      esp3d_log_e("WDT Reset failed");
    }
  }
#endif  // DISABLE_WDT_ESP3DLIB_TASK
#endif  // ARDUINO_ARCH_ESP32
}

// wait function
void ESP3DHal::wait(uint32_t milliseconds) {
#if defined(ASYNCWEBSERVER)
  uint32_t timeout = millis();
  while ((millis() - timeout) < milliseconds) {
    wdtFeed();
  }
#else   // !(ASYNCWEBSERVER
  wdtFeed();
  // before 0 was acceptable, now it seems need to put 5 to have some effect if
  // on esp32 core 0
  delay(milliseconds < 5 ? 5 : milliseconds);
#endif  // !ASYNCWEBSERVER
}

uint16_t ESP3DHal::getChipID() {
#ifdef ARDUINO_ARCH_ESP8266
  return ESP.getChipId();
#endif  // ARDUINO_ARCH_ESP8266
#ifdef ARDUINO_ARCH_ESP32
  return (uint16_t)(ESP.getEfuseMac() >> 32);
#endif  // ARDUINO_ARCH_ESP32
}

bool ESP3DHal::has_temperature_sensor() {
#ifdef ARDUINO_ARCH_ESP8266
  return false;
#endif  // ARDUINO_ARCH_ESP8266
#ifdef ARDUINO_ARCH_ESP32
#if CONFIG_IDF_TARGET_ESP32S3
  // FIXME: not yet implemented
  return false;
#else
  return true;
#endif  // CONFIG_IDF_TARGET_ESP32S3
#endif  // ARDUINO_ARCH_ESP32
}

float ESP3DHal::temperature() {
#ifdef ARDUINO_ARCH_ESP8266
  return 0.0;
#endif  // ARDUINO_ARCH_ESP8266

#ifdef ARDUINO_ARCH_ESP32
#if CONFIG_IDF_TARGET_ESP32S3
  // FIXME: not yet implemented
  return 0.0;
#else
  return temperatureRead();
#endif  // CONFIG_IDF_TARGET_ESP32S3
#endif  // ARDUINO_ARCH_ESP32
}

bool ESP3DHal::is_pin_usable(uint pin) {
#ifdef ARDUINO_ARCH_ESP8266
  if ((pin <= 5) || ((pin >= 12) && (pin <= 16))) {
    return true;
  } else {
    return false;
  }
#endif  // ARDUINO_ARCH_ESP8266
#ifdef ARDUINO_ARCH_ESP32
  // TODO: Add support for ESP32 S2 S3 C2 C3
  if ((pin <= 5) || ((pin >= 12) && (pin <= 39))) {
    return true;
  } else {
    return false;
  }
#endif  // ARDUINO_ARCH_ESP32
}
