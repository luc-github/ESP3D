/*
  serial_service.cpp -  serial services functions class

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
#if COMMUNICATION_PROTOCOL == MKS_SERIAL || COMMUNICATION_PROTOCOL == RAW_SERIAL
#include "serial_service.h"
#include "../../core/settings_esp3d.h"
#include "../../core/esp3doutput.h"
#include "../../core/commands.h"
#if COMMUNICATION_PROTOCOL == MKS_SERIAL
#include "../mks/mks_service.h"
#endif //COMMUNICATION_PROTOCOL == MKS_SERIAL
#include "../authentication/authentication_service.h"

//Serial Parameters
#define ESP_SERIAL_PARAM SERIAL_8N1

#if ESP_SERIAL_OUTPUT == USE_SERIAL_0
#define ESP3D_SERIAL Serial
#endif //USE_SERIAL_0

#if ESP_SERIAL_OUTPUT == USE_SERIAL_1
#define ESP3D_SERIAL Serial1
#endif //USE_SERIAL_1

#if ESP_SERIAL_OUTPUT == USE_SERIAL_2
#define ESP3D_SERIAL Serial2
#endif //USE_SERIAL_2

#define ESP3DSERIAL_RUNNING_PRIORITY 1
#define ESP3DSERIAL_RUNNING_CORE 1
#define SERIAL_YIELD 10

SerialService serial_service;
#if defined(ARDUINO_ARCH_ESP32) && defined(SERIAL_INDEPENDANT_TASK)
TaskHandle_t _hserialtask= nullptr;
#endif //ARDUINO_ARCH_ESP32 

const long SupportedBaudList[] = {9600, 19200, 38400, 57600, 74880, 115200, 230400, 250000, 500000, 921600, 1958400};

#define TIMEOUT_SERIAL_FLUSH 1500
//Constructor
SerialService::SerialService()
{
    _buffer_size = 0;
    _started = false;
    _needauthentication = true;
}

//Destructor
SerialService::~SerialService()
{
    end();
}

//dedicated serial task
#if defined(ARDUINO_ARCH_ESP32) && defined(SERIAL_INDEPENDANT_TASK)
void ESP3DSerialTaskfn( void * parameter )
{
    for(;;) {
        serial_service.process();
        Hal::wait(SERIAL_YIELD);  // Yield to other tasks
    }
    vTaskDelete( NULL );
}
#endif //ARDUINO_ARCH_ESP32 

//extra parameters that do not need a begin
void SerialService::setParameters()
{
#if defined (AUTHENTICATION_FEATURE)
    _needauthentication = (Settings_ESP3D::read_byte (ESP_SECURE_SERIAL)==0)?false:true;
#else
    _needauthentication = false;
#endif //AUTHENTICATION_FEATURE
}

//Setup Serial
bool SerialService::begin()
{
    _lastflush = millis();
    //read from settings
    long br = Settings_ESP3D::read_uint32(ESP_BAUD_RATE);
    setParameters();
    _buffer_size = 0;
    //change only if different from current
    if (br != baudRate() || (ESP_RX_PIN != -1) || (ESP_TX_PIN != -1)) {
        if ( !is_valid_baudrate(br)) {
            br = Settings_ESP3D::get_default_int32_value(ESP_BAUD_RATE);
        }
        ESP3D_SERIAL.setRxBufferSize (SERIAL_RX_BUFFER_SIZE);
#ifdef ARDUINO_ARCH_ESP8266
        ESP3D_SERIAL.begin(br, ESP_SERIAL_PARAM, SERIAL_FULL, (ESP_TX_PIN == -1)?1:ESP_TX_PIN);
#if ESP_RX_PIN != -1
        ESP3D_SERIAL.pins((ESP_TX_PIN == -1)?1:ESP_TX_PIN, ESP_RX_PIN)
#endif //ESP_RX_PIN != -1
#endif //ARDUINO_ARCH_ESP8266
#if defined(ARDUINO_ARCH_ESP32)
        ESP3D_SERIAL.begin (br, ESP_SERIAL_PARAM, ESP_RX_PIN, ESP_TX_PIN);
#if defined(SERIAL_INDEPENDANT_TASK)
        //create serial task once
        if (_hserialtask == nullptr) {
            xTaskCreatePinnedToCore(
                ESP3DSerialTaskfn, /* Task function. */
                "ESP3D Serial Task", /* name of task. */
                8192, /* Stack size of task */
                NULL, /* parameter of the task */
                ESP3DSERIAL_RUNNING_PRIORITY, /* priority of the task */
                &_hserialtask, /* Task handle to keep track of created task */
                ESP3DSERIAL_RUNNING_CORE    /* Core to run the task */
            );
        }
        if (_hserialtask == nullptr) {
            log_esp3d("Serial Task creation failed");
            return false;
        }
