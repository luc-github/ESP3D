This is a downstream version of Bill Greiman's <fat16lib@sbcglobal.net>
"SdFat" library for Arduino.

No code changes were intended to be made.  Only a custom namespace called
"sdfat" was wrapped around all objects and structures in the library.
This namespace is required because the ESP8266 already has global class
types whose names conflict with SdFat's built-in ones (File, others).

"using namespace sdfat;" at the top of a sketch should make it work as-is,
and all examples have been so updated.

By performing this wrapping I hope to be able to integrate the latest
updates from upstream and pull them into an ESP8266 compatible FS (like
SPIFFS or LittleFS) to make it much more useful and work much better with
the ESP8266 Arduino ecosystem.

-Earle F. Philhower, III <earlephilhower@yahoo.com>
