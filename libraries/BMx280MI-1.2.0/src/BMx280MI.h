//Multi interface Bosch Sensortec BMP280 / BME280 pressure sensor library 
// Copyright (c) 2018-2019 Gregor Christandl <christandlg@yahoo.com>
// home: https://bitbucket.org/christandlg/bmp280mi
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.

// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA


#ifndef BMX280MI_H_
#define BMX280MI_H_

#include <Arduino.h>

class BMx280MI
{
public:
	enum filter_setting_t : uint8_t
	{
		FILTER_OFF = 0b000,
		FILTER_x02 = 0b001,
		FILTER_x04 = 0b010,
		FILTER_x08 = 0b011,
		FILTER_x16 = 0b100,
	};

	//humidity measurement oversampling register settings
	//Note for BME280: when using filter, resolution is always 20 bit. 
	enum osrs_h : uint8_t
	{
		OSRS_H_x00 = 0b000,     //deactivated
		OSRS_H_x01 = 0b001,
		OSRS_H_x02 = 0b010,
		OSRS_H_x04 = 0b011,
		OSRS_H_x08 = 0b100,
		OSRS_H_x16 = 0b101
	};

	//pressure measurement oversampling register settings
	//Note for BME280: when using filter, resolution is always 20 bit. 
	enum osrs_p : uint8_t
	{
		OSRS_P_x00 = 0b000,     //deactivated
		OSRS_P_x01 = 0b001,
		OSRS_P_x02 = 0b010,
		OSRS_P_x04 = 0b011,
		OSRS_P_x08 = 0b100,
		OSRS_P_x16 = 0b101
	};

	//temperature measurement oversampling register settings
	//Note for BME280: when using filter, resolution is always 20 bit. 
	enum osrs_t : uint8_t
	{
		OSRS_T_x00 = 0b000,		//deactivated
		OSRS_T_x01 = 0b001,		//16 bits
		OSRS_T_x02 = 0b010,		//17 bits
		OSRS_T_x04 = 0b011,		//18 bits
		OSRS_T_x08 = 0b100,		//19 bits
		OSRS_T_x16 = 0b101		//20 bits
	};

	enum power_mode_t : uint8_t
	{
		BMx280_MODE_SLEEP = 0b00,		//device in sleep mode. 
		BMx280_MODE_FORCED = 0b01,		//measurements are performed on demand. 
		BMx280_MODE_FORCED_ALT = 0b10,	//alternative value for BMx280_MODE_FORCED. 
		BMx280_MODE_NORMAL = 0b11		//measurements are performed periodically. 
	};

	enum standby_time_t : uint8_t
	{
		T_SB_0 = 0b000,		//0.5 ms
		T_SB_1 = 0b001, 	//62.5 ms
		T_SB_2 = 0b010, 	//125 ms
		T_SB_3 = 0b011, 	//250 ms
		T_SB_4 = 0b100, 	//500 ms
		T_SB_5 = 0b101, 	//1000 ms
		T_SB_6 = 0b110, 	//2000 ms (BMP280), 10ms (BME280)
		T_SB_7 = 0b111, 	//4000 ms (BMP280), 20ms (BME280)
	};

	//compensation parameters
	struct BMx280CompParams {
		uint16_t dig_T1_;
		int16_t dig_T2_;
		int16_t dig_T3_;

		uint16_t dig_P1_;
		int16_t dig_P2_;
		int16_t dig_P3_;
		int16_t dig_P4_;
		int16_t dig_P5_;
		int16_t dig_P6_;
		int16_t dig_P7_;
		int16_t dig_P8_;
		int16_t dig_P9_;

		uint8_t dig_H1_;
		int16_t dig_H2_;
		uint8_t dig_H3_;
		int16_t dig_H4_;
		int16_t dig_H5_;
		int8_t dig_H6_;
	};

	static const uint8_t BMP280_ID = 0x58;
	static const uint8_t BME280_ID = 0x60;

	BMx280MI();
	virtual ~BMx280MI();

	//initializes the sensor. 
	//@return true on success, false otherwise. 
	bool begin();

	//starts a measurement. return true if automatic measurements are enabled (normal mode) or a forced measurement
	//is currently running. 
	//@return true on success, false otherwise. 
	bool measure();

