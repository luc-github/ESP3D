/*
  esp3d_usb_serial_service.cpp -  serial services functions class

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
#if defined(USB_SERIAL_FEATURE)
#include "../../core/esp3d_commands.h"
#include "../../core/esp3d_settings.h"
#include "../../core/esp3d_string.h"
#include "../authentication/authentication_service.h"
#include "usb_serial_service.h"

const uint32_t SupportedUsbSerialBaudList[] = {
    9600,   19200,  38400,  57600,   74880,   115200, 230400,
    250000, 500000, 921600, 1000000, 1958400, 2000000};
const size_t SupportedUsbSerialBaudListSize =
    sizeof(SupportedUsbSerialBaudList) / sizeof(uint32_t);

ESP3DUsbSerialService esp3d_usb_serial_service;

#define SERIAL_COMMUNICATION_TIMEOUT 500
// Serial Parameters
#define ESP3D_USB_SERIAL_DATA_BITS (8)
#define ESP3D_USB_SERIAL_PARITY \
  (0)  // 0: 1 stopbit, 1: 1.5 stopbits, 2: 2 stopbits
#define ESP3D_USB_SERIAL_STOP_BITS \
  (0)  // 0: None, 1: Odd, 2: Even, 3: Mark, 4: Space
// Task parameters
#define ESP3D_USB_SERIAL_RX_BUFFER_SIZE 512
#define ESP3D_USB_SERIAL_TX_BUFFER_SIZE 128

#define ESP3D_USB_SERIAL_TASK_SIZE 4096
#if CONFIG_FREERTOS_UNICORE
#define ESP3D_USB_SERIAL_TASK_CORE 0
#else
#define ESP3D_USB_SERIAL_TASK_CORE 1
#endif  // CONFIG_FREERTOS_UNICORE

#define ESP3D_USB_SERIAL_TASK_PRIORITY 10

#define TIMEOUT_USB_SERIAL_FLUSH 1500
// Constructor
ESP3DUsbSerialService::ESP3DUsbSerialService() {
  _buffer_size = 0;
  _buffer_mutex = NULL;
  _device_disconnected_mutex = NULL;
  _is_connected = false;
  _xHandle = NULL;
  _vcp_ptr = NULL;
  _started = false;
#if defined(AUTHENTICATION_FEATURE)
  _needauthentication = true;
#else
  _needauthentication = false;
#endif  // AUTHENTICATION_FEATURE
  _origin = ESP3DClientType::usb_serial;
  _messagesInFIFO.setId("in");
  _messagesInFIFO.setMaxSize(0);  // no limit
  _baudRate = 0;
}

// Destructor
ESP3DUsbSerialService::~ESP3DUsbSerialService() { end(); }

// extra parameters that do not need a begin
void ESP3DUsbSerialService::setParameters() {
#if defined(AUTHENTICATION_FEATURE)
  _needauthentication =
      (ESP3DSettings::readByte(ESP_SECURE_SERIAL) == 0) ? false : true;
#else
  _needauthentication = false;
#endif  // AUTHENTICATION_FEATURE
}

void ESP3DUsbSerialService::initAuthentication() {
#if defined(AUTHENTICATION_FEATURE)
  _auth = ESP3DAuthenticationLevel::guest;
#else
  _auth = ESP3DAuthenticationLevel::admin;
#endif  // AUTHENTICATION_FEATURE
}
ESP3DAuthenticationLevel ESP3DUsbSerialService::getAuthentication() {
  if (_needauthentication) {
    return _auth;
  }
  return ESP3DAuthenticationLevel::admin;
}

/**
 * @brief Data received callback
 */
bool rx_callback(const uint8_t *data, size_t data_len, void *arg) {
  esp3d_usb_serial_service.receiveCb(data, data_len);
  return true;
}

/**
 * @brief Device event callback
 *
 * Apart from handling device disconnection it doesn't do anything useful
 */
