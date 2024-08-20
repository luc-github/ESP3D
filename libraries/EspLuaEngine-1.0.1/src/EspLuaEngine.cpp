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

#include "EspLuaEngine.h"

#include <Arduino.h>


/*Public methods*/

EspLuaEngine::EspLuaEngine() : _lua_state(nullptr) {
  _lua_state = luaL_newstate();
  if (_lua_state) {
    _loadLibraries();
  } else {
    log_e("Error : Impossible to create a new Lua state");
  }
}

EspLuaEngine::~EspLuaEngine() {
  if (_lua_state) {
    lua_close(_lua_state);
  }
}

bool EspLuaEngine::executeScript(const char* script) {
  if (luaL_dostring(_lua_state, script) != LUA_OK) {
    log_e("%s", lua_tostring(_lua_state, -1));
    lua_pop(_lua_state, 1);
    return false;
  }
  return true;
}

bool EspLuaEngine::registerFunction(const char* name, lua_CFunction function,
                                    void* userData) {
  if (!_checkPreconditions(name) || !function) {
    log_e("Error: Invalid function pointer or name");
    return false;
  }
  int top = lua_gettop(_lua_state);

  if (userData) {
    lua_pushlightuserdata(_lua_state, userData);
    lua_pushcclosure(_lua_state, function, 1);
  } else {
    lua_pushcfunction(_lua_state, function);
  }
  lua_setglobal(_lua_state, name);

  if (!_verifyGlobal(name, LUA_TFUNCTION)) {
    _cleanupOnFailure(name, top);
    return false;
  }
  lua_settop(_lua_state, top);
  log_v("Function '%s' registered successfully", name);
  return true;
}

// Implement the template method
template <typename T>
bool EspLuaEngine::registerConstant(const char* name, T value) {
  return _registerConstantImpl(name, value);
}

// Specializations template explicit, before the explicit specializations
template <>
bool EspLuaEngine::registerConstant<unsigned char>(const char* name,
                                                   unsigned char value) {
  return _registerConstantImpl(name, static_cast<int>(value));
}

// Specializations explicit
template bool EspLuaEngine::registerConstant<lua_Number>(const char*,
                                                         lua_Number);
template bool EspLuaEngine::registerConstant<const char*>(const char*,
                                                          const char*);
template bool EspLuaEngine::registerConstant<bool>(const char*, bool);
template bool EspLuaEngine::registerConstant<int>(const char*, int);
template bool EspLuaEngine::registerConstant<unsigned char>(const char*,
                                                            unsigned char);

// Implementations of the specialized methods
bool EspLuaEngine::_registerConstantImpl(const char* name, lua_Number value) {
  if (!_checkPreconditions(name)) return false;
  int top = lua_gettop(_lua_state);
  lua_pushnumber(_lua_state, value);
  lua_setglobal(_lua_state, name);
  if (!_verifyGlobal(name, LUA_TNUMBER) || !_makeReadOnly(name)) {
    _cleanupOnFailure(name, top);
    return false;
  }
  lua_settop(_lua_state, top);
  log_v("Constant '%s' (number) registered successfully", name);
  return true;
}

bool EspLuaEngine::_registerConstantImpl(const char* name, const char* value) {
  if (!_checkPreconditions(name) || !value) {
    log_e("Error: Invalid value");
    return false;
  }
  int top = lua_gettop(_lua_state);
  lua_pushstring(_lua_state, value);
  lua_setglobal(_lua_state, name);
  if (!_verifyGlobal(name, LUA_TSTRING) || !_makeReadOnly(name)) {
    _cleanupOnFailure(name, top);
    return false;
  }
  lua_settop(_lua_state, top);
  log_v("Constant '%s' (string) registered successfully", name);
  return true;
}

bool EspLuaEngine::_registerConstantImpl(const char* name, bool value) {
  if (!_checkPreconditions(name)) return false;
  int top = lua_gettop(_lua_state);
  lua_pushboolean(_lua_state, value);
  lua_setglobal(_lua_state, name);
  if (!_verifyGlobal(name, LUA_TBOOLEAN) || !_makeReadOnly(name)) {
    _cleanupOnFailure(name, top);
    return false;
  }
  lua_settop(_lua_state, top);
  log_v("Constant '%s' (boolean) registered successfully", name);
  return true;
}

bool EspLuaEngine::_registerConstantImpl(const char* name, int value) {
  if (!_checkPreconditions(name)) return false;
  int top = lua_gettop(_lua_state);
  lua_pushinteger(_lua_state, value);
  lua_setglobal(_lua_state, name);
  if (!_verifyGlobal(name, LUA_TNUMBER) || !_makeReadOnly(name)) {
    _cleanupOnFailure(name, top);
    return false;
  }
  lua_settop(_lua_state, top);
  log_v("Constant '%s' (integer) registered successfully", name);
  return true;
}

/*Private methods*/
void EspLuaEngine::_loadLibraries() {
  static const luaL_Reg loadedlibs[] = {{"_G", luaopen_base},
                                        {LUA_TABLIBNAME, luaopen_table},
                                        {LUA_STRLIBNAME, luaopen_string},
                                        {LUA_MATHLIBNAME, luaopen_math},
                                        {LUA_UTF8LIBNAME, luaopen_utf8},
                                        {NULL, NULL}};

  for (const luaL_Reg* lib = loadedlibs; lib->func; lib++) {
    luaL_requiref(_lua_state, lib->name, lib->func, 1);
    lua_pop(_lua_state, 1);
    log_v("Successfully loaded library: %s", lib->name);
  }
}

bool EspLuaEngine::_checkPreconditions(const char* name) {
  if (!_lua_state) {
    log_e("Error: Lua state is not initialized");
    return false;
  }
  if (!name || strlen(name) == 0) {
    log_e("Error: Invalid name");
    return false;
  }
  if (!lua_checkstack(_lua_state, 3)) {
    log_e("Error: Unable to allocate stack space");
    return false;
  }
  return true;
}

bool EspLuaEngine::_verifyGlobal(const char* name, int type) {
  lua_getglobal(_lua_state, name);
  bool result = (lua_type(_lua_state, -1) == type);
  lua_pop(_lua_state, 1);
  if (!result) {
    log_e("Error: Failed to register %s '%s'",
          (type == LUA_TFUNCTION) ? "function" : "constant", name);
  }
  return result;
}

void EspLuaEngine::_cleanupOnFailure(const char* name, int top) {
  lua_pushnil(_lua_state);
  lua_setglobal(_lua_state, name);
  lua_settop(_lua_state, top);
}

bool EspLuaEngine::_makeReadOnly(const char* name) {
  luaL_getmetatable(_lua_state, "Constants");
  if (lua_isnil(_lua_state, -1)) {
    lua_pop(_lua_state, 1);
    luaL_newmetatable(_lua_state, "Constants");
    lua_pushstring(_lua_state, "__newindex");
    lua_pushcfunction(_lua_state, [](lua_State* L) -> int {
      return luaL_error(L, "Attempt to modify read-only constant");
    });
    lua_settable(_lua_state, -3);
  }
  lua_getglobal(_lua_state, name);
  lua_pushvalue(_lua_state, -2);
  lua_setmetatable(_lua_state, -2);
  lua_pop(_lua_state, 2);
  return true;
}