// -*- Mode: C++; -*-
//                            Package   : omniORB
// tcpSocket.cc               Created on: 18/3/96
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
//	

/*
  $Log$
  Revision 1.8.6.4  1999/10/16 13:22:55  djr
  Changes to support compiling on MSVC.

  Revision 1.8.6.3  1999/10/14 16:22:17  djr
  Implemented logging when system exceptions are thrown.

  Revision 1.8.6.2  1999/09/24 15:01:38  djr
  Added module initialisers, and sll's new scavenger implementation.

  Revision 1.8.2.1  1999/09/21 20:37:18  sll
  -Simplified the scavenger code and the mechanism in which connections
   are shutdown. Now only one scavenger thread scans both incoming
   and outgoing connections. A separate thread do the actual shutdown.
  -omniORB::scanGranularity() now takes only one argument as there is
   only one scan period parameter instead of 2.
  -Trace messages in various modules have been updated to use the logger
   class.
  -ORBscanGranularity replaces -ORBscanOutgoingPeriod and
                                 -ORBscanIncomingPeriod.

  Revision 1.8  1999/08/23 08:35:59  sll
  Do not prepend the tcpSocketFactoryType singleton onto the
  ropeFactoryTypeList. This has been done by the ctor of ropeFactoryType.

  Revision 1.7  1999/03/11 16:25:56  djr
  Updated copyright notice

  Revision 1.6  1998/08/14 13:54:13  sll
  Added pragma hdrstop to control pre-compile header if the compiler feature
  is available.

  Revision 1.5  1998/08/13 16:10:07  sll
  Now can accept IOR with IIOP 1.1 profile. IIOP 1.1 specific part of
  the profile is ignored silently.

  Revision 1.4  1998/04/07 19:39:24  sll
  Replace cerr with omniORB::log.

  Revision 1.3  1998/03/13 17:34:16  sll
  Bug fix. No longer reject an IOR with zero length object key.

  Revision 1.2  1997/12/12 18:45:33  sll
  Added call to print out the version of gatekeeper.

  Revision 1.1  1997/12/09 18:43:12  sll
  Initial revision

  */

#include <omniORB3/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <ropeFactory.h>
#include <tcpSocket.h>
#include <gatekeeper.h>
#include <exception.h>


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


const char* tcpSocketEndpoint::protocol_name = "TCPIP";
tcpSocketFactoryType* tcpSocketFactoryType::singleton = 0;

void
tcpSocketFactoryType::init()
{
  if (singleton) return;
  singleton = new tcpSocketFactoryType;

#ifndef _MSC_VER
  //??
  if (omniORB::trace(2)) {
    omniORB::logger log;
    log << "gateKeeper is " << gateKeeper::version() << "\n";
  }
#endif
}

tcpSocketFactoryType::tcpSocketFactoryType()
{
}

CORBA::Boolean
tcpSocketFactoryType::is_IOPprofileId(IOP::ProfileId tag) const
{
  return (tag == IOP::TAG_INTERNET_IOP) ? 1 : 0;
}

CORBA::Boolean
tcpSocketFactoryType::is_protocol(const char* name) const
{
  return (strcmp(name,tcpSocketEndpoint::protocol_name) == 0) ? 1 : 0;
}

