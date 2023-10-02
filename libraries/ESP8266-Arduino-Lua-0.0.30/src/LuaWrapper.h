#ifndef LUA_WRAPPER_H
#define LUA_WRAPPER_H

#include "Arduino.h"

#define LUA_USE_C89
#include "lua/lua.hpp"

class LuaWrapper {
  public:
    LuaWrapper();
    String Lua_dostring(const String *script);
    void Lua_register(const String name, const lua_CFunction function);

  private:
    lua_State *_state;
    String addConstants();
};

#endif
