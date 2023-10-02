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
#include "../telnet/telnet_server.h"
#include "../http/http_server.h"
#include "../network/netconfig.h"
#include "../serial/serial_service.h"

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

#define MKS_TYPE_NET            (char)0x0
#define MKS_TYPE_PRINTER        (char)0x1
#define MKS_TYPE_TRANSFER       (char)0x2
#define MKS_TYPE_EXCEPTION      (char)0x3
#define MKS_TYPE_CLOUD          (char)0x4
#define MKS_TYPE_UNBIND         (char)0x5
#define MKS_TYPE_WID            (char)0x6
#define MKS_TYPE_SCAN_WIFI      (char)0x7
#define MKS_TYPE_MANUAL_IP      (char)0x8
#define MKS_TYPE_WIFI_CTRL      (char)0x9

#define CONNECT_STA         0x1
#define DISCONNECT_STA      0x2
#define REMOVE_STA_INFO     0x3

#define UNKNOW_STATE    0x0
#define ERROR_STATE     0x1
#define SUCCESS_STATE   0x2

#define NB_HOTSPOT_MAX  15

//Timeouts
#define FRAME_WAIT_TO_SEND_TIMEOUT  2000
#define ACK_TIMEOUT  5000
#define NET_FRAME_REFRESH_TIME  10000

#define UPLOAD_BAUD_RATE    1958400

bool MKSService::_started = false;
uint8_t MKSService::_frame[MKS_FRAME_SIZE] = {0};
char MKSService::_moduleId[22] = {0};
uint8_t MKSService::_uploadStatus = UNKNOW_STATE;
long MKSService::_commandBaudRate = 115200;
bool MKSService::_uploadMode = false;

bool MKSService::isHead(const char c)
{
    return (c==MKS_FRAME_HEAD_FLAG);
}
bool MKSService::isTail(const char c)
{
    return (c==MKS_FRAME_TAIL_FLAG);
}
bool MKSService::isCommand(const char c)
{
    return (c==MKS_TYPE_TRANSFER);
}
bool MKSService::isFrame(const char c)
{
    if ((c>=MKS_TYPE_NET)&& (c<=MKS_TYPE_WIFI_CTRL)) {
        return true;
    }
    return false;
}
bool MKSService::begin()
{
    //setup the pins
    pinMode(BOARD_FLAG_PIN, INPUT);
    pinMode(ESP_FLAG_PIN, OUTPUT);
    _started = true;
    //max size is 21
    sprintf (_moduleId, "HJNLM000%02X%02X%02X%02X%02X%02X", WiFi.macAddress()[0], WiFi.macAddress()[1], WiFi.macAddress()[2], WiFi.macAddress()[3], WiFi.macAddress()[4], WiFi.macAddress()[5]);
    commandMode(true);
    return true;
}

void MKSService::commandMode(bool fromSettings)
{
    if (fromSettings) {
        _commandBaudRate= Settings_ESP3D::read_uint32(ESP_BAUD_RATE);
    }
    log_esp3d("Cmd Mode");
    _uploadMode = false;
    serial_service.updateBaudRate(_commandBaudRate);

}
void MKSService::uploadMode()
{
    log_esp3d("Upload Mode");
    _uploadMode = true;
    serial_service.updateBaudRate(UPLOAD_BAUD_RATE);
}

uint MKSService::getFragmentID(uint32_t fragmentNumber, bool isLast)
{
    log_esp3d("Fragment: %d %s",fragmentNumber, isLast?" is last":"" );
    if (isLast) {
        fragmentNumber |= (1 << 31);
    } else {
        fragmentNumber &= ~(1 << 31);
    }
    log_esp3d("Fragment is now: %d",fragmentNumber);
    return fragmentNumber;
}

