// -*- Mode: C++; -*-
//                            Package   : omniORB
// inProcessIdentity.cc       Created on: 16/05/2001
//                            Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2001 AT&T Laboratories Cambridge
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
//   Identity for objects in the caller's address space which cannot
//   be called directly. This can be because they are not activated
//   yet, because they are using DSI, or because they are in a
//   different language to the caller.

/*
 $Log$
 Revision 1.1.2.3  2001/08/15 10:26:12  dpg1
 New object table behaviour, correct POA semantics.

 Revision 1.1.2.2  2001/08/03 17:41:21  sll
 System exception minor code overhaul. When a system exeception is raised,
 a meaning minor code is provided.

 Revision 1.1.2.1  2001/05/29 17:03:51  dpg1
 In process identity.

*/

#include <omniORB4/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <inProcessIdentity.h>
#include <objectTable.h>
#include <omniORB4/callDescriptor.h>
#include <omniORB4/callHandle.h>
#include <objectAdapter.h>
#include <exceptiondefs.h>


OMNI_NAMESPACE_BEGIN(omni)

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

class omniInProcessIdentity_RefHolder {
public:
  inline omniInProcessIdentity_RefHolder(omniInProcessIdentity* id)
    : pd_id(id)
  {
    pd_id->pd_refCount++;
    omni::internalLock->unlock();
  }

  inline ~omniInProcessIdentity_RefHolder() {
    omni::internalLock->lock();
    int done = --pd_id->pd_refCount > 0;
    omni::internalLock->unlock();
    if( done )  return;
    delete pd_id;
  }

private:
  omniInProcessIdentity* pd_id;
};

OMNI_NAMESPACE_END(omni)

OMNI_USING_NAMESPACE(omni)

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void
omniInProcessIdentity::dispatch(omniCallDescriptor& call_desc)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);

  omniInProcessIdentity_RefHolder rh(this);
  // omni::internalLock has been released by RefHolder constructor

  if( omniORB::traceInvocations ) {
    omniORB::logger l;
    l << "Invoke '" << call_desc.op() << "' on in process: " << this << '\n';
  }

#ifdef HAS_Cplusplus_catch_exception_by_base
  try {
#endif
    // Can we find the object in the local object table?
    if (keysize() < 0)
      OMNIORB_THROW(OBJECT_NOT_EXIST,OBJECT_NOT_EXIST_NoMatch,
		    CORBA::COMPLETED_NO);

    CORBA::ULong hash = omni::hash(key(), keysize());

    omni::internalLock->lock();
    omniLocalIdentity* id;
    id = omniObjTable::locateActive(key(), keysize(), hash, 1);

    if (id) {
      // Found an entry in the object table. Either the servant has
      // been activated since this InProcessIdentity was created, or
      // the object reference invoked upon is incompatible with the
      // servant (or both).

      if (call_desc.haslocalCallFn() &&
	  id->servant()->
	    _ptrToInterface(call_desc.objref()->_localServantTarget())) {

	// The call descriptor has a local call function and the
	// servant is compatible with the objref, so set the objref's
	// identity to the local identity, and dispatch directly.
	if (omniORB::trace(15)) {
	  omniORB::logger l;
	  l << this << " is now active. Using local identity.\n";
	}
	call_desc.objref()->_setIdentity(id);
	id->dispatch(call_desc);
	return;
      }
      // Can't do a fast local call -- create a callHandle object and
      // dispatch with that.
      omniCallHandle call_handle(&call_desc, 0);
      id->dispatch(call_handle);
      return;
    }

    omni::internalLock->unlock();

    // Can we create a suitable object on demand?

    omniObjAdapter_var adapter(omniObjAdapter::getAdapter(key(),keysize()));

    if (adapter) {
      omniCallHandle call_handle(&call_desc, 1);
      adapter->dispatch(call_handle, key(), keysize());
      return;
    }

    // Oh dear.

    OMNIORB_THROW(OBJECT_NOT_EXIST,OBJECT_NOT_EXIST_NoMatch,
		  CORBA::COMPLETED_NO);
  }
#ifdef HAS_Cplusplus_catch_exception_by_base
  catch (CORBA::Exception&) {
    throw;
  }
  catch (omniORB::LOCATION_FORWARD&) {
    throw;
  }
  catch (...){
    if (omniORB::trace(2)) {
      omniORB::logger l;
      l << "WARNING -- method \'" << call_desc.op() << "\' raised an unknown\n"
	" exception (not a legal CORBA exception).\n";
    }
    OMNIORB_THROW(UNKNOWN,UNKNOWN_UserException, CORBA::COMPLETED_MAYBE);
  }
#endif
}


void
omniInProcessIdentity::gainRef(omniObjRef*)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);

  pd_refCount++;
}


void
omniInProcessIdentity::loseRef(omniObjRef*)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);

  if( --pd_refCount > 0 )  return;

  delete this;
}


void
omniInProcessIdentity::locateRequest() {
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);

  CORBA::ULong hash = omni::hash(key(), keysize());
  omniLocalIdentity* lid = omniObjTable::locateActive(key(),keysize(),hash,1);
  if (lid) {
    // Object exists
    omni::internalLock->unlock();
    return;
  }
  // Is there an object adapter which can activate it on demand?  This
  // may activate the adapter.
  omni::internalLock->unlock();

  omniObjAdapter_var adapter(omniObjAdapter::getAdapter(key(), keysize()));
  if (adapter && adapter->objectExists(key(), keysize()))
    return;
  
  OMNIORB_THROW(OBJECT_NOT_EXIST, 0, CORBA::COMPLETED_NO);
}


omniIdentity::equivalent_fn
omniInProcessIdentity::get_real_is_equivalent() const {
  return real_is_equivalent;
}

CORBA::Boolean
omniInProcessIdentity::real_is_equivalent(const omniIdentity* id1,
					  const omniIdentity* id2) {

  const CORBA::Octet* key1 = id1->key();
  int keysize1             = id1->keysize();

  const CORBA::Octet* key2 = id2->key();
  int keysize2             = id2->keysize();

  if (keysize1 != keysize2 || memcmp((void*)key1,(void*)key2,keysize1) != 0)
      // Object keys do not match
      return 0;

  return 1;
}

_CORBA_Boolean
omniInProcessIdentity::inThisAddressSpace()
{
  return 1;
}

void*
omniInProcessIdentity::thisClassCompare(omniIdentity* id, void* vfn)
{
  classCompare_fn fn = (classCompare_fn)vfn;

  if (fn == omniInProcessIdentity::thisClassCompare)
    return (omniInProcessIdentity*)id;

  return 0;
}
