// -*- Mode: C++; -*-
//                            Package   : omniORB2
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
  Revision 1.9.4.1  1999/09/15 20:18:31  sll
  Updated to use the new cdrStream abstraction.
  Marshalling operators for NetBufferedStream and MemBufferedStream are now
  replaced with just one version for cdrStream.
  Derived class giopStream implements the cdrStream abstraction over a
  network connection whereas the cdrMemoryStream implements the abstraction
  with in memory buffer.

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

#include <omniORB2/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

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


char*
IOP::iorToEncapStr(const char *type_id,
		   const IOP::TaggedProfileList *profiles)
{
  CORBA::ULong l = strlen(type_id) + 1;

  cdrCountingStream count;
  omni::myByteOrder >>= count;
  l >>= count;
  count.put_char_array((CORBA::Char*)type_id,l);
  *profiles >>= count;

  cdrMemoryStream buf(count.total(),1);

  // create an encapsulation
  omni::myByteOrder >>= buf;
  l >>= buf;
  buf.put_char_array((CORBA::Char*)type_id,l);
  *profiles >>= buf;

  // turn the encapsulation into a hex string with "IOR:" prepended
  buf.rewindInputPtr();
  size_t s = buf.bufSize();
  CORBA::Char * data = (CORBA::Char *)buf.bufPtr();

  char *result = new char[4+s*2+1];
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
  return result;
}

void
IOP::EncapStrToIor(const char *str,
		   char*& type_id,
		   IOP::TaggedProfileList *&profiles)
{
  size_t s = (str ? strlen(str) : 0);
  if (s<4)
    throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);
  const char *p = str;
  if (p[0] != 'I' ||
      p[1] != 'O' ||
      p[2] != 'R' ||
      p[3] != ':')
    throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);

  s = (s-4)/2;  // how many octets are there in the string
  p += 4;

  cdrMemoryStream buf((int)s);
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
      throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);

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
      throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);
    v >>= buf;
  }

  buf.rewindInputPtr();
  CORBA::Boolean b;
  b <<= buf;
  buf.setByteSwapFlag(b);

  CORBA::String_var repoID;
  IOP::TaggedProfileList_var profileList;

  CORBA::ULong l;
  l <<= buf;
  if (!buf.checkInputOverrun(1,l))
    throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);

  switch (l) {

  case 0:
#ifdef NO_SLOPPY_NIL_REFERENCE
    throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);
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
    repoID = CORBA::string_alloc(1);
    repoID[0] = '\0';
#endif	
    break;

  case 1:
    repoID = CORBA::string_alloc(1);
    buf.get_char_array((CORBA::Char*)((const char*)repoID),1);
    if (repoID[0] != '\0')
      throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);
    break;

  default:
    repoID = CORBA::string_alloc(l);
    buf.get_char_array((CORBA::Char*)((const char*)repoID),l);
    if( repoID[l - 1] != '\0' )
      throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
  }

  profileList = new IOP::TaggedProfileList;
  (IOP::TaggedProfileList&)profileList <<= buf;

  type_id = repoID._retn();
  profiles = profileList._retn();
  return;
}

void
IOP::TaggedProfile::operator>>= (cdrStream &s) {
    tag >>= s;
    profile_data >>= s;
}

void
IOP::TaggedProfile::operator<<= (cdrStream &s) {
  tag <<= s;
  profile_data <<= s;
}

void
IOP::TaggedComponent::operator>>= (cdrStream& s) {
  tag >>= s;
  component_data >>= s;
}

void
IOP::TaggedComponent::operator<<= (cdrStream& s) {
  tag <<= s;
  component_data <<= s;
}


#undef Swap16
#undef Swap32