bool MKSService::sendFirstFragment(const char* filename, size_t filesize)
{
    uint fileNameLen = strlen(filename);
    uint dataLen = fileNameLen + 5;
    clearFrame();
    //Head Flag
    _frame[MKS_FRAME_HEAD_OFFSET] = MKS_FRAME_HEAD_FLAG;
    //Type Flag
    _frame[MKS_FRAME_TYPE_OFFSET] = MKS_FRAME_DATA_FIRST_FRAGMENT_TYPE;
    //Fragment size
    _frame[MKS_FRAME_DATALEN_OFFSET] = dataLen & 0xff;
    _frame[MKS_FRAME_DATALEN_OFFSET + 1] = dataLen >> 8;
    //FileName size
    _frame[MKS_FRAME_DATA_OFFSET] = strlen(filename);
    //File Size
    _frame[MKS_FRAME_DATA_OFFSET+1] = filesize & 0xff;
    _frame[MKS_FRAME_DATA_OFFSET+2] = (filesize >> 8) & 0xff;
    _frame[MKS_FRAME_DATA_OFFSET+3] = (filesize >> 16) & 0xff;
    _frame[MKS_FRAME_DATA_OFFSET+4] = (filesize >> 24) & 0xff;
    //Filename
    strncpy((char *)&_frame[MKS_FRAME_DATA_OFFSET+ 5], filename, fileNameLen);
    //Tail Flag
    _frame[dataLen + 4] = MKS_FRAME_TAIL_FLAG;
    log_esp3d("Filename: %s  Filesize: %d",filename, filesize );
    for (uint i =0; i< dataLen + 5 ; i++) {
        log_esp3d("%c %x",_frame[i],_frame[i]);
    }
    _uploadStatus = UNKNOW_STATE;
    if (canSendFrame()) {
        ESP3DOutput output(ESP_SERIAL_CLIENT);
        _uploadStatus = UNKNOW_STATE;
        if (output.write(_frame,dataLen + 5) == (dataLen + 5)) {
            log_esp3d("First fragment Ok");
            sendFrameDone();
            return true;
        }
    }
    log_esp3d("Failed");
    sendFrameDone();
    return false;
}


bool MKSService::sendFragment(const uint8_t * dataFrame, const size_t dataSize,uint fragmentID)
{
    uint dataLen = dataSize + 4;
    log_esp3d("Fragment datalen:%d",dataSize);
    //Head Flag
    _frame[MKS_FRAME_HEAD_OFFSET] = MKS_FRAME_HEAD_FLAG;
    //Type Flag
    _frame[MKS_FRAME_TYPE_OFFSET] = MKS_FRAME_DATA_FRAGMENT_TYPE;
    //Fragment size
    _frame[MKS_FRAME_DATALEN_OFFSET] = dataLen & 0xff;
    _frame[MKS_FRAME_DATALEN_OFFSET + 1] = dataLen >> 8;
    //Fragment ID
    _frame[MKS_FRAME_DATA_OFFSET ] = fragmentID & 0xff;
    _frame[MKS_FRAME_DATA_OFFSET + 1] = (fragmentID >> 8) & 0xff;
    _frame[MKS_FRAME_DATA_OFFSET + 2] = (fragmentID >> 16) & 0xff;
    _frame[MKS_FRAME_DATA_OFFSET + 3] = (fragmentID >> 24) & 0xff;
    //data
    if ((dataSize>0) && (dataFrame!=nullptr)) {
        memcpy(&_frame[MKS_FRAME_DATA_OFFSET+ 4], dataFrame, dataSize);
    }
    if (dataSize<MKS_FRAME_DATA_MAX_SIZE) {
        clearFrame(dataLen + 4);
    }
    //Tail Flag
    _frame[dataLen + 4] = MKS_FRAME_TAIL_FLAG;
    /* for (uint i =0; i< dataLen + 5 ; i++) {
             log_esp3d("%c %x",_frame[i],_frame[i]);
         }*/
    if (canSendFrame()) {
        ESP3DOutput output(ESP_SERIAL_CLIENT);
        _uploadStatus = UNKNOW_STATE;
        if (output.write(_frame,MKS_FRAME_SIZE) == MKS_FRAME_SIZE) {
            log_esp3d("Ok");
            sendFrameDone();
            return true;
        }
        log_esp3d("Error with size sent");
    }
    log_esp3d("Failed");
    sendFrameDone();
    return false;
}

