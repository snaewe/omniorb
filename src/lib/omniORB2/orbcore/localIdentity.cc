// -*- Mode: C++; -*-
//                            Package   : omniORB
// localIdentity.cc           Created on: 16/6/99
//                            Author    : David Riddoch (djr)
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
  Revision 1.1.2.3  1999/10/14 16:22:11  djr
  Implemented logging when system exceptions are thrown.

  Revision 1.1.2.2  1999/09/30 12:25:59  djr
  Minor changes.

  Revision 1.1.2.1  1999/09/22 14:26:52  djr
  Major rewrite of orbcore to support POA.

*/

#include <omniORB3/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <localIdentity.h>
#include <omniORB3/omniObjRef.h>
#include <omniORB3/omniServant.h>
#include <omniORB3/callDescriptor.h>
#include <objectAdapter.h>
#include <ropeFactory.h>
#include <exception.h>

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

class omniLocalIdentity_RefHolder {
public:
  inline omniLocalIdentity_RefHolder(omniLocalIdentity* id) : pd_id(id) {
    ASSERT_OMNI_TRACEDMUTEX_HELD(omni::internalLock, 1);
    pd_id->pd_nInvocations++;
  }

  inline ~omniLocalIdentity_RefHolder() {
    omni::internalLock.lock();
    pd_id->pd_nInvocations--;
    pd_id->pd_adapter->leaveAdapter();
    int done = pd_id->pd_nInvocations > 0;
    omni::internalLock.unlock();
    if( done )  return;
    // Object has been deactivated, and the last invocation
    // has completed.  Pass the object back to the adapter
    // so it can be etherealised.
    pd_id->adapter()->lastInvocationHasCompleted(pd_id);
  }

private:
  omniLocalIdentity* pd_id;
};

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void
omniLocalIdentity::dispatch(omniCallDescriptor& call_desc)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(omni::internalLock, 1);
  OMNIORB_ASSERT(pd_adapter && pd_servant);

  omniLocalIdentity_RefHolder rh(this);

  omni::localInvocationCount++;

#ifndef HAS_Cplusplus_catch_exception_by_base
  // The compiler cannot catch exceptions by base class, hence
  // we cannot trap invalid exceptions going through here.
  pd_adapter->dispatch(call_desc, this);

#else
  try { pd_adapter->dispatch(call_desc, this); }

  catch(CORBA::Exception&) {
    throw;
  }
  catch(omniORB::LOCATION_FORWARD&) {
    throw;
  }

  catch(...) {
    if( omniORB::trace(2) ) {
      omniORB::logger l;
      l << "WARNING -- method \'" << call_desc.op() << "\' raised an unknown\n"
	" exception (not a legal CORBA exception).\n";
    }
    OMNIORB_THROW(UNKNOWN,0, CORBA::COMPLETED_MAYBE);
  }
#endif
}


void
omniLocalIdentity::dispatch(GIOP_S& giop_s)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(omni::internalLock, 1);
  OMNIORB_ASSERT(pd_adapter && pd_servant);

  omniLocalIdentity_RefHolder rh(this);

  omni::remoteInvocationCount++;

  pd_adapter->dispatch(giop_s, this);
}


void
omniLocalIdentity::finishedWithDummyId()
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(omni::internalLock, 1);
  OMNIORB_ASSERT(!pd_servant && !pd_adapter && !pd_localRefs);

  omniORB::logs(15, "Dummy omniLocalIdentity deleted (no more local refs).");

  delete this;
}


void
omniLocalIdentity::die()
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(omni::internalLock, 0);
  OMNIORB_ASSERT(pd_nInvocations == 0);
  OMNIORB_ASSERT(pd_localRefs == 0);

  // The servant is cleaned up by the caller.
  // We do not hold a ref to the adapter.

  omniORB::logs(15, "omniLocalIdentity deleted.");

  delete this;
}


void
omniLocalIdentity::gainObjRef(omniObjRef* objref)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(omni::internalLock, 1);
  OMNIORB_ASSERT(objref);

  *(objref->_addrOfLocalRefList()) = pd_localRefs;
  pd_localRefs = objref;
}


void
omniLocalIdentity::loseObjRef(omniObjRef* objref)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(omni::internalLock, 1);
  OMNIORB_ASSERT(objref);

  omniObjRef** p = &pd_localRefs;

  while( *p != objref ) {
    OMNIORB_ASSERT(*p);
    p = (*p)->_addrOfLocalRefList();
  }

  *p = *(*p)->_addrOfLocalRefList();
  *(objref->_addrOfLocalRefList()) = 0;
}
