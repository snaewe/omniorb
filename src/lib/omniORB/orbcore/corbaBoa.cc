// -*- Mode: C++; -*-
//                            Package   : omniORB
// corbaBoa.cc                Created on: 23/7/99
//                            Author    : David Riddoch (djr)
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
//    Implementation of the BOA interface.
//	

/*
  $Log$
  Revision 1.16.2.19  2003/07/25 16:05:42  dgrisby
  BOA locking bug.

  Revision 1.16.2.18  2003/07/02 11:01:05  dgrisby
  Race condition in POA destruction.

  Revision 1.16.2.17  2003/02/17 02:03:08  dgrisby
  vxWorks port. (Thanks Michael Sturm / Acterna Eningen GmbH).

  Revision 1.16.2.16  2002/05/29 14:28:45  dgrisby
  Bug using identity after deletion in BOA. Reported by Tihomir Sokcevic.

  Revision 1.16.2.15  2002/01/16 11:31:58  dpg1
  Race condition in use of registerNilCorbaObject/registerTrackedObject.
  (Reported by Teemu Torma).

  Revision 1.16.2.14  2002/01/15 16:38:12  dpg1
  On the road to autoconf. Dependencies refactored, configure.ac
  written. No makefiles yet.

  Revision 1.16.2.13  2001/10/19 11:06:44  dpg1
  Principal support for GIOP 1.0. Correct some spelling mistakes.

  Revision 1.16.2.12  2001/09/20 09:27:43  dpg1
  Remove assertion failure on exit if not all POAs are deleted.

  Revision 1.16.2.11  2001/09/19 17:26:47  dpg1
  Full clean-up after orb->destroy().

  Revision 1.16.2.10  2001/08/17 17:14:09  sll
  Relocated old omniORB API implementation to this file.

  Revision 1.16.2.9  2001/08/15 10:26:12  dpg1
  New object table behaviour, correct POA semantics.

  Revision 1.16.2.8  2001/08/03 17:41:18  sll
  System exception minor code overhaul. When a system exeception is raised,
  a meaning minor code is provided.

  Revision 1.16.2.7  2001/06/08 17:12:21  dpg1
  Merge all the bug fixes from omni3_develop.

  Revision 1.16.2.6  2001/05/31 16:18:12  dpg1
  inline string matching functions, re-ordered string matching in
  _ptrToInterface/_ptrToObjRef

  Revision 1.16.2.5  2001/05/29 17:03:51  dpg1
  In process identity.

  Revision 1.16.2.4  2001/04/18 18:18:10  sll
  Big checkin with the brand new internal APIs.

  Revision 1.16.2.3  2000/11/09 12:27:56  dpg1
  Huge merge from omni3_develop, plus full long long from omni3_1_develop.

  Revision 1.16.2.2  2000/09/27 17:53:27  sll
  Updated to identify the ORB as omniORB4.

  Revision 1.16.2.1  2000/07/17 10:35:51  sll
  Merged from omni3_develop the diff between omni3_0_0_pre3 and omni3_0_0.

  Revision 1.17  2000/07/13 15:25:58  dpg1
  Merge from omni3_develop for 3.0 release.

  Revision 1.13.6.14  2000/06/22 10:40:13  dpg1
  exception.h renamed to exceptiondefs.h to avoid name clash on some
  platforms.

  Revision 1.13.6.13  2000/06/22 09:01:29  djr
  Fixed assertion failure (locking bug).

  Revision 1.13.6.12  2000/06/02 14:20:15  dpg1
  Using boa_lock for the nil BOA's condition variable caused an
  assertion failure on exit.

  Revision 1.13.6.11  2000/04/27 10:42:08  dpg1
  Interoperable Naming Service

  omniInitialReferences::get() renamed to omniInitialReferences::resolve().

  Revision 1.13.6.10  2000/01/27 10:55:45  djr
  Mods needed for powerpc_aix.  New macro OMNIORB_BASE_CTOR to provide
  fqname for base class constructor for some compilers.

  Revision 1.13.6.9  1999/10/29 13:18:15  djr
  Changes to ensure mutexes are constructed when accessed.

  Revision 1.13.6.8  1999/10/27 17:32:10  djr
  omni::internalLock and objref_rc_lock are now pointers.

  Revision 1.13.6.7  1999/10/14 16:22:05  djr
  Implemented logging when system exceptions are thrown.

  Revision 1.13.6.6  1999/10/04 17:08:31  djr
  Some more fixes/MSVC work-arounds.

  Revision 1.13.6.5  1999/09/30 12:24:48  djr
  Implemented the '_interface' operation for BOA servants.

  Revision 1.13.6.4  1999/09/28 10:54:32  djr
  Removed pretty-printing of object keys from object adapters.

  Revision 1.13.6.3  1999/09/24 17:11:11  djr
  New option -ORBtraceInvocations and omniORB::traceInvocations.

  Revision 1.13.6.2  1999/09/24 10:27:30  djr
  Improvements to ORB and BOA options.

  Revision 1.13.6.1  1999/09/22 14:26:44  djr
  Major rewrite of orbcore to support POA.

*/

