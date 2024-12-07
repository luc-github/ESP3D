/*
  esp3d_string.h - esp3d strings helpers

  Copyright (c) 2023 Luc Lebosse. All rights reserved.

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

#ifndef _ESP3D_STRING_H
#define _ESP3D_STRING_H
#include <time.h>
namespace esp3d_string {
const char* formatDuration(uint64_t duration);
const char* getTimeString(time_t time, bool isGMT);
const char* generateUUID(const char* seed);
const char* getContentType(const char* filename);
const char* encodeString(const char* s);
const char* formatBytes(uint64_t bytes);
bool isPrintableChar(char c);
const char* expandString(const char* s, bool formatspace = false);
const char * urlEncode(const char* s);
bool isRealTimeCommand(char c);
}  // namespace esp3d_string

#endif  //_ESP3D_STRING_H
