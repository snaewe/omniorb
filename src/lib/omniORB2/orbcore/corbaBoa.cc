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
#include <omniORB3/CORBA.h>
#include <corbaBoa.h>
#include <omniORB3/callDescriptor.h>
#include <localIdentity.h>
#include <initRefs.h>
#include <dynamicLib.h>
#include <exception.h>
#include <stdio.h>
#include <stdlib.h>


#ifndef OMNIORB_USEHOSTNAME_VAR
#define OMNIORB_USEHOSTNAME_VAR "OMNIORB_USEHOSTNAME"
#endif


#define MY_BOA_ID                      "omniORB3_BOA"
#define OLD_BOA_ID                     "omniORB2_BOA"


static omniOrbBOA*                       the_boa = 0;
static omni_tracedmutex                  boa_lock;
static omniORB::loader::mapKeyToObject_t MapKeyToObjectFunction = 0;


static
CORBA::Boolean
parse_BOA_args(int &argc,char **argv,const char *orb_identifier);

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
    OMNIORB_THROW(INITIALIZE,0,CORBA::COMPLETED_NO);

  try {
    omniObjAdapter::initialise();
  }
  catch(...) {
    OMNIORB_THROW(INITIALIZE,0, CORBA::COMPLETED_NO);
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

  if( !boa )  OMNIORB_THROW(OBJ_ADAPTER,0, CORBA::COMPLETED_NO);
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
    if( !_the_nil_ptr )  _the_nil_ptr = new omniOrbBOA(1 /* is nil */);
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
    OMNIORB_THROW(OBJECT_NOT_EXIST,0, CORBA::COMPLETED_NO);  \


omniOrbBOA::~omniOrbBOA()  {}


omniOrbBOA::omniOrbBOA(int nil)
  : OMNIORB_BASE_CTOR(CORBA::)BOA(nil),
    pd_state(IDLE),
    pd_refCount(1),
    pd_activeObjList(0),
    pd_nblocked(0),
    pd_nwaiting(0),
    pd_state_signal(nil ? &boa_lock : omni::internalLock)
{
  // NB. If nil, then omni::internalLock may be zero, so we cannot
  // use it to initialise the condition variable.  So we use
  // &boa_lock instead.  But this will of course never be used...
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
      OMNIORB_THROW(OBJECT_NOT_EXIST,0, CORBA::COMPLETED_NO);
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
    if( /* anyone stuck */ 1 )  pd_state_signal.broadcast();
  }

  if( !dont_block ) {
    pd_nblocked++;

    omni::internalLock->lock();
    boa_lock.unlock();
    pd_state_signal.wait();
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
      OMNIORB_THROW(OBJECT_NOT_EXIST,0, CORBA::COMPLETED_NO);
      break;
    }

    omni::internalLock->unlock();

    wake_blockers = state_changed && pd_nblocked > 0;

    if( state_changed ) {
      try { adapterInactive(); }
      catch(...) {
	if( wake_blockers )  pd_state_signal.broadcast();
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
  if( wake_blockers )  pd_state_signal.broadcast();
}


void
omniOrbBOA::destroy()
{
  if( _NP_is_nil() )  _CORBA_invoked_nil_pseudo_ref();

  omniOrbBOA* boa = 0;
  omniLocalIdentity* obj_list = 0;
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
	OMNIORB_THROW(OBJECT_NOT_EXIST,0, CORBA::COMPLETED_NO);
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
  omniLocalIdentity* id = obj_list;
  while( id ) {
    omni::deactivateObject(id->key(), id->keysize());
    id = id->nextInOAObjList();
  }
  omni::internalLock->unlock();

  // We need to kick anyone stuck in synchronise_request(),
  // or impl_is_ready().
  pd_state_signal.broadcast();

  // Wait until outstanding invocations have completed.
  waitForAllRequestsToComplete(0);

  // Delete the identities, but not the servant itself.
  // (See user's guide 5.4).
  {
    omniLocalIdentity* id = obj_list;
    while( id ) {
      omniLocalIdentity* next = id->nextInOAObjList();
      id->deactivate();
      OMNIORB_ASSERT(id->is_idle());
      id->die();
      id = next;
    }
  }

  // Wait for objects which have been detached to complete
  // their etherealisations.
  wait_for_detached_objects();

  CORBA::release(boa);
}


void 
omniOrbBOA::obj_is_ready(omniOrbBoaServant* servant,
			 CORBA::ImplementationDef_ptr /* ignored */)
{
  CHECK_NOT_NIL_OR_DESTROYED();
  if( !servant )  OMNIORB_THROW(BAD_PARAM,0, CORBA::COMPLETED_NO);

  servant->_obj_is_ready();
}


void 
omniOrbBOA::obj_is_ready(CORBA::Object_ptr, CORBA::ImplementationDef_ptr)
{
  CHECK_NOT_NIL_OR_DESTROYED();

  omniORB::logs(1, "CORBA::BOA::obj_is_ready() is not supported.  Use\n"
		" _obj_is_ready(boa) on the object implementation instead.");

  OMNIORB_THROW(NO_IMPLEMENT,0, CORBA::COMPLETED_NO);
}


void
omniOrbBOA::dispose(CORBA::Object_ptr obj)
{
  if( _NP_is_nil() )  _CORBA_invoked_nil_pseudo_ref();

  if( !obj || obj->_NP_is_nil() )  return;
  if( obj->_NP_is_pseudo() )
    OMNIORB_THROW(BAD_PARAM,0, CORBA::COMPLETED_NO);

  boa_lock.lock();
  omni::internalLock->lock();
  dispose(obj->_PR_getobj()->_localId());
  // The locks will have been released on return.
}


CORBA::Object_ptr
omniOrbBOA::create(const CORBA::ReferenceData& ref,
		   CORBA::_objref_InterfaceDef* intf,
		   CORBA::ImplementationDef_ptr impl)
{
  // XXX not implemented yet
  return 0;
}


CORBA::ReferenceData*
omniOrbBOA::get_id(CORBA::Object_ptr obj)
{
  // XXX not implemented yet
  return 0;
}


void
omniOrbBOA::change_implementation(CORBA::Object_ptr obj,
				  CORBA::ImplementationDef_ptr impl)
{
  // XXX not implemented yet
  return;
}


CORBA::Principal_ptr
omniOrbBOA::get_principal(CORBA::Object_ptr obj, CORBA::Environment_ptr env)
{
  // XXX not implemented yet
  return 0;
}


void
omniOrbBOA::deactivate_impl(CORBA::ImplementationDef_ptr impl)
{
  // XXX not implemented yet
  return;
}


void
omniOrbBOA::deactivate_obj(CORBA::Object_ptr obj)
{
  // XXX not implemented yet
  return;
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

  if( !strcmp(repoId, CORBA::Object::_PD_repoId) )
    return (CORBA::Object_ptr) this;
  if( !strcmp(repoId, CORBA::BOA::_PD_repoId) )
    return (CORBA::BOA_ptr) this;

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
omniOrbBOA::dispatch(GIOP_S& giop_s, omniLocalIdentity* id)
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
    l << "Dispatching remote call \'" << giop_s.operation() << "\' to: "
      << id << '\n';
  }

  if( !id->servant()->_dispatch(giop_s) ) {
    if( !id->servant()->omniServant::_dispatch(giop_s) ) {
      giop_s.RequestReceived(1);
      OMNIORB_THROW(BAD_OPERATION,0, CORBA::COMPLETED_NO);
    }
  }
}


void
omniOrbBOA::dispatch(GIOP_S& giop_s, const CORBA::Octet* key, int keysize)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 0);
  OMNIORB_ASSERT(key && keysize == sizeof(omniOrbBoaKey));

  omniORB::loader::mapKeyToObject_t loader = MapKeyToObjectFunction;
  if( !loader )  OMNIORB_THROW(OBJECT_NOT_EXIST,0, CORBA::COMPLETED_NO);

  omniOrbBoaKey k;
  memcpy(&k, key, sizeof(omniOrbBoaKey));

  CORBA::Object_ptr obj = loader(k);

  if( CORBA::is_nil(obj) )  CORBA::OBJECT_NOT_EXIST(0, CORBA::COMPLETED_NO);
  else                      throw omniORB::LOCATION_FORWARD(obj);
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

  if( !CORBA::is_nil(obj) )  throw omniORB::LOCATION_FORWARD(obj);

  return 0;
}


