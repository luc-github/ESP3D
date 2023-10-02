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
#if COMMUNICATION_PROTOCOL == MKS_SERIAL || COMMUNICATION_PROTOCOL == RAW_SERIAL || defined(ESP_SERIAL_BRIDGE_OUTPUT)
#include "serial_service.h"
#include "../../core/settings_esp3d.h"
#include "../../core/esp3doutput.h"
#include "../../core/commands.h"
#if COMMUNICATION_PROTOCOL == MKS_SERIAL
#include "../mks/mks_service.h"
#endif //COMMUNICATION_PROTOCOL == MKS_SERIAL
#include "../authentication/authentication_service.h"
#if defined (ARDUINO_ARCH_ESP8266)
#define MAX_SERIAL 2
HardwareSerial * Serials[MAX_SERIAL] = {&Serial, &Serial1};
#endif //ARDUINO_ARCH_ESP8266

#if defined (ARDUINO_ARCH_ESP32)

    #if defined (CONFIG_IDF_TARGET_ESP32C3)
        #define MAX_SERIAL 2
        HardwareSerial * Serials[MAX_SERIAL] = {&Serial, &Serial1};
    #else
        #define MAX_SERIAL 3
        HardwareSerial * Serials[MAX_SERIAL] = {&Serial, &Serial1, &Serial2};
    #endif

#endif //ARDUINO_ARCH_ESP32


//Serial Parameters
#define ESP_SERIAL_PARAM SERIAL_8N1

#define ESP3DSERIAL_RUNNING_PRIORITY 1
#define ESP3DSERIAL_RUNNING_CORE 1
#define SERIAL_YIELD 10

SerialService serial_service = SerialService(MAIN_SERIAL);
#if defined(ESP_SERIAL_BRIDGE_OUTPUT)
SerialService serial_bridge_service = SerialService(BRIDGE_SERIAL);
#endif //ESP_SERIAL_BRIDGE_OUTPUT

#if defined(ARDUINO_ARCH_ESP32) && defined(SERIAL_INDEPENDANT_TASK)
TaskHandle_t _hserialtask= nullptr;
#endif //ARDUINO_ARCH_ESP32 

const long SupportedBaudList[] = {9600, 19200, 38400, 57600, 74880, 115200, 230400, 250000, 500000, 921600, 1958400};

