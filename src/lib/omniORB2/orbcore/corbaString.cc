// -*- Mode: C++; -*-
//                            Package   : omniORB2
// corbaString.cc             Created on: 20/9/96
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 1996-1999 AT&T Laboratories Cambridge
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
//    Implementation of the String interface.
//	

/*
  $Log$
  Revision 1.13  1999/04/21 11:17:43  djr
  Strings now defined outside CORBA scope, and typedefed. New sequence types.

  Revision 1.12  1999/03/11 16:25:52  djr
  Updated copyright notice

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

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <string.h>


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

_CORBA_String_var::_CORBA_String_var(const _CORBA_String_member& s)
{
  if ((const char*)s) {
    _data = ALLOC_BYTES(strlen(s) + 1);
    strcpy(_data,s);
  }
  else
    _data = 0;
}


_CORBA_String_var&
_CORBA_String_var::operator= (const _CORBA_String_member& s)
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
_CORBA_String_var::operator[] (CORBA::ULong index) 
{
  if (!_data || (CORBA::ULong)strlen(_data) < index) {
    _CORBA_bound_check_error();	// never return
  }
  return _data[index];
}


char
_CORBA_String_var::operator[] (CORBA::ULong index) const
{
  if (!_data || (CORBA::ULong)strlen(_data) < index) {
    _CORBA_bound_check_error();	// never return
  }
  return _data[index];
}

//////////////////////////////////////////////////////////////////////
//////////////////////////// String_member ///////////////////////////
//////////////////////////////////////////////////////////////////////

void
_CORBA_String_member::operator >>= (NetBufferedStream& s) const
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
_CORBA_String_member::operator <<= (NetBufferedStream& s)
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
      if( p[len - 1] != '\0' )
        throw CORBA::MARSHAL(0, CORBA::COMPLETED_MAYBE);
    }
    catch(...) {
      FREE_BYTES(p);
      throw;
    }
  }
  else  *p = '\0';

  _ptr = p;
}


void
_CORBA_String_member::operator >>= (MemBufferedStream& s) const
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
_CORBA_String_member::operator <<= (MemBufferedStream& s)
{
  if( _ptr ) {
    FREE_BYTES(_ptr);
    _ptr = 0;
  }

  CORBA::ULong len;
  len <<= s;
  if( !len && omniORB::traceLevel > 1 )  _CORBA_null_string_ptr(1);

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
_CORBA_String_member::NP_alignedSize(size_t initialoffset) const
{
  size_t alignedsize = omni::align_to(initialoffset,omni::ALIGN_4);
  if (!_ptr) {
    alignedsize += 4 + 1;
  }
  else {
    alignedsize += 5 + strlen((char*)_ptr);
  }
  return alignedsize;
}

//////////////////////////////////////////////////////////////////////
////////////////// _CORBA_Unbounded_Sequence__String /////////////////
//////////////////////////////////////////////////////////////////////

_CORBA_Unbounded_Sequence__String::
_CORBA_Unbounded_Sequence__String(_CORBA_ULong max,
				  _CORBA_ULong length,
				  char**       value,
				  _CORBA_Boolean release)
  : pd_max(max), pd_len(length), pd_buf(new _CORBA_String_member[max])
{
  if( release ) {
    for( _CORBA_ULong i = 0; i < length; i++ )  pd_buf[i]._ptr = value[i];
    delete[] value;
  }
  else {
    for( _CORBA_ULong i = 0; i < length; i++ )
      pd_buf[i] = (const char*) value[i];
  }
}


_CORBA_Unbounded_Sequence__String&
_CORBA_Unbounded_Sequence__String::operator= (
			      const _CORBA_Unbounded_Sequence__String& s)
{
  if( pd_max < s.pd_max ) {
    delete[] pd_buf;
    pd_buf = new _CORBA_String_member[s.pd_max];
  }
  pd_max = s.pd_max;
  pd_len = s.pd_len;
  for( _CORBA_ULong i = 0; i < pd_len; i++ )  pd_buf[i] = s.pd_buf[i];
  return *this;
}


void
_CORBA_Unbounded_Sequence__String::length(_CORBA_ULong len)
{
  if( len > pd_max ) {
    _CORBA_String_member* newbuf = new _CORBA_String_member[len];
    for( _CORBA_ULong i = 0; i < pd_len; i++ ) {
      newbuf[i]._ptr = pd_buf[i]._ptr;
      pd_buf[i]._ptr = 0;
    }
    pd_max = len;
    delete[] pd_buf;
    pd_buf = newbuf;
  }
  // If we've shrunk we need to clear the entries at the top.
  for( _CORBA_ULong i = len; i < pd_len; i++ )  pd_buf[i] = (char*) 0;
  pd_len = len;
}


size_t
_CORBA_Unbounded_Sequence__String::NP_alignedSize(size_t size) const
{
  size = omni::align_to(size, omni::ALIGN_4) + 4;

  for( _CORBA_ULong i = 0; i < pd_len; i++ ) {
    size = omni::align_to(size, omni::ALIGN_4);
    if( pd_buf[i]._ptr )  size += strlen(pd_buf[i]) + 5;
    else                  size += 5;
  }

  return size;
}


template<class buf_t>
inline void marshal_ss(const _CORBA_String_member* pd_buf,
		       _CORBA_ULong pd_len, buf_t& s)
{
  _CORBA_ULong(pd_len) >>= s;

  for( _CORBA_ULong i = 0; i < pd_len; i++ ) {
    char* p = pd_buf[i]._ptr;

    if( p ) {
      _CORBA_ULong len = strlen(p) + 1;
      len >>= s;
      s.put_char_array((CORBA::Char*) p, len);
    }
    else {
      if( omniORB::traceLevel > 1 )  _CORBA_null_string_ptr(0);
      CORBA::ULong(1) >>= s;
      CORBA::Char('\0') >>= s;
    }
  }
}


void
_CORBA_Unbounded_Sequence__String::operator >>= (NetBufferedStream& s) const
{
  marshal_ss(pd_buf, pd_len, s);
}


void
_CORBA_Unbounded_Sequence__String::operator >>= (MemBufferedStream& s) const
{
  marshal_ss(pd_buf, pd_len, s);
}


template<class buf_t>
inline void unmarshal_ss(_CORBA_String_member* pd_buf,
			 _CORBA_ULong slen, buf_t& s)
{
  for( _CORBA_ULong i = 0; i < slen; i++ ) {
    char*& p = pd_buf[i]._ptr;

    if( p ) { FREE_BYTES(p); p = 0; }

    _CORBA_ULong len;
    len <<= s;
    if( !len && omniORB::traceLevel > 1 )  _CORBA_null_string_ptr(1);

    _CORBA_ULong nbytes = len ? len : 1;
    char* ps = ALLOC_BYTES(nbytes);

    if( len ) {
      try {
	s.get_char_array((CORBA::Char*) ps, len);
	if( ps[len - 1] != '\0' )
          throw CORBA::MARSHAL(0, CORBA::COMPLETED_MAYBE);
      }
      catch(...) {
	FREE_BYTES(ps);
	throw;
      }
    }
    else *ps = '\0';

    p = ps;
  }
}


void
_CORBA_Unbounded_Sequence__String::operator <<= (NetBufferedStream& s)
{
  _CORBA_ULong slen;
  slen <<= s;
  length(slen);
  unmarshal_ss(pd_buf, slen, s);
}


void
_CORBA_Unbounded_Sequence__String::operator <<= (MemBufferedStream& s)
{
  _CORBA_ULong slen;
  slen <<= s;
  length(slen);
  unmarshal_ss(pd_buf, slen, s);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

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
