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
 Revision 1.2.2.7  2001/06/11 18:01:18  sll
 Fixed silly mistake in debugging message.

 Revision 1.2.2.6  2001/05/31 16:18:13  dpg1
 inline string matching functions, re-ordered string matching in
 _ptrToInterface/_ptrToObjRef

 Revision 1.2.2.5  2001/04/18 18:18:07  sll
 Big checkin with the brand new internal APIs.

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
#include <omniORB4/omniInterceptors.h>

#include <stdlib.h>
#include <stdio.h>

OMNI_NAMESPACE_BEGIN(omni)

static char                             initialised = 0;
static int                              num_active_oas = 0;
static omni_tracedmutex                 oa_lock;
static omnivector<_OMNI_NS(orbServer)*> oa_servers;
static omnivector<const char*>          oa_endpoints;
static _OMNI_NS(Rope)*                  oa_loopback = 0;
static void instantiate_defaultloopback(omnivector<const char*>& endpoints);


omni_tracedmutex     omniObjAdapter::sd_detachedObjectLock;
omni_tracedcondition omniObjAdapter::sd_detachedObjectSignal(
				&omniObjAdapter::sd_detachedObjectLock);

omniObjAdapter::Options omniObjAdapter::options;


//////////////////////////////////////////////////////////////////////
omniObjAdapter::~omniObjAdapter() {}


//////////////////////////////////////////////////////////////////////
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


//////////////////////////////////////////////////////////////////////
_CORBA_Boolean
omniObjAdapter::isInitialised()
{
  omni_tracedmutex_lock sync(oa_lock);

  return initialised;
}


//////////////////////////////////////////////////////////////////////
static
const char*
instantiate_endpoint(const char* uri,CORBA::Boolean no_publish,
		     CORBA::Boolean no_listen) {

  omnivector<orbServer*>::iterator j,last;
  const char* address = 0;
  j = oa_servers.begin();
  last = oa_servers.end();
  for ( ; j != last; j++ ) {
    address = (*j)->instantiate(uri,no_publish,no_listen);
    if (address) break;
  }
  return address;
}

