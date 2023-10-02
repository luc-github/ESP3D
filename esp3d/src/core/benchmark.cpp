/*
  debug_esp3d.cpp -  debug esp3d functions class

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

#include "../include/esp3d_config.h"
#if defined(ESP_BENCHMARK_FEATURE)
#include "benchmark.h"
#include "../modules/websocket/websocket_server.h"
void report_esp3d(const char * format, ...)
{
    char    buffer[64];
    char*   temp = buffer;
    va_list arg;
    va_list copy;
    va_start(arg, format);
    va_copy(copy, arg);
    size_t len = vsnprintf(NULL, 0, format, arg);
    va_end(copy);
    if (len >= sizeof(buffer)) {
        temp = new char[len + 1];
        if (temp == NULL) {
            return;
        }
    }
    len = vsnprintf(temp, len + 1, format, arg);
    String str = String("REPORT:") + String(temp);
    websocket_terminal_server.pushMSG(str.c_str());
    va_end(arg);
    if (temp != buffer) {
        delete[] temp;
    }
}

void benchMark(const char* title, uint64_t bench_start,uint64_t bench_end, size_t bench_transfered)
{
    float rate = 1.F * bench_transfered / (bench_end - bench_start) * 1000;
    if (rate <1024) {
        report_esp3d("REPORT: %s %llu bytes in %llu ms, %.2f bytes/s", title, bench_transfered, bench_end - bench_start, rate);
    } else {
        report_esp3d("REPORT: %s %llu bytes in %llu ms, %.2f Kbytes/s", title, bench_transfered, bench_end - bench_start, rate/1024);
    }

}
#endif //ESP_BENCHMARK_FEATURE
