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
  Revision 1.11  1999/02/26 10:41:45  djr
  Removed brackets round 'new char[(int)(len)]' to work around MSVC bug.

  Revision 1.10  1999/02/01 15:40:44  djr
  Initialise strings allocated with string_alloc() to zero length.
  Inline uses of string_alloc() and string_free().

  Revision 1.10  1999/02/01 14:42:36  djr
  Initialise allocated strings to zero length.
  Corrected several cases where a string is duplicated - the buffer allocated
  was 1 byte too long.

  Revision 1.9  1999/01/07 15:41:29  djr
  *** empty log message ***

  Revision 1.8  1998/04/07 19:33:01  sll
  Replace cerr with omniORB::log

// Revision 1.7  1998/01/27  15:33:11  ewc
// Added support for type any
//
// Revision 1.6  1998/01/21  12:13:04  sll
// Now accepts null pointer as marshalling argument. Substituted with a
// proper nil string.  Print a warning if traceLevel > 1.
// Now unmarshal zero size string. Substituted with a proper nil string.
// Print a warning if traceLevel > 1.
//
// Revision 1.5  1997/12/18  17:32:40  sll
// *** empty log message ***
//
// Revision 1.4  1997/05/06  15:11:48  sll
// Public release.
//
 */

#include <omniORB2/CORBA.h>


#define ALLOC_BYTES(len)  new char[(int)(len)]
#define FREE_BYTES(p)     delete[] (p)


char*
CORBA::string_alloc(CORBA::ULong len)
{
  // We initialise the string to zero length to help prevent errors
  // if this string is copied before it is initialied. This is easy
  // to do when assigning the returned value to a CORBA::String_var.
  char* s = ALLOC_BYTES(len + 1);
  if( s )  *s = '\0';
  return s;
}


void
CORBA::string_free(char* p)
{
  FREE_BYTES(p);
}


char*
CORBA::string_dup(const char* p)
{
  if (p) {
    char* q = ALLOC_BYTES(strlen(p) + 1);
    if (q) {
      strcpy(q,p);
      return q;
    }
  }
  return 0;
}

//////////////////////////////////////////////////////////////////////
///////////////////////////// String_var /////////////////////////////
//////////////////////////////////////////////////////////////////////

CORBA::String_var::String_var(const CORBA::String_member& s)
{
  if ((const char*)s) {
    _data = ALLOC_BYTES(strlen(s) + 1);
    strcpy(_data,s);
  }
  else
    _data = 0;
}


CORBA::String_var&
CORBA::String_var::operator= (const CORBA::String_member& s)
{
  if (_data) {
    FREE_BYTES(_data);
    _data = 0;
  }
  if ((const char*)s) {
    _data = ALLOC_BYTES(strlen(s) + 1);
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
CORBA::String_member::operator>>= (NetBufferedStream& s) const
{
  if( _ptr ) {
    CORBA::ULong _len = strlen((char*)_ptr) + 1;
    _len >>= s;
    s.put_char_array((CORBA::Char*)_ptr, _len);
  }
  else {
    if (omniORB::traceLevel > 1) {
      _CORBA_null_string_ptr(0);
    }
    CORBA::ULong(1) >>= s;
    CORBA::Char('\0') >>= s;
  }
}


void
CORBA::String_member::operator<<= (NetBufferedStream& s)
{
  if( _ptr ) {
    FREE_BYTES(_ptr);
    _ptr = 0;
  }

  CORBA::ULong len;
  len <<= s;
  if( !len && omniORB::traceLevel > 1 )  _CORBA_null_string_ptr(1);

  CORBA::ULong nbytes = len ? len : 1;
  char* p = ALLOC_BYTES(nbytes);
  if( !p )  throw CORBA::NO_MEMORY(0, CORBA::COMPLETED_MAYBE);

  if( len ) {
    try {
      s.get_char_array((CORBA::Char*)p, len);
    }
    catch(...) {
      FREE_BYTES(p);
      throw;
    }
    if( p[len - 1] != '\0' ) {
      FREE_BYTES(p);
      throw CORBA::MARSHAL(0, CORBA::COMPLETED_MAYBE);
    }
  }
  else  *p = '\0';

  _ptr = p;
}


void
CORBA::String_member::operator>>= (MemBufferedStream& s) const
{
  if( _ptr ) {
    CORBA::ULong _len = strlen((char*)_ptr) + 1;
    _len >>= s;
    s.put_char_array((CORBA::Char*)_ptr, _len);
  }
  else {
    if (omniORB::traceLevel > 1) {
      _CORBA_null_string_ptr(0);
    }
    CORBA::ULong(1) >>= s;
    CORBA::Char('\0') >>= s;
  }
}


void
CORBA::String_member::operator<<= (MemBufferedStream& s)
{
  if( _ptr ) {
    FREE_BYTES(_ptr);
    _ptr = 0;
  }

  CORBA::ULong len;
  len <<= s;
  if( !len || s.RdMessageUnRead() < len )
    throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);

  char* p = ALLOC_BYTES(len);
  if( !p )  throw CORBA::NO_MEMORY(0, CORBA::COMPLETED_MAYBE);

  s.get_char_array((CORBA::Char*)p, len);
  if( p[len - 1] != '\0' ) {
    FREE_BYTES(p);
    throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
  }

  _ptr = p;
}


size_t
CORBA::String_member::NP_alignedSize(size_t initialoffset) const
{
  size_t alignedsize = omni::align_to(initialoffset,omni::ALIGN_4);
  if (!_ptr) {
    alignedsize += 4 + 1;
  }
  else {
    alignedsize += 4 + strlen((char*)_ptr) + 1;
  }
  return alignedsize;
}


void 
_CORBA_null_string_ptr(_CORBA_Boolean unmarshal)
{
  if (unmarshal) {
    omniORB::log << "Warning: unmarshal received a zero size string.\n"
		 << "         Substituted with a proper nil string \"\\0\".\n";
    omniORB::log.flush();
  }
  else {
    omniORB::log << "Warning: try to marshal a null pointer as a string.\n"
		 << "         Substituted with a proper nil string \"\\0\".\n";
    omniORB::log.flush();
  }

}
