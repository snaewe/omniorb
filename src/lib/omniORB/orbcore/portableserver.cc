// -*- Mode: C++; -*-
//                            Package   : omniORB
// portableserver.cc          Created on: 11/5/99
//                            Author    : David Riddoch (djr)
//
//    Copyright (C) 1996-1999 AT&T Research Cambridge
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
//    Misc code from PortableServer module.
//
 
/*
  $Log$
  Revision 1.2.2.2  2000/09/27 18:04:43  sll
  Use new string allocation functions. Updated to use the new calldescriptor.

  Revision 1.2.2.1  2000/07/17 10:35:58  sll
  Merged from omni3_develop the diff between omni3_0_0_pre3 and omni3_0_0.

  Revision 1.3  2000/07/13 15:25:55  dpg1
  Merge from omni3_develop for 3.0 release.

  Revision 1.1.2.10  2000/06/27 16:23:25  sll
  Merged OpenVMS port.

  Revision 1.1.2.9  2000/06/22 10:40:17  dpg1
  exception.h renamed to exceptiondefs.h to avoid name clash on some
  platforms.

  Revision 1.1.2.8  2000/04/27 10:52:12  dpg1
  Interoperable Naming Service

  omniInitialReferences::get() renamed to omniInitialReferences::resolve().

  Revision 1.1.2.7  2000/01/03 18:43:32  djr
  Fixed bug in ref counting of POA Policy objects.

  Revision 1.1.2.6  1999/10/29 13:18:20  djr
  Changes to ensure mutexes are constructed when accessed.

  Revision 1.1.2.5  1999/10/27 17:32:16  djr
  omni::internalLock and objref_rc_lock are now pointers.

  Revision 1.1.2.4  1999/10/16 13:22:54  djr
  Changes to support compiling on MSVC.

  Revision 1.1.2.3  1999/10/14 16:22:16  djr
  Implemented logging when system exceptions are thrown.

  Revision 1.1.2.2  1999/10/04 17:08:34  djr
  Some more fixes/MSVC work-arounds.

  Revision 1.1.2.1  1999/09/22 14:27:04  djr
  Major rewrite of orbcore to support POA.

*/

#define ENABLE_CLIENT_IR_SUPPORT
#include <omniORB4/CORBA.h>
#include <poaimpl.h>
#include <localIdentity.h>
#include <omniORB4/callDescriptor.h>
#include <initRefs.h>
#include <dynamicLib.h>
#include <exceptiondefs.h>

//////////////////////////////////////////////////////////////////////
////////////////////////// Policy Interfaces /////////////////////////
//////////////////////////////////////////////////////////////////////


_init_in_def_( const CORBA::ULong
	       PortableServer::THREAD_POLICY_ID = 16; )
_init_in_def_( const CORBA::ULong
	       PortableServer::LIFESPAN_POLICY_ID = 17; )
_init_in_def_( const CORBA::ULong
	       PortableServer::ID_UNIQUENESS_POLICY_ID = 18; )
_init_in_def_( const CORBA::ULong
	       PortableServer::ID_ASSIGNMENT_POLICY_ID = 19; )
_init_in_def_( const CORBA::ULong
	       PortableServer::IMPLICIT_ACTIVATION_POLICY_ID = 20; )
_init_in_def_( const CORBA::ULong
	       PortableServer::SERVANT_RETENTION_POLICY_ID = 21; )
_init_in_def_( const CORBA::ULong
	       PortableServer::REQUEST_PROCESSING_POLICY_ID = 22; )


#define DEFINE_POLICY_OBJECT(name)  \
  \
PortableServer::name::~name() {}  \
  \
CORBA::Policy_ptr  \
PortableServer::name::copy()  \
{  \
  if( _NP_is_nil() )  _CORBA_invoked_nil_pseudo_ref();  \
  return new name(pd_value);  \
}  \
  \
