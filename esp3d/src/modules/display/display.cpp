/*
  display.cpp -  display functions class

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

#if defined(DISPLAY_DEVICE)
#include "display.h"
bool Display::dispatch(ESP3DMessage* message) {
  if (!message || !_started) {
    return false;
  }
  if (message->size > 0 && message->data) {
    switch (message->request_id.id) {
      case ESP_OUTPUT_IP_ADDRESS:
        updateIP();
        break;
      case ESP_OUTPUT_STATUS:
        setStatus((const char*)message->data);
        break;
      case ESP_OUTPUT_PROGRESS:
        progress((uint8_t)atoi((const char*)message->data));
        break;
      case ESP_OUTPUT_STATE:
        switch (atoi((const char*)message->data)) {
          case ESP_STATE_DISCONNECTED:
            setStatus("Disconnected");
            break;
          default:
            return false;
            break;
        }
        break;
      default:
        return false;
        break;
    }

    esp3d_message_manager.deleteMsg(message);
    return true;
  }
  return false;
}
#endif  // DISPLAY_DEVICE