#define ENABLE_CLIENT_IR_SUPPORT
#include <omniORB4/CORBA.h>
#include <omniORB4/IOP_S.h>
#include <corbaBoa.h>
#include <omniORB4/callDescriptor.h>
#include <omniORB4/callHandle.h>
#include <omniORB4/objTracker.h>
#include <objectTable.h>
#include <initRefs.h>
#include <dynamicLib.h>
#include <exceptiondefs.h>
#include <orbParameters.h>
#include <stdio.h>
#include <stdlib.h>

#if defined(UnixArchitecture) || defined(__VMS)
#  ifdef __vxWorks__
#    include <time.h>
#  else
#    include <sys/time.h>
#  endif
#  include <unistd.h>
#elif defined(NTArchitecture)
#  include <sys/types.h>
#  include <sys/timeb.h>
#  include <process.h>
#endif

OMNI_USING_NAMESPACE(omni)

static const char* boa_ids[] = { "omniORB4_BOA",
				 "omniORB3_BOA", 
				 "omniORB2_BOA", 
				 0 };

static omniOrbBOA*                       the_boa = 0;
static omni_tracedmutex                  boa_lock;
static omniORB::loader::mapKeyToObject_t MapKeyToObjectFunction = 0;


static
CORBA::Boolean
parse_BOA_args(int &argc,char **argv,const char *boa_identifier);

//////////////////////////////////////////////////////////////////////
///////////////////////////// CORBA::BOA /////////////////////////////
//////////////////////////////////////////////////////////////////////

CORBA::BOA::~BOA() {}


CORBA::BOA_ptr
CORBA::ORB::BOA_init(int& argc, char** argv, const char* boa_identifier)
{
  omni_tracedmutex_lock sync(boa_lock);

  if( the_boa ) {
    the_boa->incrRefCount_locked();
    return the_boa;
  }

  if( !parse_BOA_args(argc, argv, boa_identifier) )
    OMNIORB_THROW(INITIALIZE,INITIALIZE_InvalidORBInitArgs,
		  CORBA::COMPLETED_NO);

  try {
    omniObjAdapter::initialise();
  }
  catch(...) {
    OMNIORB_THROW(INITIALIZE,INITIALIZE_FailedBOAInit, CORBA::COMPLETED_NO);
  }

  the_boa = new omniOrbBOA(0 /* not nil */);
  the_boa->incrRefCount_locked();
  return the_boa;
}


CORBA::BOA_ptr
CORBA::BOA::getBOA()
{
  boa_lock.lock();
  omniOrbBOA* boa = the_boa;
  if( boa )  boa->incrRefCount_locked();
  boa_lock.unlock();

  if( !boa )  OMNIORB_THROW(OBJ_ADAPTER,OBJ_ADAPTER_BOANotInitialised, 
			    CORBA::COMPLETED_NO);
  return boa;
}


CORBA::BOA_ptr 
CORBA::BOA::_duplicate(CORBA::BOA_ptr obj)
{
  if( !CORBA::is_nil(obj) )  obj->_NP_incrRefCount();

  return obj;
}


CORBA::BOA_ptr
CORBA::BOA::_narrow(CORBA::Object_ptr obj)
{
  if( CORBA::is_nil(obj) || !obj->_NP_is_pseudo() )  return _nil();

  BOA_ptr p = (BOA_ptr) obj->_ptrToObjRef(_PD_repoId);

  if( p )  p->_NP_incrRefCount();

  return p ? p : _nil();
}


CORBA::BOA_ptr
CORBA::BOA::_nil()
{
  static omniOrbBOA* _the_nil_ptr = 0;
  if( !_the_nil_ptr ) {
    omni::nilRefLock().lock();
    if( !_the_nil_ptr ) {
      _the_nil_ptr = new omniOrbBOA(1 /* is nil */);
      registerNilCorbaObject(_the_nil_ptr);
    }
    omni::nilRefLock().unlock();
  }
  return _the_nil_ptr;
}


const char* CORBA::BOA::_PD_repoId = "IDL:omg.org/CORBA/BOA:1.0";

//////////////////////////////////////////////////////////////////////
///////////////////////////// omniOrbBOA /////////////////////////////
//////////////////////////////////////////////////////////////////////

#define CHECK_NOT_NIL_OR_DESTROYED()  \
  if( _NP_is_nil() )  _CORBA_invoked_nil_pseudo_ref();  \
  if( pd_state == DESTROYED )  \
    OMNIORB_THROW(OBJECT_NOT_EXIST,OBJECT_NOT_EXIST_BOANotInitialised, \
		  CORBA::COMPLETED_NO);  \


omniOrbBOA::~omniOrbBOA()
{
  if (pd_state_signal) delete pd_state_signal;
}


omniOrbBOA::omniOrbBOA(int nil)
  : OMNIORB_BASE_CTOR(CORBA::)BOA(nil),
    omniObjAdapter(nil),
    pd_state(IDLE),
    pd_refCount(1),
    pd_activeObjList(0),
    pd_nblocked(0),
    pd_nwaiting(0),
    pd_state_signal(0)
{
  if (!nil)
    pd_state_signal = new omni_tracedcondition(omni::internalLock);

  // NB. If nil, then omni::internalLock may be zero, so we cannot use
  // it to initialise the condition variable. However, since the
  // condition variable will never be used, we don't bother to create
  // it.
}


