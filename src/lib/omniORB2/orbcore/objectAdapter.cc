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

#include <omniORB3/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <objectAdapter.h>
#include <localIdentity.h>
#include <ropeFactory.h>
#include <scavenger.h>
#include <initRefs.h>
#include <poaimpl.h>
#include <corbaBoa.h>
#include <exceptiondefs.h>

#include <stdlib.h>

#ifndef __atmos__
#include <tcpSocket.h>
#define _tcpIncomingFactory tcpSocketMTincomingFactory
#define _tcpIncomingRope    tcpSocketIncomingRope
#define _tcpEndpoint        tcpSocketEndpoint
#else
#include <tcpATMos.h>
#define _tcpIncomingFactory tcpATMosMTincomingFactory
#define _tcpIncomingRope    tcpATMosIncomingRope
#define _tcpEndpoint        tcpATMosEndpoint
#endif


#ifndef OMNIORB_USEHOSTNAME_VAR
#define OMNIORB_USEHOSTNAME_VAR "OMNIORB_USEHOSTNAME"
#endif


static ropeFactoryList* incomingFactories = 0;
static char             initialised = 0;
static int              num_active_oas = 0;
static Rope*            loopback = 0;
static omni_tracedmutex oa_lock;

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

  omniORB::logs(10, "Initialising incoming rope factories.");

  try {
    if( !incomingFactories ) {
      incomingFactories = new ropeFactoryList;
      incomingFactories->insert(new _tcpIncomingFactory);
    }

    ropeFactory_iterator iter(incomingFactories);
    incomingRopeFactory* factory;

    while( (factory = (incomingRopeFactory*) iter()) ) {
      if( factory->getType()->is_protocol(_tcpEndpoint::protocol_name) ) {

	if( options.incomingPorts.size() ) {
	  for( ListenPortList::iterator i = options.incomingPorts.begin();
	       i != options.incomingPorts.end(); i++ ) {

	    _tcpEndpoint e((const CORBA::Char*)(char*) i->host, i->port);
	    factory->instantiateIncoming(&e, 1);

	  }
	}
	else {
	  // Instantiate a rope.  Let the OS pick a port number.
	  const char* hostname = getenv(OMNIORB_USEHOSTNAME_VAR);
	  if( !hostname )  hostname = "";
	  _tcpEndpoint e((const CORBA::Char*) hostname, 0);
	  factory->instantiateIncoming(&e, 1);
	}

      }
    }

    if( !options.noBootstrapAgent )
      omniInitialReferences::initialise_bootstrap_agentImpl();
  }
  catch (...) {
    throw;//?? hmm - this is probably quite bad.
  }

  initialised = 1;
}


void
omniObjAdapter::shutdown()
{
  omni_tracedmutex_lock sync(oa_lock);

  if( !initialised )  return;

  OMNIORB_ASSERT(num_active_oas == 0);

  omniORB::logs(10, "Shutting-down incoming rope factories.");

  {
    // Beware of a possible deadlock where the scavenger thread and this
    // thread both try to grep the mutex in factory->anchor().  To prevent
    // this from happening, put this block of code in a separate scope.
    ropeFactory_iterator iter(incomingFactories);
    incomingRopeFactory* factory;

    while( (factory = (incomingRopeFactory*) iter()) )
      factory->removeIncoming();
  }

  StrandScavenger::removeRopeFactories(incomingFactories);

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
    omniORB::logs(10, "Starting incoming rope factories.");
    {
      // Beware of a possible deadlock where the scavenger thread and this
      // thread both try to grep the mutex in factory->anchor().  To prevent
      // this from happening, put this block of code in a separate scope.
      ropeFactory_iterator iter(incomingFactories);
      incomingRopeFactory* factory;

      while( (factory = (incomingRopeFactory*) iter()) )
	factory->startIncoming();
    }
    //?? Hmmm.  What if done adapterActive, adapterInactive, adapterActive?
    StrandScavenger::addRopeFactories(incomingFactories);
  }

  pd_isActive = 1;
}


void
omniObjAdapter::adapterInactive()
{
  omni_tracedmutex_lock sync(oa_lock);

  if( !pd_isActive )  return;

  if( --num_active_oas == 0 ) {
    omniORB::logs(10, "Stopping incoming rope factories.");

    ropeFactory_iterator iter(incomingFactories);
    incomingRopeFactory* factory;

    while( (factory = (incomingRopeFactory*) iter()) )
      factory->stopIncoming();
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

ropeFactoryList*
omniObjAdapter::incomingRopeFactories()
{
  if( !initialised )
    throw omniORB::fatalException(__FILE__, __LINE__,
	  "omniObjAdapter::incomingRopeFactories() -- not initialised!");

  return incomingFactories;
}


Rope*
omniObjAdapter::defaultLoopBack()
{
  omni_tracedmutex_lock sync(oa_lock);

  if( !loopback ) {

    Endpoint* myaddr = 0;

    // Locate the incoming tcpSocket Rope, read its address and
    // use this address to create a new outgoing tcpSocket Rope.
    {
      ropeFactory_iterator iter(incomingFactories);
      incomingRopeFactory* factory;

      while( (factory = (incomingRopeFactory*) iter()) ) {
	if( factory->getType()->is_protocol(_tcpEndpoint::protocol_name) ) {
	  Rope_iterator riter(factory);
	  _tcpIncomingRope* r = (_tcpIncomingRope*) riter();
	  if( r )
	    r->this_is(myaddr);
	  else
	    // This is tough!!! Haven't got a loop back!
	    // May be the BOA has been destroyed!!!
	    OMNIORB_THROW(COMM_FAILURE,0,CORBA::COMPLETED_MAYBE);
	}
      }
    }

    {
      ropeFactory_iterator iter(globalOutgoingRopeFactories);
      outgoingRopeFactory* factory;

      while( (factory = (outgoingRopeFactory*) iter()) )
	if( (loopback = factory->findOrCreateOutgoing((Endpoint*) myaddr)) )
	  break;
    }
    delete myaddr;
  }

  return loopback;
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
