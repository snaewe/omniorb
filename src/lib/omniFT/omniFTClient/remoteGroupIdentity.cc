// -*- Mode: C++; -*-
//                            Package   : omniORB
// remoteGroupIdentity.cc     Created on: 18/01/01
//                            Author    : Bob Gruber (reg)
//
//    Copyright (C) 1996-1999 AT&T Research Cambridge
//
//    This file is part of the omniORB library.
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
  Revision 1.1.2.1  2001/02/23 19:35:20  sll
  Merged interim FT client stuff.

*/

#include <omniORB4/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <remoteGroupIdentity.h>
#include <omniORB4/callDescriptor.h>
// #include <dynamicLib.h>
#include <exceptiondefs.h>


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

class omniRemoteGroupIdentity_RefHolder {
public:
  inline omniRemoteGroupIdentity_RefHolder(omniRemoteGroupIdentity* id) : pd_id(id) {
    pd_id->pd_refCount++;
    omni::internalLock->unlock();
  }

  inline ~omniRemoteGroupIdentity_RefHolder() {
    omni::internalLock->lock();
    int done = --pd_id->pd_refCount > 0;
    omni::internalLock->unlock();
    if( done )  return;
    delete pd_id;
  }

private:
  omniRemoteGroupIdentity* pd_id;
};

//////////////////////////////////////////////////////////////////////
//////////////////////// Call Marshaller      ////////////////////////
//////////////////////////////////////////////////////////////////////

class RemoteGroupIdentityCallArgumentsMarshaller : public giopMarshaller {
public:
  RemoteGroupIdentityCallArgumentsMarshaller(cdrStream& s,
					     omniCallDescriptor& desc) : 
    pd_s(s), pd_desc(desc) {}

  void marshalData() {
    pd_desc.marshalArguments(pd_s);
  }

  size_t dataSize(size_t initialoffset) {
    cdrCountingStream s(pd_s.TCS_C(),pd_s.TCS_W(),initialoffset);
    pd_desc.marshalArguments(s);
    return s.total();
  }

private:
  cdrStream&     pd_s;
  omniCallDescriptor& pd_desc;
};

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

// Constructor consumes <ior>, copies <key>; initial ref count of 0.

omniRemoteGroupIdentity::omniRemoteGroupIdentity(omniIOR* ior, CORBA::Boolean locked)
  : omniIdentity((const CORBA::Octet*)ior->iiop.object_key.get_buffer(),
		 ior->iiop.object_key.length()),
  pd_refCount(0), pd_ior(ior),pd_selected_target(0)
{
  CORBA::ULong total = ior->iopProfiles->length();
  CORBA::ULong index = 0;
  for ( ; index < total; index++) {
    if (ior->iopProfiles[index].tag == IOP::TAG_INTERNET_IOP) {
      // construct new IOR with this internet profile and all multiple components profiles;
      // construct a new omniRemoteIdentity using this IOR
      IOP::TaggedProfileList* nioplist = new IOP::TaggedProfileList();
      nioplist->length(1);
      (*nioplist)[0] = ior->iopProfiles[index]; // causes a copy
      CORBA::ULong index2 = 0;
      for ( ; index2 < total; index2++) {
	if (ior->iopProfiles[index2].tag == IOP::TAG_MULTIPLE_COMPONENTS) {
	  nioplist->length(nioplist->length()+1);
	  (*nioplist)[nioplist->length()-1] = ior->iopProfiles[index2]; // causes a copy
	}
      }
      omniIOR* nior = new omniIOR(CORBA::string_dup(ior->repositoryID), nioplist);
      omniRemoteIdentity* nid = omni::createRemoteIdentity(nior); // consumes nior
      if (nid) {
	pd_targets.length(pd_targets.length() + 1);
	pd_targets[pd_targets.length()-1] = nid;
      } // else invalid IOR so pd_targets not changed
    }
  }
}