void
omniOrbBOA::impl_is_ready(CORBA::ImplementationDef_ptr,
			  CORBA::Boolean dont_block)
{
  if( _NP_is_nil() )  _CORBA_invoked_nil_pseudo_ref();

  int state_changed = 0;
  boa_lock.lock();
  {
    omni::internalLock->lock();

    switch( pd_state ) {
    case IDLE:
      state_changed = 1;
      pd_state = ACTIVE;
      break;

    case ACTIVE:
      break;

    case DESTROYED:
      omni::internalLock->unlock();
      boa_lock.unlock();
      OMNIORB_THROW(OBJECT_NOT_EXIST,OBJECT_NOT_EXIST_BOANotInitialised, 
		    CORBA::COMPLETED_NO);
      break;
    }

    omni::internalLock->unlock();
  }

  if( state_changed ) {
    try { adapterActive(); }
    catch(...) {
      boa_lock.unlock();
      throw;
    }

    // Wake-up anyone stuck in synchronise_request().
    if( /* anyone stuck */ 1 )  pd_state_signal->broadcast();
  }

  if( !dont_block ) {
    pd_nblocked++;

    omni::internalLock->lock();
    boa_lock.unlock();
    pd_state_signal->wait();
    omni::internalLock->unlock();

    boa_lock.lock();
    --pd_nblocked;
    boa_lock.unlock();
  }
  else
    boa_lock.unlock();
}


void
omniOrbBOA::impl_shutdown()
{
  if( _NP_is_nil() )  _CORBA_invoked_nil_pseudo_ref();

  int wake_blockers = 0;

  {
    omni_tracedmutex_lock sync(boa_lock);
    int state_changed = 0;

    omni::internalLock->lock();

    switch( pd_state ) {
    case IDLE:
      break;

    case ACTIVE:
      state_changed = 1;
      pd_state = IDLE;
      break;

    case DESTROYED:
      omni::internalLock->unlock();
      OMNIORB_THROW(OBJECT_NOT_EXIST,OBJECT_NOT_EXIST_BOANotInitialised,
		    CORBA::COMPLETED_NO);
      break;
    }

    omni::internalLock->unlock();

    wake_blockers = state_changed && pd_nblocked > 0;

    if( state_changed ) {
      try { adapterInactive(); }
      catch(...) {
	if( wake_blockers )  pd_state_signal->broadcast();
	throw;
      }
    }
  }

  // We cannot wait for request completion -- since if this is
  // called from a CORBA operation we would block forever.
  //  However, if the application is using the BOA only, then
  // adapterInactive will give us the behaviour we desire.
  // There is only a problem if using both POA and BOA.

  // Wake-up anyone stuck in impl_is_ready().
  if( wake_blockers )  pd_state_signal->broadcast();
}


void
omniOrbBOA::destroy()
{
  if( _NP_is_nil() )  _CORBA_invoked_nil_pseudo_ref();

  omniOrbBOA* boa = 0;
  omniObjTableEntry* obj_list = 0;
  int do_inactive = 0;

  {
    boa_lock.lock();
    {
      omni::internalLock->lock();

      switch( pd_state ) {
      case IDLE:
	pd_state = DESTROYED;
	break;

      case ACTIVE:
	do_inactive = 1;
	pd_state = DESTROYED;
	break;

      case DESTROYED:
	omni::internalLock->unlock();
	boa_lock.unlock();
	OMNIORB_THROW(OBJECT_NOT_EXIST,OBJECT_NOT_EXIST_BOANotInitialised,
		      CORBA::COMPLETED_NO);
	break;
      }

      omni::internalLock->unlock();
    }

    OMNIORB_ASSERT(the_boa == this);
    boa = the_boa;
    the_boa = 0;

    // Grab the list of active objects.
    if( pd_activeObjList )  pd_activeObjList->reRootOAObjList(&obj_list);

    boa_lock.unlock();
  }

  // This can go outside of the <boa_lock> critical section,
  // since once we are in the DESTROYED state, we will never
  // return to the active state.
  if( do_inactive ) {
    try { adapterInactive(); }
    catch(...) {}
  }

  // Remove all my objects from the object table.
  omni::internalLock->lock();
  omniObjTableEntry* entry = obj_list;
  while( entry ) {
    while (entry->state() == omniObjTableEntry::ACTIVATING)
      entry->wait(omniObjTableEntry::ACTIVE |
		  omniObjTableEntry::DEACTIVATING |
		  omniObjTableEntry::ETHEREALISING);

    if (entry->state() == omniObjTableEntry::ACTIVE)
      entry->setDeactivatingOA();

    entry = entry->nextInOAObjList();
  }

  // We need to kick anyone stuck in synchronise_request(),
  // or impl_is_ready().
  pd_state_signal->broadcast();

  // Wait until outstanding invocations have completed.
  waitForAllRequestsToComplete(1);

  entry = obj_list;
  while( entry ) {
    if (entry->state() & omniObjTableEntry::DEACTIVATING)
      entry->setEtherealising();

    OMNIORB_ASSERT(entry->is_idle());
    entry = entry->nextInOAObjList();
  }

  // Kill the identities, but not the servants themselves.
  // (See user's guide 5.4).
  entry = obj_list;
  while( entry ) {
    OMNIORB_ASSERT(entry->is_idle());

    omniObjTableEntry* next = entry->nextInOAObjList();
    entry->setDead();
    entry = next;
  }

  omni::internalLock->unlock();

  // Wait for objects which have been detached to complete
  // their etherealisations.
  wait_for_detached_objects();

  adapterDestroyed();
  CORBA::release(boa);
}


