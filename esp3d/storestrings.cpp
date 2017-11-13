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
//Constructor
STORESTRINGS_CLASS::STORESTRINGS_CLASS (int maxsize, int maxstringlength)
{
//for rolling buffer
//if max size is reached then remove oldest one and add the new one
    _maxsize=maxsize;
//to limit the storage space
    _maxstringlength=maxstringlength;
//need space for the "..."
    if (_maxstringlength<4 && _maxstringlength!=-1) {
        _maxstringlength=4;
    }
}
//Destructor
STORESTRINGS_CLASS::~STORESTRINGS_CLASS ()
{
    // clear list and content
    clear();
}

bool STORESTRINGS_CLASS::setsize(int size)
{
    _maxsize=size;
    return true;
}
bool STORESTRINGS_CLASS::setlength(int len)
{
    if (len < 4) {
        return false;
    }
    _maxstringlength = len;
    return true;
}

//Clear list and content
void STORESTRINGS_CLASS::clear()
{
    //while list is not empty
    while(_charlist.size()) {
        //remove element
        char * str = _charlist.pop();
        //destroy it
        delete str;
    }
}

bool STORESTRINGS_CLASS::add (const __FlashStringHelper *str)
{
    String stmp;
    stmp=str;
    return add(stmp.c_str());
}
//Add element in storage
bool STORESTRINGS_CLASS::add (const char * string)
{
    //if we reach max size
    if (_maxsize==_charlist.size()) {
        //remove oldest one
        char * str = _charlist.shift();
        delete str;
    }
    //add new one
    //get size including \0 at the end
    size_t size = strlen(string)+1;
    bool need_resize=false;
    if ( (_maxstringlength!=-1) && (size >_maxstringlength+1 )) {
        need_resize = true;
        size=_maxstringlength+1;
    }
    //reserve memory
    char * ptr = new char[size*sizeof(char)];
    //copy string to storage
    if (need_resize) {
        //copy maximum length minus 3
        strncpy(ptr,string,_maxstringlength-3);
        strcpy(ptr+_maxstringlength-3,"...");
    } else {
        //copy as it is
        strcpy(ptr,string);
    }
    //add storage to list
    _charlist.add(ptr);
    return true;
}
//Remove element at pos position
bool STORESTRINGS_CLASS::remove(int pos)
{
    //be sure index is in range
    if (pos<0 && pos>(_charlist.size()-1)) {
        return false;
    }
    //remove item from list
    char * str = _charlist.remove(pos);
    //destroy item
    delete str;
    return true;
}
//Get element at pos position
const char * STORESTRINGS_CLASS::get(int pos)
{
    //be sure index is in range
    if (pos<0 && pos>(_charlist.size()-1)) {
        return NULL;
    }
    return (const char *) _charlist.get(pos);
}
//Get index for defined string
int STORESTRINGS_CLASS::get_index(const char * string)
{
    //parse the list until it is found
    for (int p=0; p<_charlist.size(); p++) {
        if (strcmp ( _charlist.get(p),  string)==0) {
            return p;
        }
    }
    //if not found return -1
    return -1;
}