void MKSService::sendWifiHotspots()
{

    uint8_t ssid_name_length;
    uint dataOffset = 1;
    uint8_t total_hotspots = 0;
    uint8_t currentmode = WiFi.getMode();
    if(currentmode==WIFI_AP) {
        WiFi.mode(WIFI_AP_STA);
    }
    clearFrame();
    //clean memory
    WiFi.scanDelete();
    int n = WiFi.scanNetworks();
    log_esp3d("scan done");
    if (n == 0) {
        log_esp3d("no networks found");
    } else {
        log_esp3d("%d networks found", n);
        clearFrame();
        _frame[MKS_FRAME_HEAD_OFFSET] = MKS_FRAME_HEAD_FLAG;
        _frame[MKS_FRAME_TYPE_OFFSET] = MKS_FRAME_DATA_HOTSPOTS_LIST_TYPE;
        for (uint8_t i = 0; i < n; ++i) {
            int8_t signal_rssi = 0;
            if(total_hotspots > NB_HOTSPOT_MAX) {
                break;
            }
            signal_rssi = WiFi.RSSI(i);
            // Print SSID and RSSI for each network found
            log_esp3d("%d: %s (%d) %s",i + 1,WiFi.SSID(i).c_str(),  signal_rssi,(WiFi.encryptionType(i) == ENC_TYPE_NONE)?" ":"*" );
            ssid_name_length = WiFi.SSID(i).length();
            if(ssid_name_length > MAX_SSID_LENGTH) {
                log_esp3d("Name too long, ignored" );
                continue;
            }
            if(signal_rssi < MIN_RSSI) {
                log_esp3d("Signal too low, ignored" );
                continue;
            }
            _frame[MKS_FRAME_DATA_OFFSET + dataOffset] = ssid_name_length;
            for (uint8_t p = 0; p < ssid_name_length; p++) {
                _frame[MKS_FRAME_DATA_OFFSET + dataOffset+1+p] = WiFi.SSID(i)[p];
            }
            _frame[MKS_FRAME_DATA_OFFSET + dataOffset + ssid_name_length + 1] = WiFi.RSSI(i);
            dataOffset+=ssid_name_length+2;
            total_hotspots++;
        }
        _frame[MKS_FRAME_DATA_OFFSET] = total_hotspots;
        _frame[MKS_FRAME_DATA_OFFSET + dataOffset] = MKS_FRAME_TAIL_FLAG;
        _frame[MKS_FRAME_DATALEN_OFFSET] = dataOffset & 0xff;
        _frame[MKS_FRAME_DATALEN_OFFSET + 1] = dataOffset >> 8;
        log_esp3d("Size of data in frame %d ", dataOffset);
        for (uint i =0; i< dataOffset + 5 ; i++) {
            log_esp3d("%c %x",_frame[i],_frame[i]);
        }
        if (canSendFrame()) {
            ESP3DOutput output(ESP_SERIAL_CLIENT);
            if (output.write(_frame,dataOffset+5) == (dataOffset+5)) {
                log_esp3d("Ok");
                sendFrameDone();
            } else {
                log_esp3d("Send scan failed");
            }
        } else {
            log_esp3d("Cannot send scan");
        }
        //clean memory
        WiFi.scanDelete();
    }
    //Restore mode
    WiFi.mode((WiFiMode_t)currentmode);
    sendFrameDone();
}

