--[[
    Echo Serial

    This example echo serial outout to serial with echo: header 
    Put this script in the ESP3D flash file system and call it from the init script in 
    configuration.h.
    #define ESP_AUTOSTART_SCRIPT "[ESP300]/FS/detectpush.lua"

    Be sure to have the following line comment out in the configure.h:
    #define ESP_LUA_INTERPRETER_FEATURE
    ]]--

-- Setup
local availableMsg = 0
local message = ""
while (true) do
    -- check if have data
    availableMsg = available()
    -- if any message
    if (availableMsg > 0) then
        message = "echo:" .. readData()
        print(message)
    end
    -- yield to other tasks
    yield()
end
