/*
  sensor.h -  sensor functions class

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

#ifndef _ESP3D_SENSOR_H
#define _ESP3D_SENSOR_H

class ESP3DSensorDevice {
 public:
  ESP3DSensorDevice() {}
  virtual ~ESP3DSensorDevice() {}
  virtual bool begin() { return false; }
  virtual void end() {}
  virtual bool isModelValid(uint8_t model) { return false; }
  virtual uint8_t getIDFromString(const char *) { return 0; }
  virtual uint8_t nbType() { return 0; }
  virtual uint8_t GetModel(uint8_t i = 0) { return 0; }
  virtual const char *GetCurrentModelString() { return "None"; }
  virtual const char *GetModelString(uint8_t i = 0) { return "None"; }
  virtual const char *GetData() { return ""; }
};

class ESP3DSensor {
 public:
  ESP3DSensor();
  ~ESP3DSensor();
  bool begin();
  void end();
  void handle();
  bool setInterval(uint interval);
  bool isModelValid(uint8_t model);
  const char *GetCurrentModelString();
  uint8_t getIDFromString(const char *s);
  uint8_t nbType();
  uint interval() { return _interval; }
  uint8_t GetModel(uint8_t i = 0);
  const char *GetModelString(uint8_t i = 0);
  const char *GetData();
  bool started() { return _started; }

 protected:
  bool _started;
  uint32_t _interval;
  uint32_t _lastReadTime;
  ESP3DSensorDevice *_device;
};

extern ESP3DSensor esp3d_sensor;

#endif  //_ESP3D_SENSOR_H
