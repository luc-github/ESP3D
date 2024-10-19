--[[
    Detect push button

    This example detects a push button connected to pin 4 and sends the command
    to the printer screen when the button is pressed.

    The push button is connected to pin 4 and the LOW value is when the button
    is pressed.

    Put this script in the ESP3D flash file system and call it from the init script in 
    configuration.h.
    #define ESP_AUTOSTART_SCRIPT "[ESP300]/FS/detectpush.lua"

    You can also start it manually from the serial monitor by typing:
    [ESP300]/FS/detectpush.lua
    
    Be sure to have the following line comment out in the configure.h:
    #define ESP_LUA_INTERPRETER_FEATURE
    ]]--

-- Setup
-- pin 0 is connected to the push button
local pin = 0
-- LOW is the value when the button is pressed
local trigger_value = LOW
-- send ESP3D command to display current IP address to the printer screen
local command = "[ESP212]IP:%ESP_IP%\n"
-- variable to read the pin value
local pinval
local lastpush = millis()
-- define the pin mode
pinMode(pin, INPUT_PULLUP)
-- Main loop
while (true) do
    -- read the pin value
    pinval = digitalRead(pin)
    -- if the pin value is `trigger_value` then send the command
    if (pinval == trigger_value) then
        -- do not overflow the system so only send command if at least 1 second elapsed since last one
        if ((millis()-lastpush) > 1000) then
            lastpush = millis()
            -- send the command to the esp3d
            print(command)
        end
    end
    -- yield to other tasks
    yield()
end