void*  \
PortableServer::name::_ptrToObjRef(const char* repoId)  \
{  \
  OMNIORB_ASSERT(repoId );  \
  \
  if( !strcmp(repoId, CORBA::Object::_PD_repoId) )  \
    return (CORBA::Object_ptr) this;  \
  if( !strcmp(repoId, CORBA::Policy::_PD_repoId) )  \
    return (CORBA::Policy_ptr) this;  \
  if( !strcmp(repoId, PortableServer::name::_PD_repoId) )  \
    return (PortableServer::name##_ptr) this;  \
  \
  return 0;  \
}  \
  \
PortableServer::name##_ptr  \
PortableServer::name::_duplicate(PortableServer::name##_ptr obj)  \
{  \
  if( !CORBA::is_nil(obj) )  obj->_NP_incrRefCount();  \
  \
  return obj;  \
}  \
  \
PortableServer::name##_ptr  \
PortableServer::name::_narrow(CORBA::Object_ptr obj)  \
{  \
  if( CORBA::is_nil(obj) )  return _nil();  \
  \
  name##_ptr p = (name##_ptr) obj->_ptrToObjRef(name::_PD_repoId);  \
  \
  if( p )  p->_NP_incrRefCount();  \
  \
  return p ? p : _nil();  \
}  \
  \
PortableServer::name##_ptr  \
PortableServer::name::_nil()  \
{  \
  static name* _the_nil_ptr = 0;  \
  if( !_the_nil_ptr ) {  \
    omni::nilRefLock().lock();  \
    if( !_the_nil_ptr )  _the_nil_ptr = new name;  \
    omni::nilRefLock().unlock();  \
  }  \
  return _the_nil_ptr;  \
}  \
  \
const char*  \
PortableServer::name::_PD_repoId = "IDL:omg.org/PortableServer/" #name PS_VERSION;


DEFINE_POLICY_OBJECT(ThreadPolicy)
DEFINE_POLICY_OBJECT(LifespanPolicy)
DEFINE_POLICY_OBJECT(IdUniquenessPolicy)
DEFINE_POLICY_OBJECT(IdAssignmentPolicy)
DEFINE_POLICY_OBJECT(ImplicitActivationPolicy)
DEFINE_POLICY_OBJECT(ServantRetentionPolicy)
DEFINE_POLICY_OBJECT(RequestProcessingPolicy)

#undef DEFINE_POLICY_OBJECT

//////////////////////////////////////////////////////////////////////
///////////////////////////// ServantBase ////////////////////////////
//////////////////////////////////////////////////////////////////////

PortableServer::ServantBase::~ServantBase() {}


PortableServer::POA_ptr
PortableServer::ServantBase::_default_POA()
{
  return omniOrbPOA::rootPOA();
}


CORBA::_objref_InterfaceDef*
PortableServer::ServantBase::_get_interface()
{
  // Return 0 to indicate to _do_get_interface() that we have not
  // been overriden.  We cannot implement this method here, because
  // we would have to do a downcast to InterfaceDef, which would
  // introduce a dependency on the dynamic library.
  return 0;
}


void
PortableServer::ServantBase::_add_ref()
{
  // empty
}


void
PortableServer::ServantBase::_remove_ref()
{
  // empty
}


void*
PortableServer::ServantBase::_do_this(const char* repoId)
{
  OMNIORB_ASSERT(repoId);

  if( 0 /*?? in context of invocation on this servant */ ) {

    // Return a reference to the object being invoked.
#if defined(__DECCXX) && __DECCXX_VER >= 60000000
    // Compaq C++ 6.2 warns about this even though this code 
    // cannot ever execute
    return 0;
#endif
  }
  else {

    PortableServer::POA_var poa = this->_default_POA();

    {
      omni_tracedmutex_lock sync(*omni::internalLock);

      omniLocalIdentity* id = _identities();

      if( id && !id->servantsNextIdentity() ) {
	// We only have a single activation -- return a
	// reference to it.
	omniObjRef* ref = omni::createObjRef(_mostDerivedRepoId(), repoId, id);
	OMNIORB_ASSERT(ref);
	return ref->_ptrToObjRef(repoId);
      }
    }

    if( CORBA::is_nil(poa) )
      OMNIORB_THROW(OBJ_ADAPTER,0, CORBA::COMPLETED_NO);

    return ((omniOrbPOA*)(PortableServer::POA_ptr) poa)->
      servant__this(this, repoId);
  }
}


omniObjRef*
PortableServer::ServantBase::_do_get_interface()
{
  CORBA::_objref_InterfaceDef* p = _get_interface();
  if( p )  return p->_PR_getobj();

  // If we get here then we assume that _get_interface has not
  // been overriden, and provide the default implementation.

  // repoId should not be empty for statically defined
  // servants.  This version should not have been called
  // if it is a dynamic implementation.
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
    call_desc(omniDynamicLib::ops->lookup_id_lcfn, "lookup_id", 10, repoId);
  repository->_PR_getobj()->_invoke(call_desc);

  return call_desc.result() ? call_desc.result()->_PR_getobj() : 0;
}


void*
PortableServer::ServantBase::_downcast()
{
  return (Servant) this;
}

//////////////////////////////////////////////////////////////////////
///////////////////////// RefCountServantBase ////////////////////////
//////////////////////////////////////////////////////////////////////

static omni_tracedmutex ref_count_lock;


PortableServer::RefCountServantBase::~RefCountServantBase() {}


void
PortableServer::RefCountServantBase::_add_ref()
{
  ref_count_lock.lock();
  pd_refCount++;
  ref_count_lock.unlock();
}


void
PortableServer::RefCountServantBase::_remove_ref()
{
  ref_count_lock.lock();
  int done = --pd_refCount > 0;
  ref_count_lock.unlock();
  if( done )  return;

  if( pd_refCount < 0 ) {
    omniORB::logs(1, "RefCountServantBase has negative ref count!");
    return;
  }

  omniORB::logs(15, "RefCountServantBase has zero ref count -- deleted.");

  delete this;
}

//////////////////////////////////////////////////////////////////////
//////////////////////// C++ Mapping Specific ////////////////////////
//////////////////////////////////////////////////////////////////////

char*
PortableServer::ObjectId_to_string(const ObjectId& id)
{
  int len = id.length();

  char* s = _CORBA_String_helper::alloc(len);

  for( int i = 0; i < len; i++ )
    if( (char) (s[i] = id[i]) == '\0' ) {
      delete[] s;
      OMNIORB_THROW(BAD_PARAM,0, CORBA::COMPLETED_NO);
    }

  s[len] = '\0';
  return s;
}


PortableServer::ObjectId*
PortableServer::string_to_ObjectId(const char* s)
{
  int len = strlen(s);
  ObjectId* pid = new ObjectId(len);
  ObjectId& id = *pid;

  id.length(len);

  for( int i = 0; i < len; i++ )  id[i] = *s++;

  return pid;
}
