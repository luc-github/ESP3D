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
#include "PrintBasic.h"
#if ENABLE_ARDUINO_FEATURES == 0
#include <math.h>

size_t PrintBasic::print(long n, uint8_t base) {
  if (n < 0 && base == 10) {
    return print('-') + printNum(-n, base);
  }
  return printNum(n, base);
}
size_t PrintBasic::printNum(unsigned long n, uint8_t base) {
  const uint8_t DIM = 8 * sizeof(long);
  char buf[DIM];
  char *str = &buf[DIM];

  if (base < 2) return 0;

  do {
    char c = n % base;
    n /= base;
    *--str = c + (c < 10 ? '0' : 'A' - 10);
  } while (n);
  return write(str, &buf[DIM] - str);
}

size_t PrintBasic::printDouble(double n, uint8_t prec) {
  // Max printable 32-bit floating point number. AVR uses 32-bit double.
  const double maxfp = static_cast<double>(0XFFFFFF00UL);
  size_t rtn = 0;

  if (isnan(n)) {
    return write("NaN");
  }
  if (n < 0) {
    n = -n;
    rtn += print('-');
  }
  if (isinf(n)) {
    return rtn + write("Inf");
  }
  if (n > maxfp) {
    return rtn + write("Ovf");
  }

  double round = 0.5;
  for (uint8_t i = 0; i < prec; ++i) {
    round *= 0.1;
  }

  n += round;

  uint32_t whole = (uint32_t)n;
  rtn += print(whole);

  if (prec) {
    rtn += print('.');
    double fraction = n - static_cast<double>(whole);
    for (uint8_t i = 0; i < prec; i++) {
      fraction *= 10.0;
      uint8_t digit = fraction;
      rtn += print(digit);
      fraction -= digit;
    }
  }
  return rtn;
}
#endif  //  ENABLE_ARDUINO_FEATURES == 0
