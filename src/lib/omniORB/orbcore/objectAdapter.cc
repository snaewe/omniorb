// -*- Mode: C++; -*-
//                            Package   : omniORB
// objectAdapter.cc           Created on: 5/3/99
//                            Author    : David Riddoch (djr)
//
//    Copyright (C) 1996, 1999 AT&T Research Cambridge
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
 Revision 1.2.2.4.2.1  2001/02/23 16:50:36  sll
 SLL work in progress.

 Revision 1.2.2.4  2000/11/09 12:27:57  dpg1
 Huge merge from omni3_develop, plus full long long from omni3_1_develop.

 Revision 1.2.2.3  2000/10/03 17:39:46  sll
 DefaultLoopback now works.

 Revision 1.2.2.2  2000/09/27 18:17:19  sll
 Use the new omniIOR class in defaultLoopBack().

 Revision 1.2.2.1  2000/07/17 10:35:55  sll
 Merged from omni3_develop the diff between omni3_0_0_pre3 and omni3_0_0.

 Revision 1.3  2000/07/13 15:25:57  dpg1
 Merge from omni3_develop for 3.0 release.

 Revision 1.1.2.8  2000/06/22 10:40:15  dpg1
 exception.h renamed to exceptiondefs.h to avoid name clash on some
 platforms.

 Revision 1.1.2.7  2000/04/27 10:50:49  dpg1
 Interoperable Naming Service

 Include initRefs.h instead of bootstrap_i.h.

 Revision 1.1.2.6  2000/03/03 09:44:04  djr
 Fix to prevent tracedmutex assertion failure.

 Revision 1.1.2.5  1999/10/27 17:32:12  djr
 omni::internalLock and objref_rc_lock are now pointers.

 Revision 1.1.2.4  1999/10/14 16:22:12  djr
 Implemented logging when system exceptions are thrown.

 Revision 1.1.2.3  1999/09/24 17:11:13  djr
 New option -ORBtraceInvocations and omniORB::traceInvocations.

 Revision 1.1.2.2  1999/09/24 15:01:34  djr
 Added module initialisers, and sll's new scavenger implementation.

 Revision 1.1.2.1  1999/09/22 14:26:55  djr
 Major rewrite of orbcore to support POA.

*/

#include <omniORB4/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <objectAdapter.h>
#include <localIdentity.h>
#include <initRefs.h>
#include <poaimpl.h>
#include <corbaBoa.h>
#include <exceptiondefs.h>
#include <giopServer.h>
#include <giopRope.h>

#include <stdlib.h>
#include <stdio.h>

static char                       initialised = 0;
static int                        num_active_oas = 0;
static _OMNI_NS(Rope)*            loopback = 0;
static omni_tracedmutex           oa_lock;

static _OMNI_NS(giopServer)*      oa_server = 0;

static _OMNI_NS(giopEndpointList) oa_giop_endpoints;

OMNI_NAMESPACE_BEGIN(omni)

omni_tracedmutex     omniObjAdapter::sd_detachedObjectLock;
omni_tracedcondition omniObjAdapter::sd_detachedObjectSignal(
				&omniObjAdapter::sd_detachedObjectLock);

omniObjAdapter::Options omniObjAdapter::options;


omniObjAdapter::~omniObjAdapter() {}


omniObjAdapter*
omniObjAdapter::getAdapter(const _CORBA_Octet* key, int keysize)
{
  omniObjAdapter* adapter;

  adapter = omniOrbPOA::getAdapter(key, keysize);
  if( adapter )  return adapter;

  if( keysize == sizeof(omniOrbBoaKey) )
    return omniOrbBOA::theBOA();

  return 0;
}


_CORBA_Boolean
omniObjAdapter::isInitialised()
{
  omni_tracedmutex_lock sync(oa_lock);

  return initialised;
}


void
omniObjAdapter::initialise()
{
  omni_tracedmutex_lock sync(oa_lock);

  if( initialised )  return;

  omniORB::logs(10, "Initialising incoming endpoints.");

  try {
    if (!oa_server)
      oa_server = new giopServer();

    if( options.endpoints.size() ) {
      for( EndpointList::iterator i = options.endpoints.begin();
	   i != options.endpoints.end(); i++ ) {
      
	giopEndpoint* e = giopEndpoint::str2Endpoint(*i);
	if (!e) {
	  // XXX In future, also add calls to other non-giop transports
	  // to give them a chance to instantiate an endpoint.
	  if (omniORB::trace(0)) {
	    omniORB::logger log;
	    log << "Error: Unable to create an endpoint of this description: "
		<< (const char*)*i
		<< "\n";
	  }
	  OMNIORB_THROW(INITIALIZE,0,CORBA::COMPLETED_NO);
	}
	oa_server->instantiate(e);
	oa_giop_endpoints.push_back(e);
      }
    }
    else {
      // instantiate a default tcp port.
      const char* hostname = getenv(OMNIORB_USEHOSTNAME_VAR);
      if( !hostname )  hostname = "";
      CORBA::String_var estr = CORBA::string_alloc(strlen(hostname)+8);
      sprintf(estr,"tcp/%s:0",hostname);
      giopEndpoint* e = giopEndpoint::str2Endpoint(estr);
      oa_server->instantiate(e);
      oa_giop_endpoints.push_back(e);
    }

    if( !options.noBootstrapAgent )
      omniInitialReferences::initialise_bootstrap_agentImpl();
  }
  catch (const CORBA::INITIALIZE&) {
    throw;
  }
  catch (...) {
    OMNIORB_THROW(INITIALIZE,0,CORBA::COMPLETED_NO);
  }

  initialised = 1;
}


