// -*- Mode: C++; -*-
//                            Package   : omniORB
// poamanager.cc              Created on: 12/5/99
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
//    Internal implementation of the PortableServer::POAManager.
//

/*
  $Log$
  Revision 1.1.2.1  1999/09/22 14:27:02  djr
  Major rewrite of orbcore to support POA.

*/

#include <omniORB3/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <poamanager.h>
#include <poaimpl.h>
#include <exception.h>

//////////////////////////////////////////////////////////////////////
///////////////////// PortableServer::POAManager /////////////////////
//////////////////////////////////////////////////////////////////////

OMNIORB_DEFINE_USER_EX_WITHOUT_MEMBERS(PortableServer::POAManager,
				       AdapterInactive,
       "IDL:omg.org/PortableServer/POAManager/AdapterInactive" PS_VERSION)


PortableServer::POAManager::~POAManager() {}


PortableServer::POAManager_ptr
PortableServer::POAManager::_duplicate(PortableServer::POAManager_ptr obj)
{
  if( !CORBA::is_nil(obj) )  obj->_NP_incrRefCount();

  return obj;
}


PortableServer::POAManager_ptr
PortableServer::POAManager::_narrow(CORBA::Object_ptr obj)
{
  if( CORBA::is_nil(obj) || !obj->_NP_is_pseudo() )  return _nil();

  POAManager_ptr p = (POAManager_ptr) obj->_ptrToObjRef(_PD_repoId);

  if( p )  p->_NP_incrRefCount();

  return p ? p : _nil();
}


static omniOrbPOAManager the_nil_pm(1);


PortableServer::POAManager_ptr
PortableServer::POAManager::_nil()
{
  return &the_nil_pm;
}


const char* PortableServer::POAManager::_PD_repoId =
  "IDL:omg.org/PortableServer/POAManager" PS_VERSION;

//////////////////////////////////////////////////////////////////////
////////////////////////// omniOrbPOAManager /////////////////////////
//////////////////////////////////////////////////////////////////////

#define CHECK_NOT_NIL()  \
  if( _NP_is_nil() )  _CORBA_invoked_nil_pseudo_ref()


static omni_mutex pm_lock;


omniOrbPOAManager::~omniOrbPOAManager() {}


void
omniOrbPOAManager::activate()
{
  CHECK_NOT_NIL();

  omni_mutex_lock sync(pm_lock);

  if( pd_state == INACTIVE )  throw AdapterInactive();
  if( pd_state == ACTIVE   )  return;

  pd_state = ACTIVE;

  for( CORBA::ULong i = 0; i < pd_poas.length(); i++ )
    pd_poas[i]->pm_change_state(pd_state);
}


void
omniOrbPOAManager::hold_requests(CORBA::Boolean wait_for_completion)
{
  CHECK_NOT_NIL();

  if( wait_for_completion && 0 /*?? in context of invocation */ )
    throw CORBA::BAD_INV_ORDER(0, CORBA::COMPLETED_NO);

  POASeq poas;
  {
    omni_mutex_lock sync(pm_lock);

    if( pd_state == INACTIVE )  throw AdapterInactive();
    if( pd_state == HOLDING  )  return;

    pd_state = HOLDING;
    poas.length(pd_poas.length());

    for( CORBA::ULong i = 0; i < pd_poas.length(); i++ ) {
      pd_poas[i]->pm_change_state(pd_state);
      if( wait_for_completion ) {
	poas[i] = pd_poas[i];
	poas[i]->incrRefCount();
      }
    }
  }

  if( wait_for_completion )
    for( CORBA::ULong i = 0; i < poas.length(); i++ ) {
      poas[i]->pm_waitForReqCmpltnOrSttChnge(HOLDING);
      poas[i]->decrRefCount();
    }
}


void
omniOrbPOAManager::discard_requests(CORBA::Boolean wait_for_completion)
{
  CHECK_NOT_NIL();

  if( wait_for_completion && 0 /*?? in context of invocation */ )
    throw CORBA::BAD_INV_ORDER(0, CORBA::COMPLETED_NO);

  POASeq poas;
  {
    omni_mutex_lock sync(pm_lock);

    if( pd_state == INACTIVE   )  throw AdapterInactive();
    if( pd_state == DISCARDING )  return;

    pd_state = DISCARDING;
    poas.length(pd_poas.length());

    for( CORBA::ULong i = 0; i < pd_poas.length(); i++ ) {
      pd_poas[i]->pm_change_state(pd_state);
      if( wait_for_completion ) {
	poas[i] = pd_poas[i];
	poas[i]->incrRefCount();
      }
    }
  }

  if( wait_for_completion )
    for( CORBA::ULong i = 0; i < poas.length(); i++ ) {
      poas[i]->pm_waitForReqCmpltnOrSttChnge(DISCARDING);
      poas[i]->decrRefCount();
    }
}