	//@return true if a measurement was completed, false otherwise. 
	//checks if a measurement is currently running. if no measurement is running, reads measurement data from the sensor. 
	//important: values returned by get*() functions will not change unless hasValue() was called and returned true. 
	//if automatic measurements are enabled calling this function may cause the values returned by getHumidity(), 
	//getPressure() and getTemperature() to change. 
	bool hasValue();
	
	//@return the last measured humidity value, in %RH. uses the calculation code taken from the datasheet. 
	float getHumidity();

	//@return the last measured pressure, in Pa. uses the 32bit calculation code taken from the datasheet. 
	float getPressure();

	//@return the last measured pressure, in Pa. uses the 64bit calculcation code taken from the datasheet.
	double getPressure64();

	//@return the last measured temperature, in deg C. uses the 32bit calculation code taken from the datasheet. 
	float getTemperature();
	
	//triggers a measurement and returns the measured humidity. do not use in 
	//event loops, as it blocks. 
	//@return humidtity in %RH or NAN if the measurement failed (or the sensor is not a BME280). 
	float readHumidity();

	//triggers a measurement and returns the measured temperature. do not use in 
	//event loops, as it blocks. 
	//@return temperature in deg C or NAN if the measurement failed. 
	float readTemperature();

	//triggers a measurement and returns the measured pressure. do not use in 
	//event loops, as it blocks. 
	//@return pressure in Pa or NAN if the measurement failed. 
	float readPressure();

	//@return the ID of the BMP280. the sensor will always return 0x58 (BMP280) or 0x60 (BME280), so this function 
	//can be used as a communication check. 
	uint8_t readID();

	//reads the compensation parameters from the sensor. 
	//@return sensors compensation parameters as BMx280CompParams struct. 
	BMx280CompParams readCompensationParameters();

	//checks if the sensor ID matches that of a BME280 sensor. 
	//@param (optional) update_id if true the internally kept sensor ID is updated. 
	//@return true if the sensor is a BME280, false otherwise. 
	bool isBME280(bool update_id = false);

	/*
	resets all registers to default values. */
	void resetToDefaults();

	//@return pressure oversampling setting
	uint8_t readOversamplingHumidity();

	//@param value to set
	//@return true on success, false otherwise. 
	bool writeOversamplingHumidity(uint8_t value);

	//@return pressure oversampling setting
	uint8_t readOversamplingPressure();

	//@param value to set
	//@return true on success, false otherwise. 
	bool writeOversamplingPressure(uint8_t value);

	//@return pressure oversampling setting
	uint8_t readOversamplingTemperature();

	//@param value to set
	//@return true on success, false otherwise. 
	bool writeOversamplingTemperature(uint8_t value);

	//@return filter setting as filter_setting_t.
	uint8_t readFilterSetting();

	//@param filter setting as filter_setting_t.
	//@return true on success, false otherwise. 
	bool writeFilterSetting(uint8_t setting);

	//@return sensors power mode as power_mode_t.
	uint8_t readPowerMode();

	//sets the sensors power mode. 
	//@param power mode as power_mode_t
	//@return true on success, false otherwise. 
	bool writePowerMode(uint8_t mode);

	//@return standby time as standby_time_t. 
	uint8_t readStandbyTime();

	//sets the sensors standby time. only has an effect if measurements are done in 'normal' (automatic) mode. 
	//@param standby time as standby_time_t. 
	//@return true on success, false otherwise. 
	bool writeStandbyTime(uint8_t standby_time);

private:
	enum bmx280_register_t : uint8_t
	{
		BMx280_REG_ID = 0xD0,			//contains 0x58 after power on
		BMx280_REG_RESET = 0xE0,		//write 0xB6 to reset
		BMx280_REG_STATUS = 0xF3,		//bit 0: im_update, bit 3: measuring
		BMx280_REG_CTRL_MEAS = 0xF4,	//sets data acquisition options of device	
		BMx280_REG_CONFIG = 0xF5,		//sets the rate, filter and interface options of the device.
		BMx280_REG_PRESS = 0xF7,		//raw pressure measurement output data up[19:0] in registers 0xF7...0xF9
		BMx280_REG_TEMP = 0xFA,			//raw temperature measurement output data ut[19:0] in registers 0xFA...0xFC

