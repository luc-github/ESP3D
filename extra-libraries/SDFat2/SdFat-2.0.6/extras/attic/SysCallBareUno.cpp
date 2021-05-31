/**
 * Copyright (c) 2011-2020 Bill Greiman
 * This file is part of the SdFat library for SD memory cards.
 *
 * MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
#include "SysCall.h"
#if 0  // defined(__AVR_ATmega328P__) && !ENABLE_ARDUINO_FEATURES
#include <avr/interrupt.h>

// ISR for timer 2 Compare A interrupt
volatile uint16_t timer2 = 0;
ISR(TIMER2_COMPA_vect) {
  timer2++;
}
SdMillis_t SysCall::curTimeMS() {
  if (TIMSK2 != (1 << OCIE2A)) {
    // use system clock (clkI/O).
    ASSR &= ~(1 << AS2);
    // Clear Timer on Compare Match (CTC) mode
    TCCR2A = (1 << WGM21);
    // Only need 64x prescale bits in TCCR2B
    TCCR2B = (1 << CS22);
    // set TOP so timer period is 1 ms.
    #if F_CPU/64000 > 250
    #error F_CPU too large.
    #endif  // F_CPU/64000 > 250
    OCR2A = F_CPU/64000UL - 1;
    // Enable interrupt.
    TIMSK2 = (1 << OCIE2A);
  }
  cli();
  uint16_t rtn = timer2;
  sei();
  return rtn;
}
#endif  // defined(__AVR_ATmega328P__) && !ENABLE_ARDUINO_FEATURES
