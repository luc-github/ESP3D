/*
  storestrings.h - rolling storage class

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

#ifndef STORESTRINGS_h
#define STORESTRINGS_h
#include <Arduino.h>
#include "GenLinkedList.h"
class STORESTRINGS_CLASS
{
public:
    STORESTRINGS_CLASS (int maxsize = -1, int maxstringlength=-1);
    ~STORESTRINGS_CLASS ();
    bool add (const char * string);
    inline bool add (String & string)
    {
        return add(string.c_str());
    };
    bool add (const __FlashStringHelper *str);
    bool remove(int pos);
    const char * get(int pos);
    int get_index(const char * string);
    void clear();
    inline int size()
    {
        return _charlist.size();
    };
    bool setsize(int size);
    bool setlength(int len);
    inline int getsize()
    {
        return _maxsize;
    };
    inline int getlength()
    {
        return _maxstringlength;
    };

private:
    int _maxsize;
    int _maxstringlength;
    GenLinkedList<char *> _charlist;
};

#endif