#endif //SERIAL_INDEPENDANT_TASK
#endif //ARDUINO_ARCH_ESP32
    }
    _started = true;
    return true;
}
//End serial
bool SerialService::end()
{
    flush();
    delay (100);
    swap();
    ESP3D_SERIAL.end();
    _buffer_size = 0;
    _started = false;
    return true;
}

//return the array of long and array size
const long * SerialService::get_baudratelist(uint8_t * count)
{
    if (count) {
        *count = sizeof(SupportedBaudList)/sizeof(long);
    }
    return SupportedBaudList;
}

//check if value is in baudrate list
bool SerialService::is_valid_baudrate(long br)
{
    uint8_t listesize = sizeof(SupportedBaudList)/sizeof(long);
    for (uint8_t i = 0; i < listesize ; i++) {
        if (SupportedBaudList[i] == br) {
            return true;
        }
    }
    return false;
}

//Function which could be called in other loop
void SerialService::process()
{
    //Do we have some data waiting
    size_t len = available();
    if (len > 0) {
        //if yes read them
        log_esp3d("Got %d chars in serial", len);
        uint8_t * sbuf = (uint8_t *)malloc(len);
        if(sbuf) {
            size_t count = readBytes(sbuf, len);
            //push to buffer
            if (count > 0) {
                push2buffer(sbuf, count);
            }
            //freen buffer
            free(sbuf);
        }
    }
    //we cannot left data in buffer too long
    //in case some commands "forget" to add \n
    if (((millis() - _lastflush) > TIMEOUT_SERIAL_FLUSH) && (_buffer_size > 0)) {
        flushbuffer();
    }
}

//Function which could be called in other loop
void SerialService::handle()
{
//for ESP32 there is dedicated task for it
#if !(defined(ARDUINO_ARCH_ESP32) && defined(SERIAL_INDEPENDANT_TASK))
    process();
#endif //ARDUINO_ARCH_ESP8266

}

void SerialService::flushbuffer()
{
    ESP3DOutput output(ESP_SERIAL_CLIENT);
    _buffer[_buffer_size] = 0x0;
    //dispatch command
    esp3d_commands.process(_buffer, _buffer_size, &output,_needauthentication?LEVEL_GUEST:LEVEL_ADMIN);
    _lastflush = millis();
    _buffer_size = 0;
}

//push collected data to buffer and proceed accordingly
void SerialService::push2buffer(uint8_t * sbuf, size_t len)
{
    log_esp3d("buffer get %d data ", len);
#if COMMUNICATION_PROTOCOL == MKS_SERIAL
    static bool isFrameStarted = false;
    static bool isCommandFrame = false;
    static uint8_t type;
    //expected size
    static int16_t framePos = -1;
    //currently received
    static uint datalen = 0;
    for (size_t i = 0; i < len; i++) {
        log_esp3d("Data : %c %x", sbuf[i],sbuf[i]);
        framePos++;
        _lastflush = millis();
        //so frame head was detected
        if (isFrameStarted) {
            //checking it is a valid Frame header
            if (framePos==1) {
                log_esp3d("type = %x",sbuf[i]);
                if(MKSService::isFrame(char(sbuf[i]))) {
                    if (MKSService::isCommand(char(sbuf[i]))) {
                        isCommandFrame =true;
                        log_esp3d("type: Command");
                    } else {
                        log_esp3d("type: other");
                        type = sbuf[i];
                        isCommandFrame =false;
                    }
                } else {
                    log_esp3d("wrong frame type");
                    isFrameStarted = false;
                    _buffer_size = 0;
                }
            } else if ((framePos==2) || (framePos==3)) {
                //add size to int
                if (framePos==2) {
                    datalen = sbuf[i];
                } else {
                    datalen += (sbuf[i]<<8);
                    log_esp3d("Data len: %d", datalen);
                    if (datalen > (ESP3D_SERIAL_BUFFER_SIZE -5)) {
                        log_esp3d("Overflow in data len");
                        isFrameStarted = false;
                        _buffer_size = 0;
                    }
                }
            } else if (MKSService::isTail(char(sbuf[i]))) {
                log_esp3d("got tail");
                _buffer[_buffer_size]='\0';
                log_esp3d("size is %d", _buffer_size);
                //let check integrity
                if (_buffer_size == datalen) {
                    log_esp3d("Flushing buffer");
                    if (isCommandFrame) {
                        flushbuffer();
                    } else {
                        MKSService::handleFrame(type,(const uint8_t*)_buffer, _buffer_size);
                    }
                } else {
                    log_esp3d("Error in data len");
                }
                //clear frame infos
                _buffer_size = 0;
                isFrameStarted = false;

            } else {
                //it is data
                if (_buffer_size < ESP3D_SERIAL_BUFFER_SIZE -5) {
                    _buffer[_buffer_size] = sbuf[i];
                    _buffer_size++;
                } else {
                    log_esp3d("Overflow in data len");
                    isFrameStarted = false;
                    _buffer_size = 0;
                }

            }
        } else {
            //frame is not started let see if it is a head
            if (MKSService::isHead(char(sbuf[i]))) {
                log_esp3d("got head");
                //yes it is
                isFrameStarted = true;
                framePos =0;
                _buffer_size = 0;
            } else {
                //no so let reset all and just ignore it
                //TODO should we handle these data ?
                log_esp3d("Unidentified data : %c %x", sbuf[i],sbuf[i]);
                isCommandFrame = false;
                framePos = -1;
                datalen = 0;
            }
        }
    }
#else
    for (size_t i = 0; i < len; i++) {
        _lastflush = millis();
        //command is defined
        if ((char(sbuf[i]) == '\n')|| (char(sbuf[i]) == '\r')) {
            if (_buffer_size < ESP3D_SERIAL_BUFFER_SIZE) {
                _buffer[_buffer_size] = sbuf[i];
                _buffer_size++;
            }
            flushbuffer();
        } else if (isPrintable (char(sbuf[i]) ))  {
            if (_buffer_size < ESP3D_SERIAL_BUFFER_SIZE) {
                _buffer[_buffer_size] = sbuf[i];
                _buffer_size++;
            } else {
                flushbuffer();
                _buffer[_buffer_size] = sbuf[i];
                _buffer_size++;
            }
        } else { //it is not printable char
            //clean buffer first
            if (_buffer_size > 0) {
                flushbuffer();
            }
            //process char
            _buffer[_buffer_size] = sbuf[i];
            _buffer_size++;
            flushbuffer();
        }
    }
#endif
}

