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
- **Note:** This is a blocking function. For long-running scripts, consider integrating it into a separate task to allow the rest of your firmware to continue running.


### `bool registerFunction(const char* name, lua_CFunction function)`
Registers a C function to be callable from Lua scripts.
- **Parameters:**
  - `name`: The name to use for the function in Lua.
  - `function`: A pointer to the C function to register.
- **Returns:** `true` if the function is successfully registered, `false` otherwise.

### `template<typename T> bool registerConstant(const char* name, T value)`
Registers a constant value that can be accessed from Lua scripts.
- **Parameters:**
  - `name`: The name to use for the constant in Lua.
  - `value`: The value of the constant. Supported types include `lua_Number`, `const char*`, `bool`, `int`, and `unsigned char`.
- **Returns:** `true` if the constant is successfully registered, `false` otherwise.

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
lua.executeScript(R"(
    print("PI is: " .. PI)
    print("Project: " .. PROJECT_NAME)
    myFunction()
)");
```