		BMx280_REG_DIG_T1 = 0x88,		//dig_T1, unsigned short
		BMx280_REG_DIG_T2 = 0x8A,		//dig_T2, signed short
		BMx280_REG_DIG_T3 = 0x8C,		//dig_T3, signed short

		BMx280_REG_DIG_P1 = 0x8E,		//dig_P1, unsigned short
		BMx280_REG_DIG_P2 = 0x90,		//dig_P2, signed short
		BMx280_REG_DIG_P3 = 0x92,		//dig_P3, signed short
		BMx280_REG_DIG_P4 = 0x94,		//dig_P4, signed short
		BMx280_REG_DIG_P5 = 0x96,		//dig_P5, signed short
		BMx280_REG_DIG_P6 = 0x98,		//dig_P6, signed short
		BMx280_REG_DIG_P7 = 0x9A,		//dig_P7, signed short
		BMx280_REG_DIG_P8 = 0x9C,		//dig_P8, signed short
		BMx280_REG_DIG_P9 = 0x9E,		//dig_P9, signed short

		BME280_REG_DIG_H1 = 0xA1,		//dig_H1, unsigned char
		BME280_REG_DIG_H2 = 0xE1,		//dig_H2, signed short
		BME280_REG_DIG_H3 = 0xE3,		//dig_H3, unsigned char
		BME280_REG_DIG_H4_MSB = 0xE4,	//dig_H4_MSB, byte
		BME280_REG_DIG_H4_LSB = 0xE5,	//dig_H4_LSB, byte
		BME280_REG_DIG_H5_MSB = 0xE6,	//dig_H5_MSB, byte
		BME280_REG_DIG_H5_LSB = 0xE5,	//dig_H5_LSB, byte
		BME280_REG_DIG_H6 = 0xE7,		//dig_H6, signed char

		BME280_REG_CTRL_HUM = 0xF2,		//Controls oversampling of humidity data. 
		BME280_REG_HUM = 0xFD			//raw humidity measurement output data uh[15:0]
	};

	enum bmx280_mask_t : uint8_t
	{
		BMx280_MASK_ID = 0b11111111,
		BMP280_MASK_RESET = 0b11111111,

		BMx280_MASK_OSRS_H = 0b00000111,

		//register 0xF3
		BMx280_MASK_STATUS_IM_UPDATE = 0b00000001,
		BMx280_MASK_STATUS_MEASURING = 0b00001000,

		//register 0xF4
		BMx280_MASK_MODE = 0b00000011,
		BMx280_MASK_OSRS_P = 0b00011100,
		BMx280_MASK_OSRS_T = 0b11100000,

		//register 0xF5
		//BMx280_MASK_SPI3W_EN = 0b00000001,		//SPI 3 Wire is not supported. 
		BMx280_MASK_FILTER = 0b00011100,
		BMx280_MASK_T_SB = 0b11100000,
	};

	//32 bit masks for burst reading. 
	enum bmx280_mask_32bit_t : uint32_t
	{
		BMx280_MASK_DIG_T1 = 0x0000FFFF,		//dig_T1, unsigned short
		BMx280_MASK_DIG_T2 = 0x0000FFFF,		//dig_T2, signed short
		BMx280_MASK_DIG_T3 = 0x0000FFFF,		//dig_T3, signed short

		BMx280_MASK_DIG_P1 = 0x0000FFFF,		//dig_P1, unsigned short
		BMx280_MASK_DIG_P2 = 0x0000FFFF,		//dig_P2, signed short
		BMx280_MASK_DIG_P3 = 0x0000FFFF,		//dig_P3, signed short
		BMx280_MASK_DIG_P4 = 0x0000FFFF,		//dig_P4, signed short
		BMx280_MASK_DIG_P5 = 0x0000FFFF,		//dig_P5, signed short
		BMx280_MASK_DIG_P6 = 0x0000FFFF,		//dig_P6, signed short
		BMx280_MASK_DIG_P7 = 0x0000FFFF,		//dig_P7, signed short
		BMx280_MASK_DIG_P8 = 0x0000FFFF,		//dig_P8, signed short
		BMx280_MASK_DIG_P9 = 0x0000FFFF,		//dig_P9, signed short

