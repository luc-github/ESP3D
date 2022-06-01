/*
  pins.h -  pins definition file

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

//Serial Pins
//-1 means use default pins of your board what ever the serial you choose
//   * UART 0 possible options are (1, 3), (2, 3) or (15, 13)
//   * UART 1 allows only TX on 2 if UART 0 is not (2, 3)
#ifndef ESP_RX_PIN
#define ESP_RX_PIN -1
#endif //~ESP_RX_PIN
#ifndef ESP_TX_PIN
#define ESP_TX_PIN -1
#endif //~ESP_TX_PIN

#ifndef ESP_DEBUG_RX_PIN
#define ESP_DEBUG_RX_PIN -1
#endif //~ESP_DEBUG_RX_PIN
#ifndef ESP_DEBUG_TX_PIN
#define ESP_DEBUG_TX_PIN -1
#endif //~ESP_DEBUG_TX_PIN

//I2C Pins
#ifndef ESP_SDA_PIN
#define ESP_SDA_PIN SDA
#endif //~ESP_SDA_PIN

#ifndef ESP_SCL_PIN
#define ESP_SCL_PIN SCL
#endif //~ESP_SCL_PIN

//Pins for the support of connected camera
#if CAMERA_DEVICE == CAMERA_MODEL_CUSTOM
#define CAM_LED_PIN       4
#define CAM_PULLUP1       -1
#define CAM_PULLUP2       -1
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22
#endif //CAMERA_MODEL_CUSTOM

#if CAMERA_DEVICE == CAMERA_MODEL_WROVER_KIT
#define CAM_LED_PIN      -1
#define CAM_PULLUP1      -1
#define CAM_PULLUP2      -1
#define PWDN_GPIO_NUM    -1
#define RESET_GPIO_NUM   -1
#define XCLK_GPIO_NUM    21
#define SIOD_GPIO_NUM    26
#define SIOC_GPIO_NUM    27

#define Y9_GPIO_NUM      35
#define Y8_GPIO_NUM      34
#define Y7_GPIO_NUM      39
#define Y6_GPIO_NUM      36
#define Y5_GPIO_NUM      19
#define Y4_GPIO_NUM      18
#define Y3_GPIO_NUM       5
#define Y2_GPIO_NUM       4
#define VSYNC_GPIO_NUM   25
#define HREF_GPIO_NUM    23
#define PCLK_GPIO_NUM    22
#endif //CAMERA_MODEL_WROVER_KIT

#if CAMERA_DEVICE == CAMERA_MODEL_ESP_EYE
#define CAM_LED_PIN       -1
#define CAM_PULLUP1       13
#define CAM_PULLUP2       14
#define PWDN_GPIO_NUM    -1
#define RESET_GPIO_NUM   -1
#define XCLK_GPIO_NUM    4
#define SIOD_GPIO_NUM    18
#define SIOC_GPIO_NUM    23

#define Y9_GPIO_NUM      36
#define Y8_GPIO_NUM      37
#define Y7_GPIO_NUM      38
#define Y6_GPIO_NUM      39
#define Y5_GPIO_NUM      35
#define Y4_GPIO_NUM      14
#define Y3_GPIO_NUM      13
#define Y2_GPIO_NUM      34
#define VSYNC_GPIO_NUM   5
#define HREF_GPIO_NUM    27
#define PCLK_GPIO_NUM    25
#endif //CAMERA_MODEL_ESP_EYE

#if CAMERA_DEVICE == CAMERA_MODEL_M5STACK_PSRAM
#define CAM_LED_PIN       -1
#define CAM_PULLUP1       -1
#define CAM_PULLUP2       -1
#define PWDN_GPIO_NUM     -1
#define RESET_GPIO_NUM    15
#define XCLK_GPIO_NUM     27
#define SIOD_GPIO_NUM     25
#define SIOC_GPIO_NUM     23

#define Y9_GPIO_NUM       19
#define Y8_GPIO_NUM       36
#define Y7_GPIO_NUM       18
#define Y6_GPIO_NUM       39
#define Y5_GPIO_NUM        5
#define Y4_GPIO_NUM       34
#define Y3_GPIO_NUM       35
#define Y2_GPIO_NUM       32
#define VSYNC_GPIO_NUM    22
#define HREF_GPIO_NUM     26
#define PCLK_GPIO_NUM     21
#endif //CAMERA_MODEL_M5STACK_PSRAM

#if CAMERA_DEVICE == CAMERA_MODEL_M5STACK_WIDE
#define CAM_LED_PIN       -1
#define CAM_PULLUP1       -1
#define CAM_PULLUP2       -1
#define PWDN_GPIO_NUM     -1
#define RESET_GPIO_NUM    15
#define XCLK_GPIO_NUM     27
#define SIOD_GPIO_NUM     22
#define SIOC_GPIO_NUM     23

#define Y9_GPIO_NUM       19
#define Y8_GPIO_NUM       36
#define Y7_GPIO_NUM       18
#define Y6_GPIO_NUM       39
#define Y5_GPIO_NUM        5
#define Y4_GPIO_NUM       34
#define Y3_GPIO_NUM       35
#define Y2_GPIO_NUM       32
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     26
#define PCLK_GPIO_NUM     21
#endif //CAMERA_MODEL_M5STACK_WIDE

#if CAMERA_DEVICE == CAMERA_MODEL_AI_THINKER
#define CAM_LED_PIN       -1 //used by SD so must left unset
#define CAM_PULLUP1       -1
#define CAM_PULLUP2       -1

#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22
#endif //CAMERA_MODEL_AI_THINKER

//Pins for the support of SD Card Reader
//-1 means use default pins of your board defined core
//this are overwrited if defined in configuration.h or myconfig.h
#ifndef ESP_SD_CS_PIN
#define ESP_SD_CS_PIN           -1
#endif //ESP_SD_CS_PIN
//These are hardcoded on ESP8266 to 12/13/14
//so modifications are ignored on ESP8266
#ifndef ESP_SD_MISO_PIN
#define ESP_SD_MISO_PIN         -1
#endif //ESP_SD_MISO_PIN
#ifndef ESP_SD_MOSI_PIN
#define ESP_SD_MOSI_PIN         -1
#endif //ESP_SD_MOSI_PIN
#ifndef ESP_SD_SCK_PIN
#define ESP_SD_SCK_PIN          -1
#endif //ESP_SD_SCK_PIN

#ifndef ESP_SD_DETECT_PIN
#define ESP_SD_DETECT_PIN       -1
#endif //ESP_SD_DETECT_PIN 

#if defined (PIN_RESET_FEATURE) && !defined(ESP3D_RESET_PIN)
#define ESP3D_RESET_PIN 0
#endif //PIN_RESET_FEATURE

#ifdef SD_DEVICE_CONNECTION
#ifndef ESP_SD_DETECT_VALUE
#define ESP_SD_DETECT_VALUE LOW
#endif //ESP_SD_DETECT_VALUE
#if SD_DEVICE_CONNECTION == ESP_SHARED_SD
#ifndef ESP_FLAG_SHARED_SD_PIN
#define ESP_FLAG_SHARED_SD_PIN -1
#endif //ESP_PIN_SHARED_SD
#ifndef ESP_FLAG_SHARED_SD_VALUE
#define ESP_FLAG_SHARED_SD_VALUE 0
#endif //ESP_FLAG_SHARED_SD_VALUE
#endif //SD_DEVICE_CONNECTION == ESP_SHARED_SD
#endif //SD_DEVICE_CONNECTION
