// -*- Mode: C++; -*-
//                            Package   : omniORB2
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
 Revision 1.9.4.2  1999/09/25 17:00:21  sll
 Merged changes from omni2_8_develop branch.

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

 Revision 1.9.4.1  1999/09/15 20:18:29  sll
 Updated to use the new cdrStream abstraction.
 Marshalling operators for NetBufferedStream and MemBufferedStream are now
 replaced with just one version for cdrStream.
 Derived class giopStream implements the cdrStream abstraction over a
 network connection whereas the cdrMemoryStream implements the abstraction
 with in memory buffer.

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

#include <omniORB2/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <ropeFactory.h>
#include <scavenger.h>
#include <objectManager.h>
#ifndef __atmos__
#include <tcpSocket.h>
#define _tcpOutgoingFactory tcpSocketMToutgoingFactory
#else
#include <tcpATMos.h>
#define _tcpOutgoingFactory tcpATMosMToutgoingFactory
#endif

ropeFactoryType* ropeFactoryTypeList = 0;
ropeFactoryList* globalOutgoingRopeFactories;

omniObject*
ropeFactory::iopProfilesToRope(GIOPObjectInfo* objectInfo)
{
  const IOP::TaggedProfileList* profiles = objectInfo->iopProfiles();

  ropeFactoryType* factorytype = ropeFactoryTypeList;

  while (factorytype) {
    CORBA::ULong i;
    for (i=0; i < profiles->length(); i++) {
      if (factorytype->is_IOPprofileId((*profiles)[i].tag)) {
	Endpoint_var addr;
	(void) factorytype->decodeIOPprofile((*profiles)[i],
					     addr.out(),
					     objectInfo);
	{
	// Determine if this is a local object
	// In future, we have to partially decode the object key to
	// determine which object manager it belongs to.
	// For the moment, there is only one object manager- rootObjectManager.

	  omniObjectManager* manager = omniObjectManager::root(1);
	  if (manager) {

	    ropeFactory_iterator iter(manager->incomingRopeFactories());
	    incomingRopeFactory* factory;
	    while ((factory = (incomingRopeFactory*) iter())) {
	      objectInfo->rope_ = factory->findIncoming((Endpoint*)addr);
	      if (objectInfo->rope()) {
		// The endpoint is actually one of those exported by this 
		// address space.
		omniObject* result = omni::locateObject(manager,
					 *((omniObjectKey*)objectInfo->key()));
		if (result) { // Got it
		  return result;
		}
		// the object cannot be found by locateObject().
		// Instead of letting the exception propagate all the way 
		// upwards, treat this as a foreign object and creates a 
		// proxy object.
	      }
	    }
	  }
	  else {
	    // root object manager has not been initialised, this object
	    // cannot be a local object. Treat this as a foreign object
	  }
	}
	{
	  // Reach here because this is not a local object
	  ropeFactory_iterator iter(globalOutgoingRopeFactories);
	  outgoingRopeFactory* factory;
	  while ((factory = (outgoingRopeFactory*) iter())) {
	    objectInfo->rope_ = factory->findOrCreateOutgoing((Endpoint*)addr);
	    if (objectInfo->rope()) {
	      // Got it
	      return 0;
	    }
	  }
	}
      }
    }
    factorytype = factorytype->next;
  }
  // Reach here if none of the ropeFactories support the profiles.
  throw CORBA::INV_OBJREF(0,CORBA::COMPLETED_NO);
#ifdef NEED_DUMMY_RETURN
  return 0;
#endif
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

outgoingRopeFactory::~outgoingRopeFactory() {}

ropeFactoryList::~ropeFactoryList() {}

ropeFactoryList_ThreadSafe::~ropeFactoryList_ThreadSafe() {}

ropeFactory_iterator::~ropeFactory_iterator() { pd_l.unlock(); }


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
