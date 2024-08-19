# EspLuaEngine for ESP boards



Unleash the power of flexible scripting on your ESP32 with EspLuaEngine! This library brings the versatility of Lua 5.4.7 to ESP devices, enabling dynamic and adaptable IoT applications.

While primarily optimized for ESP32, EspLuaEngine is also compatible with ESP8266 and ESP8685 platforms, albeit with reduced performance. This cross-platform support allows you to leverage Lua scripting across a range of ESP microcontrollers.

[![Lua 5.4.7](https://img.shields.io/badge/Lua%205.4.7-blue?style=plastic)](https://www.lua.org)    

## 🌟 Key Features

- **Highly Flexible**: Easily extend Lua with custom C functions and constants tailored to your project needs.
- **Resource-Efficient**: Optimized Lua 5.4.7 implementation designed for ESP32's constrained environment.
- **Arduino-Compatible**: Seamless integration with the Arduino framework for ESP32.
- **Customizable**: Add only the functionalities you need, keeping your project lean and efficient.

## 🚀 Quick Start

```cpp
#include "EspLuaEngine.h"

EspLuaEngine* lua;

void setup() {
  Serial.begin(115200);
  lua = new EspLuaEngine();
  
  // Register custom functions and constants
  lua->registerFunction("myCustomFunction", l_myCustomFunction);
  lua->registerConstant("MY_CONSTANT", 42);
  
  // Execute a Lua script
  lua->executeScript("print('Hello from ' .. _VERSION)");
}

void loop() {
  // Your Arduino loop code here
}
```

## 💡 Example Scripts

EspLuaEngine comes with several example scripts demonstrating its capabilities:

1. **HelloWorld**: A basic introduction to Lua scripting on ESP32.
2. **GPIO**: Demonstrates GPIO control using Lua scripts.
3. **Files**: Showcases file operations using the ESP32's file system.

Check out the `examples/` directory for the full scripts and more advanced use cases.

## 🔧 Customization

EspLuaEngine shines in its ability to adapt to your project's specific needs:

- **Add Custom Functions**: Extend Lua with C functions tailored to your hardware or application.
- **Define Project-Specific Constants**: Easily add constants that are relevant to your project.
- **Select Modules**: Include only the Lua modules you need, optimizing for size and performance.

## 📚 Documentation

For detailed API reference and customization guides, visit our [Wiki](https://github.com/luc-github/EspLuaEngine/wiki).

## 🤝 Contributing

We welcome contributions! Whether it's adding new features, improving documentation, or reporting issues, your input is valuable.

## 💖Supporters

### 💎Become a sponsor or a supporter
 * A sponsor is a recurent donator    
   As my sponsorship is not displayed by github your logo / avatar will be added to the readme page with eventually with a link to your site.    
 * A supporter is per time donator 
   To thank you for your support, your logo / avatar will be added to the readme page with eventually with a link to your site.  

 Every support is welcome, indeed helping users / developing new features need time and devices, donations contribute a lot to make things happen, thank you.

* liberapay <a href="https://liberapay.com/ESP3D/donate"><img alt="Donate using Liberapay" src="https://liberapay.com/assets/widgets/donate.svg"></a> 
* Paypal [<img src="https://www.paypalobjects.com/en_US/i/btn/btn_donateCC_LG_global.gif" border="0" alt="PayPal – The safer, easier way to pay online.">](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=FQL59C749A78L)
* ko-fi [![ko-fi](https://ko-fi.com/img/githubbutton_sm.svg)](https://ko-fi.com/G2G0C0QT7)   

## 📜 License

This project is licensed under the LGPL-3.0 License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- Inspired by [ESP8266-Arduino-Lua](https://github.com/fdu/ESP8266-Arduino-Lua)
- Built on the robust foundation of [Lua 5.4.7](https://www.lua.org/)

---

Elevate your ESP32 projects with the flexibility of EspLuaEngine! 🚀✨
