# Yet Another Arduino BME280 and BMP280 Digital Pressure Sensor Library

home: https://bitbucket.org/christandlg/bmx280mi

sensors: https://www.bosch-sensortec.com/bst/products/all_products/bmp280 / https://www.bosch-sensortec.com/bst/products/all_products/bme280 

## Features:

- Supports I2C (via the Wire library) and SPI (via the SPI library) interfaces
- Supports other I2C and SPI libraries via inheritance
- Supports 64 bit pressure calculation
- Never blocks or delays (except for convenience functions)

## Changelog:
- 1.2.0
	- fixed a bug #3 where getPressure(), getPressure64() and getHumitiy would depend on getTemperature() to be called immediately before (since last time hasValue() returned true) in order to return correct values. 

- 1.1.2
	- updated documentation on function hasValue()
	- updated examples

- 1.1.1
	- fixed an error in 64 bit pressure calculation

- 1.1.0
	- added 64 bit pressure calculation

- 1.0.0
	- added new classes BMx280TwoWire and BMx280SPIClass for TwoWire and SPIClass interfaces
	-moved BMx280I2C and BMx280SPI classes into their own respecitve source files, further separating data processing from communications
	- when updating from an earlier version and using the BMx280I2C or BMx280SPI classes, change ```#include <BMx280MI.h>``` to ```#include <BMx280I2C.h>``` or ```#include <BMx280SPI.h>```, respectively
	- simplified function BMx280MI::measure()

- 0.0.2
	- humidity measurement implemented	

- 0.0.1
	- initial release