CORBA::Boolean
tcpSocketFactoryType::decodeIOPprofile(const IOP::TaggedProfile& profile,
				       Endpoint*&     addr,
				       CORBA::Octet*& objkey,
				       size_t&        objkeysize) const
{
  if (profile.tag != IOP::TAG_INTERNET_IOP)
    return 0;

  CORBA::Char*  host;
  CORBA::UShort port;
  CORBA::ULong begin = 0;
  CORBA::ULong end = 0;

  // profile.profile_data[0] - byteorder
  end += 1;
  if (profile.profile_data.length() <= end)
    OMNIORB_THROW(MARSHAL,0,CORBA::COMPLETED_NO);

  CORBA::Boolean byteswap = ((profile.profile_data[begin] == 
			      omni::myByteOrder) ? 0 : 1);

  // profile.profile_data[1] - iiop_version.major
  // profile.profile_data[2] - iiop_version.minor
  begin = end;
  end = begin + 2;
  if (profile.profile_data.length() <= end)
    OMNIORB_THROW(MARSHAL,0,CORBA::COMPLETED_NO);
  
  // iiop_version.major must be 1
  if (profile.profile_data[begin]   != 1)
    OMNIORB_THROW(MARSHAL,0,CORBA::COMPLETED_NO);
  
  // iiop_version.minor is either 0 or above.
  CORBA::Octet minor_version = profile.profile_data[begin+1];

  // profile.profile_data[3] - padding
  // profile.profile_data[4] - profile.profile_data[7] host string length
  begin = end + 1;
  end = begin + 4;
  if (profile.profile_data.length() <= end)
    OMNIORB_THROW(MARSHAL,0,CORBA::COMPLETED_NO);
  {
    CORBA::ULong len;
    if (!byteswap) {
      len = ((CORBA::ULong &) profile.profile_data[begin]);
    }
    else {
      CORBA::ULong t = ((CORBA::ULong &) profile.profile_data[begin]);
      len = Swap32(t);
    }

    // profile.profile_data[8] - profile.profile_data[8+len-1] host string
    begin = end;
    end = begin + len;
    if (profile.profile_data.length() <= end)
      OMNIORB_THROW(MARSHAL,0,CORBA::COMPLETED_NO);

    // Is this string null terminated?
    if (((char)profile.profile_data[end-1]) != '\0')
      OMNIORB_THROW(MARSHAL,0,CORBA::COMPLETED_NO);

    host = (CORBA::Char*)&profile.profile_data[begin];
  }
    
  // align to CORBA::UShort
  begin = (end + 1) & ~(1);
  // profile.profile_data[begin] port number
  end = begin + 2;
  if (profile.profile_data.length() <= end)
    OMNIORB_THROW(MARSHAL,0,CORBA::COMPLETED_NO);
  if (!byteswap) {
    port = ((CORBA::UShort &) profile.profile_data[begin]);
  }
  else {
    CORBA::UShort t = ((CORBA::UShort &) profile.profile_data[begin]);
    port = Swap16(t);
  }

  // align to CORBA::ULong
  begin = (end + 3) & ~(3);
  // profile.profile_data[begin]  object key length
  end = begin + 4;
  if (profile.profile_data.length() < end)
    OMNIORB_THROW(MARSHAL,0,CORBA::COMPLETED_NO);

  if (profile.profile_data.length() == end) {
    objkeysize = 0;
    objkey = new CORBA::Octet[1];
  }
  else {
    CORBA::ULong len;
    if (!byteswap) {
      len = ((CORBA::ULong &) profile.profile_data[begin]);
    }
    else {
      CORBA::ULong t = ((CORBA::ULong &) profile.profile_data[begin]);
      len = Swap32(t);
    }

    begin = end;
    end = begin + len;
    if (profile.profile_data.length() < end)
      OMNIORB_THROW(MARSHAL,0,CORBA::COMPLETED_NO);

    if (minor_version == 0) {
      // This profile is IIOP 1.0. The encapsulated profile must end exactly
      // at the end of the object key.
      if (profile.profile_data.length() != end) {
	OMNIORB_THROW(MARSHAL,0,CORBA::COMPLETED_NO);
      }
    }
    else {
      // This profile is IIOP 1.1 or above, any data in the profile that
      // occurs after the object key is silently ignored.
      // Nothing to do.
    }

    // extract object key
    objkeysize = len;
    objkey = new CORBA::Octet[objkeysize];
    memcpy((void *)objkey,(void *)&(profile.profile_data[begin]),objkeysize);
  }
  addr = new tcpSocketEndpoint(host,port);
  return 1;
}

