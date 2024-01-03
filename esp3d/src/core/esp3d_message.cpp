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

#if defined(ESP_LOG_FEATURE)
int msg_counting = 0;
#endif  // ESP_LOG_FEATURE

bool ESP3DMessageManager::deleteMsg(ESP3DMessage* message) {
  if (!message) return false;
  if (message->data) {
    free(message->data);
  }
  free(message);
  message = NULL;
#if defined(ESP_LOG_FEATURE)
  esp3d_log("Deletion : Now we have %ld msg", --msg_counting);
#endif  // ESP_LOG_FEATURE
  return true;
}

ESP3DMessage* ESP3DMessageManager::newMsg() {
  ESP3DMessage* newMsgPtr = (ESP3DMessage*)malloc(sizeof(ESP3DMessage));
  if (newMsgPtr) {
#if defined(ESP_LOG_FEATURE)
    esp3d_log("Creation : Now we have %ld msg", ++msg_counting);
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
  return newMsgPtr;
}

ESP3DMessage* ESP3DMessageManager::newMsg(ESP3DRequest requestId) {
  ESP3DMessage* newMsgPtr = newMsg();
  if (newMsgPtr) {
    newMsgPtr->origin = ESP3DClientType::command;
    newMsgPtr->request_id = requestId;
  }
  return newMsgPtr;
}

bool ESP3DMessageManager::copyMsgInfos(ESP3DMessage* newMsgPtr,
                                       ESP3DMessage msg) {
  if (!newMsgPtr) {
    return false;
  }
  newMsgPtr->origin = msg.origin;
  newMsgPtr->target = msg.target;
  newMsgPtr->authentication_level = msg.authentication_level;
  newMsgPtr->request_id = msg.request_id;
  newMsgPtr->type = msg.type;
  return true;
}

ESP3DMessage* ESP3DMessageManager::copyMsgInfos(ESP3DMessage msg) {
  ESP3DMessage* newMsgPtr = newMsg();
  if (newMsgPtr) {
    copyMsgInfos(newMsgPtr, msg);
  }
  return newMsgPtr;
}

ESP3DMessage* ESP3DMessageManager::copyMsg(ESP3DMessage msg) {
  ESP3DMessage* newMsgPtr = newMsg(msg.origin, msg.target, msg.data, msg.size,
                                   msg.authentication_level);
  if (newMsgPtr) {
    newMsgPtr->request_id = msg.request_id;
    newMsgPtr->type = msg.type;
  }
  return newMsgPtr;
}

ESP3DMessage* ESP3DMessageManager::newMsg(
    ESP3DClientType origin, ESP3DClientType target, const uint8_t* data,
    size_t length, ESP3DAuthenticationLevel authentication_level) {
  ESP3DMessage* newMsgPtr = newMsg(origin, target, authentication_level);
  if (newMsgPtr) {
    if (!setDataContent(newMsgPtr, data, length)) {
      deleteMsg(newMsgPtr);
      newMsgPtr = nullptr;
      esp3d_log_e("newMsg failed for origin %d, target %d, data %s",
                  (uint8_t)origin, (uint8_t)target,
                  data ? (char*)data : "null");
    }
  }
  return newMsgPtr;
}

ESP3DMessage* ESP3DMessageManager::newMsg(
    ESP3DClientType origin, ESP3DClientType target,
    ESP3DAuthenticationLevel authentication_level) {
  ESP3DMessage* newMsgPtr = newMsg();
  if (newMsgPtr) {
    newMsgPtr->origin = origin;
    newMsgPtr->target = target;
    newMsgPtr->authentication_level = authentication_level;
  }
  return newMsgPtr;
}

bool ESP3DMessageManager::setDataContent(ESP3DMessage* msg, const uint8_t* data,
                                         size_t length) {
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
