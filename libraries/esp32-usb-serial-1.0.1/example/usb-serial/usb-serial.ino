/*
  Sample code to use the esp32-usb-serial library
3 Copyright (c) 2023 Luc Lebosse. All rights reserved.

  This code is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This code is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <Arduino.h>

#include "esp32_usb_serial.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#define ESP_USB_SERIAL_BAUDRATE 115200
#define ESP_USB_SERIAL_DATA_BITS (8)
#define ESP_USB_SERIAL_PARITY \
  (0)  // 0: 1 stopbit, 1: 1.5 stopbits, 2: 2 stopbits
#define ESP_USB_SERIAL_STOP_BITS \
  (0)  // 0: None, 1: Odd, 2: Even, 3: Mark, 4: Space

#define ESP_USB_SERIAL_RX_BUFFER_SIZE 512
#define ESP_USB_SERIAL_TX_BUFFER_SIZE 128
#define ESP_USB_SERIAL_TASK_SIZE 4096
#define ESP_USB_SERIAL_TASK_CORE 1
#define ESP_USB_SERIAL_TASK_PRIORITY 10

SemaphoreHandle_t device_disconnected_sem;
std::unique_ptr<CdcAcmDevice> vcp;
bool isConnected = false;
bool usbReady = false;
TaskHandle_t xHandle;

/**
 * @brief Data received callback
 */
bool rx_callback(const uint8_t *data, size_t data_len, void *arg) {
  Serial.write(data, data_len);
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
      Serial.printf("CDC-ACM error has occurred, err_no = %d\n",
                    event->data.error);
      break;
    case CDC_ACM_HOST_DEVICE_DISCONNECTED:
      Serial.println("Device suddenly disconnected");
      xSemaphoreGive(device_disconnected_sem);
      isConnected = false;
      break;
    case CDC_ACM_HOST_SERIAL_STATE:
      Serial.printf("Serial state notif 0x%04X\n",
                    event->data.serial_state.val);
      break;
    case CDC_ACM_HOST_NETWORK_CONNECTION:
      Serial.println("Network connection established");
      break;
    default:
      Serial.println("Unknown event");
      break;
  }
}

void connectDevice() {
  if (!usbReady || isConnected) {
    return;
  }
  const cdc_acm_host_device_config_t dev_config = {
      .connection_timeout_ms = 5000,  // 5 seconds, enough time to plug the
                                      // device in or experiment with timeout
      .out_buffer_size = ESP_USB_SERIAL_TX_BUFFER_SIZE,
      .in_buffer_size = ESP_USB_SERIAL_RX_BUFFER_SIZE,
      .event_cb = handle_event,
      .data_cb = rx_callback,
      .user_arg = NULL,
  };
  cdc_acm_line_coding_t line_coding = {
      .dwDTERate = ESP_USB_SERIAL_BAUDRATE,
      .bCharFormat = ESP_USB_SERIAL_STOP_BITS,
      .bParityType = ESP_USB_SERIAL_PARITY,
      .bDataBits = ESP_USB_SERIAL_DATA_BITS,
  };
  // You don't need to know the device's VID and PID. Just plug in any device
  // and the VCP service will pick correct (already registered) driver for the
  // device
  Serial.println("Opening any VCP device...");
  vcp = std::unique_ptr<CdcAcmDevice>(esp_usb::VCP::open(&dev_config));

  if (vcp == nullptr) {
    Serial.println("Failed to open VCP device, retrying...");
    return;
  }

  vTaskDelay(10);

  Serial.println("USB detected");

  if (vcp->line_coding_set(&line_coding) == ESP_OK) {
    Serial.println("USB Connected");
    isConnected = true;
    uint16_t vid = esp_usb::getVID();
    uint16_t pid = esp_usb::getPID();
    Serial.printf("USB device with VID: 0x%04X (%s), PID: 0x%04X (%s) found\n",
                  vid, esp_usb::getVIDString(), pid, esp_usb::getPIDString());
    xSemaphoreTake(device_disconnected_sem, portMAX_DELAY);
    vTaskDelay(10);

    vcp = nullptr;
  } else {
    Serial.println("USB device not identified");
  }
}

// this task only handle connection
static void esp_usb_serial_connection_task(void *pvParameter) {
  (void)pvParameter;
  while (1) {
    /* Delay */
    vTaskDelay(pdMS_TO_TICKS(10));
    if (!usbReady) {
      break;
    }
    connectDevice();
  }
  /* A task should NEVER return */
  vTaskDelete(NULL);
}

void handle() {
  if (!usbReady) return;
  if (Serial.available()) {
    size_t size = Serial.available();
    uint8_t *data = (uint8_t *)malloc(size);
    if (data) {
      size = Serial.readBytes(data, size);
      if (vcp && vcp->tx_blocking(data, size) == ESP_OK) {
        if (!(vcp && vcp->set_control_line_state(true, true) == ESP_OK)) {
          Serial.println("Failed set line");
        }
      } else {
        Serial.println("Failed to send message");
      }
      free(data);
    }
  }
}

void setup() {
  Serial.begin(115200);
  if (ESP_OK != usb_serial_init()) {
    Serial.println("Initialisation failed");
  } else {
    if (ESP_OK != usb_serial_create_task()) {
      Serial.println("Task Creation failed");
    } else {
      Serial.println("Success");
    }
    device_disconnected_sem = xSemaphoreCreateBinary();
    if (device_disconnected_sem == NULL) {
      Serial.println("Semaphore creation failed");
      return;
    }
    BaseType_t res = xTaskCreatePinnedToCore(
        esp_usb_serial_connection_task, "esp_usb_serial_task",
        ESP_USB_SERIAL_TASK_SIZE, NULL, ESP_USB_SERIAL_TASK_PRIORITY, &xHandle,
        ESP_USB_SERIAL_TASK_CORE);
    if (res != pdPASS || !xHandle) {
      Serial.println("Task creation failed");
      return;
    }
    Serial.println("USB Serial Connection Task created successfully");
  }
  usbReady = true;
}

void loop() {
  if (usbReady) {
    handle();
  }
}
