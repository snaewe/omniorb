// -*- Mode: C++; -*-
//                            Package   : omniORB
// corbaString.cc             Created on: 20/9/96
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2005-2006 Apasphere Ltd
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
  Revision 1.19.2.5  2006/06/06 16:39:37  dgrisby
  marshalRawString and chunking stream did not byte-swap length fields
  when required to.

  Revision 1.19.2.4  2006/05/22 15:44:51  dgrisby
  Make sure string length and body are never split across a chunk
  boundary.

  Revision 1.19.2.3  2006/04/28 18:40:46  dgrisby
  Merge from omni4_0_develop.

  Revision 1.19.2.2  2005/12/08 14:22:31  dgrisby
  Better string marshalling performance; other minor optimisations.

  Revision 1.19.2.1  2003/03/23 21:02:21  dgrisby
  Start of omniORB 4.1.x development branch.

  Revision 1.17.2.10  2002/01/02 18:15:42  dpg1
  Platform fixes/additions.

  Revision 1.17.2.9  2001/10/17 16:47:08  dpg1
  New minor codes

  Revision 1.17.2.8  2001/09/19 17:26:48  dpg1
  Full clean-up after orb->destroy().

  Revision 1.17.2.7  2001/08/03 17:41:19  sll
  System exception minor code overhaul. When a system exeception is raised,
  a meaning minor code is provided.

  Revision 1.17.2.6  2000/12/05 17:39:31  dpg1
  New cdrStream functions to marshal and unmarshal raw strings.

  Revision 1.17.2.5  2000/11/22 14:37:59  dpg1
  Code set marshalling functions now take a string length argument.

  Revision 1.17.2.4  2000/11/15 17:19:36  sll
  Added cdrStream::marshalString and unmarshalString.

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

OMNI_USING_NAMESPACE(omni)

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
  if (p) return _CORBA_String_helper::dup(p);
  return 0;
}

//////////////////////////////////////////////////////////////////////
///////////////////////// cdrStream raw string ///////////////////////
//////////////////////////////////////////////////////////////////////

char*
cdrStream::unmarshalRawString() {
  _CORBA_ULong len; len <<= *this;

  if (len == 0)
    OMNIORB_THROW(MARSHAL,MARSHAL_StringNotEndWithNull,
		  (CORBA::CompletionStatus)completion());

  if (!checkInputOverrun(1, len))
    OMNIORB_THROW(MARSHAL, MARSHAL_PassEndOfMessage,
		  (CORBA::CompletionStatus)completion());

  char* s = _CORBA_String_helper::alloc(len - 1);
  get_octet_array((_CORBA_Octet*)s, len);

  if (s[len-1] != '\0')
    OMNIORB_THROW(MARSHAL,MARSHAL_StringNotEndWithNull,
		  (CORBA::CompletionStatus)completion());

  return s;
}

#ifndef Swap32
#define Swap32(l) ((((l) & 0xff000000) >> 24) | \
		   (((l) & 0x00ff0000) >> 8)  | \
		   (((l) & 0x0000ff00) << 8)  | \
		   (((l) & 0x000000ff) << 24))
#else
#error "Swap32 has already been defined"
#endif

_CORBA_ULong
cdrStream::marshalRawString(const char* s)
{
  // In the common case that the string fits inside the stream's
  // buffer, we make a single pass over the string. If the string does
  // not fit inside the bufer, we have to find the length of the
  // remaining portion before marshalling it, meaning some of the
  // string is passed over twice.

  char* current = (char*)omni::align_to((omni::ptr_arith_t)pd_outb_mkr,
					omni::ALIGN_4);
  char* limit   = (char*)pd_outb_end;

  _CORBA_ULong* lenp = (_CORBA_ULong*)current;
  current += 4;
  _CORBA_ULong len;

  if (current >= limit) {
    // No characters from the string will fit in the current buffer.
    // The length might fit, but we can't put it in this buffer since
    // in a chunked stream it must stay with the string contents.
    len = strlen(s) + 1;
    declareArrayLength(omni::ALIGN_4, len+4);
    len >>= *this;
    put_octet_array((const _CORBA_Octet*)s, len);
    return len;
  }

  pd_outb_mkr = (void*)current;

  // At least some of the string fits in the current buffer.
  while(*s && current < limit) {
    *current++ = *s++;
  }
  if (current < limit) {
    // Good -- the whole string fit in the buffer.
    *current++ = *s;

    len = (omni::ptr_arith_t)current - (omni::ptr_arith_t)pd_outb_mkr;
    pd_outb_mkr = (void*)current;

    *lenp = pd_marshal_byte_swap ? Swap32(len) : len;
  }
  else {
    // Some of the string did not fit.
    len = (omni::ptr_arith_t)current - (omni::ptr_arith_t)pd_outb_mkr;
    pd_outb_mkr = (void*)current;
    _CORBA_ULong rest = strlen(s) + 1;

    len += rest;
    if ((omni::ptr_arith_t)lenp < (omni::ptr_arith_t)pd_outb_end)
      *lenp = pd_marshal_byte_swap ? Swap32(len) : len;

    put_octet_array((const _CORBA_Octet*)s, rest);
  }
  return len;
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

  if (!s.checkInputOverrun(1,slen))
    OMNIORB_THROW(MARSHAL, MARSHAL_PassEndOfMessage,
		  (CORBA::CompletionStatus)s.completion());

  if (pd_bounded && slen > pd_max)
    OMNIORB_THROW(MARSHAL,MARSHAL_SequenceIsTooLong,
		  (CORBA::CompletionStatus)s.completion());

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

#if 0
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void
_CORBA_String_helper::
unmarshal_zero_length_string()
{
  if (orbParameters::strictIIOP) {
    if (omniORB::trace(1)) {
      omniORB::log << "Error: received an invalid zero length string.\n"
		   << "       CORBA::MARSHAL is thrown.\n";
      omniORB::log.flush();
    }
    OMNIORB_THROW(MARSHAL, MARSHAL_StringNotEndWithNull, CORBA::COMPLETED_NO);
  }
  else {
    if (omniORB::trace(1)) {
      omniORB::log << "Warning: received an invalid zero length string.\n"
		   << "         Substituted with a proper empty string.\n";
      omniORB::log.flush();
    }
  }
}
#endif