void 
omniOrbBOA::obj_is_ready(omniOrbBoaServant* servant,
			 CORBA::ImplementationDef_ptr /* ignored */)
{
  CHECK_NOT_NIL_OR_DESTROYED();
  if( !servant )  OMNIORB_THROW(BAD_PARAM,BAD_PARAM_InvalidServant,
				CORBA::COMPLETED_NO);

  servant->_obj_is_ready();
}


void 
omniOrbBOA::obj_is_ready(CORBA::Object_ptr, CORBA::ImplementationDef_ptr)
{
  CHECK_NOT_NIL_OR_DESTROYED();

  omniORB::logs(1, "CORBA::BOA::obj_is_ready() is not supported.  Use\n"
		" _obj_is_ready(boa) on the object implementation instead.");

  OMNIORB_THROW(NO_IMPLEMENT, NO_IMPLEMENT_Unsupported, CORBA::COMPLETED_NO);
}


void
omniOrbBOA::dispose(CORBA::Object_ptr obj)
{
  if( _NP_is_nil() )  _CORBA_invoked_nil_pseudo_ref();

  if( !obj || obj->_NP_is_nil() )  return;
  if( obj->_NP_is_pseudo() )
    OMNIORB_THROW(BAD_PARAM,BAD_PARAM_IsPseudoObject, CORBA::COMPLETED_NO);

  boa_lock.lock();
  omni::internalLock->lock();
  dispose(obj->_PR_getobj()->_identity());
  // The locks will have been released on return.
}


CORBA::Object_ptr
omniOrbBOA::create(const CORBA::ReferenceData& ref,
		   CORBA::_objref_InterfaceDef* intf,
		   CORBA::ImplementationDef_ptr impl)
{
  OMNIORB_THROW(NO_IMPLEMENT, NO_IMPLEMENT_Unsupported, CORBA::COMPLETED_NO);
  return 0;
}


CORBA::ReferenceData*
omniOrbBOA::get_id(CORBA::Object_ptr obj)
{
  OMNIORB_THROW(NO_IMPLEMENT, NO_IMPLEMENT_Unsupported, CORBA::COMPLETED_NO);
  return 0;
}


void
omniOrbBOA::change_implementation(CORBA::Object_ptr obj,
				  CORBA::ImplementationDef_ptr impl)
{
  OMNIORB_THROW(NO_IMPLEMENT, NO_IMPLEMENT_Unsupported, CORBA::COMPLETED_NO);
  return;
}


CORBA::Principal_ptr
omniOrbBOA::get_principal(CORBA::Object_ptr obj, CORBA::Environment_ptr env)
{
  OMNIORB_THROW(NO_IMPLEMENT, NO_IMPLEMENT_Unsupported, CORBA::COMPLETED_NO);
  return 0;
}


void
omniOrbBOA::deactivate_impl(CORBA::ImplementationDef_ptr impl)
{
  OMNIORB_THROW(NO_IMPLEMENT, NO_IMPLEMENT_Unsupported, CORBA::COMPLETED_NO);
}


void
omniOrbBOA::deactivate_obj(CORBA::Object_ptr obj)
{
  OMNIORB_THROW(NO_IMPLEMENT, NO_IMPLEMENT_Unsupported, CORBA::COMPLETED_NO);
}

///////////////////
// CORBA::Object //
///////////////////

_CORBA_Boolean
omniOrbBOA::_non_existent()
{
  if( _NP_is_nil() )  _CORBA_invoked_nil_pseudo_ref();

  omni::internalLock->lock();
  _CORBA_Boolean ret = pd_state == DESTROYED ? 1 : 0;
  omni::internalLock->unlock();

  return ret;
}


void*
omniOrbBOA::_ptrToObjRef(const char* repoId)
{
  OMNIORB_ASSERT(repoId);

  if( omni::ptrStrMatch(repoId, CORBA::BOA::_PD_repoId) )
    return (CORBA::BOA_ptr) this;
  if( omni::ptrStrMatch(repoId, CORBA::Object::_PD_repoId) )
    return (CORBA::Object_ptr) this;

  return 0;
}


void
omniOrbBOA::_NP_incrRefCount()
{
  boa_lock.lock();
  pd_refCount++;
  boa_lock.unlock();
}


void
omniOrbBOA::_NP_decrRefCount()
{
  boa_lock.lock();
  int done = --pd_refCount > 0;
  boa_lock.unlock();
  if( done )  return;

  omniORB::logs(15, "No more references to the BOA -- deleted.");

  delete this;
}

/////////////////////////////
// Override omniObjAdapter //
/////////////////////////////

void
omniOrbBOA::incrRefCount()
{
  boa_lock.lock();
  pd_refCount++;
  boa_lock.unlock();
}


void
omniOrbBOA::decrRefCount()
{
  boa_lock.lock();
  int done = --pd_refCount > 0;
  boa_lock.unlock();
  if( done )  return;

  omniORB::logs(15, "No more references to the BOA -- deleted.");

  delete this;
}


