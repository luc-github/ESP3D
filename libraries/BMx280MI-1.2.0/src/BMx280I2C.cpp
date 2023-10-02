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

#include "BMx280I2C.h"

BMx280I2C::BMx280I2C(uint8_t address) :
	BMx280TwoWire(&Wire, address)
{
	//nothing to do here...
}


BMx280I2C::~BMx280I2C()
{
	//nothing to do here...
}