//Reset Serial Setting (baud rate)
bool SerialService::reset()
{
    log_esp3d("Reset serial");
    return Settings_ESP3D::write_uint32 (ESP_BAUD_RATE, Settings_ESP3D::get_default_int32_value(ESP_BAUD_RATE));
}

void SerialService::updateBaudRate(long br)
{
    if (br!=baudRate()) {
        ESP3D_SERIAL.flush();
        ESP3D_SERIAL.updateBaudRate(br);
    }
}

//Get current baud rate
long SerialService::baudRate()
{
    long br = 0;
    br = ESP3D_SERIAL.baudRate();
#ifdef ARDUINO_ARCH_ESP32
    //workaround for ESP32
    if (br == 115201) {
        br = 115200;
    }
    if (br == 230423) {
        br = 230400;
    }
#endif //ARDUINO_ARCH_ESP32
    return br;
}

size_t SerialService::write(uint8_t c)
{
    return ESP3D_SERIAL.write(c);
}

size_t SerialService::write(const uint8_t *buffer, size_t size)
{
    if ((uint)ESP3D_SERIAL.availableForWrite() >= size) {
        return ESP3D_SERIAL.write(buffer, size);
    } else {
        size_t sizetosend = size;
        size_t sizesent = 0;
        uint8_t *buffertmp=(uint8_t *)buffer;
        uint32_t starttime = millis();
        //loop until all is sent or timeout
        while (sizetosend>0 && ((millis() - starttime) < 100)) {
            size_t available = ESP3D_SERIAL.availableForWrite();
            if(available>0) {
                //in case less is sent
                available = ESP3D_SERIAL.write(&buffertmp[sizesent], (available >= sizetosend)?sizetosend:available);
                sizetosend-=available;
                sizesent+=available;
                starttime=millis();
            } else {
                Hal::wait(5);
            }
        }
        return sizesent;
    }
}

int SerialService::availableForWrite()
{
    return ESP3D_SERIAL.availableForWrite();
}

int SerialService::available()
{
    return ESP3D_SERIAL.available();
}

int SerialService::read()
{
    return ESP3D_SERIAL.read();
}

size_t SerialService::readBytes(uint8_t * sbuf, size_t len)
{
    return ESP3D_SERIAL.readBytes(sbuf, len);
}

void SerialService::flush()
{
    ESP3D_SERIAL.flush();
}

void SerialService::swap()
{
#ifdef ARDUINO_ARCH_ESP8266
    ESP3D_SERIAL.swap();
#endif //ARDUINO_ARCH_ESP8266
}

#endif //COMMUNICATION_PROTOCOL == MKS_SERIAL || COMMUNICATION_PROTOCOL == RAW_SERIAL