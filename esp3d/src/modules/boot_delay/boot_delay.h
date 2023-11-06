/*
  boot_delay.h -  boot delay functions class

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

#ifndef _BOOT_DELAY_H
#define _BOOT_DELAY_H

#include <cstdint>

typedef void(progress_t)(uint8_t percent);

class BootDelay {
 public:
  BootDelay();
  ~BootDelay();
  bool begin();
  void end();
  void handle();
  bool started();

 private:
  bool _started;
  uint32_t _startdelay;
  uint32_t _totalduration;
};

#endif  //_BOOT_DELAY_H
