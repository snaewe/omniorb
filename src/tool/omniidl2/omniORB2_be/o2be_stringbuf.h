// -*- Mode: C++; -*-
//                          Package   : omniidl2
// o2be_stringbuf.h         Created on: 20/4/1999
//			    Author    : David Riddoch (djr)
//
//    Copyright (C) 1996-1999 AT&T Laboratories Cambridge
//
//  This file is part of omniidl2.
//
//  Omniidl2 is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
//  USA.
//
// Description:
//

#ifndef __O2BE_STRINGBUF_H__
#define __O2BE_STRINGBUF_H__

#include <string.h>
#include <stdio.h>


#define STRING_BUF_INC  64


class StringBuf {
public:
  inline StringBuf(size_t size = STRING_BUF_INC) {
    pd_current = pd_start = new char[size];
    pd_end = pd_start + size;
    *pd_current = '\0';
  }
  inline ~StringBuf() { delete[] pd_start; }

  inline operator const char* () { return pd_start; }

  inline void operator += (const char* s) {
    size_t len = strlen(s);
    if( len > free_space() )  reserve(len);
    memcpy(pd_current, s, len + 1);
    pd_current += len;
  }

  inline void operator += (char c) {
    if( free_space() < 1 )  reserve(1);
    *pd_current++ = c;
    *pd_current = '\0';
  }

  inline void operator += (int i) {
    char s[20];
    sprintf(s, "%d", i);
    *this += s;
  }

  inline void clear() { pd_current = pd_start; *pd_current = '\0'; }

  void reserve(size_t n);
  // Allocate enough buffer st. there are at least n bytes of free space.

  char* release();
  // Return the string, and re-initialises this string to empty.
  // Caller takes responsibility for releasing the memory.

private:
  inline size_t free_space() { return pd_end - pd_current - 1; }

  char* pd_start;
  char* pd_current;
  char* pd_end;
};


#endif  // __O2BE_STRINGBUF_H__
