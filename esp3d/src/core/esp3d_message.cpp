/*
  esp3d_message.cpp -  output functions class

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
// #define ESP_LOG_FEATURE LOG_OUTPUT_SERIAL0
#include "esp3d_message.h"

#include "../include/esp3d_config.h"

ESP3DRequest no_id{.id = 0};

ESP3DMessageManager esp3d_message_manager;

ESP3DMessageManager::ESP3DMessageManager() {
  _mutex = xSemaphoreCreateMutex();
#if defined(ESP_LOG_FEATURE)
  _msg_counting = 0;
#endif  // ESP_LOG_FEATURE
}

ESP3DMessageManager::~ESP3DMessageManager() { vSemaphoreDelete(_mutex); }

bool ESP3DMessageManager::deleteMsg(ESP3DMessage* message) {
  esp3d_log("Delete msg");
  if (xSemaphoreTake(_mutex, portMAX_DELAY) == pdTRUE) {
    bool result = _deleteMsg(message);
    xSemaphoreGive(_mutex);
    esp3d_log("Delete msg done");
    return result;
  } else {
    esp3d_log_e("Mutex not taken");
  }
  return false;
}

bool ESP3DMessageManager::_deleteMsg(ESP3DMessage* message) {
  esp3d_log("_Delete msg");
  if (!message) {
    esp3d_log_e("Message is null");
    return false;
  }
  if (message->data) {
    esp3d_log("Free data");
    free(message->data);
  }
  free(message);
  message = NULL;
#if defined(ESP_LOG_FEATURE)
  esp3d_log("Deletion : Now we have %ld msg", --_msg_counting);
#endif  // ESP_LOG_FEATURE
  return true;
}

ESP3DMessage* ESP3DMessageManager::newMsg() {
  esp3d_log("New msg");
  if (xSemaphoreTake(_mutex, portMAX_DELAY) == pdTRUE) {
    ESP3DMessage* newMsgPtr = nullptr;
    newMsgPtr = _newMsg();
    xSemaphoreGive(_mutex);
    return newMsgPtr;
  } else {
    esp3d_log_e("Mutex not taken");
  }
  return nullptr;
}

ESP3DMessage* ESP3DMessageManager::_newMsg() {
  esp3d_log("_New msg");
  ESP3DMessage* newMsgPtr = (ESP3DMessage*)malloc(sizeof(ESP3DMessage));
  if (newMsgPtr) {
#if defined(ESP_LOG_FEATURE)
    esp3d_log("Creation : Now we have %ld msg", ++_msg_counting);
#endif  // ESP_LOG_FEATURE
    newMsgPtr->data = nullptr;
    newMsgPtr->size = 0;
    newMsgPtr->origin = ESP3DClientType::no_client;
    newMsgPtr->target = ESP3DClientType::all_clients;
    newMsgPtr->authentication_level = ESP3DAuthenticationLevel::guest;
    newMsgPtr->request_id.id = millis();
    newMsgPtr->type = ESP3DMessageType::head;
  } else {
    esp3d_log_e("Out of memory");
  }
  esp3d_log("Message created");
  return newMsgPtr;
}

ESP3DMessage* ESP3DMessageManager::newMsg(ESP3DRequest requestId) {
  esp3d_log("New msg");
  if (xSemaphoreTake(_mutex, portMAX_DELAY) == pdTRUE) {
    ESP3DMessage* newMsgPtr = nullptr;
    newMsgPtr = _newMsg(requestId);
    xSemaphoreGive(_mutex);
    esp3d_log("New msg done");
    return newMsgPtr;
  } else {
    esp3d_log_e("Mutex not taken");
  }
  return nullptr;
}

ESP3DMessage* ESP3DMessageManager::_newMsg(ESP3DRequest requestId) {
  esp3d_log("_New msg");
  ESP3DMessage* newMsgPtr = _newMsg();
  if (newMsgPtr) {
    esp3d_log("New msg done");
    newMsgPtr->origin = ESP3DClientType::command;
    newMsgPtr->request_id = requestId;
  } else {
    esp3d_log_e("newMsgPtr is null");
  }
  return newMsgPtr;
}

bool ESP3DMessageManager::copyMsgInfos(ESP3DMessage* newMsgPtr,
                                       ESP3DMessage msg) {
  esp3d_log("Copy msg infos");
  if (xSemaphoreTake(_mutex, portMAX_DELAY) == pdTRUE) {
    bool result = _copyMsgInfos(newMsgPtr, msg);
    xSemaphoreGive(_mutex);
    esp3d_log("Copy msg infos done");
    return result;
  } else {
    esp3d_log_e("Mutex not taken");
  }
  return NULL;
}

bool ESP3DMessageManager::_copyMsgInfos(ESP3DMessage* newMsgPtr,
                                        ESP3DMessage msg) {
  esp3d_log("_Copy msg infos");
  if (!newMsgPtr) {
    esp3d_log_e("newMsgPtr is null");
    return false;
  }
  esp3d_log("Copying msg infos");
  newMsgPtr->origin = msg.origin;
  newMsgPtr->target = msg.target;
  newMsgPtr->authentication_level = msg.authentication_level;
  newMsgPtr->request_id = msg.request_id;
  newMsgPtr->type = msg.type;

  return true;
}

ESP3DMessage* ESP3DMessageManager::copyMsgInfos(ESP3DMessage msg) {
  esp3d_log("Copy msg infos");
  if (xSemaphoreTake(_mutex, portMAX_DELAY) == pdTRUE) {
    ESP3DMessage* newMsgPtr = _copyMsgInfos(msg);
    xSemaphoreGive(_mutex);
    esp3d_log("Copy msg infos done");
    return newMsgPtr;
  } else {
    esp3d_log_e("Mutex not taken");
  }
  return nullptr;
}

ESP3DMessage* ESP3DMessageManager::_copyMsgInfos(ESP3DMessage msg) {
  esp3d_log("_Copy msg infos");
  ESP3DMessage* newMsgPtr = _newMsg();
  if (newMsgPtr) {
    _copyMsgInfos(newMsgPtr, msg);
  } else {
    esp3d_log_e("newMsg is null");
  }
  return newMsgPtr;
}

ESP3DMessage* ESP3DMessageManager::copyMsg(ESP3DMessage msg) {
  esp3d_log("Copy msg");
  if (xSemaphoreTake(_mutex, portMAX_DELAY) == pdTRUE) {
    ESP3DMessage* newMsgPtr = _copyMsg(msg);
    xSemaphoreGive(_mutex);
    return newMsgPtr;
  } else {
    esp3d_log_e("Mutex not taken");
  }
  return nullptr;
}

ESP3DMessage* ESP3DMessageManager::_copyMsg(ESP3DMessage msg) {
  esp3d_log("_Copy msg");
  ESP3DMessage* newMsgPtr = _newMsg(msg.origin, msg.target, msg.data, msg.size,
                                    msg.authentication_level);
  if (newMsgPtr) {
    newMsgPtr->request_id = msg.request_id;
    newMsgPtr->type = msg.type;
  } else {
    esp3d_log_e("newMsgPtr is null");
  }
  return newMsgPtr;
}

ESP3DMessage* ESP3DMessageManager::newMsg(
    ESP3DClientType origin, ESP3DClientType target, const uint8_t* data,
    size_t length, ESP3DAuthenticationLevel authentication_level) {
  esp3d_log("New msg");
  if (xSemaphoreTake(_mutex, portMAX_DELAY) == pdTRUE) {
    ESP3DMessage* newMsgPtr =
        _newMsg(origin, target, data, length, authentication_level);
    xSemaphoreGive(_mutex);
    esp3d_log("New msg done");
    return newMsgPtr;
  } else {
    esp3d_log_e("Mutex not taken");
  }
  return nullptr;
}

ESP3DMessage* ESP3DMessageManager::_newMsg(
    ESP3DClientType origin, ESP3DClientType target, const uint8_t* data,
    size_t length, ESP3DAuthenticationLevel authentication_level) {
  ESP3DMessage* newMsgPtr = _newMsg(origin, target, authentication_level);
  esp3d_log("_New msg");
  if (newMsgPtr) {
    if (!_setDataContent(newMsgPtr, data, length)) {
      _deleteMsg(newMsgPtr);
      newMsgPtr = nullptr;
      esp3d_log_e("newMsg failed for origin %d, target %d, data %s",
                  (uint8_t)origin, (uint8_t)target,
                  data ? (char*)data : "null");
    } else {
      esp3d_log("Message created");
    }
  } else {
    esp3d_log_e("newMsgPtr is null");
  }
  return newMsgPtr;
}

ESP3DMessage* ESP3DMessageManager::newMsg(
    ESP3DClientType origin, ESP3DClientType target,
    ESP3DAuthenticationLevel authentication_level) {
  esp3d_log("New msg");
  if (xSemaphoreTake(_mutex, portMAX_DELAY) == pdTRUE) {
    ESP3DMessage* newMsgPtr = _newMsg(origin, target, authentication_level);
    xSemaphoreGive(_mutex);
    esp3d_log("New msg done");
    return newMsgPtr;
  } else {
    esp3d_log_e("Mutex not taken");
  }
  return nullptr;
}

ESP3DMessage* ESP3DMessageManager::_newMsg(
    ESP3DClientType origin, ESP3DClientType target,
    ESP3DAuthenticationLevel authentication_level) {
  esp3d_log("_New msg");
  ESP3DMessage* newMsgPtr = _newMsg();
  if (newMsgPtr) {
    newMsgPtr->origin = origin;
    newMsgPtr->target = target;
    newMsgPtr->authentication_level = authentication_level;
  } else {
    esp3d_log_e("newMsgPtr is null");
  }
  esp3d_log("_New msg done");
  return newMsgPtr;
}

bool ESP3DMessageManager::setDataContent(ESP3DMessage* msg, const uint8_t* data,
                                         size_t length) {
  esp3d_log("Set data content");
  if (xSemaphoreTake(_mutex, portMAX_DELAY) == pdTRUE) {
    bool result = _setDataContent(msg, data, length);
    xSemaphoreGive(_mutex);
    esp3d_log("Set data content done");
    return result;
  } else {
    esp3d_log_e("Mutex not taken");
  }
  return false;
}

bool ESP3DMessageManager::_setDataContent(ESP3DMessage* msg,
                                          const uint8_t* data, size_t length) {
  esp3d_log("_Set data content");
  if (!msg) {
    esp3d_log_e("no valid msg container");
    return false;
  }
  if (!data || length == 0) {
    esp3d_log_e("no data to set for %d origin, %d target", (uint8_t)msg->origin,
                (uint8_t)msg->target);
    return false;
  }
  if (msg->data) {
    free(msg->data);
  }

  // add some security in case data is called as string so add 1 byte for \0
  msg->data = (uint8_t*)malloc(sizeof(uint8_t) * (length + 1));
  if (msg->data) {
    memcpy(msg->data, data, length);
    msg->size = length;
    msg->data[length] =
        '\0';  // add some security in case data is called as string
    esp3d_log("Data content set to %s", msg->data);
    return true;
  }
  esp3d_log_e("Out of memory");
  return false;
}
