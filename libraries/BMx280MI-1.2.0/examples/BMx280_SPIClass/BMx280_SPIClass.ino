// BMx280_SPI.ino
//
// shows how to use the BMx280 library with the sensor connected using SPI.
//
// Copyright (c) 2018 Gregor Christandl
//
// connect the AS3935 to the Arduino like this:
//
// Arduino - AS3935
// 5V ------ VCC
// GND ----- GND
// MOSI ---- SDA
// SCK ----- SCL
// MISO ---- SDO
// D4 ------ CSB
// note: CSB _must_ be pulled low during BMx280 startup to enable the SPI interface. 

#include <Arduino.h>
#include <SPI.h>

#include <BMx280SPIClass.h>

#define PIN_CS 4

//create an BMx280SPI object using pin 4 as chip select pin
BMx280SPIClass bmx280(&SPI, PIN_CS);

void setup() {
	// put your setup code here, to run once:
	Serial.begin(9600);

	//wait for serial connection to open (only necessary on some boards)
	while (!Serial);

	SPI.begin();

	//begin() checks the Interface, reads the sensor ID (to differentiate between BMP280 and BME280)
	//and reads compensation parameters.
	if (!bmx280.begin())
	{
		Serial.println("begin() failed. check your BMx280 Interface and chip select pin.");
		while (1);
	}

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

	//important: measurement data is read from the sensor in function hasValue() only. 
	//make sure to call get*() functions only after hasValue() has returned true. 
	Serial.print("Pressure: "); Serial.println(bmx280.getPressure());
	Serial.print("Pressure (64 bit): "); Serial.println(bmx280.getPressure64());
	Serial.print("Temperature: "); Serial.println(bmx280.getTemperature());

	if (bmx280.isBME280())
	{
		Serial.print("Humidity: "); 
		Serial.println(bmx280.getHumidity());
	}
}