void
omniOrbBOA::dispatch(omniCallHandle& handle, omniLocalIdentity* id)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);
  OMNIORB_ASSERT(id);  OMNIORB_ASSERT(id->servant());
  OMNIORB_ASSERT(id->adapter() == this);

  enterAdapter();

  if( pd_state != ACTIVE )  synchronise_request();

  startRequest();

  omni::internalLock->unlock();

  if( omniORB::traceInvocations ) {
    omniORB::logger l;
    l << "Dispatching remote call \'" 
      << handle.operation_name() << "\' to: "
      << id << '\n';
  }

  if( !id->servant()->_dispatch(handle) ) {
    if( !id->servant()->omniServant::_dispatch(handle) ) {
      handle.SkipRequestBody();
      OMNIORB_THROW(BAD_OPERATION,BAD_OPERATION_UnRecognisedOperationName,
		    CORBA::COMPLETED_NO);
    }
  }
}


void
omniOrbBOA::dispatch(omniCallHandle& handle,
		     const CORBA::Octet* key, int keysize)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 0);
  OMNIORB_ASSERT(key && keysize == sizeof(omniOrbBoaKey));

  omniORB::loader::mapKeyToObject_t loader = MapKeyToObjectFunction;
  if( !loader )  OMNIORB_THROW(OBJECT_NOT_EXIST,
			       OBJECT_NOT_EXIST_NoMatch, CORBA::COMPLETED_NO);

  omniOrbBoaKey k;
  memcpy(&k, key, sizeof(omniOrbBoaKey));

  CORBA::Object_ptr obj = loader(k);

  if( CORBA::is_nil(obj) )  
    OMNIORB_THROW(OBJECT_NOT_EXIST,
		  OBJECT_NOT_EXIST_NoMatch, CORBA::COMPLETED_NO);
  else
    throw omniORB::LOCATION_FORWARD(obj,0);
}


void
omniOrbBOA::dispatch(omniCallDescriptor& call_desc, omniLocalIdentity* id)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);
  OMNIORB_ASSERT(id);  OMNIORB_ASSERT(id->servant());
  OMNIORB_ASSERT(id->adapter() == this);

  enterAdapter();

  if( pd_state != ACTIVE )  synchronise_request();

  startRequest();

  omni::internalLock->unlock();

  if( omniORB::traceInvocations ) {
    omniORB::logger l;
    l << "Dispatching local call \'" << call_desc.op() << "\' to "
      << id << '\n';
  }

  call_desc.doLocalCall(id->servant());
}


int
omniOrbBOA::objectExists(const _CORBA_Octet* key, int keysize)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 0);
  OMNIORB_ASSERT(key && keysize == sizeof(omniOrbBoaKey));

  omniORB::loader::mapKeyToObject_t loader = MapKeyToObjectFunction;
  if( !loader )  return 0;

  omniOrbBoaKey k;
  memcpy(&k, key, sizeof(omniOrbBoaKey));

  CORBA::Object_ptr obj = loader(k);

  if( !CORBA::is_nil(obj) )  throw omniORB::LOCATION_FORWARD(obj,0);

  return 0;
}


void
omniOrbBOA::lastInvocationHasCompleted(omniLocalIdentity* id)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);

  omniObjTableEntry* entry = omniObjTableEntry::downcast(id);
  OMNIORB_ASSERT(entry);
  // This function should only ever be called with a localIdentity
  // which is an objectTableEntry, since those are the only ones which
  // can be deactivated.

  if (entry->state() == omniObjTableEntry::DEACTIVATING_OA) {
    if (omniORB::trace(15)) {
      omniORB::logger l;
      l << "BOA not etherealising object " << entry <<".\n";
    }
    omni::internalLock->unlock();
    return;
  }

  if( omniORB::trace(15) ) {
    omniORB::logger l;
    l << "BOA etherealising object.\n"
      << " id: " << id->servant()->_mostDerivedRepoId() << "\n";
  }

  omniServant* servant = id->servant();

  entry->setEtherealising();
  entry->setDead();

  omni::internalLock->unlock();
  delete servant;
  met_detached_object();
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

omniOrbBOA*
omniOrbBOA::theBOA()
{
  boa_lock.lock();
  omniOrbBOA* boa = the_boa;
  if( boa )  boa->incrRefCount_locked();
  boa_lock.unlock();

  return boa;
}


void
omniOrbBOA::dispose(omniIdentity* id)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(boa_lock, 1);
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);

  if( pd_state == DESTROYED ) {
    omni::internalLock->unlock();
    boa_lock.unlock();
    OMNIORB_THROW(OBJECT_NOT_EXIST,OBJECT_NOT_EXIST_NoMatch,
		  CORBA::COMPLETED_NO);
  }

  omniObjTableEntry* entry = omniObjTableEntry::downcast(id);

  if( !entry || entry->state() != omniObjTableEntry::ACTIVE ) {
    omni::internalLock->unlock();
    boa_lock.unlock();
    return;
  }

  entry->setDeactivating();
  entry->removeFromOAObjList();

  if( entry->is_idle() ) {
    detached_object();
    boa_lock.unlock();
    lastInvocationHasCompleted(entry);
  }
  else {
    // When outstanding requests have completed the object
    // will be etherealised.
    omni::internalLock->unlock();
    detached_object();
    boa_lock.unlock();

    omniORB::logs(15, "Object is still busy -- etherealise later.");
  }
}


