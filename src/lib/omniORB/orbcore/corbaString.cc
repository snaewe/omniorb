// -*- Mode: C++; -*-
//                            Package   : omniORB2
// corbaString.cc             Created on: 20/9/96
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 1996, 1997 Olivetti & Oracle Research Laboratory
//
//    This file is part of the omniORB library
//
//    The omniORB library is free software; you can redistribute it and/or
//    modify it under the terms of the GNU Library General Public
//    License as published by the Free Software Foundation; either
//    version 2 of the License, or (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//    Library General Public License for more details.
//
//    You should have received a copy of the GNU Library General Public
//    License along with this library; if not, write to the Free
//    Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
//    02111-1307, USA
//
//
// Description:
//      Implementation of the String interface
//	

/*
  $Log$
  Revision 1.6  1998/01/21 12:13:04  sll
  Now accepts null pointer as marshalling argument. Substituted with a
  proper nil string.  Print a warning if traceLevel > 1.
  Now unmarshal zero size string. Substituted with a proper nil string.
  Print a warning if traceLevel > 1.

// Revision 1.5  1997/12/18  17:32:40  sll
// *** empty log message ***
//
// Revision 1.4  1997/05/06  15:11:48  sll
// Public release.
//
 */

#include <omniORB2/CORBA.h>

char *
CORBA::string_alloc(CORBA::ULong len)
{
  return new char[(int)len+1];
}

void
CORBA::string_free(char *p)
{
  delete [] p;
}

char *
CORBA::string_dup(const char *p)
{
  if (p) {
    char *q = CORBA::string_alloc((CORBA::ULong)(strlen(p)+1));
    if (q) {
      strcpy(q,p);
      return q;
    }
  }
  return 0;
}

CORBA::String_member&
CORBA::String_member::operator= (const CORBA::String_var &s)
{
  if (_ptr) {
    string_free(_ptr);
    _ptr = 0;
  }
  if ((const char *)s) {
    _ptr = string_alloc((ULong)(strlen(s)+1));
    strcpy(_ptr,s);
  }
  return *this;
}

CORBA::String_var::String_var(const CORBA::String_member &s)
{
  if ((const char*)s) {
    _data = string_alloc((ULong)(strlen(s)+1));
    strcpy(_data,s);
  }
  else
    _data = 0;
}


CORBA::String_var&
CORBA::String_var::operator= (const CORBA::String_member &s)
{
  if (_data) {
    string_free(_data);
    _data = 0;
  }
  if ((const char*)s) {
    _data = string_alloc((ULong)(strlen(s)+1));
    strcpy(_data,s);
  }
  return *this;
}


char &
CORBA::String_var::operator[] (CORBA::ULong index) 
{
  if (!_data || (CORBA::ULong)strlen(_data) < index) {
    _CORBA_bound_check_error();	// never return
  }
  return _data[index];
}

char
CORBA::String_var::operator[] (CORBA::ULong index) const
{
  if (!_data || (CORBA::ULong)strlen(_data) < index) {
    _CORBA_bound_check_error();	// never return
  }
  return _data[index];
}

void
CORBA::String_member::operator>>= (NetBufferedStream &s) const
{
  CORBA::ULong _len;
  if (!_ptr) {
    if (omniORB::traceLevel > 1) {
      _CORBA_null_string_ptr(0);
    }
    _len = 1;
    _len >>= s;
    CORBA::Char _dummy = '\0';
    _dummy >>= s;
  }
  else {
    _len = strlen((char *)_ptr)+1;
    _len >>= s;
    s.put_char_array((CORBA::Char *)_ptr,_len);
  }
  return;
}

void
CORBA::String_member::operator<<= (NetBufferedStream &s)
{  
  CORBA::ULong _len;
  _len <<= s;
  if (!_len) {
    if (omniORB::traceLevel > 1) {
      _CORBA_null_string_ptr(1);
    }
    _len = 1;
  }
  else if (s.RdMessageUnRead() < _len)
    throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
  if (_ptr) {
    CORBA::string_free(_ptr);
    _ptr = 0;
  }
  _ptr = CORBA::string_alloc(_len);
  if (!_ptr)
    throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
  if (_len > 1)
    s.get_char_array((CORBA::Char *)_ptr,_len);
  else
    *((CORBA::Char*)_ptr) <<= s;
    
  return;
}

void
CORBA::String_member::operator>>= (MemBufferedStream &s) const
{
  CORBA::ULong _len;
  if (!_ptr) {
    if (omniORB::traceLevel > 1) {
      _CORBA_null_string_ptr(0);
    }
    _len = 1;
    _len >>= s;
    CORBA::Char _dummy = '\0';
    _dummy >>= s;
  }
  else {
    _len = strlen((char *)_ptr)+1;
    _len >>= s;
    s.put_char_array((CORBA::Char *)_ptr,_len);
  }
  return;
}

void
CORBA::String_member::operator<<= (MemBufferedStream &s)
{
  CORBA::ULong _len;
  _len <<= s;
  if (!_len) {
    if (omniORB::traceLevel > 1) {
      _CORBA_null_string_ptr(1);
    }
    _len = 1;
  }
  else if (s.unRead() < _len)
    throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
  if (_ptr) {
    CORBA::string_free(_ptr);
    _ptr = 0;
  }
  _ptr = CORBA::string_alloc(_len);
  if (!_ptr)
    throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
  if (_len > 1)
    s.get_char_array((CORBA::Char *)_ptr,_len);
  else
    *((CORBA::Char*)_ptr) <<= s;
  return;
}

size_t
CORBA::String_member::NP_alignedSize(size_t initialoffset) const
{
  size_t alignedsize = omni::align_to(initialoffset,omni::ALIGN_4);
  if (!_ptr) {
    alignedsize += 4 + 1;
  }
  else {
    alignedsize += 4 + strlen((char *)_ptr) + 1;
  }
  return alignedsize;
}


void 
_CORBA_null_string_ptr(_CORBA_Boolean unmarshal)
{
  if (unmarshal) {
    cerr << "Warning: unmarshal received a zero size string.\n"
	 << "         Substituted with a proper nil string \"\\0\"."
	 << endl;
  }
  else {
    cerr << "Warning: try to marshal a null pointer as a string.\n"
	 << "         Substituted with a proper nil string \"\\0\"."
	 << endl;
  }

}
