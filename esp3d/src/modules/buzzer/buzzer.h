/*
  buzzer.h - ESP3D buzzer class

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
#ifndef _BUZZER_H
#define _BUZZER_H
#define BEEP_FREQUENCY 3000

struct tone_data {
  int frequency;
  int duration;
  bool processing;
  tone_data* _next;
};

class BuzzerDevice {
 public:
  BuzzerDevice();
  ~BuzzerDevice();
  void playsound(int frequency, int duration);
  bool started() { return _started; }
  bool begin();
  void end();
  void handle();
  tone_data* getNextTone();
  bool isPlaying();
  void waitWhilePlaying();
  void beep(int count = 1, int delay = 0, int frequency = BEEP_FREQUENCY);

 private:
  tone_data* _head;
  tone_data* _tail;
  bool _started;
  void purgeData();
  bool addToneToList(int frequency, int duration);
  void no_tone();
};
extern BuzzerDevice esp3d_buzzer;
#endif  //_BUZZER_H
