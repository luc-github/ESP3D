/*
  mks_service.cpp -  mks communication service functions class

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
#if COMMUNICATION_PROTOCOL == MKS_SERIAL
#include "mks_service.h"
#include "../../core/settings_esp3d.h"
#include "../../core/esp3doutput.h"
#include "../network/netconfig.h"
#include "../wifi/wificonfig.h"

#define MKS_FRAME_DATA_MAX_SIZE  (MKS_FRAME_SIZE - 5 - 4)

//Flag Pins
#define ESP_FLAG_PIN        0
#define BOARD_FLAG_PIN      4
//Flag pins values
#define BOARD_READY_FLAG_VALUE    LOW

//Frame offsets
#define MKS_FRAME_HEAD_OFFSET       0
#define MKS_FRAME_TYPE_OFFSET       1
#define MKS_FRAME_DATALEN_OFFSET    2
#define MKS_FRAME_DATA_OFFSET       4

//Frame flags
#define MKS_FRAME_HEAD_FLAG (char)0xa5
#define MKS_FRAME_TAIL_FLAG (char)0xfc

//Network states
#define MKS_FRAME_NETWORK_OK_STATE (char)0x0a
#define MKS_FRAME_NETWORK_FAIL_STATE (char)0x05
#define MKS_FRAME_NETWORK_ERROR_STATE (char)0x0e

//Network modes
#define MKS_FRAME_NETWORK_AP_MODE (char)0x01
#define MKS_FRAME_NETWORK_STA_MODE (char)0x02
#define MKS_FRAME_NETWORK_APSTA_MODE (char)0x03

//Cloud states
#define MKS_FRAME_CLOUD_BINDED_STATE (char)0x12
#define MKS_FRAME_CLOUD_NOT_BINDED_STATE (char)0x13
#define MKS_FRAME_CLOUD_DISCONNECTED_STATE (char)0x10
#define MKS_FRAME_CLOUD_DISABLED_STATE (char)0x00


//Data types
#define MKS_FRAME_DATA_NETWORK_TYPE         (char)0x0
#define MKS_FRAME_DATA_COMMAND_TYPE         (char)0x1
#define MKS_FRAME_DATA_FIRST_FRAGMENT_TYPE  (char)0x2
#define MKS_FRAME_DATA_FRAGMENT_TYPE        (char)0x3
#define MKS_FRAME_DATA_HOTSPOTS_LIST_TYPE   (char)0x4
#define MKS_FRAME_DATA_STATIC_IP_TYPE       (char)0x5

#define CLOUD_HOST_ADDRESS "baizhongyun.cn"
#define CLOUD_HOST_PORT    12345
#define CLOUD_SERVICE_PORT   8080

//Timeouts
#define FRAME_WAIT_TO_SEND_TIMEOUT  2000
#define NET_FRAME_REFRESH_TIME  10000

bool MKSService::_started = false;
char MKSService::_frame[MKS_FRAME_SIZE] = {0};
char MKSService:: _moduleId[21] = {0};

bool MKSService::begin()
{
    //setup the pins
    pinMode(BOARD_FLAG_PIN, INPUT);
    pinMode(ESP_FLAG_PIN, OUTPUT);
    _started = true;
    strcpy(_moduleId,"12345");
    return true;
}

bool MKSService::canSendFrame()
{
    log_esp3d("Is board ready for frame?");
    digitalWrite(ESP_FLAG_PIN, HIGH);
    uint32_t startTime = millis();
    while( (millis() - startTime) <  FRAME_WAIT_TO_SEND_TIMEOUT) {
        if (digitalRead(BOARD_FLAG_PIN) == BOARD_READY_FLAG_VALUE) {
            log_esp3d("Yes");
            return true;
        }
    }
    log_esp3d("Time out no board answer");
    return false;
}

bool MKSService::sendNetworkFrame()
{

    size_t dataOffset = 0;;
    String s;
    static uint32_t lastsend  = 0;
    if ((millis() - lastsend)> NET_FRAME_REFRESH_TIME) {
        lastsend = millis();
        log_esp3d("Network frame preparation");
        //Prepare
        clearFrame();
        _frame[MKS_FRAME_HEAD_OFFSET] = MKS_FRAME_HEAD_FLAG;
        _frame[MKS_FRAME_TYPE_OFFSET] = MKS_FRAME_DATA_NETWORK_TYPE;
        if (NetConfig::getMode() == ESP_WIFI_STA) {
            log_esp3d("STA Mode");
            if(WiFi.status() == WL_CONNECTED) {
                ///////////////////////////////////
                //IP Segment
                //IP value
                IPAddress ip = NetConfig::localIPAddress();
                _frame[MKS_FRAME_DATA_OFFSET] = ip[0];
                _frame[MKS_FRAME_DATA_OFFSET + 1] = ip[1];
                _frame[MKS_FRAME_DATA_OFFSET + 2] = ip[2];
                _frame[MKS_FRAME_DATA_OFFSET + 3] = ip[3];
                log_esp3d("IP %d.%d.%d.%d", _frame[MKS_FRAME_DATA_OFFSET],_frame[MKS_FRAME_DATA_OFFSET + 1],_frame[MKS_FRAME_DATA_OFFSET + 2],_frame[MKS_FRAME_DATA_OFFSET + 3]);
                //////////////////////////////////
                //State Segment
                //Connected state (OK)
                _frame[MKS_FRAME_DATA_OFFSET + 6] = MKS_FRAME_NETWORK_OK_STATE;
            } else {
                ///////////////////////////////////
                //IP Segment
                //No need - bytes are already cleared
                //////////////////////////////////
                //State Segment
                //Connected state (Disconnected)
                _frame[MKS_FRAME_DATA_OFFSET + 6] = MKS_FRAME_NETWORK_FAIL_STATE;
            }
            //////////////////////////////////
            //Mode Segment
            _frame[MKS_FRAME_DATA_OFFSET + 7] = MKS_FRAME_NETWORK_STA_MODE;
            //////////////////////////////////
            //Wifi_name_len Segment
            s = Settings_ESP3D::read_string(ESP_STA_SSID);
            _frame[MKS_FRAME_DATA_OFFSET + 8] = s.length();
            dataOffset = MKS_FRAME_DATA_OFFSET + 9;
            //////////////////////////////////
            //Wifi_name Segment
            strcpy(&_frame[dataOffset], s.c_str());
            dataOffset+=s.length();
            //////////////////////////////////
            //Wifi_key_len Segment
            s = Settings_ESP3D::read_string(ESP_STA_PASSWORD);
            _frame[dataOffset] = s.length();
            dataOffset++;
            //////////////////////////////////
            //Wifi_key Segment
            strcpy(&_frame[dataOffset], s.c_str());
            dataOffset+=s.length();
        } else if (NetConfig::getMode() == ESP_WIFI_AP) {
            log_esp3d("AP Mode");
            ///////////////////////////////////
            //IP Segment
            //IP value
            IPAddress ip = NetConfig::localIPAddress();
            _frame[MKS_FRAME_DATA_OFFSET] = ip[0];
            _frame[MKS_FRAME_DATA_OFFSET + 1] = ip[1];
            _frame[MKS_FRAME_DATA_OFFSET + 2] = ip[2];
            _frame[MKS_FRAME_DATA_OFFSET + 3] = ip[3];
            //////////////////////////////////
            //State Segment
            //Connected state (OK)
            _frame[MKS_FRAME_DATA_OFFSET + 6] = MKS_FRAME_NETWORK_OK_STATE;
            //////////////////////////////////
            //Mode Segment
            _frame[MKS_FRAME_DATA_OFFSET + 7] = MKS_FRAME_NETWORK_AP_MODE;
            //////////////////////////////////
            //Wifi_name_len Segment
            String s = Settings_ESP3D::read_string(ESP_AP_SSID);
            _frame[MKS_FRAME_DATA_OFFSET + 8] = s.length();
            dataOffset = MKS_FRAME_DATA_OFFSET + 9;
            //////////////////////////////////
            //Wifi_name Segment
            strcpy(&_frame[dataOffset], s.c_str());
            dataOffset+=s.length();
            //////////////////////////////////
            //Wifi_key_len Segment
            s = Settings_ESP3D::read_string(ESP_AP_PASSWORD);
            _frame[dataOffset] = s.length();
            dataOffset++;
            //////////////////////////////////
            //Wifi_key Segment
            strcpy(&_frame[dataOffset], s.c_str());
            dataOffset+=s.length();
        } else {
            //not supported
            log_esp3d("Mode not supported : %d ", NetConfig::getMode());
            return false;
        }
        //////////////////////////////////
        //Cloud Services port Segment
        //hard coded
        _frame[MKS_FRAME_DATA_OFFSET +4] = CLOUD_SERVICE_PORT & 0xff;
        _frame[MKS_FRAME_DATA_OFFSET +5] = (CLOUD_SERVICE_PORT>> 8 ) & 0xff;
        log_esp3d("Cloud port: %d", CLOUD_SERVICE_PORT);

        //////////////////////////////////
        //Cloud State Segment
        //hard coded as disabled in upstream FW
        _frame[dataOffset] = MKS_FRAME_CLOUD_DISABLED_STATE;
        dataOffset++;
        //////////////////////////////////
        //Cloud host len Segment
        //hard coded
        _frame[dataOffset] = strlen(CLOUD_HOST_ADDRESS);
        dataOffset++;
        //////////////////////////////////
        //Cloud host Segment
        //hard coded
        strcpy(&_frame[dataOffset], CLOUD_HOST_ADDRESS);
        dataOffset+=strlen(CLOUD_HOST_ADDRESS);
        //////////////////////////////////
        //Cloud host port Segment
        //hard coded
        _frame[dataOffset] = CLOUD_HOST_PORT & 0xff;
        dataOffset++;
        _frame[dataOffset] = (CLOUD_HOST_PORT>> 8 ) & 0xff;
        dataOffset++;
        //////////////////////////////////
        //Module id len Segment
        //???
        _frame[dataOffset] = strlen(_moduleId);
        dataOffset++;
        //////////////////////////////////
        //Module id  Segment
        //???
        strcpy(&_frame[dataOffset], _moduleId);
        dataOffset+=strlen(_moduleId);
        //////////////////////////////////
        //FW version len Segment
        //???
        _frame[dataOffset] = strlen(FW_VERSION);
        dataOffset++;
        //////////////////////////////////
        //FW version  Segment
        //???
        strcpy(&_frame[dataOffset], FW_VERSION);
        dataOffset+=strlen(FW_VERSION);
        //////////////////////////////////
        //Tail Segment
        _frame[dataOffset] = MKS_FRAME_TAIL_FLAG;

        //////////////////////////////////
        //Data len Segment
        //Calculated from above
        _frame[MKS_FRAME_DATALEN_OFFSET] = (dataOffset-4) & 0xff;
        _frame[MKS_FRAME_DATALEN_OFFSET+1] = ((dataOffset-4) >> 8) & 0xff;
        log_esp3d("Size of data in frame %d ", dataOffset-4);
        if (canSendFrame()) {
            ESP3DOutput output(ESP_SERIAL_CLIENT);
            if (output.write((const uint8_t *)_frame,dataOffset+1) == (dataOffset+1)) {
                log_esp3d("Ok");
                return true;
            }
        }
        log_esp3d("Failed");
    }

    return false;
}
void MKSService::clearFrame()
{
    memset(_frame, 0, sizeof(_frame));
}
void MKSService::handle()
{
    if (_started ) {
        sendNetworkFrame();
    }
    //network frame every 10s
}
void MKSService::end()
{
    _started = false;
}




#endif //COMMUNICATION_PROTOCOL == MKS_SERIAL
