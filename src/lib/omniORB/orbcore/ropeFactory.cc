// -*- Mode: C++; -*-
//                            Package   : omniORB2
// ropeFactory.cc             Created on: 30/9/97
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
 Revision 1.1  1997/12/09 18:43:19  sll
 Initial revision

*/

#include <omniORB2/CORBA.h>
#include <ropeFactory.h>
#include <objectManager.h>

ropeFactoryType* ropeFactoryTypeList = 0;
ropeFactoryList globalOutgoingRopeFactories;


omniObject*
ropeFactory::iopProfilesToRope(const IOP::TaggedProfileList *profiles,
			       CORBA::Octet *&objkey,
			       size_t &keysize,
			       Rope_var& rope)
{
  ropeFactoryType* factorytype = ropeFactoryTypeList;

  while (factorytype) {
    CORBA::ULong i;
    for (i=0; i < profiles->length(); i++) {
      if (factorytype->is_IOPprofileId((*profiles)[i].tag)) {
	Endpoint_var addr;
	Endpoint* addrp;
	(void) factorytype->decodeIOPprofile((*profiles)[i],addrp,objkey,
					     keysize);
	addr = addrp;

	{
	// Determine if this is a local object
	// In future, we have to partially decode the object key to
	// determine which object manager it belongs to.
	// For the moment, there is only one object manager- rootObjectManager.

	  omniObjectManager* manager = omniObjectManager::root(1);
	  if (manager) {

	    ropeFactory_iterator iter(*(manager->incomingRopeFactories()));
	    incomingRopeFactory* factory;
	    while ((factory = (incomingRopeFactory*) iter())) {
	      if (rope = factory->findIncoming((Endpoint*)addr)) {
		// The endpoint is actually one of those exported by this 
		// address space.
		rope = 0;
		try {
		  omniObject* result = omni::locateObject(manager,
			   			 *((omniObjectKey*)objkey));
		  // Got it
		  return result;
		}
		catch (const CORBA::OBJECT_NOT_EXIST&) {
		  // the object cannot be found by locateObject().
		  // Instead of letting the exception propagate all the way 
		  // upwards, treat this as a foreign object and creates a 
		  // proxy object.
		  break;
		}
	      }
	    }
	  }
	  else {
	    // root object manager has not been initialised, this object
	    // cannot be a local object. Treat this as a foreign object
	    rope = 0;
	  }
	}
	{
	  // Reach here because this is not a local object
	  ropeFactory_iterator iter(globalOutgoingRopeFactories);
	  outgoingRopeFactory* factory;
	  while ((factory = (outgoingRopeFactory*) iter())) {
	    if (rope = factory->findOrCreateOutgoing((Endpoint*)addr)) {
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


