// -*- Mode: C++; -*-
//                            Package   : omniORB
// ior.cc                     Created on: 5/7/96
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
//	*** PROPRIETORY INTERFACE ***
//      
 
/*
  $Log$
  Revision 1.11  2000/07/13 15:25:57  dpg1
  Merge from omni3_develop for 3.0 release.

  Revision 1.9.6.5  2000/06/22 10:40:15  dpg1
  exception.h renamed to exceptiondefs.h to avoid name clash on some
  platforms.

  Revision 1.9.6.4  2000/04/27 10:50:30  dpg1
  Interoperable Naming Service

  IOR: prefix is not case sensitive.

  Revision 1.9.6.3  1999/10/14 16:22:11  djr
  Implemented logging when system exceptions are thrown.

  Revision 1.9.6.2  1999/09/27 08:48:32  djr
  Minor corrections to get rid of warnings.

  Revision 1.9.6.1  1999/09/22 14:26:51  djr
  Major rewrite of orbcore to support POA.

  Revision 1.9  1999/05/25 17:06:14  sll
  Make sure all padding bytes are converted to 0s in the stringified IOR.

  Revision 1.8  1999/03/11 16:25:53  djr
  Updated copyright notice

  Revision 1.7  1998/08/14 13:48:04  sll
  Added pragma hdrstop to control pre-compile header if the compiler feature
  is available.

  Revision 1.6  1997/12/09 17:32:39  sll
  Removed obsoluted functions IIOP::profileToEncapStream and
  IIOP::EncapStreamToProfile.
  IOP::EncapStrToIor now accepts dodgy nil object reference.

  Revision 1.5  1997/08/21 22:04:23  sll
  minor cleanup to get rid of purify's warnings.

// Revision 1.4  1997/05/06  15:21:58  sll
// Public release.
//
  */

#include <omniORB3/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <exceptiondefs.h>


#ifndef Swap16
#define Swap16(s) ((((s) & 0xff) << 8) | (((s) >> 8) & 0xff))
#else
#error "Swap16 has already been defined"
#endif

#ifndef Swap32
#define Swap32(l) ((((l) & 0xff000000) >> 24) | \
		   (((l) & 0x00ff0000) >> 8)  | \
		   (((l) & 0x0000ff00) << 8)  | \
		   (((l) & 0x000000ff) << 24))
#else
#error "Swap32 has already been defined"
#endif


CORBA::Char*
IOP::iorToEncapStr(const CORBA::Char* type_id,
		   const IOP::TaggedProfileList* profiles)
{
  OMNIORB_ASSERT(type_id);
  OMNIORB_ASSERT(profiles);

  MemBufferedStream buf;

  CORBA::ULong l = strlen((const char *)type_id) + 1;

  // lets make an effort to ensure that all the padding bytes are zero'ed.
  {
    size_t bufsize = 8 + l;
    bufsize = profiles->_NP_alignedSize(bufsize);
    CORBA::Char dummy = 0;
    for (size_t i=0; i < bufsize; i++) dummy >>= buf;
    buf.rewind_inout_mkr();
  }

  // create an encapsulation
  omni::myByteOrder >>= buf;
  l >>= buf;
  buf.put_char_array(type_id,l);
  *profiles >>= buf;

  // turn the encapsulation into a hex string with "IOR:" prepended
  buf.rewind_in_mkr();
  size_t s = buf.unRead();
  CORBA::Char * data = (CORBA::Char *)buf.data();

  char *result = new char[4+s*2+1];
  if (!result)
    OMNIORB_THROW(NO_MEMORY,0,CORBA::COMPLETED_NO);
  result[4+s*2] = '\0';
  result[0] = 'I';
  result[1] = 'O';
  result[2] = 'R';
  result[3] = ':';
  for (int i=0; i < (int)s; i++) {
    int j = 4 + i*2;
    int v = (data[i] & 0xf0);
    v = v >> 4;
    if (v < 10)
      result[j] = '0' + v;
    else
      result[j] = 'a' + (v - 10);
    v = ((data[i] & 0xf));
    if (v < 10)
      result[j+1] = '0' + v;
    else
      result[j+1] = 'a' + (v - 10);
  }
  return (CORBA::Char*) result;
}


