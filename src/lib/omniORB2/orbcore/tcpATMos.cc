// -*- Mode: C++; -*-
//                            Package   : omniORB2
// tcpATMos.cc                Created on: 18/3/96
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
//	

/*
  $Log$
  Revision 1.4  1998/08/14 13:53:25  sll
  Added pragma hdrstop to control pre-compile header if the compiler feature
  is available.

  Revision 1.3  1998/04/07 19:38:46  sll
  Replace cerr with omniORB::log.

  Revision 1.2  1997/12/12 18:45:49  sll
  Added call to print out the version of gatekeeper.

  Revision 1.1  1997/12/09 18:43:17  sll
  Initial revision

  */

#include <omniORB2/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <ropeFactory.h>
#include <tcpATMos.h>
#include <gatekeeper.h>

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


const char* tcpATMosEndpoint::protocol_name = "TCPIP";
tcpATMosFactoryType* tcpATMosFactoryType::singleton = 0;

void
tcpATMosFactoryType::init()
{
  if (singleton) return;
  singleton = new tcpATMosFactoryType;
  singleton->next = ropeFactoryTypeList;
  ropeFactoryTypeList = singleton;

  if (omniORB::traceLevel >= 2) {
    omniORB::log << "omniORB2 gateKeeper is " << gateKeeper::version()
		 << "\n";
    omniORB::log.flush();
  }

  // XXX There is no known order in the startup of ATMos processes. In
  // order to give the IP stack a chance to bootstrap to the
  // ready-to-service state, we put a delay here to stop this process from
  // proceeding for a while. As this function is guaranteed to be called
  // once at initialisation time, this seems to be a good point to insert
  // the delay.
  if (omniORB::traceLevel >= 10) {
    omniORB::log << "tcpATMosFactoryType::init(): delay process startup...\n";
    omniORB::log.flush();
  }
  omni_thread::sleep(15);
  if (omniORB::traceLevel >= 10) {
    omniORB::log << "tcpATMosFactoryType::init(): continue with process startup...\n";
    omniORB::log.flush();
  }
}

tcpATMosFactoryType::tcpATMosFactoryType()
{
}

CORBA::Boolean
tcpATMosFactoryType::is_IOPprofileId(IOP::ProfileId tag) const
{
  return (tag == IOP::TAG_INTERNET_IOP) ? 1 : 0;
}

CORBA::Boolean
tcpATMosFactoryType::is_protocol(const char* name) const
{
  return (strcmp(name,tcpATMosEndpoint::protocol_name) == 0) ? 1 : 0;
}

CORBA::Boolean
tcpATMosFactoryType::decodeIOPprofile(const IOP::TaggedProfile& profile,
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
    throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);

  CORBA::Boolean byteswap = ((profile.profile_data[begin] == 
			      omni::myByteOrder) ? 0 : 1);

  // profile.profile_data[1] - iiop_version.major
  // profile.profile_data[2] - iiop_version.minor
  begin = end;
  end = begin + 2;
  if (profile.profile_data.length() <= end)
    throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);
  if (profile.profile_data[begin]   != (CORBA::Octet)IIOP::current_major ||
      profile.profile_data[begin+1] != (CORBA::Octet)IIOP::current_minor)
    throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);

  // profile.profile_data[3] - padding
  // profile.profile_data[4] - profile.profile_data[7] host string length
  begin = end + 1;
  end = begin + 4;
  if (profile.profile_data.length() <= end)
    throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);
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
      throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);

    // Is this string null terminated?
    if (((char)profile.profile_data[end-1]) != '\0')
      throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);

    host = (CORBA::Char*)&profile.profile_data[begin];
  }
    
  // align to CORBA::UShort
  begin = (end + 1) & ~(1);
  // profile.profile_data[begin] port number
  end = begin + 2;
  if (profile.profile_data.length() <= end)
    throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);
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
  if (profile.profile_data.length() <= end)
    throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);
  {
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
      throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);

    // extract object key
    objkeysize = len;
  }
  objkey = new CORBA::Octet[objkeysize];
  memcpy((void *)objkey,(void *)&(profile.profile_data[begin]),objkeysize);
  addr = new tcpATMosEndpoint(host,port);
  return 1;
}

