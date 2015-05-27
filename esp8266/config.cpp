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

//read a string 
//a string is multibyte + \0, this is won't work if 1 char is multibyte like chinese char 
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

//read a buffer of size_buffer
bool CONFIG::read_buffer(word pos, byte byte_buffer[], word size_buffer)
{
  //check if parameters are acceptable
  if (size_buffer==0 ||  pos+size_buffer > EEPROM_SIZE || byte_buffer== NULL)return false;
  word i=0;
  //read until max size is reached
  while (i<size_buffer )
  {
    byte_buffer[i]=EEPROM.read(pos+i);
    i++;
  } 
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

//write a buffer 
bool CONFIG::write_buffer(word pos, const byte * byte_buffer, word size_buffer)
{
  //check if parameters are acceptable
  if (size_buffer==0 ||  pos+size_buffer > EEPROM_SIZE || byte_buffer== NULL)return false;
  //copy the value(s)
  for (word i = 0; i < size_buffer; i++) {
    EEPROM.write(pos + i, byte_buffer[i]);
  }
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
  if(!CONFIG::write_string(EP_SSID,PROGMEM2CHAR(DEFAULT_SSID),strlen(PROGMEM2CHAR(DEFAULT_SSID))))return false;
  if(!CONFIG::write_string(EP_PASSWORD,PROGMEM2CHAR(DEFAULT_PASSWORD),strlen(PROGMEM2CHAR(DEFAULT_PASSWORD))))return false;
  if(!CONFIG::write_byte(EP_IP_MODE,DEFAULT_IP_MODE))return false;
  if(!CONFIG::write_buffer(EP_IP_VALUE,DEFAULT_IP_VALUE,IP_LENGH))return false;
  if(!CONFIG::write_buffer(EP_MASK_VALUE,DEFAULT_MASK_VALUE,IP_LENGH))return false;
  if(!CONFIG::write_buffer(EP_GATEWAY_VALUE,DEFAULT_GATEWAY_VALUE,IP_LENGH))return false;
  if(!CONFIG::write_buffer(EP_BAUD_RATE,(const byte *)&DEFAULT_BAUD_RATE,BAUD_LENGH))return false;
  if(!CONFIG::write_byte(EP_PHY_MODE,DEFAULT_PHY_MODE))return false;
  if(!CONFIG::write_byte(EP_SLEEP_MODE,DEFAULT_SLEEP_MODE))return false;
  if(!CONFIG::write_byte(EP_CHANNEL,DEFAULT_CHANNEL))return false;
  if(!CONFIG::write_byte(EP_AUTH_TYPE,DEFAULT_AUTH_TYPE))return false;
  if(!CONFIG::write_byte(EP_SSID_VISIBLE,DEFAULT_SSID_VISIBLE))return false;
  return true;
}

void CONFIG::print_config()
{
 //use bigest size for buffer
  char sbuf[MAX_PASSWORD_LENGH+1];
  byte bbuf=0;
  int ibuf=0;
  if (CONFIG::read_byte(EP_WIFI_MODE, &bbuf ))Serial.println(byte(bbuf));
  if (CONFIG::read_string(EP_SSID, sbuf , MAX_SSID_LENGH))Serial.println(sbuf);
  if (CONFIG::read_string(EP_PASSWORD, sbuf , MAX_PASSWORD_LENGH))Serial.println(sbuf);
  if (CONFIG::read_byte(EP_IP_MODE, &bbuf ))Serial.println(byte(bbuf));
  if (CONFIG::read_buffer(EP_IP_VALUE,(byte *)sbuf , IP_LENGH))Serial.println(wifi_config.ip2str((byte *)sbuf));
  if (CONFIG::read_buffer(EP_MASK_VALUE, (byte *)sbuf  , IP_LENGH))Serial.println(wifi_config.ip2str((byte *)sbuf));
  if (CONFIG::read_buffer(EP_GATEWAY_VALUE, (byte *)sbuf  , IP_LENGH))Serial.println(wifi_config.ip2str((byte *)sbuf));
  if (CONFIG::read_buffer(EP_BAUD_RATE,  (byte *)&ibuf , BAUD_LENGH))Serial.println(ibuf);
  if (CONFIG::read_byte(EP_PHY_MODE, &bbuf ))Serial.println(byte(bbuf));
  if (CONFIG::read_byte(EP_SLEEP_MODE, &bbuf ))Serial.println(byte(bbuf));
  if (CONFIG::read_byte(EP_CHANNEL, &bbuf ))Serial.println(byte(bbuf));
  if (CONFIG::read_byte(EP_AUTH_TYPE, &bbuf ))Serial.println(byte(bbuf));
  if (CONFIG::read_byte(EP_SSID_VISIBLE, &bbuf ))Serial.println(byte(bbuf));
} 
