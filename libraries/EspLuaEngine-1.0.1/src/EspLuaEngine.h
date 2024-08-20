/*
  EspLuaEngine project

  Copyright (c) 2024 Luc Lebosse. All rights reserved.

  This code is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This code is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#pragma once
#define LUA_USE_C89
#include "lua-5.4.7/src/lua.hpp"

class EspLuaEngine {
 public:
  EspLuaEngine();
  ~EspLuaEngine();

  bool executeScript(const char* script);
  bool registerFunction(const char* name, lua_CFunction function,
                        void* userData = nullptr);
  template <typename T>
  bool registerConstant(const char* name, T value);
  lua_State* getLuaState() { return _lua_state; }

 private:
  lua_State* _lua_state;
  void _loadLibraries();
  bool _checkPreconditions(const char* name);
  bool _verifyGlobal(const char* name, int type);
  void _cleanupOnFailure(const char* name, int top);
  bool _makeReadOnly(const char* name);
  bool _registerConstantImpl(const char* name, lua_Number value);
  bool _registerConstantImpl(const char* name, const char* value);
  bool _registerConstantImpl(const char* name, bool value);
  bool _registerConstantImpl(const char* name, int value);
};
