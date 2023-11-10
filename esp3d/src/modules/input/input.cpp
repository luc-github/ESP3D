/*
  input.cpp -  input functions class

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
#if defined(INPUT_DEVICE)
#include "../../core/esp3d_message.h"
#include "../../core/esp3d_settings.h"
#include "input.h"

Input esp3d_input;

Input::Input() { _started = false; }

Input::~Input() { end(); }

bool Input::begin() {
  bool res = true;
  _started = false;
  if (!res) {
    end();
  }
  _started = res;
  return _started;
}

void Input::end() {
  if (!_started) {
    return;
  }
  _started = false;
}

bool Input::started() { return _started; }

void Input::handle() {
  if (_started) {
  }
}

#endif  // INPUT_DEVICE