void handle_event(const cdc_acm_host_dev_event_data_t *event, void *user_ctx) {
  switch (event->type) {
    case CDC_ACM_HOST_ERROR:
     esp3d_log_e("CDC-ACM error has occurred, err_no = %d\n",
                    event->data.error);
      break;
    case CDC_ACM_HOST_DEVICE_DISCONNECTED:
      esp3d_log("Device suddenly disconnected");
      xSemaphoreGive(device_disconnected_sem);
      setConnected(false)
      break;
    case CDC_ACM_HOST_SERIAL_STATE:
      esp3d_log("Serial state notif 0x%04X\n",
                    event->data.serial_state.val);
      break;
    case CDC_ACM_HOST_NETWORK_CONNECTION:
      esp3d_log("Network connection established");
      break;
    default:
      esp3d_log("Unknown event");
      break;
  }
}




void ESP3DUsbSerialService::receiveCb(const uint8_t *data, size_t data_len, void *arg) {
  if (!started()) {
    return;
  }
   if (xSemaphoreTake(_buffer_mutex, portMAX_DELAY)) {
   for (size_t i = 0; i < data_len; i++) {
         _buffer[_buffer_size] = data[i];
         _buffer_size++;
         if (_buffer_size > ESP3D_SERIAL_BUFFER_SIZE ||
             _buffer[_buffer_size - 1] == '\n' ||
             _buffer[_buffer_size - 1] == '\r') {
           if (_buffer[_buffer_size - 1] == '\r') {
             _buffer[_buffer_size - 1] = '\n';
           }
           flushbuffer();
         }
     }
     xSemaphoreGive(_buffer_mutex);
   } else {
     esp3d_log_e("Mutex not taken");
   }
}

// this task only handle connection
static void esp3d_usb_serial_connection_task(void *pvParameter) {
  (void)pvParameter;
  while (1) {
    /* Delay */
    vTaskDelay(pdMS_TO_TICKS(10));
    if (! esp3d_usb_serial_service.started()) {
      break;
    }
    esp3d_usb_serial_service.connectDevice();
  }
  /* A task should NEVER return */
  vTaskDelete(NULL);
}

void ESP3DUsbSerialService::connectDevice() {
  if (!_started || _is_connected) {
    return;
  }
  const cdc_acm_host_device_config_t dev_config = {
      .connection_timeout_ms = 5000,  // 5 seconds, enough time to plug the
                                      // device in or experiment with timeout
      .out_buffer_size = ESP3D_USB_SERIAL_TX_BUFFER_SIZE,
      .in_buffer_size = ESP3D_USB_SERIAL_RX_BUFFER_SIZE,
      .event_cb = handle_event,
      .data_cb = rx_callback,
      .user_arg = NULL,
  };
  cdc_acm_line_coding_t line_coding = {
      .dwDTERate = _baudRate,
      .bCharFormat = ESP3D_USB_SERIAL_STOP_BITS,
      .bParityType = ESP3D_USB_SERIAL_PARITY,
      .bDataBits = ESP3D_USB_SERIAL_DATA_BITS,
  };
  // You don't need to know the device's VID and PID. Just plug in any device
  // and the VCP service will pick correct (already registered) driver for the
  // device
 esp3d_log("Waiting for USB device");
  _vcp_ptr = std::unique_ptr<CdcAcmDevice>(esp_usb::VCP::open(&dev_config));

  if (_vcp_ptr == nullptr) {
    esp3d_log_w("USB device not found");
    return;
  }

  vTaskDelay(10);

  esp3d_log("USB device found");

  if (_vcp_ptr->line_coding_set(&line_coding) == ESP_OK) {
    esp3d_log("Line coding set");
    _is_connected = true;
    uint16_t vid = esp_usb::getVID();
    uint16_t pid = esp_usb::getPID();
    esp3d_log_d("USB device with VID: 0x%04X (%s), PID: 0x%04X (%s) found\n",
                  vid, esp_usb::getVIDString(), pid, esp_usb::getPIDString());
    //TODO:             
    //Do notification to user ?1
    xSemaphoreTake(device_disconnected_sem, portMAX_DELAY);
    vTaskDelay(10);
    _vcp_ptr = nullptr;
  } else {
   esp3d_log_e("Line coding set failed");
  }
}

