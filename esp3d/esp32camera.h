#pragma once
#include "Arduino.h"

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

#define LED_GPIO_NUM       4
// Set to 1 if you want the camera rotated 180 degrees by default, 0 otherwise
#define ROTATE_CAMERA_180  1

// Image streaming is running on a separate web server from the one for camera status and the rest of ESP3D.
// Both servers can only handle one connection at a time
bool cameraInit(void);
void startCameraFrameServer();
String getCameraStatusJSON(void);
int setCameraStatus(String variable, String value);
