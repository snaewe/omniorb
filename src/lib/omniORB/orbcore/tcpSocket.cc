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
  Revision 1.11.2.2  2000/09/27 18:36:51  sll
  Rewritten encodeIOPprofile() and decodeIOPprofile().

  Revision 1.11.2.1  2000/07/17 10:35:59  sll
  Merged from omni3_develop the diff between omni3_0_0_pre3 and omni3_0_0.

  Revision 1.12  2000/07/13 15:25:55  dpg1
  Merge from omni3_develop for 3.0 release.

  Revision 1.8.6.5  2000/06/22 10:40:17  dpg1
  exception.h renamed to exceptiondefs.h to avoid name clash on some
  platforms.

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

#include <omniORB4/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <giopStreamImpl.h>
#include <ropeFactory.h>
#include <tcpSocket.h>
#include <gatekeeper.h>
#include <exceptiondefs.h>
#include <omniORB4/omniInterceptors.h>
#include <objectAdapter.h>

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
tcpSocketFactoryType::decodeIOPprofile(omniIOR* ior,
				       CORBA::Boolean selected_profile)
{
  
  if (!selected_profile) {
    CORBA::ULong total = ior->iopProfiles->length();
    CORBA::ULong index = 0;
    for ( ; index < total; index++) {
      if (ior->iopProfiles[index].tag == IOP::TAG_INTERNET_IOP)
	break;
    }
    if (index == total) return 0;
    ior->addr_selected_profile_index = index;
  }

  IIOP::decodeProfile(ior->iopProfiles[ior->addr_selected_profile_index],
		      ior->iiop);

  // If there is any multiple component profile, decode its content as well
  // because these tagged components apply to all IIOP profiles, this one
  // included.
  {
    CORBA::ULong total = ior->iopProfiles->length();
    CORBA::ULong index = 0;
    for ( ; index < total; index++) {
      if (ior->iopProfiles[index].tag == IOP::TAG_MULTIPLE_COMPONENTS)
	IIOP::decodeMultiComponentProfile(ior->iopProfiles[index],ior->iiop);
    }
  }
  
  // Call interceptors
  omniORB::getInterceptors()->decodeIOR.visit(ior);

  if (!ior->selectedRopeFactoryType)
    ior->selectedRopeFactoryType = this;

  return 1;
}


CORBA::Boolean
tcpSocketFactoryType::encodeIOPprofile(omniIOR* ior)
{
  if (!strlen(ior->iiop.address.host)) {
    // No endpoint address has been set. Fill up the address field
    // with the incoming addresses.

    ropeFactory_iterator iter(omniObjAdapter::incomingRopeFactories());
    incomingRopeFactory* rp;
    while( (rp = (incomingRopeFactory*) iter()) ) {
      if (rp->getType() == this) {

	const tcpSocketMTincomingFactory::Endpoints& endpts = 
	  ((tcpSocketMTincomingFactory*)rp)->getEndpoints();

	ior->iiop.address = endpts[0]->address();
	if (endpts.length() > 1) {
	  for (CORBA::ULong index = 1; index < endpts.length(); index++) {
	    IIOP::addAlternativeIIOPAddress(ior->iiop.components, 
					    endpts[index]->address());
	  }
	}
	break;
      }
    }
  }
  ior->iiop.version = giopStreamImpl::maxVersion()->version();


  // Call interceptors
  omniORB::getInterceptors()->encodeIOR.visit(ior);

  if (!ior->iopProfiles.operator->()) {
    ior->iopProfiles = new IOP::TaggedProfileList();
  }
  CORBA::ULong last = ior->iopProfiles->length();
  ior->iopProfiles->length(last+1);
  IIOP::encodeProfile(ior->iiop,ior->iopProfiles[last]);

  return 1;
}

tcpSocketEndpoint::tcpSocketEndpoint(const char* h,CORBA::UShort p)
    : Endpoint((CORBA::Char *)tcpSocketEndpoint::protocol_name) 
{
  host(h);
  port(p);
}

tcpSocketEndpoint::tcpSocketEndpoint(const IIOP::Address& addr)
    : Endpoint((CORBA::Char *)tcpSocketEndpoint::protocol_name) 
{
  pd_address = addr;
}

tcpSocketEndpoint::tcpSocketEndpoint(const tcpSocketEndpoint *e)
    : Endpoint((CORBA::Char *)tcpSocketEndpoint::protocol_name) 
{
  host(e->host());
  port(e->port());
}

tcpSocketEndpoint&
tcpSocketEndpoint::operator=(const tcpSocketEndpoint &e) 
{
  if (&e == this) return *this;
  host(e.host());
  port(e.port());
  return *this;
}
  
CORBA::Boolean
tcpSocketEndpoint::operator==(const tcpSocketEndpoint *e)
{
  if (e == this) return 1;

  if ((strcmp((const char *)host(),(const char *)e->host())==0) && 
      (port() == e->port()))
    return 1;
  else
    return 0;
}

CORBA::Boolean 
tcpSocketEndpoint::is_equal(const IIOP::Address& addr) const
{
  if ((strcmp((const char *)host(),(const char *)addr.host)==0) && 
      (port() == addr.port))
    return 1;
  else
    return 0;

}

tcpSocketEndpoint::~tcpSocketEndpoint()
{
}

const char* 
tcpSocketEndpoint::host() const
{ 
  return pd_address.host;
}

void 
tcpSocketEndpoint::host(const char* p) 
{
  pd_address.host = p;
}

CORBA::UShort
tcpSocketEndpoint::port() const
{ 
  return pd_address.port;
}

void
tcpSocketEndpoint::port(const CORBA::UShort p) 
{ 
  pd_address.port = p;
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
tcpSocketIncomingRope::this_is(const IIOP::Address& addr) const
{
  return me->is_equal(addr);
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

CORBA::Boolean
tcpSocketOutgoingRope::remote_is(const IIOP::Address& addr) const
{
  return remote->is_equal(addr);
}