void MKSService::handleFrame(const uint8_t type, const uint8_t * dataFrame, const size_t dataSize )
{
    log_esp3d("Command is %d", type);
    switch(type) {
    //wifi setup
    case MKS_TYPE_NET:
        log_esp3d("************MKS_TYPE_NET*************");
        messageWiFiConfig(dataFrame, dataSize);
        break;
    //not supported in Marlin
    //Confirmed as private source
    case MKS_TYPE_PRINTER :
        //ignored
        log_esp3d("************MKS_TYPE_PRINTER*************");
        break;
    //File transfer if not command
    case MKS_TYPE_TRANSFER :
        //todo
        log_esp3d("************MKS_TYPE_TRANSFER*************");
        break;
    //Error when doing transfer
    case MKS_TYPE_EXCEPTION :
        log_esp3d("************MKS_TYPE_EXCEPTION*************");
        messageException(dataFrame, dataSize);
        break;
    //not supported (cloud)
    case MKS_TYPE_CLOUD :
        //ignored
        log_esp3d("************MKS_TYPE_CLOUD*************");
        break;
    //not supported (cloud)
    case MKS_TYPE_WID :
        //ignored
        log_esp3d("************MKS_TYPE_WID*************");
        break;
    //hot spot list
    case MKS_TYPE_SCAN_WIFI :
        log_esp3d("************MKS_TYPE_SCAN_WIFI*************");
        sendWifiHotspots();
        break;
    //setup Manual IP
    //not supported in Marlin, so do same for the moment
    case MKS_TYPE_MANUAL_IP :
        //ignored
        log_esp3d("************MKS_TYPE_MANUAL_IP*************");
        break;
    //On/Off Wifi
    case MKS_TYPE_WIFI_CTRL :
        log_esp3d("************MKS_TYPE_WIFI_CTRL*************");
        messageWiFiControl(dataFrame,dataSize);
        break;
    default:
        log_esp3d("Unknow type");
    }
}

void MKSService::messageWiFiControl(const uint8_t * dataFrame, const size_t dataSize )
{
    if(dataSize != 1) {
        return;
    }
    switch (dataFrame[0]) {
    case CONNECT_STA:
        log_esp3d("CONNECT_STA");
        if (!NetConfig::started()) {
            NetConfig::begin();
        }
        break;
    case DISCONNECT_STA:
        log_esp3d("CONNECT_STA");
        if (NetConfig::started()) {
            NetConfig::end();
        }
        break;
    case REMOVE_STA_INFO:
        log_esp3d("REMOVE_STA_INFO");
        if (NetConfig::started()) {
            NetConfig::end();
        }
        Settings_ESP3D::reset(true);
        break;
    default:
        log_esp3d("WiFi control flag not supported");
    }
}
//Exception handle - but actually not used
void MKSService::messageException(const uint8_t * dataFrame, const size_t dataSize )
{
    if(dataSize != 1) {
        return;
    }
    if ((dataFrame[0] == ERROR_STATE) || (dataFrame[0] == SUCCESS_STATE)) {
        _uploadStatus = dataFrame[0];
        log_esp3d("Tranfer: %s",dataFrame[0] == ERROR_STATE?"Error":"Success" );
    } else {
        _uploadStatus = UNKNOW_STATE;
        log_esp3d("Tranfer state unknown" );
    }
}

