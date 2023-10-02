/*
  buzzer.cpp - ESP3D buzzer class

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

#ifdef BUZZER_DEVICE
#include <Ticker.h>
#include "buzzer.h"
#include "../../core/settings_esp3d.h"
#include <Arduino.h>
BuzzerDevice esp3d_buzzer;
Ticker buzzer_tick;
#define BEEP_DURATION    200


void process()
{
    if (esp3d_buzzer.started()) {
        tone_data * current = esp3d_buzzer.getNextTone();
        if (current) {
            tone(ESP3D_BUZZER_PIN,(unsigned int)current->frequency, (unsigned long) current->duration);
            buzzer_tick.once_ms(current->duration, process);
        }
    }
}

BuzzerDevice::BuzzerDevice()
{
    _head = nullptr;
    _tail = nullptr;
    _started = false;
}

bool BuzzerDevice::begin()
{
    if(_started) {
        end();
    }
    if (Settings_ESP3D::read_byte(ESP_BUZZER) == 1) {
        _started = true;
        playsound(5000, 240);
        playsound(3000, 120);
    }
    return _started;
}
void BuzzerDevice::end()
{
    if(!_started) {
        return;
    }
    purgeData();
    _started = false;
    no_tone();
}


void BuzzerDevice::handle()
{
    //Nothing to do as handled by ticker
}

void BuzzerDevice::beep(int count, int delay, int frequency)
{
    if (_started) {
        while (count > 0) {
            playsound(frequency,BEEP_DURATION);
            if (delay > 0 ) {
                playsound(0,delay);
            }
            waitWhilePlaying();
            count--;
        }
    }
}

void BuzzerDevice::no_tone()
{
    noTone(ESP3D_BUZZER_PIN);
}

bool BuzzerDevice::isPlaying()
{
    return !(_head == nullptr);
}
void BuzzerDevice::waitWhilePlaying()
{
    while (_head != nullptr) {
        delay(10);
    }
}

bool BuzzerDevice::addToneToList(int frequency, int duration)
{
    tone_data * tmp = (tone_data*)malloc(sizeof(tone_data));
    bool startprocess = false;
    if (tmp) {
        tmp->_next = nullptr;
        tmp->frequency = frequency;
        tmp->duration = duration;
        tmp->processing = false;
        if (_tail) {
            _tail->_next=tmp;
        } else { //no tail means also no head
            _head = tmp;
            startprocess = true;//no ongoing list, so lets start it
        }
        _tail = tmp;
        if(startprocess) {
            process();
        }
        return true;
    }
    return false;
}

tone_data * BuzzerDevice::getNextTone()
{
    if (_head) {
        if (_head->processing == false) {
            _head->processing = true;
        } else {
            tone_data * tmp = _head->_next;
            free(_head);
            _head = tmp;
            if (!_head) {
                _tail=_head;
                no_tone();
            }
        }
    }
    return _head;
}

void BuzzerDevice::purgeData()
{
    while (_head) {
        tone_data * tmp = _head->_next;
        free(_head);
        _head = tmp;
    }
    _tail = nullptr;
}

BuzzerDevice::~BuzzerDevice()
{
    end();
}

void BuzzerDevice::playsound(int frequency, int duration)
{
    if (_started) {
        addToneToList(frequency, duration);
    }
}

#endif //BUZZER_DEVICE
