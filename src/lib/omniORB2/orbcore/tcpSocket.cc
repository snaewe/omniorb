// -*- Mode: C++; -*-
//                            Package   : omniORB2
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
  Revision 1.8.4.1  1999/09/15 20:18:28  sll
  Updated to use the new cdrStream abstraction.
  Marshalling operators for NetBufferedStream and MemBufferedStream are now
  replaced with just one version for cdrStream.
  Derived class giopStream implements the cdrStream abstraction over a
  network connection whereas the cdrMemoryStream implements the abstraction
  with in memory buffer.

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

#include <omniORB2/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <giopStreamImpl.h>
#include <giopObjectInfo.h>
#include <ropeFactory.h>
#include <tcpSocket.h>
#include <gatekeeper.h>

const char* tcpSocketEndpoint::protocol_name = "TCPIP";
tcpSocketFactoryType* tcpSocketFactoryType::singleton = 0;

void
tcpSocketFactoryType::init()
{
  if (singleton) return;
  singleton = new tcpSocketFactoryType;

  if (omniORB::traceLevel >= 2) {
    omniORB::log << "omniORB2 gateKeeper is " << gateKeeper::version() 
		 << "\n";
    omniORB::log.flush();
  }
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


#if 0
CORBA::Boolean
tcpSocketFactoryType::decodeIOPprofile(const IOP::TaggedProfile& profile,
					  // return values:
					  Endpoint*&     addr,
					  CORBA::Octet*& objkey,
					  size_t&        objkeysize,
				       GIOPObjectInfo_var& objectInfo) const
#endif
CORBA::Boolean
tcpSocketFactoryType::decodeIOPprofile(const IOP::TaggedProfile& profile,
				       // return values:
				       Endpoint*&     addr,
				       GIOPObjectInfo* objectInfo) const

{
  if (profile.tag != IOP::TAG_INTERNET_IOP)
    return 0;

  cdrEncapsulationStream s(profile.profile_data.get_buffer(),
			   profile.profile_data.length(),
			   1); /* No 8 bytes align data member, so
				  4 bytes align is sufficent. */

  IIOP::ProfileBody body;

  body.version.major <<= s;
  body.version.minor <<= s;
  
  if (body.version.major != 1) 
    throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);

  body.host <<= s;

  body.port <<= s;

  body.object_key <<= s;

  switch (body.version.minor) {
  case 0:
    // Check if the profile body ends here.
    if (s.checkInputOverrun(1,1))
      throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);
    break;
  default:
    body.components <<= s;
  }

  // XXX Examine all the profile components and extract the component
  //     information we are interested in.

  objectInfo->version_.major = body.version.major;
  objectInfo->version_.minor = body.version.minor;
  CORBA::ULong max = body.object_key.maximum();
  CORBA::ULong len = body.object_key.length();
  objectInfo->object_key_.replace(max,len,body.object_key.get_buffer(1),1);
  addr = new tcpSocketEndpoint((CORBA::Char*)body.host._ptr,body.port);
  body.host._ptr = 0;

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

  CORBA::ULong hlen = strlen((const char *)tcpaddr->host()) + 1;
  profile.tag = IOP::TAG_INTERNET_IOP;

  GIOP::Version ver = giopStreamImpl::maxVersion()->version();

  CORBA::ULong bufsize;
  {
    cdrCountingStream s;
    omni::myByteOrder >>= s;
    ver.major >>= s;
    ver.minor >>= s;
    hlen >>= s;
    s.put_char_array((CORBA::Char*)tcpaddr->host(),hlen);
    tcpaddr->port() >>= s;
    ::operator>>=((CORBA::ULong) objkeysize,s);
    s.put_char_array((CORBA::Char*)objkey,objkeysize);

    if (ver.minor > 0) {
      // XXX extra component puts here.
      ::operator>>=((CORBA::ULong)0,s);
    }
    bufsize = s.total();
  }

  {
    cdrEncapsulationStream s(bufsize,1);
    ver.major >>= s;
    ver.minor >>= s;
    hlen >>= s;
    s.put_char_array((CORBA::Char*)tcpaddr->host(),hlen);
    tcpaddr->port() >>= s;
    ::operator>>=((CORBA::ULong) objkeysize,s);
    s.put_char_array((CORBA::Char*)objkey,objkeysize);

    if (ver.minor > 0) {
      // XXX extra component puts here.
      ::operator>>=((CORBA::ULong)0,s);
    }

    CORBA::Octet* p;
    CORBA::ULong max;
    CORBA::ULong len;
    s.getOctetStream(p,max,len);
    profile.profile_data.replace(max,len,p,1);
  }
}


tcpSocketEndpoint::tcpSocketEndpoint(CORBA::Char *h,CORBA::UShort p)
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
      throw CORBA::NO_MEMORY(0,CORBA::COMPLETED_NO);
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
      throw CORBA::NO_MEMORY(0,CORBA::COMPLETED_NO);
    return 1;
  }
}


#undef Swap16
#undef Swap32