void
omniObjAdapter::shutdown()
{
  omni_tracedmutex_lock sync(oa_lock);

  if( !initialised )  return;

  OMNIORB_ASSERT(num_active_oas == 0);

  omniORB::logs(10, "Shutting-down all incoming endpoints.");

  oa_server->remove();
  delete oa_server;
  oa_server = 0;

  oa_giop_endpoints.erase(oa_giop_endpoints.begin(),oa_giop_endpoints.end());

  if( loopback ) {
    loopback->decrRefCount();
    loopback = 0;
  }

  initialised = 0;
}


void
omniObjAdapter::adapterActive()
{
  omni_tracedmutex_lock sync(oa_lock);

  OMNIORB_ASSERT(initialised);

  if( pd_isActive )  return;

  if( num_active_oas++ == 0 ) {
    omniORB::logs(10, "Starting serving incoming endpoints.");
    oa_server->start();
  }

  pd_isActive = 1;
}


void
omniObjAdapter::adapterInactive()
{
  omni_tracedmutex_lock sync(oa_lock);

  if( !pd_isActive )  return;

  if( --num_active_oas == 0 ) {
    omniORB::logs(10, "Stopping serving incoming endpoints.");
    oa_server->stop();
  }

  pd_isActive = 0;
}


void
omniObjAdapter::waitForActiveRequestsToComplete(int locked)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, locked);

  if( !locked )  omni::internalLock->lock();

  OMNIORB_ASSERT(pd_nReqActive >= 0);

  pd_signalOnZeroInvocations++;
  while( pd_nReqActive )  pd_signal.wait();
  pd_signalOnZeroInvocations--;

  if( !locked )  omni::internalLock->unlock();
}


void
omniObjAdapter::waitForAllRequestsToComplete(int locked)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, locked);

  if( !locked )  omni::internalLock->lock();

  OMNIORB_ASSERT(pd_nReqInThis >= 0);

  pd_signalOnZeroInvocations++;
  while( pd_nReqInThis )  pd_signal.wait();
  pd_signalOnZeroInvocations--;

  if( !locked )  omni::internalLock->unlock();
}


void
omniObjAdapter::met_detached_object()
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 0);

  sd_detachedObjectLock.lock();

  OMNIORB_ASSERT(pd_nDetachedObjects > 0);

  int do_signal = --pd_nDetachedObjects == 0 && pd_signalOnZeroDetachedObjects;

  sd_detachedObjectLock.unlock();

  if( do_signal )  sd_detachedObjectSignal.broadcast();
}


void
omniObjAdapter::wait_for_detached_objects()
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 0);

  sd_detachedObjectLock.lock();
  pd_signalOnZeroDetachedObjects++;

  OMNIORB_ASSERT(pd_nDetachedObjects >= 0);

  while( pd_nDetachedObjects )  sd_detachedObjectSignal.wait();

  pd_signalOnZeroDetachedObjects--;
  sd_detachedObjectLock.unlock();
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

Rope*
omniObjAdapter::defaultLoopBack()
{
  omni_tracedmutex_lock sync(oa_lock);

  if( !loopback ) {

    // Until we have a fast in-memory loop back. We use a tcp loopback to
    // talk to ourself.
    for ( giopEndpointList::iterator i = oa_giop_endpoints.begin();
	  i != oa_giop_endpoints.end(); i++) {

      if (strcmp((*i)->type(),"tcp") == 0) {
	giopAddress* addr = giopAddress::str2Address((*i)->address());
	omni_tracedmutex_lock sync(*omniTransportLock);
	loopback = new giopRope(addr,1);
	break;
      }
    }
    if (!loopback) {
      // This is tough!!! Haven't got a loop back!
      // May be the object adaptor has been destroyed!!!
      OMNIORB_THROW(COMM_FAILURE,0,CORBA::COMPLETED_MAYBE);
    }
  }
  return loopback;
}

CORBA::Boolean
omniObjAdapter::matchMyEndpoints(const giopAddress* addr)
{
  for ( giopEndpointList::iterator i = oa_giop_endpoints.begin();
	i != oa_giop_endpoints.end(); i++) {
    if (strcmp((*i)->address(),addr->address())) return 1;
  }
  return 0;
}

omniObjAdapter::omniObjAdapter()
  : pd_nReqInThis(0),
    pd_nReqActive(0),
    pd_signalOnZeroInvocations(0),
    pd_signal(omni::internalLock ? omni::internalLock : &omni::nilRefLock()),
    pd_nDetachedObjects(0),
    pd_signalOnZeroDetachedObjects(0),
    pd_isActive(0)
{
  // NB. We always initialise pd_signal with omni::internalLock.
  // The case that it is initialised with &oa_lock only happends
  // for 'nil' object adapters, which may be contructed before
  // the ORB is initialised.  Thus omni::internalLock is not
  // initialised, and we have to use something else...
}


omniObjAdapter::
Options::~Options() {
  for( EndpointList::iterator i = options.endpoints.begin();
       i != options.endpoints.end(); i++ ) {
    CORBA::string_free(*i);
  }
}

OMNI_NAMESPACE_END(omni)