void
omniOrbBOA::synchronise_request()
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);

  // Wait until the request can proceed, or discard it.

  // We limit the number of threads stuck here, so as to more
  // closely match the semantics of the old BOA -- where
  // clients would be stuck in a queue on the socket, waiting
  // to be allowed to connect.
  //  NB. If only the BOA is used, then the old semantics are
  // preserved anyway.  This will only apply when the BOA and
  // POA are used together.
  if( pd_nwaiting == /*??* max */ 5 ) {
    startRequest();
    omni::internalLock->unlock();
    OMNIORB_THROW(TRANSIENT,TRANSIENT_POANoResource, CORBA::COMPLETED_NO);
  }

  pd_nwaiting++;
  while( pd_state == IDLE )  pd_state_signal->wait();
  pd_nwaiting--;

  switch( pd_state ) {
  case IDLE: // get rid of warnings!
  case ACTIVE:
    break;

  case DESTROYED:
    startRequest();
    omni::internalLock->unlock();
    OMNIORB_THROW(OBJECT_NOT_EXIST,OBJECT_NOT_EXIST_BOANotInitialised,
		  CORBA::COMPLETED_NO);
  }
}

//////////////////////////////////////////////////////////////////////
////////////////////////// omniOrbBoaServant /////////////////////////
//////////////////////////////////////////////////////////////////////

omniOrbBoaServant::~omniOrbBoaServant()  {}


omniOrbBoaServant::omniOrbBoaServant()
{
  omniORB::generateNewKey(pd_key);
}


void
omniOrbBoaServant::_dispose()
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 0);
  ASSERT_OMNI_TRACEDMUTEX_HELD(boa_lock, 0);

  boa_lock.lock();

  if( !the_boa ) {
    boa_lock.unlock();
    OMNIORB_THROW(OBJ_ADAPTER,OBJ_ADAPTER_BOANotInitialised,
		  CORBA::COMPLETED_NO);
  }
  // We have to grab a ref to the boa here, since <boa_lock> is
  // released during omniOrbBOA::dispose() -- thus the_boa could
  // be released under our feet!
  the_boa->incrRefCount_locked();
  omniOrbBOA* boa = the_boa;
  CORBA::BOA_var ref_holder(boa);

  omni::internalLock->lock();
  if (_activations().size() != 0) {
    OMNIORB_ASSERT(_activations().size() == 1);
    boa->dispose(_activations()[0]);
  }
}


void
omniOrbBoaServant::_obj_is_ready()
{
  boa_lock.lock();

  if( !the_boa ) {
    boa_lock.unlock();
    OMNIORB_THROW(OBJ_ADAPTER,OBJ_ADAPTER_BOANotInitialised,
		  CORBA::COMPLETED_NO);
  }

  omniObjKey key((const CORBA::Octet*) &pd_key, sizeof(omniOrbBoaKey));

  omni::internalLock->lock();

  omniObjTableEntry* entry = omniObjTable::newEntry(key);

  if( !entry ) {
    omni::internalLock->unlock();
    boa_lock.unlock();
    OMNIORB_THROW(OBJECT_NOT_EXIST,
		  OBJECT_NOT_EXIST_NoMatch, CORBA::COMPLETED_NO);
  }

  entry->setActive(this, the_boa);

  omni::internalLock->unlock();
  entry->insertIntoOAObjList(the_boa->activeObjList());
  boa_lock.unlock();
}


void*
omniOrbBoaServant::_this(const char* repoId)
{
  OMNIORB_ASSERT(repoId);

  omni::internalLock->lock();

  omniObjRef* objref;

  if (_activations().size() > 0) {
    omniObjTableEntry* entry = _activations()[0];
    
    objref = omni::createLocalObjRef(_mostDerivedRepoId(), repoId, entry);
  }
  else {
    objref = omni::createLocalObjRef(_mostDerivedRepoId(), repoId,
				     (const CORBA::Octet*)&pd_key,
				     sizeof(omniOrbBoaKey));
  }
  omni::internalLock->unlock();

  OMNIORB_ASSERT(objref);

  return objref->_ptrToObjRef(repoId);
}