void
IOP::EncapStrToIor(const CORBA::Char* str,
		   CORBA::Char*& type_id,
		   IOP::TaggedProfileList*& profiles)
{
  size_t s = (str ? strlen((const char *)str) : 0);
  if (s<4)
    OMNIORB_THROW(MARSHAL,0,CORBA::COMPLETED_NO);
  const char *p = (const char *) str;
  if ((p[0] != 'I' && p[0] != 'i') ||
      (p[1] != 'O' && p[1] != 'o') ||
      (p[2] != 'R' && p[2] != 'r') ||
      (p[3] != ':'))
    OMNIORB_THROW(MARSHAL,0,CORBA::COMPLETED_NO);

  s = (s-4)/2;  // how many octets are there in the string
  p += 4;

  MemBufferedStream buf((int)s);
  for (int i=0; i<(int)s; i++) {
    int j = i*2;
    CORBA::Octet v;

    if (p[j] >= '0' && p[j] <= '9') {
      v = ((p[j] - '0') << 4);
    }
    else if (p[j] >= 'a' && p[j] <= 'f') {
      v = ((p[j] - 'a' + 10) << 4);
    }
    else if (p[j] >= 'A' && p[j] <= 'F') {
      v = ((p[j] - 'A' + 10) << 4);
    }
    else
      OMNIORB_THROW(MARSHAL,0,CORBA::COMPLETED_NO);

    if (p[j+1] >= '0' && p[j+1] <= '9') {
      v += (p[j+1] - '0');
    }
    else if (p[j+1] >= 'a' && p[j+1] <= 'f') {
      v += (p[j+1] - 'a' + 10);
    }
    else if (p[j+1] >= 'A' && p[j+1] <= 'F') {
      v += (p[j+1] - 'A' + 10);
    }
    else
      OMNIORB_THROW(MARSHAL,0,CORBA::COMPLETED_NO);
    v >>= buf;
  }

  buf.rewind_in_mkr();
  CORBA::Boolean b;
  b <<= buf;
  buf.byteOrder(b);

  type_id = 0;
  profiles = 0;
  try {
    CORBA::ULong l;
    l <<= buf;
    if (l > buf.unRead())
      OMNIORB_THROW(MARSHAL,0,CORBA::COMPLETED_NO);

    switch (l) {

    case 0:
#ifdef NO_SLOPPY_NIL_REFERENCE
      OMNIORB_THROW(MARSHAL,0,CORBA::COMPLETED_NO);
#else
      // According to the CORBA specification 2.0 section 10.6.2:
      //   Null object references are indicated by an empty set of
      //   profiles, and by a NULL type ID (a string which contain
      //   only *** a single terminating character ***).
      //
      // Therefore the idlen should be 1.
      // Visibroker for C++ (Orbeline) 2.0 Release 1.51 gets it wrong
      // and sends out a 0 len string.
      // We quietly accept it here. Turn this off by defining
      //   NO_SLOPPY_NIL_REFERENCE
      type_id = new CORBA::Char[1];
      type_id[0] = (CORBA::Char)'\0';
#endif
      break;

    case 1:
      type_id = new CORBA::Char[1];
      buf.get_char_array(type_id,1);
      if (type_id[0] != (CORBA::Char)'\0')
	OMNIORB_THROW(MARSHAL,0,CORBA::COMPLETED_NO);
      break;

    default:
      type_id = new CORBA::Char[l];
      if (!type_id)
	OMNIORB_THROW(NO_MEMORY,0,CORBA::COMPLETED_NO);
      buf.get_char_array(type_id,l);
    }

    profiles = new IOP::TaggedProfileList;
    if (!profiles)
      OMNIORB_THROW(NO_MEMORY,0,CORBA::COMPLETED_NO);
    *profiles <<= buf;
  }
  catch (...) {
    if (type_id) delete [] type_id;
    if (profiles) delete profiles;
    throw;
  }
  return;
}


void
IOP::TaggedProfile::operator>>= (NetBufferedStream &s) {
    tag >>= s;
    profile_data >>= s;
}


void
IOP::TaggedProfile::operator<<= (NetBufferedStream &s) {
  tag <<= s;
  profile_data <<= s;
}


void
IOP::TaggedProfile::operator>>= (MemBufferedStream &s) {
    tag >>= s;
    profile_data >>= s;
}


void
IOP::TaggedProfile::operator<<= (MemBufferedStream &s) {
  tag <<= s;
  profile_data <<= s;
}


#undef Swap16
#undef Swap32

#if defined(__GNUG__)

// The following template classes are defined before the template functions
// inline void _CORBA_Sequence<T>::operator<<= (NetBufferedStream &s) etc
// are defined.
// G++ (2.7.2 or may be later versions as well) does not compile in the
// template functions as a result.
// The following is a workaround which explicitly instantiate the classes
// again.

template class _CORBA_Sequence<IOP::TaggedProfile>;
template class _CORBA_Unbounded_Sequence<IOP::TaggedProfile>;

#endif
