# EspLuaEngine for ESP32

## 📚 API Reference

### `EspLuaEngine()`
Constructor. Initializes a new Lua state and loads standard libraries.

### `~EspLuaEngine()`
Destructor. Closes the Lua state and frees resources.

### `bool executeScript(const char* script)`
Executes a Lua script.
- **Parameters:**
  - `script`: A null-terminated string containing the Lua code to execute.
- **Returns:** `true` if the script executes successfully, `false` otherwise.
- **Note:** This function is non-blocking and supports pause and stop operations.

### `bool registerFunction(const char* name, lua_CFunction function, void* userData = nullptr)`
Registers a C function to be callable from Lua scripts.
- **Parameters:**
  - `name`: The name to use for the function in Lua.
  - `function`: A pointer to the C function to register.
  - `userData`: Optional user data to be passed to the function.
- **Returns:** `true` if the function is successfully registered, `false` otherwise.

### `template<typename T> bool registerConstant(const char* name, T value)`
Registers a constant value that can be accessed from Lua scripts.
- **Parameters:**
  - `name`: The name to use for the constant in Lua.
  - `value`: The value of the constant. Supported types include `lua_Number`, `const char*`, `bool`, `int`, and `unsigned char`.
- **Returns:** `true` if the constant is successfully registered, `false` otherwise.

### `void resetState()`
Resets the Lua state, clearing all registered functions and constants.

### `void setPauseFunction(PauseFunction func)`
Sets a custom function to be called when execution is paused.
- **Parameters:**
  - `func`: A function of type `std::function<void(EspLuaEngine*)>` to be called during pauses.

### `void pauseExecution()`
Pauses the execution of the current script.

### `void resumeExecution()`
Resumes the execution of a paused script.

### `void stopExecution()`
Stops the execution of the current script.

### `bool isPaused()`
Checks if the script execution is currently paused.
- **Returns:** `true` if paused, `false` otherwise.

### `bool isRunning()`
Checks if a script is currently running.
- **Returns:** `true` if running, `false` otherwise.

### `Status getStatus()`
Gets the current status of the EspLuaEngine.
- **Returns:** An enum of type `EspLuaEngine::Status` with possible values:
  - `Idle`: No script is currently running.
  - `Running`: A script is currently executing.
  - `Paused`: Script execution is paused.

### `bool hasError()`
Checks if an error occurred during the last script execution.
- **Returns:** `true` if an error occurred, `false` otherwise.

### `const char* getLastError()`
Gets the last error message.
- **Returns:** A string containing the last error message, or an empty string if no error occurred.

### `lua_State* getLuaState()`
Gets the underlying Lua state.
- **Returns:** A pointer to the `lua_State` object.

## 🔧 Usage Examples

```cpp
EspLuaEngine lua;

// Register a custom function
lua.registerFunction("myFunction", l_myCustomFunction);

// Register constants
lua.registerConstant("PI", 3.14159);
lua.registerConstant("PROJECT_NAME", "MyESP32Project");
lua.registerConstant("DEBUG_MODE", true);

// Execute a Lua script
if (lua.executeScript(R"(
    print("PI is: " .. PI)
    print("Project: " .. PROJECT_NAME)
    myFunction()
)")) {
    Serial.println("Script executed successfully");
} else {
    Serial.print("Error executing script: ");
    Serial.println(lua.getLastError());
}

// Pause and resume execution
lua.pauseExecution();
// Do something while paused
lua.resumeExecution();

// Check status
if (lua.getStatus() == EspLuaEngine::Status::Running) {
    Serial.println("Script is running");
}

// Stop execution
lua.stopExecution();

// Reset state
lua.resetState();
```


## 📝 Notes

- The EspLuaEngine now supports non-blocking script execution with pause and stop capabilities.
- Use `setPauseFunction()` to define custom behavior during pauses.
- Always check for errors after executing a script using `hasError()` and `getLastError()`.
- The engine supports multitasking environments, making it suitable for complex ESP32 projects.
- **Important note for ESP8266 users:** While state monitoring and control are straightforward on ESP32 due to its task management capabilities, implementation on ESP8266 may require additional libraries or the use of an interrupt system. Currently, this functionality is not fully supported on ESP8266 platforms.

## 🚀 Platform-Specific Considerations

### ESP32
On ESP32, the EspLuaEngine takes full advantage of the FreeRTOS task management system, allowing for efficient multitasking and state control without additional setup.

### ESP8266
For ESP8266 users:   

- The current implementation may not fully support all state monitoring and control features.
- To achieve similar functionality as on ESP32, you might need to:
  1. Implement a custom interrupt-based system for state checks.
  2. Use additional libraries for task management (e.g., `TaskScheduler`).
  3. Carefully manage your main loop to prevent blocking while allowing for state checks.

Please note that these advanced features on ESP8266 are not officially supported in the current version of EspLuaEngine and may require custom modifications to the library.