void MKSService::messageWiFiConfig(const uint8_t * dataFrame, const size_t dataSize )
{
    String ssid;
    String password;
    String savedSsid;
    String savedPassword;
    bool needrestart =  false;
    //Sanity check
    if(dataSize  <2) {
        log_esp3d("Invalid data");
        return;
    }
    if((dataFrame[0] != MKS_FRAME_NETWORK_AP_MODE) && (dataFrame[0] != MKS_FRAME_NETWORK_STA_MODE)) {
        log_esp3d("Invalid mode");
        return;
    }
    if ((dataFrame[1] > dataSize - 3) || (dataFrame[1]==0) || (dataFrame[1]>MAX_SSID_LENGTH)) {
        log_esp3d("Invalid ssid size");
        return;
    }
    if ((uint)(dataFrame[1]+3)> dataSize) {
        log_esp3d("Overflow password size");
        return;
    }
    if ((dataFrame[dataFrame[1]+2])> MAX_PASSWORD_LENGTH) {
        log_esp3d("Invalid password size");
        return;
    }
    //get SSID and password
    for(uint8_t i = 0; i < dataFrame[1]; i++) {
        ssid+=(char)dataFrame[2+i];
    }
    for(uint8_t j = 0; j < dataFrame[2+dataFrame[1]]; j++) {
        password+=(char)dataFrame[3+j+dataFrame[1]];
    }
    if (dataFrame[0] == MKS_FRAME_NETWORK_AP_MODE) {
        if (Settings_ESP3D::read_byte(ESP_RADIO_MODE)!=ESP_WIFI_AP) {
            Settings_ESP3D::write_byte(ESP_RADIO_MODE,ESP_WIFI_AP);
            needrestart=true;
        }
        savedSsid=Settings_ESP3D::read_string(ESP_AP_SSID);
        savedPassword=Settings_ESP3D::read_string(ESP_AP_PASSWORD);
        if (savedSsid!=ssid) {
            Settings_ESP3D::write_string(ESP_AP_SSID,ssid.c_str());
            needrestart =true;
        }
        if (savedPassword!=password) {
            Settings_ESP3D::write_string(ESP_AP_PASSWORD,password.c_str());
            needrestart =true;
        }
    } else {
        if (Settings_ESP3D::read_byte(ESP_RADIO_MODE)!=ESP_WIFI_STA) {
            Settings_ESP3D::write_byte(ESP_RADIO_MODE,ESP_WIFI_STA);
            needrestart =true;
        }
        savedSsid=Settings_ESP3D::read_string(ESP_STA_SSID);
        savedPassword=Settings_ESP3D::read_string(ESP_STA_PASSWORD);
        if (savedSsid!=ssid) {
            Settings_ESP3D::write_string(ESP_STA_SSID,ssid.c_str());
            needrestart =true;
        }
        if (savedPassword!=password) {
            Settings_ESP3D::write_string(ESP_STA_PASSWORD,password.c_str());
            needrestart =true;
        }
        if (needrestart) {
            //change also to DHCP for new value
            Settings_ESP3D::write_byte(ESP_STA_IP_MODE,DHCP_MODE);
        }

    }
    if (needrestart) {
        log_esp3d("Modifications done - restarting network");
        NetConfig::begin();
    }
}

bool MKSService::canSendFrame()
{
    log_esp3d("Is board ready for frame?");
    digitalWrite(ESP_FLAG_PIN, BOARD_READY_FLAG_VALUE);
    uint32_t startTime = millis();
    while( (millis() - startTime) <  FRAME_WAIT_TO_SEND_TIMEOUT) {
        if (digitalRead(BOARD_FLAG_PIN) == BOARD_READY_FLAG_VALUE) {
            log_esp3d("Yes");
            return true;
        }
        Hal::wait(0);
    }
    log_esp3d("Time out no board answer");
    return false;
}

void MKSService::sendFrameDone()
{
    digitalWrite(ESP_FLAG_PIN, !BOARD_READY_FLAG_VALUE);

}

bool MKSService::sendGcodeFrame(const char* cmd)
{
    if (_uploadMode) {
        return false;
    }
    String tmp = cmd;
    if (tmp.endsWith("\n")) {
        tmp[tmp.length()-1]='\0';
    }
    log_esp3d("Packing: *%s*, size=%d", tmp.c_str(), strlen(tmp.c_str()));
    clearFrame();
    _frame[MKS_FRAME_HEAD_OFFSET] = MKS_FRAME_HEAD_FLAG;
    _frame[MKS_FRAME_TYPE_OFFSET] = MKS_FRAME_DATA_COMMAND_TYPE;
    for(uint i = 0 ; i < strlen(tmp.c_str()); i++) {
        _frame[MKS_FRAME_DATA_OFFSET + i]=tmp[i];
    }
    _frame[MKS_FRAME_DATA_OFFSET + strlen(tmp.c_str())] = '\r';
    _frame[MKS_FRAME_DATA_OFFSET + strlen(tmp.c_str())+1] = '\n';
    _frame[MKS_FRAME_DATA_OFFSET + strlen(tmp.c_str())+2] = MKS_FRAME_TAIL_FLAG;
    _frame[MKS_FRAME_DATALEN_OFFSET] = (strlen(tmp.c_str())+2) & 0xff;
    _frame[MKS_FRAME_DATALEN_OFFSET+1] = ((strlen(tmp.c_str())+2) >> 8) & 0xff;

    log_esp3d("Size of data in frame %d ", strlen(tmp.c_str())+2);
    //for (uint i =0; i< strlen(tmp.c_str())+7;i++){
    //log_esp3d("%c %x",_frame[i],_frame[i]);
    //}

    if (canSendFrame()) {
        ESP3DOutput output(ESP_SERIAL_CLIENT);
        if (output.write(_frame,strlen(tmp.c_str())+7) == (strlen(tmp.c_str())+7)) {
            log_esp3d("Ok");
            sendFrameDone();
            return true;
        }
    }
    log_esp3d("Failed");
    sendFrameDone();
    return false;
}

