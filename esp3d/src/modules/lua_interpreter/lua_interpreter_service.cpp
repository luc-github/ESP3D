/*
  lua_interpreter_service.cpp - ESP3D lua interpreter service class

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

#include "../../include/esp3d_config.h"

#ifdef ESP_LUA_INTERPRETER_FEATURE
#include "lua_interpreter_service.h"
#include "../../core/settings_esp3d.h"
#include "../../core/hal.h"
#include <LuaWrapper.h>

LuaWrapper luawrapper;
LuaInterpreter esp3d_lua_interpreter;

LuaInterpreter::LuaInterpreter()
{
    _started = false;
}

bool LuaInterpreter::begin()
{
    if(_started) {
        end();
    }
    _started = true;
    return _started;
}
void LuaInterpreter::end()
{
    if(!_started) {
        return;
    }
    _started = false;
}


void LuaInterpreter::handle()
{
    //Nothing to do as handled by ticker / task
}


LuaInterpreter::~LuaInterpreter()
{
    end();
}


#endif //ESP_LUA_INTERPRETER_FEATURE
