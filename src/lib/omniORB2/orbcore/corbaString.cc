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
  Revision 1.14.2.2  2000/02/09 14:44:30  djr
  Set release flag in string member when unmarshalling strings.

  Revision 1.14.2.1  2000/01/31 11:05:59  djr
  Fixed bug in unmarshalling of String_member.

  Revision 1.14  1999/06/18 20:52:29  sll
  Updated with new sequence string implementation.

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
  // if this string is copied before it is initialised.  This is easy
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
  if( pd_rel && _ptr )  FREE_BYTES(_ptr);
  _ptr = 0;

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
  pd_rel = 1;
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
  if( pd_rel && _ptr )  FREE_BYTES(_ptr);
  _ptr = 0;

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
  pd_rel = 1;
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

size_t
_CORBA_Sequence__String::NP_alignedSize(size_t size) const
{
  size = omni::align_to(size, omni::ALIGN_4) + 4;

  for( _CORBA_ULong i = 0; i < pd_len; i++ ) {
    size = omni::align_to(size, omni::ALIGN_4);
    if( pd_data[i] )  size += strlen(pd_data[i]) + 5;
    else                  size += 5;
  }

  return size;
}


template<class buf_t>
inline void marshal_ss(char** buf,
		       _CORBA_ULong pd_len, buf_t& s)
{
  _CORBA_ULong(pd_len) >>= s;

  for( _CORBA_ULong i = 0; i < pd_len; i++ ) {
    char* p = buf[i];

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
_CORBA_Sequence__String::operator >>= (NetBufferedStream& s) const
{
  marshal_ss(pd_data, pd_len, s);
}


void
_CORBA_Sequence__String::operator >>= (MemBufferedStream& s) const
{
  marshal_ss(pd_data, pd_len, s);
}


template<class buf_t>
inline void unmarshal_ss(char** buf,
			 _CORBA_ULong slen, 
			 _CORBA_Boolean rel,
			 buf_t& s)
{
  for( _CORBA_ULong i = 0; i < slen; i++ ) {
    char*& p = (char*&) buf[i];

    if( p && rel) { FREE_BYTES(p); p = 0; }

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
_CORBA_Sequence__String::operator <<= (NetBufferedStream& s)
{
  _CORBA_ULong slen;
  slen <<= s;
  if (slen > s.RdMessageUnRead() || (pd_bounded && slen > pd_max)) {
    _CORBA_marshal_error();
    // never reach here
  }
  length(slen);
  unmarshal_ss(pd_data, slen, pd_rel, s);
}


void
_CORBA_Sequence__String::operator <<= (MemBufferedStream& s)
{
  _CORBA_ULong slen;
  slen <<= s;
  if (s.unRead() < slen || (pd_bounded && slen > pd_max)) {
    _CORBA_marshal_error();
    // never reach here
  }
  length(slen);
  unmarshal_ss(pd_data, slen, pd_rel, s);
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