bool MKSService::sendNetworkFrame()
{

    size_t dataOffset = 0;
    String s;
    static uint32_t lastsend  = 0;
    if (_uploadMode) {
        return false;
    }
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
            strcpy((char *)&_frame[dataOffset], s.c_str());
            dataOffset+=s.length();
            //////////////////////////////////
            //Wifi_key_len Segment
            s = Settings_ESP3D::read_string(ESP_STA_PASSWORD);
            _frame[dataOffset] = s.length();
            dataOffset++;
            //////////////////////////////////
            //Wifi_key Segment
            strcpy((char *)&_frame[dataOffset], s.c_str());
            dataOffset+=s.length();
        } else if (NetConfig::getMode() == ESP_WIFI_AP || (NetConfig::getMode() == ESP_AP_SETUP)) {
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
            strcpy((char *)&_frame[dataOffset], s.c_str());
            dataOffset+=s.length();
            //////////////////////////////////
            //Wifi_key_len Segment
            s = Settings_ESP3D::read_string(ESP_AP_PASSWORD);
            _frame[dataOffset] = s.length();
            dataOffset++;
            //////////////////////////////////
            //Wifi_key Segment
            strcpy((char *)&_frame[dataOffset], s.c_str());
            dataOffset+=s.length();
        } else {
            //not supported
            log_esp3d("Mode not supported : %d ", NetConfig::getMode());
            return false;
        }
        //////////////////////////////////
        //Cloud Services port Segment
        //hard coded
        _frame[MKS_FRAME_DATA_OFFSET +4] = (telnet_server.port()) & 0xff;
        _frame[MKS_FRAME_DATA_OFFSET +5] = ((telnet_server.port()) >> 8 ) & 0xff;
        log_esp3d("Cloud port: %d", (telnet_server.port()));

        //////////////////////////////////
        //Cloud State Segment
        //hard coded as disabled in upstream FW
        _frame[dataOffset] = MKS_FRAME_CLOUD_DISABLED_STATE;
        dataOffset++;
        //////////////////////////////////
        //Cloud host len Segment
        //Use ESP3D IP instead
        s = NetConfig::localIPAddress().toString();
        _frame[dataOffset] = s.length();
        dataOffset++;
        //////////////////////////////////
        //Cloud host Segment
        //Use ESP3D IP instead
        strcpy((char *)&_frame[dataOffset], s.c_str());
        dataOffset+=s.length();
        //////////////////////////////////
        //Cloud host port Segment
        //use webserver port instead
        _frame[dataOffset] = (HTTP_Server::port()) & 0xff;
        dataOffset++;
        _frame[dataOffset] = ((HTTP_Server::port())>> 8 ) & 0xff;
        dataOffset++;
        //////////////////////////////////
        //Module id len Segment
        //Use hostname instead
        _frame[dataOffset] = strlen(_moduleId);
        dataOffset++;
        //////////////////////////////////
        //Module id  Segment
        strcpy((char *)&_frame[dataOffset], _moduleId);
        dataOffset+=strlen(_moduleId);
        //////////////////////////////////
        //FW version len Segment
        _frame[dataOffset] = strlen(FW_VERSION)+6;
        dataOffset++;
        //////////////////////////////////
        //FW version  Segment
        strcpy((char *)&_frame[dataOffset], "ESP3D_" FW_VERSION);
        dataOffset+=strlen(FW_VERSION)+6;
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
            if (output.write(_frame,dataOffset+1) == (dataOffset+1)) {
                log_esp3d("Ok");
                sendFrameDone();
                return true;
            }
        }
        sendFrameDone();
        log_esp3d("Failed");
    }

    return false;
}

void MKSService::clearFrame(uint start)
{
    memset(&_frame[start], 0, sizeof(_frame)-start);
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