void
omniOrbBOA::lastInvocationHasCompleted(omniLocalIdentity* id)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 0);

  if( omniORB::trace(15) ) {
    omniORB::logger l;
    l << "BOA etherealising detached object.\n"
      << " id: " << id->servant()->_mostDerivedRepoId() << "\n";
  }

  delete id->servant();
  met_detached_object();
  id->die();
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
omniOrbBOA::dispose(omniLocalIdentity* lid)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(boa_lock, 1);
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);

  if( pd_state == DESTROYED ) {
    omni::internalLock->unlock();
    boa_lock.unlock();
    OMNIORB_THROW(OBJECT_NOT_EXIST,0, CORBA::COMPLETED_NO);
  }

  if( !lid || !lid->servant() ) {
    omni::internalLock->unlock();
    boa_lock.unlock();
    return;
  }

  omniLocalIdentity* id = omni::deactivateObject(lid->key(), lid->keysize());
  if( !id ) {
    omni::internalLock->unlock();
    boa_lock.unlock();
    return;
  }
  OMNIORB_ASSERT(id == lid);

  id->deactivate();
  id->removeFromOAObjList();

  if( id->is_idle() ) {
    omni::internalLock->unlock();
    boa_lock.unlock();

    omniORB::logs(15, "Object is idle -- delete now.");

    delete id->servant();
    id->die();
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
    OMNIORB_THROW(COMM_FAILURE,0, CORBA::COMPLETED_NO);
  }

  pd_nwaiting++;
  while( pd_state == IDLE )  pd_state_signal.wait();
  pd_nwaiting--;

  switch( pd_state ) {
  case IDLE: // get rid of warnings!
  case ACTIVE:
    break;

  case DESTROYED:
    startRequest();
    omni::internalLock->unlock();
    OMNIORB_THROW(OBJ_ADAPTER,0, CORBA::COMPLETED_NO);
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
    OMNIORB_THROW(OBJ_ADAPTER,0, CORBA::COMPLETED_NO);
  }
  // We have to grab a ref to the boa here, since <boa_lock> is
  // released during omniOrbBOA::dispose() -- thus the_boa could
  // be released under our feet!
  the_boa->incrRefCount_locked();
  omniOrbBOA* boa = the_boa;
  CORBA::BOA_var ref_holder(boa);

  omni::internalLock->lock();
  boa->dispose(_identities());
}