void
tcpSocketFactoryType::encodeIOPprofile(const Endpoint* addr,
				       const CORBA::Octet* objkey,
				       const size_t objkeysize,
				       IOP::TaggedProfile& profile) const
{
  tcpSocketEndpoint* tcpaddr = tcpSocketEndpoint::castup(addr);
  if (!tcpaddr)
    throw omniORB::fatalException(__FILE__,__LINE__,
				  "Endpoint is not tcpSocket");

  profile.tag = IOP::TAG_INTERNET_IOP;

  CORBA::ULong hlen = strlen((const char *)tcpaddr->host()) + 1;
  {
    // calculate the total size of the encapsulated stream
    CORBA::ULong total = 8 + hlen;        // first 4 bytes + aligned host
    total = ((total + 1) & ~(1)) + 2;     // aligned port value
    total = ((total + 3) & ~(3)) + 4 +	// aligned object key
      objkeysize;
    
    profile.profile_data.length(total);
  }

  profile.profile_data[0] = omni::myByteOrder;
  profile.profile_data[1] = 1;       // IIOP major version no. = 1
  profile.profile_data[2] = 0;       // IIOP minor version no. = 0
  profile.profile_data[3] = 0;
  {
    CORBA::ULong &l = (CORBA::ULong &) profile.profile_data[4];
    l = hlen;
  }
  memcpy((void *)&(profile.profile_data[8]),(void *)tcpaddr->host(),hlen);
  CORBA::ULong idx = ((8 + hlen) + 1) & ~(1);
  {
    CORBA::UShort &l = (CORBA::UShort &) profile.profile_data[idx];
    l = tcpaddr->port();
  }
  idx = ((idx + 2) + 3) & ~(3);
  {
    CORBA::ULong &l = (CORBA::ULong &) profile.profile_data[idx];
    l = objkeysize;
  }
  if (objkeysize) {
    idx += 4;
    memcpy((void *)&profile.profile_data[idx],
	   (void *)objkey,objkeysize);
  }
}


tcpSocketEndpoint::tcpSocketEndpoint(const CORBA::Char *h,CORBA::UShort p)
    : Endpoint((CORBA::Char *)tcpSocketEndpoint::protocol_name) 
{
  pd_host = 0;
  pd_port = 0;
  host(h);
  port(p);
}

tcpSocketEndpoint::tcpSocketEndpoint(const tcpSocketEndpoint *e)
    : Endpoint((CORBA::Char *)tcpSocketEndpoint::protocol_name) 
{
  pd_host = 0;
  pd_port = 0;
  host(e->host());
  port(e->port());
}

tcpSocketEndpoint&
tcpSocketEndpoint::operator=(const tcpSocketEndpoint &e) 
{
  host(e.host());
  port(e.port());
  return *this;
}
  
CORBA::Boolean
tcpSocketEndpoint::operator==(const tcpSocketEndpoint *e)
{
  if ((strcmp((const char *)pd_host,
	      (const char *)e->host())==0) && (pd_port == e->port()))
    return 1;
  else
    return 0;
}
 
tcpSocketEndpoint::~tcpSocketEndpoint()
{
  if (pd_host) delete [] pd_host;
}

CORBA::Char* 
tcpSocketEndpoint::host() const
{ 
  return pd_host;
}

void 
tcpSocketEndpoint::host(const CORBA::Char *p) 
{
  if (pd_host) delete [] pd_host;
  if (p) {
    pd_host = new CORBA::Char [strlen((char *)p) + 1];
    strcpy((char *)pd_host,(char *)p);
  }
  else {
    pd_host = new CORBA::Char [1];
    pd_host[0] = '\0';
  }
  return;
}

CORBA::UShort
tcpSocketEndpoint::port() const
{ 
  return pd_port;
}

void
tcpSocketEndpoint::port(const CORBA::UShort p) 
{ 
  pd_port = p;
}
  
tcpSocketEndpoint *
tcpSocketEndpoint::castup(const Endpoint *e) 
{
  if (e->is_protocol((CORBA::Char *)tcpSocketEndpoint::protocol_name)) {
    return (tcpSocketEndpoint *)e;
  }
  else {
    return 0;
  }
}

CORBA::Boolean
tcpSocketIncomingRope::this_is(Endpoint *&e)
{
  if (e) {
    tcpSocketEndpoint *te = tcpSocketEndpoint::castup(e);
    if (!te)
      return 0;
    if (*te == me)
      return 1;
    else
      return 0;
  }
  else {
    e = new tcpSocketEndpoint(me);
    if (!e)
      OMNIORB_THROW(NO_MEMORY,0,CORBA::COMPLETED_NO);
    return 1;
  }
}

CORBA::Boolean
tcpSocketOutgoingRope::remote_is(Endpoint *&e)
{
  if (e) {
    tcpSocketEndpoint *te = tcpSocketEndpoint::castup(e);
    if (!te)
      return 0;
    if (*te == remote)
      return 1;
    else
      return 0;
  }
  else {
    e = new tcpSocketEndpoint(remote);
    if (!e)
      OMNIORB_THROW(NO_MEMORY,0,CORBA::COMPLETED_NO);
    return 1;
  }
}


#undef Swap16
#undef Swap32