omniObjRef*
omniOrbBoaServant::_do_get_interface()
{
  // repoId should not be empty for statically defined
  // servants.  We do not support dynamic BOA servants.
  const char* repoId = _mostDerivedRepoId();
  OMNIORB_ASSERT(repoId && *repoId);

  // Obtain the object reference for the interface repository.
  CORBA::Object_var repository = CORBA::Object::_nil();
  try {
    repository = omniInitialReferences::resolve("InterfaceRepository");
  }
  catch (...) {
  }
  if( CORBA::is_nil(repository) )
    OMNIORB_THROW(INTF_REPOS,INTF_REPOS_NotAvailable, CORBA::COMPLETED_NO);

  // Make a call to the interface repository.
  omniStdCallDesc::_cCORBA_mObject_i_cstring
    call_desc(omniDynamicLib::ops->lookup_id_lcfn, "lookup_id", 10, repoId);
  repository->_PR_getobj()->_invoke(call_desc);

  return call_desc.result() ? call_desc.result()->_PR_getobj() : 0;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

static
CORBA::Boolean
isValidId(const char* id) {
  const char** p = boa_ids;
  while (*p) {
    if (strcmp(*p,id) == 0) return 1;
    p++;
  }
  return 0;
}

static
const char*
myBoaId() {
  return boa_ids[0];
}

static
void
move_args(int& argc,char **argv,int idx,int nargs)
{
  if ((idx+nargs) <= argc)
    {
      for (int i=idx+nargs; i < argc; i++) {
	argv[i-nargs] = argv[i];
      }
      argc -= nargs;
    }
}


static
CORBA::Boolean
parse_BOA_args(int& argc, char** argv, const char* boa_identifier)
{
  CORBA::Boolean boaId_match = 0;
  if (boa_identifier) {
    if (!isValidId(boa_identifier)) {
      if (omniORB::trace(1)) {
	omniORB::logger l;
	l << "BOA_init failed -- the BOAid (" << boa_identifier << ")"
	  " is not " << myBoaId() << "\n";
      }
      return 0;
    }
    if( omniORB::trace(1) && strcmp(boa_identifier, myBoaId()) ) {
      if( omniORB::trace(1) ) {
	omniORB::logger l;
	l << "WARNING -- using BOAid " << boa_identifier 
	  << " (should be " << myBoaId() << ")." << "\n";
      }
    }
  }

  int idx = 1;
  while (argc > idx)
    {
      // -BOAxxxxxxxx ?
      if (strlen(argv[idx]) < 4 ||
	  !(argv[idx][0] == '-' && argv[idx][1] == 'B' &&
	    argv[idx][2] == 'O' && argv[idx][3] == 'A'))
	{
	  idx++;
	  continue;
	}
	  
      // -BOAid <id>
      if (strcmp(argv[idx],"-BOAid") == 0) {
	if ((idx+1) >= argc) {
	  omniORB::logs(1, "BOA_init failed: missing -BOAid parameter.");
	  return 0;
	}
	if (!isValidId(argv[idx+1]) )
	  {
	    if ( omniORB::trace(1) ) {
	      omniORB::logger l;
	      l << "BOA_init failed -- the BOAid (" <<
		argv[idx+1] << ") is not " << myBoaId() << "\n";
	    }
	    return 0;
	  }
	if( strcmp(argv[idx + 1], myBoaId()) ) {
	  if( omniORB::trace(1) ) {
	    omniORB::logger l;
	    l << "WARNING -- using BOAid " << boa_identifier 
	      << " (should be " << myBoaId() << ")." << "\n";
	  }
	}
	boaId_match = 1;
	move_args(argc,argv,idx,2);
	continue;
      }

      // -BOAno_bootstrap_agent
      if (strcmp(argv[idx],"-BOAno_bootstrap_agent") == 0) {
	orbParameters::supportBootstrapAgent = 0;
	move_args(argc,argv,idx,1);
	continue;
      }

      // -BOAiiop_port <port number>[,<port number>]*
      // -BOAiiop_name_port <hostname[:port number]> -- obsoleted options
      if (strcmp(argv[idx],"-BOAiiop_port") == 0 ||
	  strcmp(argv[idx],"-BOAiiop_name_port") == 0) {

	if (omniORB::trace(1)) {
	  omniORB::logger log;
	  log << "BOA_init failed: "
	      << argv[idx] << " is now obsolete, use -ORBendpoint instead.\n";
	}
	return 0;
      }

      // -BOAhelp
      if (strcmp(argv[idx],"-BOAhelp") == 0) {
	omniORB::logger l;
	l <<
	  "Valid -BOA<options> are:\n"
	  "    -BOAid omniORB4_BOA\n"
	  "    -BOAno_bootstrap_agent\n";
	move_args(argc,argv,idx,1);
	continue;
      }

      // Reach here only if the argument in this form: -BOAxxxxx
      // is not recognised.
      if ( omniORB::trace(1) ) {
	omniORB::logger l;
	l << "BOA_init failed: unknown BOA argument (" << argv[idx] << ")\n";
      }
      return 0;
    }

  if (!boa_identifier && !boaId_match) {
    omniORB::logs(1, "BOA_init failed: BOAid is not specified.");
    return 0;
  }
  return 1;
}

//////////////////////////////////////////////////////////////////////
/////////////////////////// omniORB::loader //////////////////////////
//////////////////////////////////////////////////////////////////////

void 
omniORB::loader::set(omniORB::loader::mapKeyToObject_t NewMapKeyToObject) 
{
  MapKeyToObjectFunction = NewMapKeyToObject;
}

//////////////////////////////////////////////////////////////////////
///////////////////////// omniORB::objectKey /////////////////////////
//////////////////////////////////////////////////////////////////////

static omni_tracedmutex key_lock;
static omniORB::objectKey omniORB_seed;


void
omniORB::generateNewKey(omniORB::objectKey& k)
{
  static int first_time = 1;

  omni_tracedmutex_lock sync(key_lock);

  if ( first_time ) 
    {
      first_time = 0;

      // one-time initialisation of the seed value
      // initialise the seed of the objectKey generator
      // Guarantee that no two keys generated on the same machine are the same
      // ever.
#ifdef HAVE_GETTIMEOFDAY
      // Use gettimeofday() to obtain the current time. Use this to
      // initialise the 32-bit field hi and med in the seed.
      // On unices, add the process id to med.
      // Initialise lo to 0.
      struct timeval v;
#ifdef GETTIMEOFDAY_TIMEZONE
      gettimeofday(&v,0);
#else
      gettimeofday(&v);
#endif
      omniORB_seed.hi = v.tv_sec;
      omniORB_seed.med = (v.tv_usec << 12);
#ifdef HAVE_GETPID
      omniORB_seed.med += getpid();
#else
      // without the process id, there is no guarantee that the keys generated
      // by this process do not clash with those generated by other processes.
#endif
      omniORB_seed.lo = 0;
#elif defined(__WIN32__)
      // Unique number on NT
      // Use _ftime() to obtain the current system time. 
#ifndef __BCPLUSPLUS__
      struct _timeb v;
      _ftime(&v);
      omniORB_seed.hi = v.time;
      omniORB_seed.med = v.millitm + _getpid();
      omniORB_seed.lo = 0;
#else
      struct timeb v;
      ftime(&v);
      omniORB_seed.hi = v.time;
      omniORB_seed.med = v.millitm + getpid();
      omniORB_seed.lo = 0;
#endif
#elif defined(__VMS) && __VMS_VER < 70000000
      // VMS systems prior to 7.0
      timeb v;
      ftime(&v);
      omniORB_seed.hi = v.time;
      omniORB_seed.med = v.millitm + getpid();
      omniORB_seed.lo = 0;
#elif defined(__vxWorks__)
      struct timespec v;
      clock_gettime(CLOCK_REALTIME, &v);
      omniORB_seed.hi = v.tv_sec;
      omniORB_seed.med = (v.tv_nsec << 2);
      omniORB_seed.lo = 0;
#endif
      // 
      // Make sure that the three fields are in little-endian, byte swap
      // if necessary.
      if (omni::myByteOrder) {
	// big endian
	omniORB_seed.hi =  ((((omniORB_seed.hi) & 0xff000000) >> 24) | 
			     (((omniORB_seed.hi) & 0x00ff0000) >> 8)  | 
			     (((omniORB_seed.hi) & 0x0000ff00) << 8)  | 
			     (((omniORB_seed.hi) & 0x000000ff) << 24));
	omniORB_seed.med =  ((((omniORB_seed.med) & 0xff000000) >> 24) | 
			     (((omniORB_seed.med) & 0x00ff0000) >> 8)  | 
			     (((omniORB_seed.med) & 0x0000ff00) << 8)  | 
			     (((omniORB_seed.med) & 0x000000ff) << 24));
      }
  }
  omniORB_seed.lo++;  // note: seed.lo is in native endian
  k = omniORB_seed;
  if (omni::myByteOrder) {
    // big endian
    k.lo =  ((((k.lo) & 0xff000000) >> 24) | 
	     (((k.lo) & 0x00ff0000) >> 8)  | 
	     (((k.lo) & 0x0000ff00) << 8)  | 
	     (((k.lo) & 0x000000ff) << 24));
  }
  return;
}


omniORB::objectKey
omniORB::nullkey()
{
  omniORB::objectKey n;
  n.hi = n.med = n.lo = 0;
  return n;
}


#if defined(HAS_Cplusplus_Namespace)
namespace omniORB {
#endif

  // Some compilers which support namespace cannot handle operator==
  // definition prefix by omniORB::. We therefore enclose the
  // operator definitions in namespace omniORB scoping to avoid the problem.

int 
operator==(const omniORB::objectKey &k1,const omniORB::objectKey &k2)
{
  return (k1.hi == k2.hi &&
	  k1.med == k2.med &&
	  k1.lo == k2.lo) ? 1 : 0;
}


int 
operator!=(const omniORB::objectKey &k1,const omniORB::objectKey &k2)
{
  return (k1.hi != k2.hi ||
	  k1.med != k2.med ||
	  k1.lo != k2.lo) ? 1 : 0;
}

#if defined(HAS_Cplusplus_Namespace)
}
#endif


omniORB::seqOctets* 
omniORB::keyToOctetSequence(const omniORB::objectKey &k1)
{
  omniORB::seqOctets* result = new omniORB::seqOctets;
  result->length(sizeof(omniORB::objectKey));
  const CORBA::Octet* p = (const CORBA::Octet*) &k1;
  for (unsigned int i=0; i< sizeof(omniORB::objectKey); i++) {
    result->operator[](i) = p[i];
  }
  return result;
}


omniORB::objectKey
omniORB::octetSequenceToKey(const omniORB::seqOctets& seq)
{
  if (seq.length() != sizeof(omniORB::objectKey)) {
    OMNIORB_THROW(MARSHAL,MARSHAL_SequenceIsTooLong, CORBA::COMPLETED_NO);
  }
  omniORB::objectKey result;
  CORBA::Octet* p = (CORBA::Octet*) &result;
  for (unsigned int i=0; i< sizeof(omniORB::objectKey); i++) {
    p[i] = seq[i];
  }
  return result;
}