void
omniOrbBoaServant::_obj_is_ready()
{
  boa_lock.lock();

  // Why not throw OBJ_ADAPTER?
  if( !the_boa ) {
    boa_lock.unlock();
    OMNIORB_THROW(OBJECT_NOT_EXIST,0, CORBA::COMPLETED_NO);
  }

  omniObjKey key((const CORBA::Octet*) &pd_key, sizeof(omniOrbBoaKey));

  omni::internalLock->lock();

  omniLocalIdentity* id = omni::activateObject(this, the_boa, key);

  // Why throw this?
  if( !id ) {
    omni::internalLock->unlock();
    boa_lock.unlock();
    OMNIORB_THROW(INV_OBJREF,0, CORBA::COMPLETED_NO);
  }

  omni::internalLock->unlock();
  id->insertIntoOAObjList(the_boa->activeObjList());
  boa_lock.unlock();
}


void*
omniOrbBoaServant::_this(const char* repoId)
{
  OMNIORB_ASSERT(repoId);

  CORBA::ULong hash = omni::hash((const CORBA::Octet*) &pd_key,
				 sizeof(omniOrbBoaKey));

  omni::internalLock->lock();
  omniLocalIdentity* id = _identities();
  if( !id )
    // We do know the key, so we can generate a reference anyway.
    id = omni::locateIdentity((const CORBA::Octet*) &pd_key,
			      sizeof(omniOrbBoaKey), hash, 1);

  omniObjRef* objref = omni::createObjRef(_mostDerivedRepoId(), repoId, id);
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
    OMNIORB_THROW(INTF_REPOS,0, CORBA::COMPLETED_NO);

  // Make a call to the interface repository.
  omniStdCallDesc::_cCORBA_mObject_i_cstring
    call_desc(omniDynamicLib::ops->lookup_id_lcfn, "lookup_id", 10, 0, repoId);
  repository->_PR_getobj()->_invoke(call_desc);

  return call_desc.result() ? call_desc.result()->_PR_getobj() : 0;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

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
parse_BOA_args(int& argc, char** argv, const char* orb_identifier)
{
  CORBA::Boolean orbId_match = 0;
  if( orb_identifier && strcmp(orb_identifier, MY_BOA_ID)
                     && strcmp(orb_identifier, OLD_BOA_ID) ) {
    if( omniORB::trace(1) ) {
      omniORB::logger l;
      l << "BOA_init failed -- the BOAid (" << orb_identifier << ")"
	" is not " << MY_BOA_ID << "\n";
    }
    return 0;
  }
  if( omniORB::trace(1) && !strcmp(orb_identifier, OLD_BOA_ID) )
    omniORB::logs(1, "WARNING -- using BOAid " OLD_BOA_ID
		  " (should be " MY_BOA_ID ").");

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
	if (strcmp(argv[idx+1], MY_BOA_ID) != 0)
	  {
	    if ( omniORB::trace(1) ) {
	      omniORB::logger l;
	      l << "BOA_init failed -- the BOAid (" <<
		argv[idx+1] << ") is not " << MY_BOA_ID << "\n";
	    }
	    return 0;
	  }
	if( !strcmp(argv[idx + 1], OLD_BOA_ID) )
	  omniORB::logs(1, "WARNING -- using BOAid " OLD_BOA_ID
			" (should be " MY_BOA_ID ").");
	orbId_match = 1;
	move_args(argc,argv,idx,2);
	continue;
      }

      // -BOAiiop_port <port number>[,<port number>]*
      if (strcmp(argv[idx],"-BOAiiop_port") == 0) {
	if ((idx+1) >= argc) {
	  omniORB::logs(1, "BOA_init failed -- missing -BOAiiop_port"
			" parameter.");
	  return 0;
	}
        unsigned int port;
	if (sscanf(argv[idx+1],"%u",&port) != 1 ||
	    (port == 0 || port >= 65536)) {
	  omniORB::logs(1, "BOA_init failed -- invalid -BOAiiop_port"
			" parameter.");
	  return 0;
	}

	const char* hostname = getenv(OMNIORB_USEHOSTNAME_VAR);
	if( !hostname )  hostname = "";
	omniObjAdapter::options.
	  incomingPorts.push_back(omniObjAdapter::ListenPort(hostname, port));

	move_args(argc,argv,idx,2);
	continue;
      }

      // -BOAiiop_name_port <hostname[:port number]>
      if (strcmp(argv[idx],"-BOAiiop_name_port") == 0) {
        if ((idx+1) >= argc) {
	  omniORB::logs(1, "BOA_init failed -- missing -BOAiiop_name_port"
			" parameter.\n"
		" usage: -BOAiiop_name_port <hostname[:port number]>+");
          return 0;
        }

        // Copy the hostname part of the argument (including :port).
        char hostname[255+1];
        strncpy(hostname, argv[idx+1], 255);
	hostname[255] = '\0';

        // Find the :port part of the argument.  If the port is
	// not specified, we default to 0 which lets the OS pick
	// a number.
        int port = 0;
        char* port_str = strchr(hostname, ':');
        if( port_str != 0 ) {
	  // if the port-number is not specified, fall back to port=0
	  if (port_str[1] == '\0')  port = 0;
	  else if( sscanf(port_str+1, "%u", &port) != 1 ||
		   (port < 0 || port >= 65536) ) {
	    if ( omniORB::trace(1) ) {
	      omniORB::logger l;
	      l << "BOA_init failed -- invalid -BOAiiop_name_port\n"
		" parameter.  Port number out of range: " << port << ".\n";
	    }
	    return 0;
	  }

	  // null terminate and isolate hostname argument
	  *port_str = 0;
        }

	omniObjAdapter::options.
	  incomingPorts.push_back(omniObjAdapter::ListenPort(hostname, port));

        move_args(argc,argv,idx,2);
        continue;
      }

      // -BOAno_bootstrap_agent
      if (strcmp(argv[idx],"-BOAno_bootstrap_agent") == 0) {
	omniObjAdapter::options.noBootstrapAgent = 1;
	move_args(argc,argv,idx,1);
	continue;
      }

      // -BOAhelp
      if (strcmp(argv[idx],"-BOAhelp") == 0) {
	omniORB::logger l;
	l <<
	  "Valid -BOA<options> are:\n"
	  "    -BOAid omniORB3_BOA\n"
	  "    -BOAiiop_port <port no.>[,<port no>]*\n"
	  "    -BOAiiop_name_port <hostname[:port no.]>\n"
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

  if (!orb_identifier && !orbId_match) {
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
