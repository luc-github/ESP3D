/*
  authentication_level_types.h -  authentication functions class

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

#ifndef _AUTHENTICATION_LEVEL_TYPE_H
#define _AUTHENTICATION_LEVEL_TYPE_H
enum class ESP3DAuthenticationLevel : uint8_t {
  guest = 0,
  user = 1,
  admin = 2,
  not_authenticated,
};

#endif  //_AUTHENTICATION_LEVEL_TYPE_H