void
tcpATMosFactoryType::encodeIOPprofile(const Endpoint* addr,
				       const CORBA::Octet* objkey,
				       const size_t objkeysize,
				       IOP::TaggedProfile& profile) const
{
  tcpATMosEndpoint* tcpaddr = tcpATMosEndpoint::castup(addr);
  if (!tcpaddr)
    throw omniORB::fatalException(__FILE__,__LINE__,
				  "Endpoint is not tcpATMos");

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
  profile.profile_data[1] = IIOP::current_major;
  profile.profile_data[2] = IIOP::current_minor;
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
  idx += 4;
  memcpy((void *)&profile.profile_data[idx],
	 (void *)objkey,objkeysize);
}


tcpATMosEndpoint::tcpATMosEndpoint(CORBA::Char *h,CORBA::UShort p)
    : Endpoint((CORBA::Char *)tcpATMosEndpoint::protocol_name) 
{
  pd_host = 0;
  pd_port = 0;
  host(h);
  port(p);
}

tcpATMosEndpoint::tcpATMosEndpoint(const tcpATMosEndpoint *e)
    : Endpoint((CORBA::Char *)tcpATMosEndpoint::protocol_name) 
{
  pd_host = 0;
  pd_port = 0;
  host(e->host());
  port(e->port());
}

tcpATMosEndpoint&
tcpATMosEndpoint::operator=(const tcpATMosEndpoint &e) 
{
  host(e.host());
  port(e.port());
  return *this;
}
  
CORBA::Boolean
tcpATMosEndpoint::operator==(const tcpATMosEndpoint *e)
{
  if ((strcmp((const char *)pd_host,
	      (const char *)e->host())==0) && (pd_port == e->port()))
    return 1;
  else
    return 0;
}
 
tcpATMosEndpoint::~tcpATMosEndpoint()
{
  if (pd_host) delete [] pd_host;
}

CORBA::Char* 
tcpATMosEndpoint::host() const
{ 
  return pd_host;
}

void 
tcpATMosEndpoint::host(const CORBA::Char *p) 
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
tcpATMosEndpoint::port() const
{ 
  return pd_port;
}

void
tcpATMosEndpoint::port(const CORBA::UShort p) 
{ 
  pd_port = p;
}
  
tcpATMosEndpoint *
tcpATMosEndpoint::castup(const Endpoint *e) 
{
  if (e->is_protocol((CORBA::Char *)tcpATMosEndpoint::protocol_name)) {
    return (tcpATMosEndpoint *)e;
  }
  else {
    return 0;
  }
}

CORBA::Boolean
tcpATMosIncomingRope::this_is(Endpoint *&e)
{
  if (e) {
    tcpATMosEndpoint *te = tcpATMosEndpoint::castup(e);
    if (!te)
      return 0;
    if (*te == me)
      return 1;
    else
      return 0;
  }
  else {
    e = new tcpATMosEndpoint(me);
    if (!e)
      throw CORBA::NO_MEMORY(0,CORBA::COMPLETED_NO);
    return 1;
  }
}

CORBA::Boolean
tcpATMosOutgoingRope::remote_is(Endpoint *&e)
{
  if (e) {
    tcpATMosEndpoint *te = tcpATMosEndpoint::castup(e);
    if (!te)
      return 0;
    if (*te == remote)
      return 1;
    else
      return 0;
  }
  else {
    e = new tcpATMosEndpoint(remote);
    if (!e)
      throw CORBA::NO_MEMORY(0,CORBA::COMPLETED_NO);
    return 1;
  }
}


#undef Swap16
#undef Swap32
