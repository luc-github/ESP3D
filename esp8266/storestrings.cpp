/* 
  storestrings.cpp - rolling storage class

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
#include "storestrings.h"
#define MAX_STORAGE 20

STORESTRINGS_CLASS::STORESTRINGS_CLASS (uint8_t size){

	storage_size = (size<MAX_STORAGE)?size:MAX_STORAGE;
	storage = new String[storage_size];
	storage_cursor=0;
}

STORESTRINGS_CLASS::~STORESTRINGS_CLASS (){
	 delete storage;
	 storage_size = 0;
}

bool STORESTRINGS_CLASS::add (const char * string){
	//check if if there is something to add
	if (strlen(string)> 0 )
	{	//is current cursor available or need a shift
		if (storage_cursor > storage_size-1)
		{
			for (uint i=0;i<storage_size-1;i++)
				{
				storage[i]=storage[i+1];
				}
			storage_cursor--;
		}
		storage[storage_cursor]=String(string);
		storage_cursor++;
		
	}
	else return false;
}

String STORESTRINGS_CLASS::get_index_at(uint pos)
{
	if (pos > storage_size-1) return storage[storage_size-1];
	else return storage[pos];
}

uint STORESTRINGS_CLASS::get_size()
{
	return storage_size;
}

int STORESTRINGS_CLASS::get_used_max_index()
{
if (storage_cursor > storage_size-1) return storage_size-1;
else return storage_cursor-1;	
}
