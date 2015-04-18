/* 
  CONFIG.CPP- esp8266 configuration class

  Copyright (c) 2014 Luc Lebosse. All rights reserved.
 
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#include "config.h"
#include <EEPROM.h>

//read a string or a unique byte/flag
//a flag is 1 byte, a string is multibyte + \0, this is won't work if 1 char is multibyte like chinese char 
bool CONFIG::read_string(word pos, char byte_buffer[], word size_max)
{
  //check if parameters are acceptable
  if (size_max==0 ||  pos+size_max+1 > EEPROM_SIZE || byte_buffer== NULL)return false;
  byte b=0;
  word i=0;
  //read first byte
  b = EEPROM.read(pos + i);
  byte_buffer[i]=b;
  i++;
  //read until max size is reached or \0 is found
  while (i<size_max+1 && b!=0)
  {
    b = EEPROM.read(pos+i);
    byte_buffer[i]=b; 
    i++;
  } 
  //if it is a string be sure there is an 0 at the end
  if (b!=0)byte_buffer[i-1]=0x00; 
  return true;
}
//read a flag / byte
bool CONFIG::read_byte(word pos, byte * value)
{
  //check if parameters are acceptable
  if (pos+1 > EEPROM_SIZE)return false;
  value[0] = EEPROM.read(pos);
  return true;
}

//write a string (array of byte with a 0x00  at the end)
bool CONFIG::write_string(word pos, const char * byte_buffer, word size_buffer)
{
  //check if parameters are acceptable
  if (size_buffer==0 ||  pos+size_buffer+1 > EEPROM_SIZE || byte_buffer== NULL)return false;
  //copy the value(s)
  for (word i = 0; i < size_buffer; i++) {
    EEPROM.write(pos + i, byte_buffer[i]);
  }
  
  //0 terminal
  EEPROM.write(pos + size_buffer, 0x00);
  EEPROM.commit();
  return true;
}

//read a flag / byte
bool CONFIG::write_byte(word pos, const byte value)
{
  //check if parameters are acceptable
  if (pos+1 > EEPROM_SIZE)return false;
  EEPROM.write(pos, value);
  EEPROM.commit();
  return true;
}

bool CONFIG::reset_config()
{
  if(!CONFIG::write_byte(EP_WIFI_MODE,DEFAULT_WIFI_MODE))return false;
  if(!CONFIG::write_string(EP_SSID,DEFAULT_SSID,strlen(DEFAULT_SSID)))return false;
  if(!CONFIG::write_string(EP_PASSWORD,DEFAULT_PASSWORD,strlen(DEFAULT_PASSWORD)))return false;
  if(!CONFIG::write_byte(EP_IP_MODE,DEFAULT_IP_MODE))return false;
  if(!CONFIG::write_string(EP_IP_VALUE,DEFAULT_IP_VALUE,strlen(DEFAULT_IP_VALUE)))return false;
  if(!CONFIG::write_string(EP_MASK_VALUE,DEFAULT_MASK_VALUE,strlen(DEFAULT_MASK_VALUE)))return false;
  if(!CONFIG::write_string(EP_GATEWAY_VALUE,DEFAULT_GATEWAY_VALUE,strlen(DEFAULT_GATEWAY_VALUE)))return false;
  if(!CONFIG::write_string(EP_BAUD_RATE,DEFAULT_BAUD_RATE,strlen(DEFAULT_BAUD_RATE)))return false;
  return true;
}

void CONFIG::print_config()
{
  char sbuf[70];
  byte bbuf=0;
  if (CONFIG::read_byte(EP_WIFI_MODE, &bbuf ))Serial.println(byte(bbuf));
  if (CONFIG::read_string(EP_SSID, sbuf , MAX_SSID_LENGH))Serial.println(sbuf);
  if (CONFIG::read_string(EP_PASSWORD, sbuf , MAX_PASSWORD_LENGH))Serial.println(sbuf);
  if (CONFIG::read_byte(EP_IP_MODE, &bbuf ))Serial.println(byte(bbuf));
  if (CONFIG::read_string(EP_IP_VALUE, sbuf , MAX_IP_LENGH))Serial.println(sbuf);
  if (CONFIG::read_string(EP_MASK_VALUE, sbuf , MAX_IP_LENGH))Serial.println(sbuf);
  if (CONFIG::read_string(EP_GATEWAY_VALUE, sbuf , MAX_IP_LENGH))Serial.println(sbuf);
  if (CONFIG::read_string(EP_BAUD_RATE, sbuf , MAX_BAUD_LENGH))Serial.println(sbuf);
} 
