// BMx280_I2C.ino
//
// shows how to use the BMP280 / BMx280 library with the sensor connected using I2C.
//
// Copyright (c) 2018 Gregor Christandl
//
// connect the AS3935 to the Arduino like this:
//
// Arduino - BMP280 / BME280
// 3.3V ---- VCC
// GND ----- GND
// SDA ----- SDA
// SCL ----- SCL
// some BMP280/BME280 modules break out the CSB and SDO pins as well: 
// 5V ------ CSB (enables the I2C interface)
// GND ----- SDO (I2C Address 0x76)
// 5V ------ SDO (I2C Address 0x77)
// other pins can be left unconnected.

#include <Arduino.h>
#include <Wire.h>

#include <BMx280I2C.h>

#define I2C_ADDRESS 0x76

//create a BMx280I2C object using the I2C interface with I2C Address 0x76
BMx280I2C bmx280(I2C_ADDRESS);

void setup() {
  // put your setup code here, to run once:
	Serial.begin(9600);

	//wait for serial connection to open (only necessary on some boards)
	while (!Serial);

	Wire.begin();

	//begin() checks the Interface, reads the sensor ID (to differentiate between BMP280 and BME280)
	//and reads compensation parameters.
	if (!bmx280.begin())
	{
		Serial.println("begin() failed. check your BMx280 Interface and I2C Address.");
		while (1);
	}

	if (bmx280.isBME280())
		Serial.println("sensor is a BME280");
	else
		Serial.println("sensor is a BMP280");

	//reset sensor to default parameters.
	bmx280.resetToDefaults();

	//by default sensing is disabled and must be enabled by setting a non-zero
	//oversampling setting.
	//set an oversampling setting for pressure and temperature measurements. 
	bmx280.writeOversamplingPressure(BMx280MI::OSRS_P_x16);
	bmx280.writeOversamplingTemperature(BMx280MI::OSRS_T_x16);

	//if sensor is a BME280, set an oversampling setting for humidity measurements.
	if (bmx280.isBME280())
		bmx280.writeOversamplingHumidity(BMx280MI::OSRS_H_x16);
}

void loop() {
  // put your main code here, to run repeatedly:

	delay(1000);

	//start a measurement
	if (!bmx280.measure())
	{
		Serial.println("could not start measurement, is a measurement already running?");
		return;
	}

	//wait for the measurement to finish
	do
	{
		delay(100);
	} while (!bmx280.hasValue());

	Serial.print("Pressure: "); Serial.println(bmx280.getPressure());
	Serial.print("Pressure (64 bit): "); Serial.println(bmx280.getPressure64());
	Serial.print("Temperature: "); Serial.println(bmx280.getTemperature());

	//important: measurement data is read from the sensor in function hasValue() only. 
	//make sure to call get*() functions only after hasValue() has returned true. 
	if (bmx280.isBME280())
	{
		Serial.print("Humidity: "); 
		Serial.println(bmx280.getHumidity());
	}
}