void
omniRemoteGroupIdentity::dispatch(omniCallDescriptor& call_desc)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);

  omniRemoteGroupIdentity_RefHolder rh(this);
  // omni::internalLock has been released by RefHolder constructor

  if( omniORB::traceInvocations ) {
    omniORB::logger l;
    l << "Invoke '" << call_desc.op() << "' on remote object group : " << this << '\n';
  }

  CORBA::ULong total = pd_targets.length();
  if (total == 0) {
    if( omniORB::traceInvocations ) {
      omniORB::logger l;
      l << "Cannot Invoke '" << call_desc.op() << "' on remote object group : " << this << " : no targets\n";
    }
    OMNIORB_THROW(OBJECT_NOT_EXIST,0, CORBA::COMPLETED_NO);
  }
  CORBA::ULong index = pd_selected_target;
  CORBA::ULong tries = 0;

  for (; tries < total; index = (index+1)%total ) {
    tries++;
    try {
      pd_targets[index]->dispatch(call_desc);
      pd_selected_target = index;  // try this target first for future dispatches/locate requests
      return;
    }
    catch(CORBA::COMM_FAILURE& ex) {
      if ((tries == total) || (ex.completed() != CORBA::COMPLETED_NO)) {
	throw; // cannot retry
      } // else continue
    }
    catch(CORBA::TRANSIENT& ex) {
      if ((tries == total) || (ex.completed() != CORBA::COMPLETED_NO)) {
	throw; // cannot retry
      } // else continue
    }
    catch(CORBA::NO_IMPLEMENT& ex) {
      if ((tries == total) || (ex.completed() != CORBA::COMPLETED_NO)) {
	throw; // cannot retry
      } // else continue
    }
    catch(CORBA::NO_RESPONSE& ex) {
      if ((tries == total) || (ex.completed() != CORBA::COMPLETED_NO)) {
	throw; // cannot retry
      } // else continue
    }
    catch(CORBA::OBJ_ADAPTER& ex) {
      if ((tries == total) || (ex.completed() != CORBA::COMPLETED_NO)) {
	throw; // cannot retry
      } // else continue
    }
    catch(CORBA::OBJECT_NOT_EXIST& ex) {
      if ((tries == total) || (ex.completed() != CORBA::COMPLETED_NO)) {
	throw; // cannot retry
      } // else continue
    }
    // for other exceptions, let omniObjRef::_invoke catch them
  }
  // should not get here
}

void
omniRemoteGroupIdentity::gainObjRef(omniObjRef* o)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);
  pd_refCount++;
  CORBA::ULong total = pd_targets.length();
  CORBA::ULong index = 0;
  for ( ; index < total; index++) {
    pd_targets[index]->gainObjRef(o);
  }
}

void
omniRemoteGroupIdentity::loseObjRef(omniObjRef* o)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);
  CORBA::ULong total = pd_targets.length();
  CORBA::ULong index = 0;
  for ( ; index < total; index++) {
    pd_targets[index]->loseObjRef(o);
  }
  if( --pd_refCount > 0 ) return;
  delete this;
}

void
omniRemoteGroupIdentity::locateRequest()
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);

  omniRemoteGroupIdentity_RefHolder rh(this);
  // omni::internalLock has been released by RefHolder constructor

  if( omniORB::trace(10) ) {
    omniORB::logger l;
    l << "LocateRequest for remote object group : " << this << '\n';
  }
  CORBA::ULong total = pd_targets.length();
  if (total == 0) {
    if( omniORB::trace(10) ) {
      omniORB::logger l;
      l << "Cannot attempt LocateRequest for remote object group : " << this << " : no targets\n";
    }
    OMNIORB_THROW(OBJECT_NOT_EXIST,0, CORBA::COMPLETED_NO);
  }

  CORBA::ULong index = pd_selected_target;
  CORBA::ULong tries = 0;

  for (; tries < total; index = (index+1)%total ) {
    tries++;
    try {
      pd_targets[index]->locateRequest();
      pd_selected_target = index;  // try this target first for future dispatches/locate requests
      return;
    }
    catch(CORBA::COMM_FAILURE& ex) {
      if (tries == total) {
	throw; // cannot retry
      } // else continue
    }
    catch(CORBA::TRANSIENT& ex) {
      if (tries == total) {
	throw; // cannot retry
      } // else continue
    }
    catch(CORBA::NO_IMPLEMENT& ex) {
      if (tries == total) {
	throw; // cannot retry
      } // else continue
    }
    catch(CORBA::NO_RESPONSE& ex) {
      if (tries == total) {
	throw; // cannot retry
      } // else continue
    }
    catch(CORBA::OBJ_ADAPTER& ex) {
      if (tries == total) {
	throw; // cannot retry
      } // else continue
    }
    catch(CORBA::OBJECT_NOT_EXIST& ex) {
      if (tries == total) {
	throw; // cannot retry
      } // else continue
    }
    // for other exceptions, let omniObjRef::_locateRequest() catch them
  }
  // should not get here
}


omniRemoteGroupIdentity::~omniRemoteGroupIdentity()
{
  omniORB::logs(15, "omniRemoteGroupIdentity deleted.");
  // each target should already have been deleted due to loseObjRef
  // (releasing all the derived iors).  the following release 
  // should only occur after that.
  pd_ior->release();
}