#define TIMEOUT_SERIAL_FLUSH 1500
//Constructor
SerialService::SerialService(uint8_t id)
{
    _buffer_size = 0;
    _started = false;
    _needauthentication = true;
    _id = id;
    switch (_id) {
    case MAIN_SERIAL:
        _rxPin = ESP_RX_PIN;
        _txPin = ESP_TX_PIN;
        _client=ESP_SERIAL_CLIENT;
        break;
#if defined(ESP_SERIAL_BRIDGE_OUTPUT)
    case BRIDGE_SERIAL:
        _rxPin = ESP_BRIDGE_RX_PIN;
        _txPin = ESP_BRIDGE_TX_PIN;
        _client=ESP_SERIAL_BRIDGE_CLIENT;
        break;
#endif //ESP_SERIAL_BRIDGE_OUTPUT
    default:
        _rxPin = ESP_RX_PIN;
        _txPin = ESP_TX_PIN;
        _client=ESP_SERIAL_CLIENT;
        break;
    }
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
bool SerialService::begin(uint8_t serialIndex)
{
    _serialIndex = serialIndex-1;
    log_esp3d("Serial %d begin for %d", _serialIndex, _id);
    if (_id== BRIDGE_SERIAL && Settings_ESP3D::read_byte(ESP_SERIAL_BRIDGE_ON)==0) {
        log_esp3d("Serial %d for %d is disabled", _serialIndex, _id);
        return true;
    }
    if(_serialIndex >= MAX_SERIAL) {
        log_esp3d("Serial %d begin for %d failed, index out of range", _serialIndex, _id);
        return false;
    }
    _lastflush = millis();
    //read from settings
    long br = 0;
    long defaultBr = 0;
    switch (_id) {
    case MAIN_SERIAL:
        br = Settings_ESP3D::read_uint32(ESP_BAUD_RATE);
        defaultBr = Settings_ESP3D::get_default_int32_value(ESP_BAUD_RATE);
        break;
#if defined(ESP_SERIAL_BRIDGE_OUTPUT)
    case BRIDGE_SERIAL:
        br = Settings_ESP3D::read_uint32(ESP_SERIAL_BRIDGE_BAUD);
        defaultBr = Settings_ESP3D::get_default_int32_value(ESP_SERIAL_BRIDGE_BAUD);
        break;
#endif //ESP_SERIAL_BRIDGE_OUTPUT
    default:
        log_esp3d("Serial %d begin for %d failed, unknown id", _serialIndex, _id);
        return false;
    }
    setParameters();
    log_esp3d("Baud rate is %d , default is %d",br, defaultBr);
    _buffer_size = 0;
    //change only if different from current
    if (br != baudRate() || (_rxPin != -1) || (_txPin != -1)) {
        if ( !is_valid_baudrate(br)) {
            br = defaultBr;
        }
        Serials[_serialIndex]->setRxBufferSize (SERIAL_RX_BUFFER_SIZE);
#ifdef ARDUINO_ARCH_ESP8266
        Serials[_serialIndex]->begin(br, ESP_SERIAL_PARAM, SERIAL_FULL, (_txPin == -1)?1:_txPin);
        if (_rxPin != -1) {
            Serials[_serialIndex]->pins((_txPin == -1)?1:_txPin, _rxPin);
        }

#endif //ARDUINO_ARCH_ESP8266
#if defined(ARDUINO_ARCH_ESP32)
        Serials[_serialIndex]->begin (br, ESP_SERIAL_PARAM, ESP_RX_PIN, ESP_TX_PIN);
#if defined(SERIAL_INDEPENDANT_TASK)
        //create serial task once
        log_esp3d("Serial %d for %d Task creation", _serialIndex,_id);
        if (_hserialtask == nullptr && _id==MAIN_SERIAL) {
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
            log_esp3d("Serial %d for %d Task creation failed",_serialIndex, _id);
            return false;
        }
#endif //SERIAL_INDEPENDANT_TASK
#endif //ARDUINO_ARCH_ESP32
    }
    _started = true;
    log_esp3d("Serial %d for %d is started", _serialIndex, _id);
    return true;
}
//End serial
bool SerialService::end()
{
    flush();
    delay (100);
    swap();
    Serials[_serialIndex]->end();
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
    if (!_started) {
        return;
    }
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
    //the serial bridge do not use independant task
    //not sure if it is sill necessary to do it for the main serial
    //TBC..
#if defined(ARDUINO_ARCH_ESP32) && defined(SERIAL_INDEPENDANT_TASK)
    if (_id==MAIN_SERIAL) {
        return;
    }
#endif //ARDUINO_ARCH_ESP32 && SERIAL_INDEPENDANT_TASK0
    process();
}

void SerialService::flushbuffer()
{
    ESP3DOutput output(_client);
    _buffer[_buffer_size] = 0x0;
    //dispatch command
    if (_started) {
        esp3d_commands.process(_buffer, _buffer_size, &output,_needauthentication?LEVEL_GUEST:LEVEL_ADMIN);
    }
    _lastflush = millis();
    _buffer_size = 0;
}

//push collected data to buffer and proceed accordingly
void SerialService::push2buffer(uint8_t * sbuf, size_t len)
{
    if (!_started) {
        return;
    }
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
    bool res = false;
    switch (_id) {
    case MAIN_SERIAL:
        return Settings_ESP3D::write_uint32 (ESP_BAUD_RATE, Settings_ESP3D::get_default_int32_value(ESP_BAUD_RATE));
#if defined(ESP_SERIAL_BRIDGE_OUTPUT)
    case BRIDGE_SERIAL:
        res =  Settings_ESP3D::write_byte (ESP_SERIAL_BRIDGE_ON, Settings_ESP3D::get_default_byte_value(ESP_SERIAL_BRIDGE_ON));
        return res && Settings_ESP3D::write_uint32 (ESP_SERIAL_BRIDGE_BAUD, Settings_ESP3D::get_default_int32_value(ESP_SERIAL_BRIDGE_BAUD));
#endif //ESP_SERIAL_BRIDGE_OUTPUT
    default:
        return res;
    }
}

void SerialService::updateBaudRate(long br)
{
    if (br!=baudRate()) {
        Serials[_serialIndex]->flush();
        Serials[_serialIndex]->updateBaudRate(br);
    }
}

//Get current baud rate
long SerialService::baudRate()
{
    long br = 0;
    br = Serials[_serialIndex]->baudRate();
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
    if (!_started) {
        return 0;
    }
    return Serials[_serialIndex]->write(c);
}

size_t SerialService::write(const uint8_t *buffer, size_t size)
{
    if (!_started) {
        return 0;
    }
    if ((uint)Serials[_serialIndex]->availableForWrite() >= size) {
        return Serials[_serialIndex]->write(buffer, size);
    } else {
        size_t sizetosend = size;
        size_t sizesent = 0;
        uint8_t *buffertmp=(uint8_t *)buffer;
        uint32_t starttime = millis();
        //loop until all is sent or timeout
        while (sizetosend>0 && ((millis() - starttime) < 100)) {
            size_t available = Serials[_serialIndex]->availableForWrite();
            if(available>0) {
                //in case less is sent
                available = Serials[_serialIndex]->write(&buffertmp[sizesent], (available >= sizetosend)?sizetosend:available);
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
    if (!_started) {
        return 0;
    }
    return Serials[_serialIndex]->availableForWrite();
}

int SerialService::available()
{
    if (!_started) {
        return 0;
    }
    return Serials[_serialIndex]->available();
}

int SerialService::read()
{
    if (!_started) {
        return -1;
    }
    return Serials[_serialIndex]->read();
}

size_t SerialService::readBytes(uint8_t * sbuf, size_t len)
{
    if (!_started) {
        return -1;
    }
    return Serials[_serialIndex]->readBytes(sbuf, len);
}

void SerialService::flush()
{
    if (!_started) {
        return ;
    }
    Serials[_serialIndex]->flush();
}

void SerialService::swap()
{
#ifdef ARDUINO_ARCH_ESP8266
    Serials[_serialIndex]->swap();
#endif //ARDUINO_ARCH_ESP8266
}

#endif //COMMUNICATION_PROTOCOL == MKS_SERIAL || COMMUNICATION_PROTOCOL == RAW_SERIAL
