// -*- Mode: C++; -*-
//                            Package   : omniORB
// current.cc                 Created on: 22 Nov 2000
//                            Author    : Sai-Lai Lo
//
//    Copyright (C) 2000 AT&T Laboratories- Cambridge
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

#include <omnithread.h>
#include <omniORB4/CORBA.h>
#include <omniORB4/omniInternal.h>
#include <omniCurrent.h>
#include <poaimpl.h>
#include <poacurrentimpl.h>
#include <initialiser.h>
#include <localIdentity.h>
#include <exceptiondefs.h>


OMNI_USING_NAMESPACE(omni)

/////////////////////////////////////////////////////////////////////////////
//            CORBA::Current                                               //
/////////////////////////////////////////////////////////////////////////////


CORBA::Current::Current(int nil) { _PR_setobj((omniObjRef*)(nil ? 0:1)); }

CORBA::Current::~Current() {}

CORBA::Current_ptr
CORBA::Current::_duplicate(CORBA::Current_ptr obj)
{
  if( !CORBA::is_nil(obj) )  obj->_NP_incrRefCount();

  return obj;
}

CORBA::Current_ptr
CORBA::Current::_narrow(CORBA::Object_ptr obj)
{
  if( CORBA::is_nil(obj) || !obj->_NP_is_pseudo() )  return _nil();

  Current_ptr p = (Current_ptr) obj->_ptrToObjRef(_PD_repoId);

  if( p )  p->_NP_incrRefCount();

  return p ? p : _nil();
}


class omniNilCurrent : public CORBA::Current {
public:
  omniNilCurrent() : CORBA::Current(1) {}
  ~omniNilCurrent() {}
};

CORBA::Current_ptr
CORBA::Current::_nil()
{
  static omniNilCurrent* _the_nil_ptr = 0;
  if( !_the_nil_ptr ) {
    omni::nilRefLock().lock();
    if( !_the_nil_ptr )  _the_nil_ptr = new omniNilCurrent();
    omni::nilRefLock().unlock();
  }
  return _the_nil_ptr;
}


const char*
CORBA::Current::_PD_repoId = "IDL:omg.org/CORBA/Current:1.0";


/////////////////////////////////////////////////////////////////////////////
//            omniCurrent                                                  //
/////////////////////////////////////////////////////////////////////////////

omni_thread::key_t omniCurrent::thread_key;

void
omniCurrent::init()
{
  static CORBA::Boolean done = 0;
  OMNIORB_ASSERT(!done);
  thread_key = omni_thread::allocate_key();
  done = 1;
}

omniCurrent::omniCurrent(omni_thread* thread)
  : pd_callDescriptor(0)
{
  thread->set_value(thread_key, this);
}

omniCurrent::~omniCurrent()
{
  OMNIORB_ASSERT(!pd_callDescriptor);
}



/////////////////////////////////////////////////////////////////////////////
//            PortableServer::Current                                      //
/////////////////////////////////////////////////////////////////////////////

static omniOrbPOACurrent* thePOACurrent = 0;

PortableServer::Current_ptr
omniOrbPOACurrent::theCurrent()
{
  omni_tracedmutex_lock l(*omni::poRcLock);

  if (!thePOACurrent) {
    thePOACurrent = new omniOrbPOACurrent;
    return thePOACurrent;
  }

  thePOACurrent->pd_refCount++;
  return thePOACurrent;
}


PortableServer::Current::~Current() {}

PortableServer::Current_ptr
PortableServer::Current::_duplicate(PortableServer::Current_ptr obj)
{
  if( !CORBA::is_nil(obj) )  obj->_NP_incrRefCount();

  return obj;
}


PortableServer::Current_ptr
PortableServer::Current::_narrow(CORBA::Object_ptr obj)
{
  if( CORBA::is_nil(obj) || !obj->_NP_is_pseudo() )  return _nil();

  Current_ptr p = (Current_ptr) obj->_ptrToObjRef(_PD_repoId);

  if( p )  p->_NP_incrRefCount();

  return p ? p : _nil();
}

PortableServer::Current_ptr
PortableServer::Current::_nil()
{
  static omniOrbPOACurrent* _the_nil_ptr = 0;
  if( !_the_nil_ptr ) {
    omni::nilRefLock().lock();
    if( !_the_nil_ptr )  _the_nil_ptr = new omniOrbPOACurrent(1);
    omni::nilRefLock().unlock();
  }
  return _the_nil_ptr;
}

const char*
PortableServer::Current::_PD_repoId = "IDL:omg.org/PortableServer/POACurrent" PS_VERSION;