		BME280_MASK_DIG_H1 = 0x000000FF,		//dig_H1, unsigned char
		BME280_MASK_DIG_H2 = 0x0000FFFF,		//dig_H2, signed short
		BME280_MASK_DIG_H3 = 0x000000FF,		//dig_H3, unsigned char
		BME280_MASK_DIG_H4_MSB = 0x000000FF,	//dig_H4_MSB, byte
		BME280_MASK_DIG_H4_LSB = 0x0000000F,	//dig_H4_LSB, byte
		BME280_MASK_DIG_H5_MSB = 0x000000FF,	//dig_H5_MSB, byte
		BME280_MASK_DIG_H5_LSB = 0x000000F0,	//dig_H5_LSB, byte
		BME280_MASK_DIG_H6 = 0x000000FF,		//dig_H6, signed char

		BME280_MASK_HUM =	0x0000FFFF,			//humidity data (16 bits)
		BMx280_MASK_PRESS = 0x00FFFFF0,			//pressure data (20 bits)
		BMx280_MASK_TEMP =	0x00FFFFF0			//temperature data (20 bits)
	};

	static const uint8_t BMx280_CMD_RESET = 0xB6;

	virtual bool beginInterface() = 0;

	/*
	updates the value of private class member variable temp_fine_, which must be up to date 
	for temperature, pressure and humidity calculations. 
	*/
	void updateTempFine();

	/*
	@param mask
	@return number of bits to shift value so it fits into mask. */
	template <class T> uint8_t getMaskShift(T mask)
	{
		uint8_t return_value = 0;

		//count how many times the mask must be shifted right until the lowest bit is set
		if (mask != 0)
		{
			while (!(mask & 1))
			{
				return_value++;
				mask >>= 1;
			}
		}

		return return_value;
	};

	/*
	@param register value of register.
	@param mask mask of value in register
	@return value of masked bits. */
	template <class T> T getMaskedBits(T reg, T mask)
	{
		//extract masked bits
		return ((reg & mask) >> getMaskShift(mask));
	};

	/*
	swaps the byte order (MSB -> LSB and LSB -> MSB) of a given uint16_t variable. 
	@param data 
	@return data*/
	uint16_t swapByteOrder(uint16_t data);

	/*
	@param register value of register
	@param mask mask of value in register
	@param value value to write into masked area
	@param register value with masked bits set to value. */
	uint8_t setMaskedBits(uint8_t reg, uint8_t mask, uint8_t value);

	/*
	reads the masked value from the register.
	@param reg register to read.
	@param mask mask of value.
	@return masked value in register. */
	uint8_t readRegisterValue(uint8_t reg, uint8_t mask);

	/*
	sets values in a register.
	@param reg register to set values in
	@param mask bits of register to set value in
	@param value value to set */
	void writeRegisterValue(uint8_t reg, uint8_t mask, uint8_t value);

	/*
	reads the masked values from multiple registers. maximum read length is 4 bytes. 
	@param reg register to read.
	@param mask mask of value.
	@param length number of bytes to read
	@return register content */
	uint32_t readRegisterValueBurst(uint8_t reg, uint32_t mask, uint8_t length);

	/*
	reads a register from the sensor. must be overwritten by derived classes.
	@param reg register to read.
	@return register content*/
	virtual uint8_t readRegister(uint8_t reg) = 0;

	/*
	reads a series of registers from the sensor. can be overwritten by derived classes. 
	if this function is not overwritten, the registers will be read using single-byte reads. 
	@param reg register to read.
	@param length number of bytes to read. maximum length is 4 bytes. 
	@return register content. LSB = last byte read. e.g., if start register = 0xE0 and length = 4, 
	the returned value will contain the register data like this: (MSB) [0xE0 0xE1 0xE2 0xE3] (LSB)*/
	virtual uint32_t readRegisterBurst(uint8_t reg, uint8_t length);

	/*
	writes a register to the sensor. must be overwritten by derived classes.
	this function is also used to send direct commands.
	@param reg register to write to.
	@param value value writeRegister write to register. */
	virtual void writeRegister(uint8_t reg, uint8_t value) = 0;

	uint8_t id_;				//the sensors ID. necessary to differentiate between BMP280 and BME280. 

	int32_t temp_fine_;

	uint16_t uh_;

	uint32_t up_;

	uint32_t ut_;

	BMx280CompParams comp_params_;
};

#endif /* BMX280MI_H_ */ 