static void
deactivate_thread_fn(void* args)
{
  OMNIORB_ASSERT(args);
  void** targs = (void**) args;

  omniOrbPOAManager::POASeq* ppoas = (omniOrbPOAManager::POASeq*) targs[0];
  omniOrbPOAManager::POASeq& poas = *ppoas;
  CORBA::Boolean etherealise = (CORBA::Boolean) (int) targs[1];
  delete[] targs;

  for( CORBA::ULong i = 0; i < poas.length(); i++ ) {
    poas[i]->pm_deactivate(etherealise);
    poas[i]->decrRefCount();
  }

  delete ppoas;
}


void
omniOrbPOAManager::deactivate(CORBA::Boolean etherealize_objects,
			      CORBA::Boolean wait_for_completion)
{
  CHECK_NOT_NIL();

  if( wait_for_completion && 0 /*?? in context of invocation */ )
    throw CORBA::BAD_INV_ORDER(0, CORBA::COMPLETED_NO);

  POASeq* ppoas = new POASeq;
  POASeq& poas = *ppoas;
  {
    omni_mutex_lock sync(pm_lock);

    if( pd_state == INACTIVE   )  throw AdapterInactive();

    pd_state = INACTIVE;
    poas.length(pd_poas.length());

    for( CORBA::ULong i = 0; i < pd_poas.length(); i++ ) {
      pd_poas[i]->pm_change_state(pd_state);
      poas[i] = pd_poas[i];
      poas[i]->incrRefCount();
    }
  }

  void** args = new void* [2];
  args[0] = ppoas;
  args[1] = (void*) (int) etherealize_objects;

  if( wait_for_completion )
    deactivate_thread_fn(args);
  else
    (new omni_thread(deactivate_thread_fn, args))->start();
}


PortableServer::POAManager::State
omniOrbPOAManager::get_state()
{
  omni_mutex_lock sync(pm_lock);
  return pd_state;
}

////////////////////////////
// Override CORBA::Object //
////////////////////////////

void*
omniOrbPOAManager::_ptrToObjRef(const char* repoId)
{
  OMNIORB_ASSERT(repoId);

  if( !strcmp(repoId, CORBA::Object::_PD_repoId) )
    return (CORBA::Object_ptr) this;
  if( !strcmp(repoId, PortableServer::POAManager::_PD_repoId) )
    return (PortableServer::POA_ptr) this;

  return 0;
}


void
omniOrbPOAManager::_NP_incrRefCount()
{
  pm_lock.lock();
  pd_refCount++;
  pm_lock.unlock();
}


void
omniOrbPOAManager::_NP_decrRefCount()
{
  pm_lock.lock();
  if( --pd_refCount > 0 ) {
    pm_lock.unlock();
    return;
  }

  if( pd_poas.length() ) {
    pm_lock.unlock();
    omniORB::logs(1, "The application has released a reference to a "
		  "POAManager\n"
		  " too many times.  This is a serious application error!");
    return;
  }

  pm_lock.unlock();

  omniORB::logs(15, "POAManager ref count is zero -- deleted.");

  delete this;
}

//////////////
// Internal //
//////////////

void
omniOrbPOAManager::gain_poa(omniOrbPOA* poa)
{
  omni_mutex_lock sync(pm_lock);

  pd_poas.length(pd_poas.length() + 1);
  pd_poas[pd_poas.length() - 1] = poa;
}


void
omniOrbPOAManager::lose_poa(omniOrbPOA* poa)
{
  omni_mutex_lock sync(pm_lock);

  CORBA::ULong i, len = pd_poas.length();

  for( i = 0; i < len; i++ )
    if( pd_poas[i] == poa )
      break;

  if( i == len )
    throw omniORB::fatalException(__FILE__, __LINE__,
				  "lose_poa(...) for POA I didn't own!");

  for( ; i < len - 1; i++ )
    pd_poas[i] = pd_poas[i + 1];

  pd_poas.length(len - 1);
}
