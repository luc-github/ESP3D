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

#include "BMx280SPIClass.h"

SPISettings BMx280SPIClass::spi_settings_ = SPISettings(2000000, MSBFIRST, SPI_MODE1);

BMx280SPIClass::BMx280SPIClass(SPIClass *spi, uint8_t chip_select) :
	cs_(chip_select)
{
	//nothing to do here...
}

BMx280SPIClass::~BMx280SPIClass()
{
	//nothing to do here...
}

bool BMx280SPIClass::beginInterface()
{
	pinMode(cs_, OUTPUT);
	digitalWrite(cs_, HIGH);		//deselect

	return true;
}

uint8_t BMx280SPIClass::readRegister(uint8_t reg)
{
	uint8_t return_value = 0;

	SPI.beginTransaction(spi_settings_);

	digitalWrite(cs_, LOW);				//select sensor

	SPI.transfer((reg & 0x3F) | 0x40);	//select register and set pin 7 (indicates read)

	return_value = SPI.transfer(0);

	digitalWrite(cs_, HIGH);			//deselect sensor

	return return_value;
}

uint32_t BMx280SPIClass::readRegisterBurst(uint8_t reg, uint8_t length)
{
	if (length > 4)
		return 0L;

	uint32_t data = 0;

	SPI.beginTransaction(spi_settings_);

	digitalWrite(cs_, LOW);				//select sensor

	SPI.transfer((reg & 0x3F) | 0x40);	//select register and set pin 7 (indicates read)

	for (uint8_t i = 0; i < length; i++)
	{
		data <<= 8;
		data |= SPI.transfer(0);
	}

	digitalWrite(cs_, HIGH);			//deselect sensor

	return data;
}

void BMx280SPIClass::writeRegister(uint8_t reg, uint8_t value)
{
	SPI.beginTransaction(spi_settings_);

	digitalWrite(cs_, LOW);				//select sensor

	SPI.transfer((reg & 0x3F));			//select regsiter 
	SPI.transfer(value);

	digitalWrite(cs_, HIGH);			//deselect sensor
}