# Lua Environment Library for ESP32

This guide details the implementation and customization of Lua for the ESP32 platform.

## Implementation Overview

The library is implemented as a C++ class wrapping the Lua C API. Source files are located in the `src/lua-x.y.z/src` folder.

### Key Configuration Steps

1. **Header File**: Use `lua.hpp` for inclusion in your C++ code.
2. **Configuration File**: Modify `src/lua-x.y.z/src/luaconf.h` with the following crucial settings:

   ```c
   #define LUA_USE_C89      // For better ESP32 compatibility
   #define LUA_32BITS 1     // For ESP32's 32-bit architecture
   #undef LUA_USE_MODULES   // To reduce memory usage (static module loading)
   ```

## File Classification

### Essential Files (Do Not Remove)
- Core API: lapi.c/h, lauxlib.c/h, lbaselib.c
- Code Generation: lcode.c/h
- Character Handling: lctype.c/h
- Debugging: ldebug.c/h
- Interpreter Core: ldo.c/h
- Function Management: lfunc.c/h
- Memory Management: lgc.c/h, lmem.c/h
- Lexical Analysis: llex.c/h
- Type System: lobject.c/h
- Virtual Machine: lopcodes.c/h, lvm.c/h
- Parsing: lparser.c/h
- State Management: lstate.c/h
- String Handling: lstring.c/h
- Table Implementation: ltable.c/h
- Metatables: ltm.c/h
- Bytecode: lundump.c/h
- I/O: lzio.c/h

### Basic Module Files (Keep as Needed)
- String Library: lstrlib.c
- Table Library: ltablib.c
- UTF-8 Support: lutf8lib.c (optional)
- Math Library: lmathlib.c (optional)

### Removable Files (for Minimal Implementation)
- Coroutines: lcorolib.c
- Debugging: ldblib.c
- Function Dumping: ldump.c
- Library Initialization: linit.c (customize as needed)
- I/O Library: liolib.c
- Dynamic Loading: loadlib.c
- OS Functions: loslib.c
- Command-line Interface: lua.c
- Compiler: luac.c

### Essential Headers
- Main API: lua.h
- Configuration: luaconf.h (modify for optimization)
- Standard Libraries: lualib.h
- Limits and Configs: llimits.h
- C++ Wrapper: lua.hpp (if using C++)

### Optional Files
- Compilation Prefixes: lprefix.h
- Jump Tables and Opcode Names: ljumptab.h, lopnames.h

## File Listing with Descriptions

| File | Description | Status |
|------|-------------|--------|
| lapi.c/h | Lua C API | Essential |
| lauxlib.c/h | Auxiliary API functions | Essential |
| lbaselib.c | Basic library | Essential |
| lcode.c/h | Code generator | Essential |
| lctype.c/h | Character classification | Essential |
| ldebug.c/h | Debugging support | Essential |
| ldo.c/h | Interpreter core | Essential |
| lfunc.c/h | Function handling | Essential |
| lgc.c/h | Garbage collector | Essential |
| llex.c/h | Lexical analyzer | Essential |
| lmem.c/h | Memory management | Essential |
| lobject.c/h | Basic types | Essential |
| lopcodes.c/h | VM opcodes | Essential |
| lparser.c/h | Parser | Essential |
| lstate.c/h | Global state | Essential |
| lstring.c/h | String handling | Essential |
| ltable.c/h | Table implementation | Essential |
| ltm.c/h | Metatables | Essential |
| lundump.c/h | Bytecode loader | Essential |
| lvm.c/h | Virtual machine | Essential |
| lzio.c/h | Low-level I/O | Essential |
| lstrlib.c | String library | Keep if needed |
| ltablib.c | Table library | Keep if needed |
| lutf8lib.c | UTF-8 support | Optional |
| lmathlib.c | Math library | Optional |
| lcorolib.c | Coroutine library | Remove if unused |
| ldblib.c | Debug library | Remove for embedded |
| ldump.c | Function dumping | Remove if unused |
| linit.c | Library initialization | Customize |
| liolib.c | I/O library | Remove for embedded |
| loadlib.c | Dynamic loading | Remove for embedded |
| loslib.c | OS functions | Remove for embedded |
| lua.c | CLI interpreter | Remove |
| luac.c | Lua compiler | Remove |
| lua.h | Main Lua API header | Essential |
| luaconf.h | Lua configuration | Essential, modify |
| lualib.h | Standard libraries header | Essential |
| llimits.h | Limits and configurations | Essential |
| lua.hpp | C++ wrapper | Keep if using C++ |
| lprefix.h | Compilation prefixes | Optional |
| ljumptab.h | Jump tables | Optional |
| lopnames.h | Opcode names | Optional |
| Makefile | Build script | Keep if needed |

## Automation Script

A Python script `cleanLua.py` is provided to automatically disable unused files, streamlining the porting process.

## Porting Steps

1. Copy the Lua source files to your ESP32 project.
2. Modify `luaconf.h` as specified above.
3. Remove or disable unnecessary files using `cleanLua.py`.
4. Adjust your build system to include only the necessary Lua files.
5. Implement the `EspLuaEngine` class to interface with Lua.
6. Test thoroughly, especially memory usage and performance on ESP32.

## Notes for Future Lua Versions

- Check for any new files or changes in file dependencies.
- Review `luaconf.h` for new configuration options.
- Update the `cleanLua.py` script if file lists change.
- Test compatibility with your existing Lua scripts.

Remember to always refer to the official Lua documentation for the specific version you're porting, as implementation details may change between releases.