//////////////////////////////////////////////////////////////////////
void
omniObjAdapter::initialise()
{
  omni_tracedmutex_lock sync(oa_lock);

  if( initialised )  return;

  omniORB::logs(10, "Initialising incoming endpoints.");

  try {

    if ( oa_servers.empty() ) {
      oa_servers.push_back(new giopServer());
      // Now if any other exotic servers want to be started, this is
      // where this should be done.
      omniInterceptors::createORBServer_T::info_T info(oa_servers);
      omniORB::getInterceptors()->createORBServer.visit(info);
    }

    omnivector<const char*> myendpoints;

    if ( !options.endpoints.empty() ) {

      Options::EndpointURIList::iterator i = options.endpoints.begin();
      for ( ; i != options.endpoints.end(); i++ ) {

	const char* address = instantiate_endpoint((*i)->uri,
						   (*i)->no_publish,
						   (*i)->no_listen);
	if (!address) {
	  if (omniORB::trace(0)) {
	    omniORB::logger log;
	    log << "Error: Unable to create an endpoint of this description: "
		<< (const char*)(*i)->uri
		<< "\n";
	  }
	  OMNIORB_THROW(INITIALIZE,0,CORBA::COMPLETED_NO);
	}
	oa_endpoints.push_back(address);
	if ( !(*i)->no_listen ) {
	  myendpoints.push_back(address);
	}
      }
    }
    else {
      // instantiate a default tcp port.
      const char* hostname = getenv(OMNIORB_USEHOSTNAME_VAR);
      if( !hostname )  hostname = "";
      const char* format = "giop:tcp:%s:%d";
      CORBA::String_var estr(CORBA::string_alloc(strlen(hostname)+
						 strlen(format) + 6));
      sprintf(estr,format,hostname,0);

      const char* address = instantiate_endpoint(estr,0,0);

      if (!address) {
	if (omniORB::trace(0)) {
	  omniORB::logger log;
	  log << "Error: Unable to create an endpoint of this description: "
	      << (const char*)estr
	      << "\n";
	}
	OMNIORB_THROW(INITIALIZE,0,CORBA::COMPLETED_NO);
      }
      oa_endpoints.push_back(address);
      myendpoints.push_back(address);
    }

    instantiate_defaultloopback(myendpoints);

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


//////////////////////////////////////////////////////////////////////
void
omniObjAdapter::shutdown()
{
  omni_tracedmutex_lock sync(oa_lock);

  if( !initialised )  return;

  OMNIORB_ASSERT(num_active_oas == 0);

  omniORB::logs(10, "Shutting-down all incoming endpoints.");

  if ( !oa_servers.empty() ) {

    omnivector<orbServer*>::iterator j,last;
    j = oa_servers.begin();
    last = oa_servers.end();
    for ( ; j != last; j++ ) {
      (*j)->remove();
      delete (*j);
    }
    oa_servers.erase(oa_servers.begin(),oa_servers.end());
  }

  oa_endpoints.erase(oa_endpoints.begin(),oa_endpoints.end());

  if( oa_loopback ) {
    oa_loopback->decrRefCount();
    oa_loopback = 0;
  }

  initialised = 0;
}


//////////////////////////////////////////////////////////////////////
void
omniObjAdapter::adapterActive()
{
  omni_tracedmutex_lock sync(oa_lock);

  OMNIORB_ASSERT(initialised);

  if( pd_isActive )  return;

  if( num_active_oas++ == 0 ) {
    omniORB::logs(10, "Starting serving incoming endpoints.");

    if ( !oa_servers.empty() ) {

      omnivector<orbServer*>::iterator j,last;
      j = oa_servers.begin();
      last = oa_servers.end();
      for ( ; j != last; j++ ) {
	(*j)->start();
      }
    }
  }

  pd_isActive = 1;
}


//////////////////////////////////////////////////////////////////////
void
omniObjAdapter::adapterInactive()
{
  omni_tracedmutex_lock sync(oa_lock);

  if( !pd_isActive )  return;

  if( --num_active_oas == 0 ) {
    omniORB::logs(10, "Stopping serving incoming endpoints.");

    if ( !oa_servers.empty() ) {

      omnivector<orbServer*>::iterator j,last;
      j = oa_servers.begin();
      last = oa_servers.end();
      for ( ; j != last; j++ ) {
	(*j)->stop();
      }
    }
  }

  pd_isActive = 0;
}


//////////////////////////////////////////////////////////////////////
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


//////////////////////////////////////////////////////////////////////
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


//////////////////////////////////////////////////////////////////////
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

//////////////////////////////////////////////////////////////////////
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
Rope*
omniObjAdapter::defaultLoopBack()
{
  omni_tracedmutex_lock sync(oa_lock);

  if (!oa_loopback) {
    // This is tough!!! Haven't got a loop back!
    // May be the object adaptor has been destroyed!!!
    OMNIORB_THROW(COMM_FAILURE,0,CORBA::COMPLETED_MAYBE);
  }
  return oa_loopback;
}

//////////////////////////////////////////////////////////////////////
static
const char*
pick_endpoint(omnivector<const char*>& endpoints,const char* prefix) {

  omnivector<const char*>::iterator i,last;

  i = endpoints.begin();
  last = endpoints.end();

  int len = strlen(prefix);
  for ( ; i != last; i++) {
    if (strncmp((*i),prefix,len) == 0) {
      return (*i);
    }
  }
  return 0;
}

//////////////////////////////////////////////////////////////////////
static
void
instantiate_defaultloopback(omnivector<const char*>& endpoints) {

  // Until we have a fast in-memory loop back. We use a stream loopback to
  // talk to ourself.

  // XXX We are restricting ourselves to just giop:* transports.
  //     We can generalise this to allow other (and faster) loopback to
  //     be used but that is probably not very useful because in the
  //     long run we would like to have a in-memory loopback.


  const char* loopback_endpoint = 0;

  // We prefer unix to tcp. ssl is the last choice.
  loopback_endpoint = pick_endpoint(endpoints,"giop:unix");

  if (!loopback_endpoint) {
    loopback_endpoint = pick_endpoint(endpoints,"giop:tcp");
  }

  if (!loopback_endpoint) {
    loopback_endpoint = pick_endpoint(endpoints,"giop:ssl");
  }

  if (loopback_endpoint) {
    giopAddress* addr = giopAddress::str2Address(loopback_endpoint);
    if (addr) {
      omni_tracedmutex_lock sync(*omniTransportLock);
      oa_loopback = new giopRope(addr,1);
    }
  }

  if (!oa_loopback) {
    if (omniORB::trace(1)) {
      omniORB::logger log;
      log << "Warning: Unable to create a loopback transport to talk to this ORB.\n"
	     "         Any attempt to talk to a local object may fail.\n"
	     "         This error is caused by the lack of a suitable endpoint\n"
             "         that is layered on top of TCP, SSL or UNIX socket.\n"
             "         To correct this problem, check if the -ORBendpoint\n"
	     "         options are consistent.\n";
    }
  }
}



//////////////////////////////////////////////////////////////////////
CORBA::Boolean
omniObjAdapter::matchMyEndpoints(const char* addr)
{
  for ( omnivector<const char*>::iterator i = oa_endpoints.begin();
	i != oa_endpoints.end(); i++) {
    if (omni::ptrStrMatch((*i),addr)) return 1;
  }
  return 0;
}

//////////////////////////////////////////////////////////////////////
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


//////////////////////////////////////////////////////////////////////
omniObjAdapter::
Options::~Options() {
  for( EndpointURIList::iterator i = options.endpoints.begin();
       i != options.endpoints.end(); i++ ) {
    delete (*i);
  }
}

OMNI_NAMESPACE_END(omni)
