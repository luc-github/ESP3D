/*
  esp32_usb_serial.cpp - esp32 usb serial library
  Copyright (c) 2023 Luc Lebosse. All rights reserved.

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

/*********************
 *      INCLUDES
 *********************/

#include "esp32_usb_serial.h"
#include "esp_log.h"
#include "esp_private/usb_phy.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "hal/usb_phy_types.h"
#include "usb/cdc_acm_host.h"
#include "usb/usb_host.h"
#include "usb/vcp.hpp"
#include "usb/vcp_ch34x.hpp"
#include "usb/vcp_cp210x.hpp"
#include "usb/vcp_ftdi.hpp"


static usb_phy_handle_t phy_hdl = NULL;
static TaskHandle_t usb_serial_xHandle = NULL;

#ifndef ESP_USB_LIB_TASK_SIZE
#define ESP_USB_LIB_TASK_SIZE 4096
#endif //ESP_USB_LIB_TASK_SIZE

#ifndef ESP_USB_LIB_TASK_CORE
#define ESP_USB_LIB_TASK_CORE 1
#endif //ESP_USB_LIB_TASK_CORE

#ifndef ESP_USB_LIB_TASK_PRIORITY
#define ESP_USB_LIB_TASK_PRIORITY 10
#endif //ESP_USB_LIB_TASK_PRIORITY

uint16_t esp_usb::getVID(){
    return esp_usb::current_vid;
}

const char * esp_usb::getVIDString(){
    switch (esp_usb::current_vid){
        case 0x10C4:
            return "CP210X";
            break;
        case 0x1A86:
            return "CH34X";
            break;
        case 0X0403:
            return "FTDI";
            break;
        default:
            break;
    };
    return "None";
}

uint16_t esp_usb::getPID(){
    return esp_usb::current_pid;
}

const char * esp_usb::getPIDString() {
    switch (esp_usb::current_pid){
        case 0x7522:
        case 0x7523:
            return "CH340";
            break;
        case 0x341:
            return "CH341";
            break;
        case 0xEA60:
            return "CP2101-CP2104";
            break;
        case 0xEA70:
            return "CP2105";
            break;
        case 0xEA71:
            return "CP2108";
            break;
        case 0x6001:
            return "FT232";
            break;
        case 0x6015:
            return "FT231";
            break;
        default:
            break;
    };
    return "None";
}

/**
 * @brief USB Host library handling task
 *
 * @param arg Unused
 */
void usb_lib_task(void *arg) {
  while (1) {
    // Start handling system events
    uint32_t event_flags;
    // exception must be enabled - so I guess there is a reason for it
    // this generate random crash when disconnected
    try {
      usb_host_lib_handle_events(portMAX_DELAY, &event_flags);
    } catch (...) {
      ESP_LOGD("USB_SERIAL","Error handling usb event");
    }
    if (event_flags & USB_HOST_LIB_EVENT_FLAGS_NO_CLIENTS) {
      if (ESP_OK != usb_host_device_free_all()) {
        ESP_LOGE("USB_SERIAL","Failed to free all devices");
      }
    }
    if (event_flags & USB_HOST_LIB_EVENT_FLAGS_ALL_FREE) {
      ESP_LOGD("USB_SERIAL","USB: All devices freed");
      // Continue handling USB events to allow device reconnection
    }
  }
}

esp_err_t usb_serial_deinit() {
  esp_err_t ret;
  if (usb_serial_xHandle) {
    ret = usb_serial_delete_task();
    if (ret != ESP_OK) {
      ESP_LOGE("USB_SERIAL","Failed to delete task for usb-host %s",
                  esp_err_to_name(ret));
    }
    vTaskDelay(10);
  }
  ret = usb_host_device_free_all();
  if (ret != ESP_OK) {
    ESP_LOGE("USB_SERIAL","Failed to free all usb device %s", esp_err_to_name(ret));
  }
  ret = usb_host_uninstall();
  // this one failed with ESP_ERR_INVALID_STATE if USB is connected
  if (ret != ESP_OK) {
    ESP_LOGE("USB_SERIAL","Failed to unsinstall usb host %s",
                esp_err_to_name(usb_host_uninstall()));
  }
  // Deinitialize the internal USB PHY
  ret = usb_del_phy(phy_hdl);
  if (ret != ESP_OK) {
    ESP_LOGE("USB_SERIAL","Failed to delete PHY %s", esp_err_to_name(ret));
  }
  phy_hdl = NULL;
  return ESP_OK;
}

esp_err_t usb_serial_init() {
  usb_phy_config_t phy_config = {
      .controller = USB_PHY_CTRL_OTG,
      .target = USB_PHY_TARGET_INT,
      .otg_mode = USB_OTG_MODE_HOST,
      .otg_speed =
          USB_PHY_SPEED_UNDEFINED,  // In Host mode, the speed is determined by
                                    // the connected device
      .ext_io_conf = NULL,
      .otg_io_conf = NULL,
  };
  if (ESP_OK != usb_new_phy(&phy_config, &phy_hdl)) {
    ESP_LOGE("USB_SERIAL","Failed to init USB PHY");
  }

  const usb_host_config_t host_config = {
      .skip_phy_setup = true,
      .intr_flags = ESP_INTR_FLAG_LEVEL1,
  };
  // Install USB Host driver. Should only be called once in entire application
  ESP_LOGD("USB_SERIAL","Installing USB Host");
  esp_err_t err = usb_host_install(&host_config);
  if (err != ESP_OK) {
    ESP_LOGE("USB_SERIAL","Failed to install USB Host %s", esp_err_to_name(err));
    return ESP_FAIL;
  };
  return ESP_OK;
}

esp_err_t usb_serial_delete_task() {
  esp_err_t err = cdc_acm_host_uninstall();
  if (err != ESP_OK) {
    vTaskDelete(usb_serial_xHandle);
    usb_serial_xHandle = NULL;
    return ESP_OK;
  }
  return err;
}

esp_err_t usb_serial_create_task() {
  // Create a task that will handle USB library events

  BaseType_t res =
      xTaskCreatePinnedToCore(usb_lib_task, "usb_lib", ESP_USB_LIB_TASK_SIZE,
                              NULL, ESP_USB_LIB_TASK_PRIORITY,
                              &usb_serial_xHandle, ESP_USB_LIB_TASK_CORE);
  if (res == pdPASS && usb_serial_xHandle) {
    ESP_LOGD("USB_SERIAL","Installing CDC-ACM driver");
    if (cdc_acm_host_install(NULL) == ESP_OK) {
      // Register VCP drivers to VCP service.
      ESP_LOGD("USB_SERIAL","Registering FT23x driver");
      esp_usb::VCP::register_driver<esp_usb::FT23x>();
      ESP_LOGD("USB_SERIAL","Registering CP210x driver");
      esp_usb::VCP::register_driver<esp_usb::CP210x>();
      ESP_LOGD("USB_SERIAL","Registering CH34x driver");
      esp_usb::VCP::register_driver<esp_usb::CH34x>();
      return ESP_OK;
    } else {
      ESP_LOGE("USB_SERIAL","Failed to install CDC-ACM driver");
    }
  } else {
    ESP_LOGE("USB_SERIAL","Failed to create task USB Host");
  }

  return ESP_FAIL;
}