// Setup Serial
bool ESP3DUsbSerialService::begin() {
  _buffer_mutex = xSemaphoreCreateMutex();
  if (_buffer_mutex == NULL) {
    esp3d_log_e("Mutex creation failed");
    return false;
  }
  _device_disconnected_mutex = xSemaphoreCreateMutex();
  if (_device_disconnected_mutex == NULL) {
    esp3d_log_e("Mutex creation failed");
    return false;
  }
  _lastflush = millis();
  // read from settings

  uint32_t br = ESP3DSettings::readUint32(ESP_USB_SERIAL_BAUD_RATE);
  uint32_t defaultBr =
      ESP3DSettings::getDefaultIntegerSetting(ESP_USB_SERIAL_BAUD_RATE);

  setParameters();
  esp3d_log("Baud rate is %d , default is %d", br, defaultBr);
  _buffer_size = 0;
  // change only if different from current
  if (br != baudRate()) {
    if (!ESP3DSettings::isValidIntegerSetting(br, ESP3D_USB_SERIAL_BAUD_RATE)) {
      br = defaultBr;
    }
    _baudRate = br;
  }
  if (ESP_OK != usb_serial_init()) {
    esp3d_log_e("USB Serial Init failed");
    return false;
  } else {
    if (ESP_OK != usb_serial_create_task()) {
      esp3d_log_e("USB Serial Create Task failed");
      return false;
    }
    device_disconnected_mutex = xSemaphoreCreateMutex();
    if (device_disconnected_mutex == NULL) {
      Serial.println("Mutex creation failed");
      return false;
    }

    BaseType_t res = xTaskCreatePinnedToCore(
        esp3d_usb_serial_connection_task, "esp3d_usb_serial_task",
        ESP3D_USB_SERIAL_TASK_SIZE, NULL, ESP3D_USB_SERIAL_TASK_PRIORITY, &xHandle,
        ESP3D_USB_SERIAL_TASK_CORE);
    if (res != pdPASS || !xHandle) {
      esp3d_log_e("Failed to create USB Serial Connection Task");
      return false;
    }
  }
  _started = true;
  return true;
}

// End serial
bool ESP3DUsbSerialService::end() {
  flush();
  delay(100);
  if (_buffer_mutex != NULL) {
    vSemaphoreDelete(_buffer_mutex);
    _buffer_mutex = NULL;
  }
  if (_device_disconnected_mutex != NULL) {
    vSemaphoreDelete(_device_disconnected_mutex);
    _device_disconnected_mutex = NULL;
  }
  // Serials[_serialIndex]->end();
  _buffer_size = 0;
  _started = false;
  _is_connected = false;
  if (_xHandle != NULL) {
    vTaskDelete(_xHandle);
    _xHandle = NULL;
    esp3d_log("Task deleted");
  }
  if (_vcp_ptr != NULL) {
    _vcp_ptr = NULL;
    esp3d_log("VCP deleted");
  }
  usb_serial_deinit();
  usb_serial_delete_task();
  initAuthentication();
  return true;
}

// return the array of uint32_t and array size
const uint32_t *ESP3DUsbSerialService::get_baudratelist(uint8_t *count) {
  if (count) {
    *count = sizeof(SupportedUsbSerialBaudList) / sizeof(uint32_t);
  }
  return SupportedUsbSerialBaudList;
}

// Function which could be called in other loop
void ESP3DUsbSerialService::handle() {
  // Do we have some data waiting
  size_t len = _messagesInFIFO.size();
  if (len > 0) {
    if (len > 10) {
      len = 10;
    }
    while (len > 0) {
      esp3d_log("Serial in fifo size %d", _messagesInFIFO.size());
      ESP3DMessage *message = _messagesInFIFO.pop();
      if (message) {
        esp3d_commands.process(message);
      } else {
        esp3d_log_e("Cannot create message");
      }
      len--;
    }
  }
}

