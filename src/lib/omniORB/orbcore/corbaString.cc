// -*- Mode: C++; -*-
//                            Package   : omniORB
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
  Revision 1.17.2.3  2000/11/03 18:46:19  sll
  Moved string marshal functions into cdrStream.

  Revision 1.17.2.2  2000/09/27 18:02:21  sll
  Updated to use the new cdrStream abstraction.

  Revision 1.17.2.1  2000/07/17 10:35:52  sll
  Merged from omni3_develop the diff between omni3_0_0_pre3 and omni3_0_0.

  Revision 1.18  2000/07/13 15:25:58  dpg1
  Merge from omni3_develop for 3.0 release.

  Revision 1.14.6.6  2000/06/27 16:15:10  sll
  New classes: _CORBA_String_element, _CORBA_ObjRef_Element,
  _CORBA_ObjRef_tcDesc_arg to support assignment to an element of a
  sequence of string and a sequence of object reference.

  Revision 1.14.6.5  2000/06/22 10:40:14  dpg1
  exception.h renamed to exceptiondefs.h to avoid name clash on some
  platforms.

  Revision 1.14.6.4  2000/02/09 15:01:29  djr
  Fixed _CORBA_String_member bug.

  Revision 1.14.6.3  2000/01/31 11:12:34  djr
  Fixed bug in unmarshalling of String_member.

  Revision 1.14.6.2  1999/10/14 16:22:07  djr
  Implemented logging when system exceptions are thrown.

  Revision 1.14.6.1  1999/09/22 14:26:47  djr
  Major rewrite of orbcore to support POA.

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

#include <omniORB4/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <exceptiondefs.h>

const char*const _CORBA_String_helper::empty_string = "";


char*
CORBA::string_alloc(CORBA::ULong len)
{
  // We initialise the string to zero length to help prevent errors
  // if this string is copied before it is initialised.  This is easy
  // to do when assigning the returned value to a CORBA::String_var.
  char* s = _CORBA_String_helper::alloc(len);
  if( s )  *s = '\0';
  return s;
}


void
CORBA::string_free(char* p)
{
  _CORBA_String_helper::free(p);
}


char*
CORBA::string_dup(const char* p)
{
  if (p) {
    char* q = _CORBA_String_helper::alloc(strlen(p));
    if (q) {
      strcpy(q,p);
      return q;
    }
  }
  return 0;
}

void
cdrStream::marshalString(const char* p, int bounded)
{
  if( p ) {
    CORBA::ULong len = strlen((char*)p) + 1;
    if (bounded && len > (CORBA::ULong)(bounded+1))
      OMNIORB_THROW(MARSHAL,0, CORBA::COMPLETED_MAYBE);
    len >>= *this;
    put_octet_array((CORBA::Octet*)p, len);
  }
  else {
    if (omniORB::traceLevel > 1) {
      _CORBA_null_string_ptr(0);
    }
    CORBA::ULong(1) >>= *this;
    marshalChar('\0');
  }
}

char*
cdrStream::unmarshalString(int bounded)
{
  CORBA::String_var p;

  CORBA::ULong len;
  len <<= *this;
  if( !len && omniORB::traceLevel > 1 )  _CORBA_null_string_ptr(1);

  CORBA::ULong nbytes = len ? len : 1;

  if (!checkInputOverrun(1,len) || (bounded && len > (CORBA::ULong)bounded)) {
    OMNIORB_THROW(MARSHAL,0, CORBA::COMPLETED_MAYBE);
  }

  p = _CORBA_String_helper::alloc(nbytes - 1);
  if( !(char*)p )  OMNIORB_THROW(NO_MEMORY,0, CORBA::COMPLETED_MAYBE);

  if( len ) {
    get_octet_array((CORBA::Octet*)((char*)p), len);
    if( p[len - 1] != '\0' )
      OMNIORB_THROW(MARSHAL,0, CORBA::COMPLETED_MAYBE);
  }
  else
    p[0] = '\0';

  return p._retn();
}

//////////////////////////////////////////////////////////////////////
//////////////////////////// String_member ///////////////////////////
//////////////////////////////////////////////////////////////////////

void
_CORBA_String_member::operator <<= (cdrStream& s)
{
  if( _ptr && _ptr != _CORBA_String_helper::empty_string )
    _CORBA_String_helper::free(_ptr);
  _ptr = 0;

  _ptr = s.unmarshalString();
}

void
_CORBA_String_member::operator >>= (cdrStream& s) const { 
  s.marshalString(_ptr); 
}

//////////////////////////////////////////////////////////////////////
////////////////// _CORBA_Unbounded_Sequence__String /////////////////
//////////////////////////////////////////////////////////////////////

void
_CORBA_Sequence_String::operator >>= (cdrStream& s) const
{
  pd_len >>= s;

  for( CORBA::ULong i = 0; i < pd_len; i++ ) {
    char* p = pd_data[i];
    s.marshalString(p);
  }
}


void
_CORBA_Sequence_String::operator <<= (cdrStream& s)
{
  _CORBA_ULong slen;
  slen <<= s;

  if (!s.checkInputOverrun(1,slen) || (pd_bounded && slen > pd_max)) {
    OMNIORB_THROW(MARSHAL,0, CORBA::COMPLETED_MAYBE);
  }

  if (!pd_rel && slen <= pd_max) {
    // obtain ownership of the array and its elements (note that this isn't
    // the most effecient solution, but neither is invoking length!)
    copybuffer(pd_len);
  }

  length(slen);

  for( _CORBA_ULong i = 0; i < slen; i++ ) {
    char*& p = (char*&) pd_data[i];

    if( p ) { _CORBA_String_helper::free(p); p = 0; }

    p = s.unmarshalString();
  }
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
