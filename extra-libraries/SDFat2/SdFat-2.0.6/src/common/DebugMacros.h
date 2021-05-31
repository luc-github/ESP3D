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
#ifndef DebugMacros_h
#define DebugMacros_h
#include "../SdFatConfig.h"
#define USE_DBG_MACROS 0

#if USE_DBG_MACROS
#include "Arduino.h"
#ifndef DBG_FILE
#error DBG_FILE not defined
#endif  // DBG_FILE
static void dbgPrint(uint16_t line) {
  Serial.print(F("DBG_FAIL: "));
  Serial.print(F(DBG_FILE));
  Serial.write('.');
  Serial.println(line);
}

#define DBG_PRINT_IF(b) if (b) {Serial.print(F(__FILE__));\
                        Serial.println(__LINE__);}
#define DBG_HALT_IF(b) if (b) {Serial.print(F("DBG_HALT "));\
                       Serial.print(F(__FILE__)); Serial.println(__LINE__);\
                       while (true) {}}
#define DBG_FAIL_MACRO dbgPrint(__LINE__);
#else  // USE_DBG_MACROS
#define DBG_FAIL_MACRO
#define DBG_PRINT_IF(b)
#define DBG_HALT_IF(b)
#endif  // USE_DBG_MACROS
#endif  // DebugMacros_h
