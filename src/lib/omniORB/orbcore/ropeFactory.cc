// -*- Mode: C++; -*-
//                            Package   : omniORB
// ropeFactory.cc             Created on: 30/9/97
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
 Revision 1.11  2000/07/04 15:22:50  dpg1
 Merge from omni3_develop.

 Revision 1.9.6.3  2000/06/27 15:40:58  sll
 Workaround for Cygnus gcc's inability to recognise _CORBA_Octet*& and
 CORBA::Octet*& are the same type.

 Revision 1.9.6.2  1999/09/24 15:01:36  djr
 Added module initialisers, and sll's new scavenger implementation.

 Revision 1.9.2.1  1999/09/21 20:37:17  sll
 -Simplified the scavenger code and the mechanism in which connections
  are shutdown. Now only one scavenger thread scans both incoming
  and outgoing connections. A separate thread do the actual shutdown.
 -omniORB::scanGranularity() now takes only one argument as there is
  only one scan period parameter instead of 2.
 -Trace messages in various modules have been updated to use the logger
  class.
 -ORBscanGranularity replaces -ORBscanOutgoingPeriod and
                                -ORBscanIncomingPeriod.

 Revision 1.9  1999/08/16 19:26:56  sll
 Added a per-compilation unit initialiser object.

 Revision 1.8  1999/08/14 16:38:53  sll
 Changed as locateObject no longer throws an exception when the object is
 not found.

 Revision 1.7  1999/07/02 19:27:21  sll
 Fixed typo in ropeFactory_iterator.

 Revision 1.6  1999/07/02 19:10:29  sll
 Added virtual destructors which have been removed from the header.

 Revision 1.5  1999/03/11 16:25:55  djr
 Updated copyright notice

 Revision 1.4  1998/08/14 13:51:22  sll
 Added pragma hdrstop to control pre-compile header if the compiler feature
 is available.

 Revision 1.3  1998/03/04 15:21:03  ewc
 Typo corrected - giopServerThreadWrapper

// Revision 1.2  1998/03/04  14:45:04  sll
// Added omniORB::giopServerThreadWrapper.
//
 Revision 1.1  1997/12/09 18:43:19  sll
 Initial revision

*/

#include <omniORB3/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <ropeFactory.h>
#include <scavenger.h>
#include <objectAdapter.h>
#include <initialiser.h>
#ifndef __atmos__
#include <tcpSocket.h>
#define _tcpOutgoingFactory tcpSocketMToutgoingFactory
#else
#include <tcpATMos.h>
#define _tcpOutgoingFactory tcpATMosMToutgoingFactory
#endif

ropeFactoryType* ropeFactoryTypeList = 0;
ropeFactoryList* globalOutgoingRopeFactories = 0;


int
ropeFactory::iopProfilesToRope(const IOP::TaggedProfileList& profiles,
			       CORBA::Octet*& key, int& keysize,
			       Rope*& rope, _CORBA_Boolean& is_local)
{
  is_local = 0;
  ropeFactoryType* factorytype = ropeFactoryTypeList;

  while( factorytype ) {

    for( CORBA::ULong i = 0; i < profiles.length(); i++ ) {

      if( factorytype->is_IOPprofileId(profiles[i].tag) ) {

	Endpoint_var addr;
	{
	  Endpoint* addrp;
	  size_t ks;
	  (void) factorytype->decodeIOPprofile(profiles[i], addrp, key, ks);
	  addr = addrp;
	  keysize = ks;
	}

	// Determine if this is a local object

	if( omniObjAdapter::isInitialised() ) {

	  ropeFactory_iterator iter(omniObjAdapter::incomingRopeFactories());

	  incomingRopeFactory* factory;

	  while( (factory = (incomingRopeFactory*) iter()) ) {
	    if( (rope = factory->findIncoming((Endpoint*) addr)) ) {
	      // The endpoint is actually one of those exported by this 
	      // address space.
	      rope->decrRefCount();
	      rope = 0;
	      is_local = 1;
	      return 1;
	    }
	  }
	}
	else {
	  // Root object manager has not been initialised, this object
	  // cannot be a local object. Treat this as a foreign object.
	  rope = 0;
	}

	// Reach here because this is not a local object.
	ropeFactory_iterator iter(globalOutgoingRopeFactories);
	outgoingRopeFactory* factory;
	while( (factory = (outgoingRopeFactory*) iter()) ) {
	  if( (rope = factory->findOrCreateOutgoing((Endpoint*)addr)) ) {
	    // Got it
	    return 1;
	  }
	}

	// Reach here because for some reason we could not instantiate
	// a rope. Continue and see if we can use another factory type.
	delete[] key;
	key = 0;
      }

    } // for( ... )

    factorytype = factorytype->next;

  } // while( factorytype )

  // Reach here if none of the ropeFactories support the profiles.
  return 0;
}


static omniORB::giopServerThreadWrapper* giopServerThreadWrapperP = 0;

void
omniORB::
giopServerThreadWrapper::setGiopServerThreadWrapper(
                                       omniORB::giopServerThreadWrapper* newp)
{
  if (!newp) return;
  giopServerThreadWrapper* p = giopServerThreadWrapperP;
  giopServerThreadWrapperP = newp;
  if (p) delete p;
}

omniORB::giopServerThreadWrapper*
omniORB::
giopServerThreadWrapper::getGiopServerThreadWrapper()
{
  return giopServerThreadWrapperP;
}

ropeFactoryType::~ropeFactoryType() {}

ropeFactory::~ropeFactory() {}

incomingRopeFactory::~incomingRopeFactory() {}


CORBA::Boolean
incomingRopeFactory::isOutgoing(Endpoint* addr) const
{
  return 0;
}


outgoingRopeFactory::~outgoingRopeFactory() {}


CORBA::Boolean
outgoingRopeFactory::isIncoming(Endpoint* addr) const
{
  return 0;
}


ropeFactoryList::~ropeFactoryList()  {}


ropeFactoryList_ThreadSafe::~ropeFactoryList_ThreadSafe()  {}


Endpoint::~Endpoint() {
  delete [] pd_protocolname;
}


/////////////////////////////////////////////////////////////////////////////
//            Module initialiser                                           //
/////////////////////////////////////////////////////////////////////////////

class omni_ropeFactory_initialiser : public omniInitialiser {
public:

  void attach() {

    globalOutgoingRopeFactories = new ropeFactoryList;

    // Initialise all the rope factories that will be used to
    // create outgoing ropes.
    globalOutgoingRopeFactories->insert(new _tcpOutgoingFactory );

    // Add rope factories for other transports here.

    // Initialise a giopServerThreadWrapper singelton
    omniORB::giopServerThreadWrapper::setGiopServerThreadWrapper(
       new omniORB::giopServerThreadWrapper);

    StrandScavenger::addRopeFactories(globalOutgoingRopeFactories);
  }

  void detach() {
    StrandScavenger::removeRopeFactories(globalOutgoingRopeFactories);
  }
};

static omni_ropeFactory_initialiser initialiser;

omniInitialiser& omni_ropeFactory_initialiser_ = initialiser;