OMNIORB_DEFINE_USER_EX_WITHOUT_MEMBERS(PortableServer::Current,
				       NoContext,
        "IDL:omg.org/PortableServer/Current/NoContext" PS_VERSION)


////////////////////////////
// Override CORBA::Object //
////////////////////////////

void*
omniOrbPOACurrent::_ptrToObjRef(const char* repoId)
{
  OMNIORB_ASSERT(repoId);

  if( omni::ptrStrMatch(repoId, PortableServer::Current::_PD_repoId) )
    return (PortableServer::Current_ptr) this;
  if( omni::ptrStrMatch(repoId, CORBA::Object::_PD_repoId) )
    return (CORBA::Object_ptr) this;

  return 0;
}

void
omniOrbPOACurrent::_NP_incrRefCount()
{
  omni::poRcLock->lock();
  pd_refCount++;
  omni::poRcLock->unlock();
}

void
omniOrbPOACurrent::_NP_decrRefCount()
{
  omni::poRcLock->lock();
  int done = --pd_refCount > 0;
  if (done)
    thePOACurrent = 0;
  omni::poRcLock->unlock();

  if( done )  return;

  delete this;
}


omniOrbPOACurrent::~omniOrbPOACurrent()
{
}

static omniCallDescriptor* getCallDesc(const char* op)
{
  omniCurrent* current = omniCurrent::get();
  if (current) {
    omniCallDescriptor* call_desc = current->callDescriptor();
    if (call_desc)
      return call_desc;
  }
  if (omniORB::trace(10)) {
    omniORB::logger l;
    l << "PortableServer::Current::" << op << "() throws NoContext\n";
  }
  throw PortableServer::Current::NoContext();
  return 0; // For paranoid compilers
}
      

PortableServer::POA_ptr
omniOrbPOACurrent::get_POA()
{
  omniCallDescriptor* call_desc = getCallDesc("get_POA");
  OMNIORB_ASSERT(call_desc->poa());
  return PortableServer::POA::_duplicate(call_desc->poa());
}

PortableServer::ObjectId*
omniOrbPOACurrent::get_object_id()
{
  omniCallDescriptor* call_desc = getCallDesc("get_object_id");
  OMNIORB_ASSERT(call_desc->poa());
  OMNIORB_ASSERT(call_desc->localId());

  return call_desc->poa()->localId_to_ObjectId(call_desc->localId());
}

CORBA::Object_ptr
omniOrbPOACurrent::get_reference()
{
  omniCallDescriptor* call_desc = getCallDesc("get_reference");
  omniObjRef* objref = real_get_reference(call_desc);
  return (CORBA::Object_ptr) objref->_ptrToObjRef(CORBA::Object::_PD_repoId);
}

omniObjRef*
omniOrbPOACurrent::real_get_reference(omniCallDescriptor* call_desc)
{
  if (call_desc->objref()) {
    omni::duplicateObjRef(call_desc->objref());
    return call_desc->objref();
  }

  omniLocalIdentity* id = call_desc->localId();
  OMNIORB_ASSERT(id);
  OMNIORB_ASSERT(id->servant());

  omni::internalLock->lock();
  omniObjRef* objref = omni::createObjRef(id->servant()->_mostDerivedRepoId(),
					  CORBA::Object::_PD_repoId, id);
  omni::internalLock->unlock();
  OMNIORB_ASSERT(objref);
  return objref;
}

PortableServer::Servant
omniOrbPOACurrent::get_servant()
{
  omniCallDescriptor* call_desc = getCallDesc("get_servant");
  return real_get_servant(call_desc);
}

#define DOWNCAST(s)  ((PortableServer::Servant) (s)->_downcast())

PortableServer::Servant
omniOrbPOACurrent::real_get_servant(omniCallDescriptor* call_desc)
{
  OMNIORB_ASSERT(call_desc->localId());
  PortableServer::Servant servant = DOWNCAST(call_desc->localId()->servant());
  OMNIORB_ASSERT(servant);
  servant->_add_ref();
  return servant;
}



/////////////////////////////////////////////////////////////////////////////
//            Module initialiser                                           //
/////////////////////////////////////////////////////////////////////////////

OMNI_NAMESPACE_BEGIN(omni)

class omni_omniCurrent_initialiser : public omniInitialiser {
public:
  void attach() {
    omniCurrent::init();
  }

  void detach() {
  }
};

static omni_omniCurrent_initialiser initialiser;

omniInitialiser& omni_omniCurrent_initialiser_ = initialiser;

OMNI_NAMESPACE_END(omni)