void ESP3DUsbSerialService::flushbuffer() {
  _buffer[_buffer_size] = 0x0;
  if (_buffer_size == 1 && _buffer[0] == '\n') {
    _buffer_size = 0;
    return;
  }

  // dispatch command
  ESP3DMessage *message = esp3d_message_manager.newMsg(
      _origin, ESP3DClientType::all_clients, (uint8_t *)_buffer, _buffer_size,
      getAuthentication());
  if (message) {
    // process command
    message->type = ESP3DMessageType::unique;
    esp3d_log("Message sent to fifo list");
    _messagesInFIFO.push(message);
  } else {
    esp3d_log_e("Cannot create message");
  }
  _lastflush = millis();
  _buffer_size = 0;
}

// push collected data to buffer and proceed accordingly
void ESP3DUsbSerialService::push2buffer(uint8_t *sbuf, size_t len) {
  if (!_started) {
    return;
  }
  esp3d_log("buffer get %d data ", len);
  /* for (size_t i = 0; i < len; i++) {
     _lastflush = millis();
     // command is defined
     if ((char(sbuf[i]) == '\n') || (char(sbuf[i]) == '\r')) {
       if (_buffer_size < ESP3D_SERIAL_BUFFER_SIZE) {
         _buffer[_buffer_size] = sbuf[i];
         _buffer_size++;
       }
       flushbuffer();
     } else if (esp3d_string::isPrintableChar(char(sbuf[i]))) {
       if (_buffer_size < ESP3D_SERIAL_BUFFER_SIZE) {
         _buffer[_buffer_size] = sbuf[i];
         _buffer_size++;
       } else {
         flushbuffer();
         _buffer[_buffer_size] = sbuf[i];
         _buffer_size++;
       }
     } else {  // it is not printable char
       // clean buffer first
       if (_buffer_size > 0) {
         flushbuffer();
       }
       // process char
       _buffer[_buffer_size] = sbuf[i];
       _buffer_size++;
       flushbuffer();
     }
   }*/
}

void ESP3DUsbSerialService::updateBaudRate(uint32_t br) {
  if (br != _baudRate) {
    _baudRate = br;
  }
}

// Get current baud rate
uint32_t ESP3DUsbSerialService::baudRate() { return _baudRate; }

size_t ESP3DUsbSerialService::writeBytes(const uint8_t *buffer, size_t size) {
  if (!_started) {
    return 0;
  } /*
   if ((uint)Serials[_serialIndex]->availableForWrite() >= size) {
     return Serials[_serialIndex]->write(buffer, size);
   } else {
     size_t sizetosend = size;
     size_t sizesent = 0;
     uint8_t *buffertmp = (uint8_t *)buffer;
     uint32_t starttime = millis();
     // loop until all is sent or timeout
     while (sizetosend > 0 && ((millis() - starttime) < 100)) {
       size_t available = Serials[_serialIndex]->availableForWrite();
       if (available > 0) {
         // in case less is sent
         available = Serials[_serialIndex]->write(
             &buffertmp[sizesent],
             (available >= sizetosend) ? sizetosend : available);
         sizetosend -= available;
         sizesent += available;
         starttime = millis();
       } else {
         ESP3DHal::wait(5);
       }
     }
     return sizesent;
   }*/
  return 0;
}

size_t ESP3DUsbSerialService::readBytes(uint8_t *sbuf, size_t len) {
  if (!_started) {
    return -1;
  }
  // return Serials[_serialIndex]->readBytes(sbuf, len);
  return 0;
}

void ESP3DUsbSerialService::flush() {
  if (!_started) {
    return;
  }
  // Serials[_serialIndex]->flush();
}

void ESP3DUsbSerialService::swap() {
  // Nothing to do
}

bool ESP3DUsbSerialService::dispatch(ESP3DMessage *message) {
  bool done = false;
  // Only is serial service is started
  if (_started) {
    // Only if message is not null
    if (message) {
      // if message is not null
      if (message->data && message->size != 0) {
        if (writeBytes(message->data, message->size) == message->size) {
          flush();
          // Delete message now
          esp3d_message_manager.deleteMsg(message);
          done = true;
        } else {
          esp3d_log_e("Error while sending data");
        }
      } else {
        esp3d_log_e("Error null data");
      }
    } else {
      esp3d_log_e("Error null message");
    }
  }
  return done;
}

#endif  // USB_SERIAL_FEATURE