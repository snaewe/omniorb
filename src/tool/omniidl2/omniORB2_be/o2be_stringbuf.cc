// -*- Mode: C++; -*-
//                          Package   : omniidl2
// o2be_stringbuf.cc        Created on: 20/4/1999
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

#include <o2be_stringbuf.h>


void
StringBuf::reserve(size_t n)
{
  size_t extra = STRING_BUF_INC;
  while( extra + free_space() < n )
    extra += STRING_BUF_INC;
  size_t newsize = (pd_end - pd_start) + extra;
  char* newptr = new char[newsize];
  strcpy(newptr, pd_start);
  pd_current = newptr + (pd_current - pd_start);
  pd_end = newptr + newsize;
  pd_start = newptr;
}


void
StringBuf::grab(char* s)
{
  if( s ) {
    delete[] pd_start;
    pd_start = s;
    pd_current = s + strlen(s);
    pd_end = pd_current + 1;
  }
  else {
    pd_current = pd_start;
    *pd_current = '\0';
  }
}


char*
StringBuf::release()
{
  char* p = new char[pd_current - pd_start + 1];
  memcpy(p, pd_start, pd_current - pd_start + 1);
  pd_current = pd_start;
  *pd_current = '\0';
  return p;
}
