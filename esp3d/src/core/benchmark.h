/*
  benchmark_esp3d.h - esp3d benchmark functions

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

#ifndef _BENCHMARK_ESP3D_H
#define _BENCHMARK_ESP3D_H
extern void benchMark(const char* title, uint64_t bench_start,uint64_t bench_end, size_t bench_transfered);
extern void report_esp3d(const char *format, ...);
#endif //_BENCHMARK_ESP3D_H 
