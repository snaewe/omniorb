// -*- Mode: C++; -*-
//                            Package   : omniORB
// poa.cc                     Created on: 14/4/99
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
//    Implementation of PortableServer::POA.
//

/*
  $Log$
  Revision 1.4  2001/02/21 14:12:12  dpg1
  Merge from omni3_develop for 3.0.3 release.

  Revision 1.1.2.24  2000/12/05 12:10:30  dpg1
  Fix bug with ServantLocator postinvoke() when operation name is >32
  characters.

  Revision 1.1.2.23  2000/11/13 12:03:24  djr
  ServantLocator::preinvoke() and ServantActivator::incarnate() did not
  pass SystemExceptions on to the client.

  Revision 1.1.2.22  2000/10/13 09:26:09  djr
  Fixed race between object-deactivation and completion of last invocation.

  Revision 1.1.2.21  2000/06/22 10:40:16  dpg1
  exception.h renamed to exceptiondefs.h to avoid name clash on some
  platforms.

  Revision 1.1.2.20  2000/06/12 11:15:52  dpg1
  Clarifying comment about TRANSIENT exceptions on exiting HOLDING
  state.

  Revision 1.1.2.19  2000/06/02 16:09:59  dpg1
  If an object is deactivated while its POA is in the HOLDING state,
  clients which were held now receive a TRANSIENT exception when the POA
  becomes active again.

  Revision 1.1.2.18  2000/05/05 18:59:36  dpg1
  Back out last change, since it doesn't work.

  Revision 1.1.2.17  2000/05/05 17:00:46  dpg1
  INS POA now has the USE_SERVANT_MANAGER policy, with the small
  exception that it raises OBJECT_NOT_EXIST rather than OBJ_ADAPTER if
  no servant activator has been registered.

  Revision 1.1.2.16  2000/04/27 10:51:39  dpg1
  Interoperable Naming Service

  Add magic INS POA.

  Revision 1.1.2.15  2000/03/01 12:28:36  dpg1
  find_POA() now correctly throws AdapterNonExistent if an
  AdapterActivator fails to activate the POA.

  Revision 1.1.2.14  2000/02/04 18:11:03  djr
  Minor mods for IRIX (casting pointers to ulong instead of int).

  Revision 1.1.2.13  2000/01/27 10:55:46  djr
  Mods needed for powerpc_aix.  New macro OMNIORB_BASE_CTOR to provide
  fqname for base class constructor for some compilers.

  Revision 1.1.2.12  2000/01/20 11:51:36  djr
  (Most) Pseudo objects now used omni::poRcLock for ref counting.
  New assertion check OMNI_USER_CHECK.

  Revision 1.1.2.11  2000/01/03 20:35:09  djr
  Added check for POA being CORBA::release()d too many times.

  Revision 1.1.2.10  1999/11/12 17:05:38  djr
  Minor mods for hp-10.20 with aCC.

  Revision 1.1.2.9  1999/10/29 13:18:18  djr
  Changes to ensure mutexes are constructed when accessed.

  Revision 1.1.2.8  1999/10/27 17:32:14  djr
  omni::internalLock and objref_rc_lock are now pointers.

  Revision 1.1.2.7  1999/10/21 12:34:06  djr
  Removed duplicate include of exception.h.

  Revision 1.1.2.6  1999/10/14 16:22:14  djr
  Implemented logging when system exceptions are thrown.

  Revision 1.1.2.5  1999/09/30 11:52:32  djr
  Implemented use of AdapterActivators in POAs.

  Revision 1.1.2.4  1999/09/28 10:54:34  djr
  Removed pretty-printing of object keys from object adapters.

  Revision 1.1.2.3  1999/09/24 17:11:14  djr
  New option -ORBtraceInvocations and omniORB::traceInvocations.

  Revision 1.1.2.2  1999/09/24 10:28:51  djr
  Added POA_Helper and POA::the_children().

  Revision 1.1.2.1  1999/09/22 14:27:00  djr
  Major rewrite of orbcore to support POA.

*/

#include <omniORB3/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <poaimpl.h>
#include <omniORB3/omniObjRef.h>
#include <omniORB3/callDescriptor.h>
#include <localIdentity.h>
#include <poamanager.h>
#include <exceptiondefs.h>
#include <ropeFactory.h>

#include <ctype.h>
#include <stdio.h>
#if defined(UnixArchitecture) || defined(__VMS)
#include <sys/time.h>
#include <unistd.h>
#elif defined(NTArchitecture)
#include <sys/types.h>
#include <sys/timeb.h>
#include <process.h>
#endif

#ifdef __atmos__
#include <kernel.h>
#include <timelib.h>
#include <sys/time.h>
#endif


#define POA_NAME_SEP            '\xff'
#define POA_NAME_SEP_STR        "\xff"
#define TRANSIENT_SUFFIX_SEP    '\xfe'
#define TRANSIENT_SUFFIX_SIZE   8

#define DOWNCAST(s)  ((PortableServer::Servant) (s)->_downcast())

#define SYS_ASSIGNED_ID_SIZE    4


//////////////////////////////////////////////////////////////////////
///////////////////// PortableServer::POA_Helper /////////////////////
//////////////////////////////////////////////////////////////////////

PortableServer::POA_ptr
PortableServer::POA_Helper::_nil()
{
  return POA::_nil();
}


CORBA::Boolean
PortableServer::POA_Helper::is_nil(POA_ptr p)
{
  return CORBA::is_nil(p);
}


void
PortableServer::POA_Helper::release(POA_ptr p)
{
  CORBA::release(p);
}


void
PortableServer::POA_Helper::duplicate(POA_ptr obj)
{
  if( !CORBA::is_nil(obj) )  obj->_NP_incrRefCount();
}

//////////////////////////////////////////////////////////////////////
///////////////////////// PortableServer::POA ////////////////////////
//////////////////////////////////////////////////////////////////////

PortableServer::POA::~POA() {}


PortableServer::POA_ptr
PortableServer::POA::_duplicate(PortableServer::POA_ptr obj)
{
  if( !CORBA::is_nil(obj) )  obj->_NP_incrRefCount();

  return obj;
}


PortableServer::POA_ptr
PortableServer::POA::_narrow(CORBA::Object_ptr obj)
{
  if( CORBA::is_nil(obj) || !obj->_NP_is_pseudo() )  return _nil();

  POA_ptr p = (POA_ptr) obj->_ptrToObjRef(_PD_repoId);

  if( p )  p->_NP_incrRefCount();

  return p ? p : _nil();
}


PortableServer::POA_ptr
PortableServer::POA::_nil()
{
  static omniOrbPOA* _the_nil_ptr = 0;
  if( !_the_nil_ptr ) {
    omni::nilRefLock().lock();
    if( !_the_nil_ptr )  _the_nil_ptr = new omniOrbPOA();
    omni::nilRefLock().unlock();
  }
  return _the_nil_ptr;
}


const char*
PortableServer::POA::_PD_repoId = "IDL:omg.org/PortableServer/POA" PS_VERSION;


PortableServer::POA_ptr
PortableServer::POA::_the_root_poa()
{
  return omniOrbPOA::rootPOA();
}


#ifdef DEFINE_CPFN
#undef DEFINE_CPFN
#endif
#define DEFINE_CPFN(policy, fn_name)  \
  \
PortableServer::policy##_ptr  \
PortableServer::POA::fn_name(PortableServer::policy##Value v)  \
{  \
  return new PortableServer::policy(v);  \
}

DEFINE_CPFN(ThreadPolicy, create_thread_policy)
DEFINE_CPFN(LifespanPolicy, create_lifespan_policy)
DEFINE_CPFN(IdUniquenessPolicy, create_id_uniqueness_policy)
DEFINE_CPFN(IdAssignmentPolicy, create_id_assignment_policy)
DEFINE_CPFN(ImplicitActivationPolicy, create_implicit_activation_policy)
DEFINE_CPFN(ServantRetentionPolicy, create_servant_retention_policy)
DEFINE_CPFN(RequestProcessingPolicy, create_request_processing_policy)

#undef DEFINE_CPFN

//////////////////////////////////////////////////////////////////////
/////////////////////////// POA Exceptions ///////////////////////////
//////////////////////////////////////////////////////////////////////

#ifdef VERSION
#undef VERSION
#endif
#define VERSION ":2.3"

OMNIORB_DEFINE_USER_EX_WITHOUT_MEMBERS(PortableServer::POA,
				       AdapterAlreadyExists,
	       "IDL:omg.org/PortableServer/AdapterAlreadyExists" PS_VERSION)
OMNIORB_DEFINE_USER_EX_WITHOUT_MEMBERS(PortableServer::POA, AdapterInactive,
	       "IDL:omg.org/PortableServer/AdapterInactive" PS_VERSION)
OMNIORB_DEFINE_USER_EX_WITHOUT_MEMBERS(PortableServer::POA, AdapterNonExistent,
	       "IDL:omg.org/PortableServer/AdapterNonExistent" PS_VERSION)
OMNIORB_DEFINE_USER_EX_WITHOUT_MEMBERS(PortableServer::POA, NoServant,
	       "IDL:omg.org/PortableServer/NoServant" PS_VERSION)
OMNIORB_DEFINE_USER_EX_WITHOUT_MEMBERS(PortableServer::POA,ObjectAlreadyActive,
	       "IDL:omg.org/PortableServer/ObjectAlreadyActive" PS_VERSION)
OMNIORB_DEFINE_USER_EX_WITHOUT_MEMBERS(PortableServer::POA, ObjectNotActive,
	       "IDL:omg.org/PortableServer/ObjectNotActive" PS_VERSION)
OMNIORB_DEFINE_USER_EX_WITHOUT_MEMBERS(PortableServer::POA,
				       ServantAlreadyActive,
	       "IDL:omg.org/PortableServer/ServantAlreadyActive" PS_VERSION)
OMNIORB_DEFINE_USER_EX_WITHOUT_MEMBERS(PortableServer::POA, ServantNotActive,
	       "IDL:omg.org/PortableServer/ServantNotActive" PS_VERSION)
OMNIORB_DEFINE_USER_EX_WITHOUT_MEMBERS(PortableServer::POA, WrongAdapter,
	       "IDL:omg.org/PortableServer/WrongAdapter" PS_VERSION)
OMNIORB_DEFINE_USER_EX_WITHOUT_MEMBERS(PortableServer::POA, WrongPolicy,
	       "IDL:omg.org/PortableServer/WrongPolicy" PS_VERSION)

OMNIORB_DEFINE_USER_EX_COMMON_FNS(PortableServer::POA, InvalidPolicy,
	       "IDL:omg.org/PortableServer/InvalidPolicy" PS_VERSION)


PortableServer::POA::InvalidPolicy::InvalidPolicy(const PortableServer::POA::InvalidPolicy& _s) : CORBA::UserException(_s)
{
  index = _s.index;
}

PortableServer::POA::InvalidPolicy::InvalidPolicy(CORBA::UShort  _index)
{
  pd_insertToAnyFn    = PortableServer::POA::InvalidPolicy::insertToAnyFn;
  pd_insertToAnyFnNCP = PortableServer::POA::InvalidPolicy::insertToAnyFnNCP;
  index = _index;
}

PortableServer::POA::InvalidPolicy& PortableServer::POA::InvalidPolicy::operator=(const PortableServer::POA::InvalidPolicy& _s)
{
  ((CORBA::UserException*) this)->operator=(_s);
  index = _s.index;
  return *this;
}

size_t
PortableServer::POA::InvalidPolicy::_NP_alignedSize(size_t _msgsize) const
{
  _msgsize = omni::align_to(_msgsize, omni::ALIGN_2) + 2;
  return _msgsize;
}

void
PortableServer::POA::InvalidPolicy::operator>>= (NetBufferedStream& _n) const
{
  index >>= _n;
}

void
PortableServer::POA::InvalidPolicy::operator<<= (NetBufferedStream& _n)
{
  index <<= _n;
}

void
PortableServer::POA::InvalidPolicy::operator>>= (MemBufferedStream& _n) const
{
  index >>= _n;
}

void
PortableServer::POA::InvalidPolicy::operator<<= (MemBufferedStream& _n)
{
  index <<= _n;
}


//////////////////////////////////////////////////////////////////////
///////////////////////////// omniOrbPOA /////////////////////////////
//////////////////////////////////////////////////////////////////////

#define CHECK_NOT_NIL()  \
 if( _NP_is_nil() )  _CORBA_invoked_nil_pseudo_ref()

#define CHECK_NOT_DYING()  \
 if( pd_dying )  OMNIORB_THROW(OBJECT_NOT_EXIST,0, CORBA::COMPLETED_NO)

#define CHECK_NOT_DESTROYED()  \
 if( pd_destroyed )  OMNIORB_THROW(OBJECT_NOT_EXIST,0, CORBA::COMPLETED_NO)

#define CHECK_NOT_NIL_OR_DESTROYED()  \
 if( _NP_is_nil() )  _CORBA_invoked_nil_pseudo_ref();  \
 if( pd_destroyed )  OMNIORB_THROW(OBJECT_NOT_EXIST,0, CORBA::COMPLETED_NO)


static void transfer_and_check_policies(omniOrbPOA::Policies& pout,
					const CORBA::PolicyList& pin);


static omni_tracedmutex     poa_lock;

static omni_tracedmutex     servant_activator_lock;
// Used to serealise invocations on ServantActivator's.  It
// comes before all other locks in the partial order -- since
// it is held whilst making an upcall into application code.

static omni_tracedcondition adapteractivator_signal(&poa_lock);
// Used to signal between threads when using an AdapterActivator
// to create a child POA.

static omniOrbPOA* theRootPOA = 0;
static omniOrbPOA* theINSPOA  = 0;
// Protected by <poa_lock>.


omniOrbPOA::~omniOrbPOA()
{
  if( pd_policy.single_threaded )  delete pd_call_lock;
}


PortableServer::POA_ptr
omniOrbPOA::create_POA(const char* adapter_name,
		       PortableServer::POAManager_ptr manager,
		       const CORBA::PolicyList& policies)
{
  CHECK_NOT_NIL();
  if( !adapter_name_is_valid(adapter_name) )
    OMNIORB_THROW(BAD_PARAM,0, CORBA::COMPLETED_NO);

  // Setup the default policies.
  Policies policy;
  policy.single_threaded = 0;
  policy.transient = 1;
  policy.multiple_id = 0;
  policy.user_assigned_id = 0;
  policy.retain_servants = 1;
  policy.req_processing = RPP_ACTIVE_OBJ_MAP;
  policy.implicit_activation = 0;

  transfer_and_check_policies(policy, policies);

  omni_tracedmutex_lock sync(poa_lock);
  omni_tracedmutex_lock sync2(pd_lock);

  CHECK_NOT_DYING();

  // If an adapter of the given name exists, but is in the
  // process of being destroyed, we should block until
  // that has completed, and then allow the new one to be
  // created.  Ref CORBA 2.3 11.3.8.4

  omniOrbPOA* p = find_child(adapter_name);
  if( p ) {
    omni_tracedmutex_lock sync(p->pd_lock);
    if( p->pd_dying ) {
      while( p->pd_destroyed != 2 )  p->pd_deathSignal.wait();
      OMNIORB_ASSERT(find_child(adapter_name) == 0);
    }
    else
      throw AdapterAlreadyExists();
  }

  if( CORBA::is_nil(manager) )
    manager = new omniOrbPOAManager();
  else
    PortableServer::POAManager::_duplicate(manager);

  omniOrbPOA* poa = new omniOrbPOA(adapter_name, (omniOrbPOAManager*) manager,
				   policy, this);

  insert_child(poa);

  poa->adapterActive();

  // Need to ensure state is not changed from HOLDING if POA is
  // being created by an adapter activator.  So in this case do
  // not attach the new poa to the manager.
  if( !is_adapteractivating_child(adapter_name) )
    ((omniOrbPOAManager*) manager)->gain_poa(poa);

  poa->incrRefCount();
  return poa;
}


PortableServer::POA_ptr
omniOrbPOA::find_POA(const char* adapter_name, CORBA::Boolean activate_it)
{
  CHECK_NOT_NIL_OR_DESTROYED();
  if( !adapter_name )  OMNIORB_THROW(BAD_PARAM,0, CORBA::COMPLETED_NO);

  omni_tracedmutex_lock sync(poa_lock);

  omniOrbPOA* poa = find_child(adapter_name);

  if( poa && !poa->pd_dying ) {
    poa->incrRefCount();
    return poa;
  }

  if( !activate_it || !pd_adapterActivator )  throw AdapterNonExistent();

  poa = attempt_to_activate_adapter(adapter_name);

  if( !poa ) throw AdapterNonExistent();

  return poa;
}


static void destroyer_thread_fn(void* poa_arg);


void
omniOrbPOA::destroy(CORBA::Boolean etherealize_objects,
		    CORBA::Boolean wait_for_completion)
{
  CHECK_NOT_NIL();
  if( wait_for_completion && 0 /*?? in context of invocation */ )
    OMNIORB_THROW(BAD_INV_ORDER,0, CORBA::COMPLETED_NO);

  // Mark self as being in the process of destruction, sever links
  // with poa manager, destroy childer, deactivate all objects
  // (possibly waiting for completion of outstanding method
  // invocations) and then remove self from parent.

  {
    omni_tracedmutex_lock sync(pd_lock);

    if( pd_destroyed )  OMNIORB_THROW(OBJECT_NOT_EXIST,0, CORBA::COMPLETED_NO);

    if( pd_dying ) {
      // Need to be able to handle multiple concurrent calls to
      // destroy.  If destruction is in progress and wait_f_c is
      // true, must wait to complete.  Otherwise can just return.
      if( wait_for_completion )
	while( pd_destroyed != 2 )  pd_deathSignal.wait();
      return;
    }

    pd_dying = 1;
  }

  // From this point on we can be sure that no other threads will
  // try and:
  //   o  create child POAs
  //   o  activate objects

  if( omniORB::trace(10) ) {
    omniORB::logger l;
    l << "Destroying POA(" << (char*) pd_name << ").\n";
  }

  pd_manager->lose_poa(this);
  // We release the reference to the manager only when we are
  // deleted -- since we want <pd_manager> to be immutable.

  if( wait_for_completion ) {
    do_destroy(etherealize_objects);
  }
  else {
    omniORB::logs(15, "Starting a POA destroyer thread.");

    void** args = new void* [2];
    args[0] = (omniOrbPOA*) this;
    args[1] = (void*) (unsigned long) etherealize_objects;
    (new omni_thread(destroyer_thread_fn, args))->start();
  }
}



char*
omniOrbPOA::the_name()
{
  CHECK_NOT_NIL_OR_DESTROYED();

  return CORBA::string_dup(pd_name);
}


PortableServer::POA_ptr
omniOrbPOA::the_parent()
{
  CHECK_NOT_NIL();
  omni_tracedmutex_lock sync(poa_lock);
  CHECK_NOT_DESTROYED();

  return pd_parent ? PortableServer::POA::_duplicate(pd_parent)
                   : PortableServer::POA::_nil();
}


PortableServer::POAList*
omniOrbPOA::the_children()
{
  CHECK_NOT_NIL_OR_DESTROYED();

  PortableServer::POAList* childer = new PortableServer::POAList;

  poa_lock.lock();
  childer->length(pd_children.length());
  for( CORBA::ULong i = 0; i < pd_children.length(); i++ ) {
    pd_children[i]->incrRefCount();
    (*childer)[i] = pd_children[i];
  }
  poa_lock.unlock();

  return childer;
}


PortableServer::POAManager_ptr
omniOrbPOA::the_POAManager()
{
  CHECK_NOT_NIL_OR_DESTROYED();

  return PortableServer::POAManager::_duplicate(pd_manager);
}


PortableServer::AdapterActivator_ptr
omniOrbPOA::the_activator()
{
  CHECK_NOT_NIL_OR_DESTROYED();

  poa_lock.lock();
  PortableServer::AdapterActivator_ptr ret = pd_adapterActivator ?
    PortableServer::AdapterActivator::_duplicate(pd_adapterActivator) :
    PortableServer::AdapterActivator::_nil();
  poa_lock.unlock();

  return ret;
}


void
omniOrbPOA::the_activator(PortableServer::AdapterActivator_ptr aa)
{
  CHECK_NOT_NIL_OR_DESTROYED();

  PortableServer::AdapterActivator_ptr neww =
    PortableServer::AdapterActivator::_duplicate(aa);
  if( CORBA::is_nil(neww) )  neww = 0;

  poa_lock.lock();
  PortableServer::AdapterActivator_ptr old = pd_adapterActivator;
  pd_adapterActivator = neww;
  poa_lock.unlock();

  if( old )  CORBA::release(old);
}


PortableServer::ServantManager_ptr
omniOrbPOA::get_servant_manager()
{
  CHECK_NOT_NIL_OR_DESTROYED();
  if( pd_policy.req_processing != RPP_SERVANT_MANAGER )
    throw WrongPolicy();

  omni_tracedmutex_lock sync(pd_lock);

  if( pd_policy.retain_servants )
    return pd_servantActivator ?
      PortableServer::ServantActivator::_duplicate(pd_servantActivator)
      : PortableServer::ServantActivator::_nil();
  else
    return pd_servantLocator ?
      PortableServer::ServantLocator::_duplicate(pd_servantLocator)
      : PortableServer::ServantLocator::_nil();
}


void
omniOrbPOA::set_servant_manager(PortableServer::ServantManager_ptr imgr)
{
  CHECK_NOT_NIL_OR_DESTROYED();

  if( pd_policy.req_processing != RPP_SERVANT_MANAGER )
    throw WrongPolicy();
  if( CORBA::is_nil(imgr) )
    OMNIORB_THROW(OBJ_ADAPTER,0, CORBA::COMPLETED_NO);

  {
    // Check that <imgr> is a local object ...
    omni::internalLock->lock();
    int islocal = imgr->_localId() ? 1 : 0;
    omni::internalLock->unlock();
    if( !islocal )  OMNIORB_THROW(BAD_PARAM,0, CORBA::COMPLETED_NO);
  }

  omni_tracedmutex_lock sync(pd_lock);

  if( pd_servantActivator || pd_servantLocator )
    OMNIORB_THROW(BAD_INV_ORDER,0, CORBA::COMPLETED_NO);

  if( pd_policy.retain_servants ) {
    pd_servantActivator = PortableServer::ServantActivator::_narrow(imgr);
    if( CORBA::is_nil(pd_servantActivator) ) {
      pd_servantActivator = 0;
      OMNIORB_THROW(OBJ_ADAPTER,0, CORBA::COMPLETED_NO);
    }
  } else {
    pd_servantLocator = PortableServer::ServantLocator::_narrow(imgr);
    if( CORBA::is_nil(pd_servantLocator) ) {
      pd_servantLocator = 0;
      OMNIORB_THROW(OBJ_ADAPTER,0, CORBA::COMPLETED_NO);
    }
  }
}


PortableServer::Servant
omniOrbPOA::get_servant()
{
  CHECK_NOT_NIL_OR_DESTROYED();

  if( pd_policy.req_processing != RPP_DEFAULT_SERVANT )
    throw WrongPolicy();

  omni_tracedmutex_lock sync(pd_lock);

  if( !pd_defaultServant )  throw NoServant();

  pd_defaultServant->_add_ref();
  return pd_defaultServant;
}


void
omniOrbPOA::set_servant(PortableServer::Servant p_servant)
{
  CHECK_NOT_NIL_OR_DESTROYED();

  if( pd_policy.req_processing != RPP_DEFAULT_SERVANT )
    throw WrongPolicy();

  pd_lock.lock();

  if( pd_defaultServant )  pd_defaultServant->_remove_ref();
  if( p_servant         )  p_servant->_add_ref();
  pd_defaultServant = p_servant;

  pd_lock.unlock();
}


PortableServer::ObjectId*
omniOrbPOA::activate_object(PortableServer::Servant p_servant)
{
  CHECK_NOT_NIL();
  if( !p_servant )  OMNIORB_THROW(BAD_PARAM,0, CORBA::COMPLETED_NO);
  if( pd_policy.user_assigned_id || !pd_policy.retain_servants )
    throw WrongPolicy();

  omni_tracedmutex_lock sync(pd_lock);
  CHECK_NOT_DYING();
  omni_tracedmutex_lock sync2(*omni::internalLock);

  if( !pd_policy.multiple_id ) {
    // Check the servant's list of identities, to ensure that
    // it isn't already active in this POA.

    omniLocalIdentity* id = p_servant->_identities();

    while( id && id->adapter() != this )  id = id->servantsNextIdentity();

    if( id )  throw ServantAlreadyActive();
  }

  PortableServer::ObjectId* ret = new PortableServer::ObjectId();
  omniLocalIdentity* id = 0;
  omniObjKey key;

  // We have to keep trying here, just in case someone has
  // activated an object with a system generated id, using
  // activate_object_with_id().

  do {
    const CORBA::Octet* oid;
    int idsize;
    create_new_key(key, &oid, &idsize);

    ret->length(idsize);
    memcpy(ret->NP_data(), oid, idsize);

    id = omni::activateObject(p_servant, this, key);

  } while( !id );

  p_servant->_add_ref();
  id->insertIntoOAObjList(&pd_activeObjList);

  return ret;
}


void
omniOrbPOA::activate_object_with_id(const PortableServer::ObjectId& oid,
				    PortableServer::Servant p_servant)
{
  CHECK_NOT_NIL();
  if( !pd_policy.retain_servants )  throw WrongPolicy();
  if( !p_servant ||
      !pd_policy.user_assigned_id && oid.length() != SYS_ASSIGNED_ID_SIZE )
    OMNIORB_THROW(BAD_PARAM,0, CORBA::COMPLETED_NO);

  omni_tracedmutex_lock sync(pd_lock);
  CHECK_NOT_DYING();
  omniLocalIdentity* id = 0;
  {
    omni_tracedmutex_lock sync2(*omni::internalLock);

    if( !pd_policy.multiple_id ) {
      // Check the servant's list of identities, to ensure that
      // it isn't already active in this POA.

      id = p_servant->_identities();

      while( id && id->adapter() != this )  id = id->servantsNextIdentity();

      if( id )  throw ServantAlreadyActive();
    }

    omniObjKey key;
    create_key(key, oid.NP_data(), oid.length());
    id = omni::activateObject(p_servant, this, key);
    if( !id )  throw ObjectAlreadyActive();
  }

  p_servant->_add_ref();
  id->insertIntoOAObjList(&pd_activeObjList);
}


void
omniOrbPOA::deactivate_object(const PortableServer::ObjectId& oid)
{
  // Arghh!  Things are supposed to happen in this order:
  //  o stop new requests being admitted
  //  o wait for requests to complete
  //  o remove from active object map
  //  o etherealise
  // Should not be possible to reactivate same object until
  // the etherealisation stage has happened.  This is probably
  // tricky -- we may be forced to put a placeholder into the
  // active object map.  Ignore for now ...

  CHECK_NOT_NIL();
  if( !pd_policy.retain_servants )  throw WrongPolicy();

  omniObjKey key;
  create_key(key, oid.NP_data(), oid.length());

  pd_lock.lock();
  CHECK_NOT_DESTROYED();
  omni::internalLock->lock();

  omniLocalIdentity* id = omni::deactivateObject(key.key(), key.size());
  if( !id ) {
    omni::internalLock->unlock();
    pd_lock.unlock();
    throw ObjectNotActive();
  }

  id->deactivate();
  id->removeFromOAObjList();

  if( id->is_idle() ) {
    // If the last method invocation has already completed on
    // this object, and it doesn't need etherealising, we
    // can just get rid of it now.
    omni::internalLock->unlock();
    PortableServer::ServantActivator_ptr sa = pd_servantActivator;
    pd_lock.unlock();

    omniORB::logs(15, "Object is idle -- etherealise now.");

    PortableServer::Servant servant = DOWNCAST(id->servant());

    if( sa )
      add_object_to_etherealisation_queue(servant, sa, id->key(),
					  id->keysize(), 0);
    else
      servant->_remove_ref();
    id->die();
  }
  else {
    // When outstanding requests have completed the object
    // will be etherealised.
    omni::internalLock->unlock();
    detached_object();
    pd_lock.unlock();

    omniORB::logs(15, "Object is still busy -- etherealise later.");
  }
}


CORBA::Object_ptr
omniOrbPOA::create_reference(const char* intf)
{
  CHECK_NOT_NIL_OR_DESTROYED();
  if( !intf )  OMNIORB_THROW(BAD_PARAM,0, CORBA::COMPLETED_NO);
  if( pd_policy.user_assigned_id )  throw WrongPolicy();

  omniObjKey key;
  omniLocalIdentity* id;
  CORBA::ULong hash;

  pd_lock.lock();
  omni::internalLock->lock();

  // We need to be sure we use a new id ...

  do {
    create_new_key(key);
    hash = omni::hash(key.key(), key.size());
    id = omni::locateIdentity(key.key(), key.size(), hash);
  } while( id );

  pd_lock.unlock();

  id = omni::locateIdentity(key.key(), key.size(), hash, 1);
  omniObjRef* objref = omni::createObjRef(intf, CORBA::Object::_PD_repoId,
					  id, 0, 0, key.return_key());
  omni::internalLock->unlock();

  OMNIORB_ASSERT(objref);

  return (CORBA::Object_ptr) objref->_ptrToObjRef(CORBA::Object::_PD_repoId);
}


CORBA::Object_ptr
omniOrbPOA::create_reference_with_id(const PortableServer::ObjectId& oid,
				     const char* intf)
{
  CHECK_NOT_NIL_OR_DESTROYED();
  if( !intf )  OMNIORB_THROW(BAD_PARAM,0, CORBA::COMPLETED_NO);
  if( !pd_policy.user_assigned_id && oid.length() != SYS_ASSIGNED_ID_SIZE )
    OMNIORB_THROW(BAD_PARAM,0, CORBA::COMPLETED_NO);

  omniObjKey key;
  create_key(key, oid.NP_data(), oid.length());
  CORBA::ULong hash = omni::hash(key.key(), key.size());

  omni::internalLock->lock();

  omniLocalIdentity* id = omni::locateIdentity(key.key(), key.size(), hash, 1);
  omniObjRef* objref = omni::createObjRef(intf, CORBA::Object::_PD_repoId,
					  id, 0, 0, key.return_key());
  omni::internalLock->unlock();

  OMNIORB_ASSERT(objref);

  return (CORBA::Object_ptr) objref->_ptrToObjRef(CORBA::Object::_PD_repoId);
}


PortableServer::ObjectId*
omniOrbPOA::servant_to_id(PortableServer::Servant p_servant)
{
  CHECK_NOT_NIL_OR_DESTROYED();
  if( !p_servant )  OMNIORB_THROW(BAD_PARAM,0, CORBA::COMPLETED_NO);
  if( pd_policy.req_processing != RPP_DEFAULT_SERVANT &&
      !pd_policy.retain_servants )
    throw WrongPolicy();

  omni_tracedmutex_lock sync(pd_lock);

  if( 0 /*?? in context of invocation */ &&
      pd_policy.req_processing == RPP_DEFAULT_SERVANT &&
      p_servant == pd_defaultServant ) {
    OMNIORB_ASSERT(0);
  }

  omni_tracedmutex_lock sync2(*omni::internalLock);

  if( !pd_policy.multiple_id ) {
    // Search the servants identities, to see if it is
    // activated in this poa.

    omniLocalIdentity* id = p_servant->_identities();

    while( id && id->adapter() != this )  id = id->servantsNextIdentity();

    if( id ) {
      OMNIORB_ASSERT(pd_poaIdSize == 0 ||
		     !strcmp((const char*) pd_poaId, (const char*) id->key()));
      int idsize = id->keysize() - pd_poaIdSize;
      OMNIORB_ASSERT(idsize >= 0);
      PortableServer::ObjectId* ret = new PortableServer::ObjectId(idsize);
      ret->length(idsize);
      memcpy(ret->NP_data(), id->key() + pd_poaIdSize, idsize);
      return ret;
    }
  }

  if( !pd_policy.implicit_activation )  throw WrongPolicy();
  CHECK_NOT_DYING();

  // If we get here then we need to do an implicit activation.

  PortableServer::ObjectId* ret = new PortableServer::ObjectId();
  omniLocalIdentity* id = 0;
  omniObjKey key;

  do {
    const CORBA::Octet* oid;
    int idsize;
    create_new_key(key, &oid, &idsize);

    ret->length(idsize);
    memcpy(ret->NP_data(), oid, idsize);

    id = omni::activateObject(p_servant, this, key);

  } while( !id );

  p_servant->_add_ref();
  id->insertIntoOAObjList(&pd_activeObjList);

  return ret;
}


CORBA::Object_ptr
omniOrbPOA::servant_to_reference(PortableServer::Servant p_servant)
{
  CHECK_NOT_NIL_OR_DESTROYED();
  if( !p_servant )  OMNIORB_THROW(BAD_PARAM,0, CORBA::COMPLETED_NO);

  if( 0 /*?? in context of request on given servant */ ) {
    // Return reference associated with request.
    // Do we even care which POA the request was
    // associated with?  I think if the request
    // was not associated with this POA, then we
    // should probably carry on below ...
  }

  if( !pd_policy.retain_servants ||
      (pd_policy.multiple_id && !pd_policy.implicit_activation) )
    throw WrongPolicy();

  omni_tracedmutex_lock sync(pd_lock);
  omni_tracedmutex_lock sync2(*omni::internalLock);

  if( !pd_policy.multiple_id ) {
    // Search the servants identities, to see if it is
    // activated in this poa.

    omniLocalIdentity* id = p_servant->_identities();

    while( id && id->adapter() != this )  id = id->servantsNextIdentity();

    if( id ) {
      OMNIORB_ASSERT(pd_poaIdSize == 0 ||
		     !strcmp(pd_poaId, (const char*) id->key()));
      omniObjRef* objref = omni::createObjRef(p_servant->_mostDerivedRepoId(),
					      CORBA::Object::_PD_repoId, id);
      OMNIORB_ASSERT(objref);
      return (CORBA::Object_ptr) objref->_ptrToObjRef(CORBA::Object
						      ::_PD_repoId);
    }
  }

  if( !pd_policy.implicit_activation )  throw WrongPolicy();
  CHECK_NOT_DYING();

  // If we get here, then either the servant is not activated in
  // this POA, or we have the multiple id policy.  So do an
  // implicit activation.

  omniLocalIdentity* id = 0;
  omniObjKey key;

  do {
    const CORBA::Octet* oid;
    int idsize;
    create_new_key(key, &oid, &idsize);

    id = omni::activateObject(p_servant, this, key);

  } while( !id );

  p_servant->_add_ref();
  id->insertIntoOAObjList(&pd_activeObjList);

  omniObjRef* objref = omni::createObjRef(p_servant->_mostDerivedRepoId(),
					  CORBA::Object::_PD_repoId, id);
  OMNIORB_ASSERT(objref);

  return (CORBA::Object_ptr) objref->_ptrToObjRef(CORBA::Object::_PD_repoId);
}


PortableServer::Servant
omniOrbPOA::reference_to_servant(CORBA::Object_ptr reference)
{
  CHECK_NOT_NIL_OR_DESTROYED();

  if( CORBA::is_nil(reference) )
    OMNIORB_THROW(BAD_PARAM,0, CORBA::COMPLETED_NO);
  if( reference->_NP_is_pseudo() )  throw WrongAdapter();

  if( !pd_policy.retain_servants &&
      pd_policy.req_processing != RPP_DEFAULT_SERVANT )
    throw WrongPolicy();

  if( pd_policy.retain_servants ) {

    omni_tracedmutex_lock sync(*omni::internalLock);

    omniObjRef* objref = reference->_PR_getobj();
    omniLocalIdentity* id = objref->_localId();
    if( !id )  throw WrongAdapter();

    if( id->keysize() < pd_poaIdSize ||
	memcmp(id->key(), (const char*) pd_poaId, pd_poaIdSize) )
      throw WrongAdapter();

    omniServant* servant = id->servant();

    if( servant ) {
      PortableServer::Servant s = DOWNCAST(servant);
      s->_add_ref();
      return s;
    }
  }

  omni_tracedmutex_lock sync(pd_lock);

  if( pd_policy.req_processing == RPP_DEFAULT_SERVANT &&
      pd_defaultServant != 0 ) {
    pd_defaultServant->_add_ref();
    return pd_defaultServant;
  }

  throw ObjectNotActive();

#ifdef NEED_DUMMY_RETURN
  return 0;
#endif
}


PortableServer::ObjectId*
omniOrbPOA::reference_to_id(CORBA::Object_ptr reference)
{
  CHECK_NOT_NIL_OR_DESTROYED();

  if( CORBA::is_nil(reference) )
    OMNIORB_THROW(BAD_PARAM,0, CORBA::COMPLETED_NO);
  if( reference->_NP_is_pseudo() )  throw WrongAdapter();

  omni_tracedmutex_lock sync(*omni::internalLock);

  omniObjRef* objref = reference->_PR_getobj();
  omniLocalIdentity* id = objref->_localId();
  if( !id )  throw WrongAdapter();

  if( id->keysize() < pd_poaIdSize ||
      memcmp(id->key(), (const char*) pd_poaId, pd_poaIdSize) )
    throw WrongAdapter();

  int idsize = id->keysize() - pd_poaIdSize;
  PortableServer::ObjectId* ret = new PortableServer::ObjectId(idsize);
  ret->length(idsize);
  memcpy(ret->NP_data(), id->key() + pd_poaIdSize, idsize);

  return ret;
}


PortableServer::Servant
omniOrbPOA::id_to_servant(const PortableServer::ObjectId& oid)
{
  CHECK_NOT_NIL_OR_DESTROYED();

  if( !pd_policy.retain_servants &&
      pd_policy.req_processing != RPP_DEFAULT_SERVANT )
    throw WrongPolicy();

  if( pd_policy.retain_servants ) {

    omniObjKey key;
    create_key(key, oid.NP_data(), oid.length());
    CORBA::ULong hash = omni::hash(key.key(), key.size());

    omni::internalLock->lock();
    omniLocalIdentity* id = omni::locateIdentity(key.key(), key.size(), hash);

    PortableServer::Servant s = 0;
    if( id && id->servant() ) {
      s = DOWNCAST(id->servant());
      s->_add_ref();
    }
    omni::internalLock->unlock();
    if( s )  return s;
  }

  omni_tracedmutex_lock sync(pd_lock);

  if( pd_policy.req_processing == RPP_DEFAULT_SERVANT &&
      pd_defaultServant != 0 ) {
    pd_defaultServant->_add_ref();
    return pd_defaultServant;
  }

  throw ObjectNotActive();

#ifdef NEED_DUMMY_RETURN
  return 0;
#endif
}


CORBA::Object_ptr
omniOrbPOA::id_to_reference(const PortableServer::ObjectId& oid)
{
  CHECK_NOT_NIL_OR_DESTROYED();

  if( !pd_policy.retain_servants )
    throw WrongPolicy();

  omniObjKey key;
  create_key(key, oid.NP_data(), oid.length());
  CORBA::ULong hash = omni::hash(key.key(), key.size());

  omni::internalLock->lock();

  omniLocalIdentity* id = omni::locateIdentity(key.key(), key.size(), hash);

  if( !(id && id->servant()) ) {
    omni::internalLock->unlock();
    throw ObjectNotActive();
  }

  omniObjRef* objref = omni::createObjRef(id->servant()->_mostDerivedRepoId(),
					  CORBA::Object::_PD_repoId, id,
					  0, 0, key.return_key());
  omni::internalLock->unlock();
  OMNIORB_ASSERT(objref);

  return (CORBA::Object_ptr) objref->_ptrToObjRef(CORBA::Object::_PD_repoId);
}

///////////////////
// CORBA::Object //
///////////////////

_CORBA_Boolean
omniOrbPOA::_non_existent()
{
  if( _NP_is_nil() )  _CORBA_invoked_nil_pseudo_ref();

  omni_tracedmutex_lock sync(pd_lock);

  return pd_destroyed ? 1 : 0;
}


void*
omniOrbPOA::_ptrToObjRef(const char* repoId)
{
  OMNIORB_ASSERT(repoId);

  if( !strcmp(repoId, CORBA::Object::_PD_repoId) )
    return (CORBA::Object_ptr) this;
  if( !strcmp(repoId, PortableServer::POA::_PD_repoId) )
    return (PortableServer::POA_ptr) this;

  return 0;
}


void
omniOrbPOA::_NP_incrRefCount()
{
  // Should be identical to incrRefCount().
  omni::poRcLock->lock();
  pd_refCount++;
  omni::poRcLock->unlock();
}


void
omniOrbPOA::_NP_decrRefCount()
{
  decrRefCount();
}

/////////////////////////////
// Override omniObjAdapter //
/////////////////////////////

void
omniOrbPOA::incrRefCount()
{
  omni::poRcLock->lock();
  pd_refCount++;
  omni::poRcLock->unlock();
}


void
omniOrbPOA::decrRefCount()
{
  omni::poRcLock->lock();
  int done = --pd_refCount > 0;
  omni::poRcLock->unlock();
  if( done )  return;

  OMNIORB_USER_CHECK(pd_destroyed == 2);
  OMNIORB_USER_CHECK(pd_refCount == 0);
  // If either of these fails then the application has released the
  // POA reference too many times.

  CORBA::release(pd_manager);
  if( pd_adapterActivator )  CORBA::release(pd_adapterActivator);
  if( pd_servantActivator )  CORBA::release(pd_servantActivator);
  if( pd_servantLocator )    CORBA::release(pd_servantLocator);

  delete this;
}


void
omniOrbPOA::dispatch(GIOP_S& giop_s, omniLocalIdentity* id)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);
  OMNIORB_ASSERT(id);  OMNIORB_ASSERT(id->servant());
  OMNIORB_ASSERT(id->adapter() == this);

  enterAdapter();

  if( pd_rq_state != (int) PortableServer::POAManager::ACTIVE )
    synchronise_request(id);

  startRequest();

  omni::internalLock->unlock();

  omni_optional_lock sync(*pd_call_lock, !pd_policy.single_threaded,
			  !pd_policy.single_threaded);

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
omniOrbPOA::dispatch(GIOP_S& giop_s, const CORBA::Octet* key, int keysize)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 0);
  OMNIORB_ASSERT(key);
  OMNIORB_ASSERT(keysize >= pd_poaIdSize &&
		 !memcmp(key, (const char*) pd_poaId, pd_poaIdSize));

  // Check that the key is the right size (if system generated).
  if( !pd_policy.user_assigned_id &&
      keysize - pd_poaIdSize != SYS_ASSIGNED_ID_SIZE )
    OMNIORB_THROW(OBJ_ADAPTER,0, CORBA::COMPLETED_NO);

  switch( pd_policy.req_processing ) {
  case RPP_ACTIVE_OBJ_MAP:
    OMNIORB_THROW(OBJECT_NOT_EXIST,0, CORBA::COMPLETED_NO);
    break;

  case RPP_DEFAULT_SERVANT:
    dispatch_to_ds(giop_s, key, keysize);
    break;

  case RPP_SERVANT_MANAGER:
    if( pd_policy.retain_servants )  dispatch_to_sa(giop_s, key, keysize);
    else                             dispatch_to_sl(giop_s, key, keysize);
    break;
  };
}


void
omniOrbPOA::dispatch(omniCallDescriptor& call_desc, omniLocalIdentity* id)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);
  OMNIORB_ASSERT(id);  OMNIORB_ASSERT(id->servant());
  OMNIORB_ASSERT(id->adapter() == this);

  enterAdapter();

  if( pd_rq_state != (int) PortableServer::POAManager::ACTIVE )
    synchronise_request(id);

  startRequest();

  omni::internalLock->unlock();

  omni_optional_lock sync(*pd_call_lock, !pd_policy.single_threaded,
			  !pd_policy.single_threaded);

  if( omniORB::traceInvocations ) {
    omniORB::logger l;
    l << "Dispatching local call \'" << call_desc.op() << "\' to "
      << id << '\n';
  }

  call_desc.doLocalCall(id->servant());
}


int
omniOrbPOA::objectExists(const _CORBA_Octet*, int)
{
  if( pd_policy.req_processing == RPP_ACTIVE_OBJ_MAP )
    return 0;

  pd_lock.lock();
  int ret = pd_defaultServant || pd_servantActivator || pd_servantLocator;
  pd_lock.unlock();
  return ret;
}


void
omniOrbPOA::lastInvocationHasCompleted(omniLocalIdentity* id)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 0);

  if( omniORB::trace(15) ) {
    omniORB::logger l;
    l << "POA(" << (char*) pd_name << ") etherealising detached object.\n"
      << " id: " << id->servant()->_mostDerivedRepoId() << "\n";
  }

  PortableServer::ServantActivator_ptr sa = 0;

  // This lock _could_ go inside the body of the 'if' below, but I want
  // to ensure that we take this lock no matter what.  The reason is to
  // ensure that detached_object() is called (in deactivate_object())
  // before met_detached_object() (below).  Otherwise we get a nasty
  // race ...
  pd_lock.lock();

  if( (pd_policy.req_processing == RPP_SERVANT_MANAGER &&
       pd_policy.retain_servants) || pd_dying ) {

    // The omniLocalIdentity still holds a reference to us, and
    // we hold a reference to the servant activator, so we don't
    // need to grab a reference to pd_servantActivator here.
    // (since it is also immutable once set).
    sa = pd_servantActivator;

    if( pd_dying && !pd_destroyed ) {
      // We cannot etherealise until apparent destruction is complete.
      // Wait for apparent destruction.
      while( !pd_destroyed )  pd_deathSignal.wait();
    }
  }

  pd_lock.unlock();

  PortableServer::Servant servant = DOWNCAST(id->servant());

  if( sa ) {
    // Delegate etherealisation to a separate thread.
    add_object_to_etherealisation_queue(servant, sa, id->key(),
					id->keysize(), 0, 1);
  }
  else {
    servant->_remove_ref();
    met_detached_object();
  }

  id->die();
}

//////////////////////
// omniORB Internal //
//////////////////////

static void generateUniqueId(CORBA::Octet* k);


omniOrbPOA::omniOrbPOA(const char* name,
		       omniOrbPOAManager* manager,
		       const Policies& policies,
		       omniOrbPOA* parent)
  : OMNIORB_BASE_CTOR(PortableServer::)POA(0),
    pd_destroyed(0),
    pd_dying(0),
    pd_refCount(1),
    pd_parent(parent),
    pd_adapterActivator(0),
    pd_servantActivator(0),
    pd_servantLocator(0),
    pd_defaultServant(0),
    pd_rq_state(PortableServer::POAManager::HOLDING),
    pd_call_lock(0),
    pd_deathSignal(&pd_lock),
    pd_oidIndex(0),
    pd_activeObjList(0)
{
  OMNIORB_ASSERT(name);
  OMNIORB_ASSERT(manager);

  pd_name = name;
  pd_manager = manager;

  if (pd_parent == (omniOrbPOA*)1) {
    // This is the magic INS POA
    OMNIORB_ASSERT(theRootPOA);
    theRootPOA->incrRefCount();
    pd_parent = theRootPOA;

    int fnlen   = strlen(pd_parent->pd_fullname) + strlen(name) + 1;
    pd_fullname = omni::allocString(fnlen);
    strcpy(pd_fullname, pd_parent->pd_fullname);
    strcat(pd_fullname, POA_NAME_SEP_STR);
    strcat(pd_fullname, name);
    
    pd_poaIdSize = 0;
    pd_poaId     = (const char*)"";
  }
  else if( pd_parent ) {
    int fnlen = strlen(parent->pd_fullname) + strlen(name) + 1;
    pd_fullname = omni::allocString(fnlen);
    strcpy(pd_fullname, parent->pd_fullname);
    strcat(pd_fullname, POA_NAME_SEP_STR);
    strcat(pd_fullname, name);

    pd_poaIdSize = fnlen + 1;
    if( policies.transient )  pd_poaIdSize += TRANSIENT_SUFFIX_SIZE + 1;
    pd_poaId = omni::allocString(pd_poaIdSize - 1);
    strcpy(pd_poaId, pd_fullname);
    if( policies.transient ) {
      ((char*) pd_poaId)[fnlen] = TRANSIENT_SUFFIX_SEP;
      generateUniqueId((_CORBA_Octet*) ((char*) pd_poaId + fnlen + 1));
      ((char*) pd_poaId)[pd_poaIdSize - 1] = '\0';
    }
  }
  else {
    // This is the root poa.
    OMNIORB_ASSERT(policies.transient);
    pd_fullname = (const char*) "";
    pd_poaIdSize = 1 + TRANSIENT_SUFFIX_SIZE + 1;
    pd_poaId = omni::allocString(pd_poaIdSize - 1);
    ((char*) pd_poaId)[0] = TRANSIENT_SUFFIX_SEP;
    generateUniqueId((_CORBA_Octet*) ((char*) pd_poaId + 1));
    ((char*) pd_poaId)[pd_poaIdSize - 1] = '\0';
  }

  // We assume that the policies given have been checked for validity.
  pd_policy = policies;

  if( pd_policy.single_threaded )  pd_call_lock = new omni_tracedmutex();
  else                             pd_call_lock = &pd_lock;
}


omniOrbPOA::omniOrbPOA()  // nil constructor
  : OMNIORB_BASE_CTOR(PortableServer::)POA(1),
    pd_destroyed(1),
    pd_dying(1),
    pd_refCount(0),
    pd_name((const char*) 0),
    pd_parent(0),
    pd_manager(0),
    pd_adapterActivator(0),
    pd_servantActivator(0),
    pd_servantLocator(0),
    pd_defaultServant(0),
    pd_rq_state(PortableServer::POAManager::INACTIVE),
    pd_poaIdSize(0),
    pd_deathSignal(&pd_lock),
    pd_oidIndex(0),
    pd_activeObjList(0)
{
}


void
omniOrbPOA::do_destroy(CORBA::Boolean etherealize_objects)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 0);
  ASSERT_OMNI_TRACEDMUTEX_HELD(pd_lock, 0);
  ASSERT_OMNI_TRACEDMUTEX_HELD(poa_lock, 0);
  OMNIORB_ASSERT(pd_dying);

  while( pd_children.length() ) {
    try { pd_children[0]->destroy(etherealize_objects, 1); }
    catch(...) {}
  }

  OMNIORB_ASSERT(pd_children.length() == 0);

  if( omniORB::trace(10) ) {
    omniORB::logger l;
    l << "Deactivating all POA(" << (char*) pd_name << ")'s objects.\n";
  }

  // Deactivate all objects in the active object map.

  omniLocalIdentity* obj_list = 0;

  pd_lock.lock();
  if( pd_activeObjList )  pd_activeObjList->reRootOAObjList(&obj_list);
  PortableServer::ServantActivator_ptr sa = pd_servantActivator;
  pd_lock.unlock();

  omni::internalLock->lock();
  deactivate_objects(obj_list);

  if( omniORB::trace(10) ) {
    omniORB::logger l;
    l << "Waiting for requests to complete on POA(" << (char*) pd_name
      << ").\n";
  }

  // There may still be requests blocked in synchronise_request(),
  // so we change the state here to INACTIVE so that they will
  // throw an appropriate exception and complete.  This also allows
  // other threads to block waiting for us to change state or be
  // destroyed.
  int old_state = pd_rq_state;
  pd_rq_state = (int) PortableServer::POAManager::INACTIVE;
  if( old_state == (int) PortableServer::POAManager::HOLDING ) {
    omni::internalLock->unlock();
    pd_signal.broadcast();
    omni::internalLock->lock();
  }

  waitForAllRequestsToComplete(1);
  complete_object_deactivation(obj_list);
  omni::internalLock->unlock();

  // Apparent destruction of POA occurs before etherealisations.
  pd_lock.lock();
  pd_destroyed = 1;
  PortableServer::Servant defaultServant = pd_defaultServant;
  pd_defaultServant = 0;
  pd_lock.unlock();

  if( omniORB::trace(10) ) {
    omniORB::logger l;
    l << "Etherealising POA(" << (char*) pd_name << ")'s objects.\n";
  }

  // Signal so that any detached objects waiting to etherealise
  // can proceed.
  pd_deathSignal.broadcast();

  // Etherealise the objects.
  this->etherealise_objects(obj_list, etherealize_objects, sa);

  // Wait for objects which have been detached to complete their
  // etherealisations.
  wait_for_detached_objects();

  if( defaultServant )  defaultServant->_remove_ref();

  poa_lock.lock();
  pd_lock.lock();
  pd_destroyed = 2;
  if( pd_parent ) {
    pd_parent->lose_child(this);
    pd_parent = 0;
    if (theINSPOA == this) theINSPOA = 0;
  } else {
    OMNIORB_ASSERT(theRootPOA == this);
    theRootPOA = 0;
  }
  poa_lock.unlock();
  pd_lock.unlock();

  pd_deathSignal.broadcast();

  try { adapterInactive(); } catch(...) {}

  if( omniORB::trace(10) ) {
    omniORB::logger l;
    l << "Destruction of POA(" << (char*) pd_name << ") complete.\n";
  }

  CORBA::release(this);
}


void
omniOrbPOA::pm_change_state(PortableServer::POAManager::State new_state)
{
  omni::internalLock->lock();
  pd_rq_state = (int) new_state;
  omni::internalLock->unlock();

  pd_signal.broadcast();
}


void
omniOrbPOA::pm_waitForReqCmpltnOrSttChnge(omniOrbPOAManager::State state)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 0);

  // Wait until all outstanding requests have completed,
  // or until the state has changed.
  omni::internalLock->lock();

  pd_signalOnZeroInvocations++;

  while( pd_rq_state == (int) state && pd_nReqActive )
    pd_signal.wait();

  pd_signalOnZeroInvocations--;

  omni::internalLock->unlock();
}


void
omniOrbPOA::pm_deactivate(_CORBA_Boolean etherealize_objects)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 0);
  ASSERT_OMNI_TRACEDMUTEX_HELD(pd_lock, 0);

  pd_lock.lock();
  if( pd_dying ) {
    // If being destroyed by another thread, then we just
    // have to wait until that completes.
    while( pd_destroyed != 2 )  pd_deathSignal.wait();
    pd_lock.unlock();
    return;
  }

  omniLocalIdentity* obj_list = 0;
  if( pd_activeObjList )  pd_activeObjList->reRootOAObjList(&obj_list);
  PortableServer::ServantActivator_ptr sa = pd_servantActivator;

  // We pretend to detach an object here, so that if some other
  // thread tries to destroy this POA, they will have to block
  // until we've finished etherealising these objects.
  if( obj_list )  detached_object();
  pd_lock.unlock();

  omni::internalLock->lock();
  deactivate_objects(obj_list);
  waitForAllRequestsToComplete(1);
  complete_object_deactivation(obj_list);
  omni::internalLock->unlock();
  if( obj_list ) {
    if( etherealize_objects )
      this->etherealise_objects(obj_list, etherealize_objects, sa);

    met_detached_object();
    wait_for_detached_objects();
  }

}


void*
omniOrbPOA::servant__this(PortableServer::Servant p_servant,
			  const char* repoId)
{
  CHECK_NOT_NIL();
  OMNIORB_ASSERT(p_servant && repoId);
  if( !pd_policy.retain_servants ||
      (pd_policy.multiple_id && !pd_policy.implicit_activation) )
    throw WrongPolicy();

  omni_tracedmutex_lock sync(pd_lock);
  CHECK_NOT_DESTROYED();
  omni_tracedmutex_lock sync2(*omni::internalLock);

  if( !pd_policy.multiple_id ) {
    // Search the servants identities, to see if it is
    // activated in this poa.

    omniLocalIdentity* id = p_servant->_identities();

    while( id && id->adapter() != this )  id = id->servantsNextIdentity();

    if( id ) {
      OMNIORB_ASSERT(pd_poaIdSize == 0 ||
		     !strcmp(pd_poaId, (const char*) id->key()));
      omniObjRef* objref = omni::createObjRef(p_servant->_mostDerivedRepoId(),
					      repoId, id);
      OMNIORB_ASSERT(objref);
      return objref->_ptrToObjRef(repoId);
    }
  }

  if( !pd_policy.implicit_activation )  throw WrongPolicy();
  CHECK_NOT_DYING();

  // If we get here, then we have the implicit activation policy,
  // and either the servant is not activated in this POA, or we
  // have the multiple id policy.

  omniLocalIdentity* id = 0;
  omniObjKey key;

  do {
    const CORBA::Octet* oid;
    int idsize;
    create_new_key(key, &oid, &idsize);

    id = omni::activateObject(p_servant, this, key);

  } while( !id );

  p_servant->_add_ref();
  id->insertIntoOAObjList(&pd_activeObjList);

  omniObjRef* objref = omni::createObjRef(p_servant->_mostDerivedRepoId(),
					  repoId, id);
  OMNIORB_ASSERT(objref);

  return objref->_ptrToObjRef(repoId);
}


static void
initialise_poa()
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(poa_lock, 1);
  OMNIORB_ASSERT(!theRootPOA);

  // Initialise the object adapter - doesn't matter if this has
  // already happened.
  omniObjAdapter::initialise();

  // The root poa differs from the default policies only in that
  // it has the IMPLICIT_ACTIVATION policy.
  omniOrbPOA::Policies policy;
  policy.single_threaded = 0;
  policy.transient = 1;
  policy.multiple_id = 0;
  policy.user_assigned_id = 0;
  policy.retain_servants = 1;
  policy.req_processing = omniOrbPOA::RPP_ACTIVE_OBJ_MAP;
  policy.implicit_activation = 1;

  omniOrbPOAManager* manager = new omniOrbPOAManager();

  theRootPOA = new omniOrbPOA("RootPOA", manager, policy, 0);
  manager->gain_poa(theRootPOA);
  theRootPOA->adapterActive();
}


PortableServer::POA_ptr
omniOrbPOA::rootPOA(int init_if_none)
{
  omni_tracedmutex_lock sync(poa_lock);

  if( !theRootPOA ) {
    if( !init_if_none )  return 0;
    ::initialise_poa();
  }

  theRootPOA->incrRefCount();
  return theRootPOA;
}


PortableServer::POA_ptr
omniOrbPOA::omniINSPOA()
{
  omni_tracedmutex_lock sync(poa_lock);

  if (!theINSPOA) {
    if (!theRootPOA)
      ::initialise_poa();

    omniOrbPOA::Policies policy;
    policy.single_threaded     = 0;
    policy.transient           = 0;
    policy.multiple_id         = 0;
    policy.user_assigned_id    = 1;
    policy.retain_servants     = 1;
    policy.req_processing      = omniOrbPOA::RPP_ACTIVE_OBJ_MAP;
    policy.implicit_activation = 1;

    omni_tracedmutex_lock sync2(theRootPOA->pd_lock);

    if (theRootPOA->pd_dying)
      OMNIORB_THROW(OBJ_ADAPTER, 0, CORBA::COMPLETED_NO);

    omniOrbPOAManager* manager = new omniOrbPOAManager();

    theINSPOA = new omniOrbPOA("omniINSPOA", manager, policy, (omniOrbPOA*)1);

    theRootPOA->insert_child(theINSPOA);

    manager->gain_poa(theINSPOA);
    theINSPOA->adapterActive();
  }
  theINSPOA->incrRefCount();
  return theINSPOA;
}


void
omniOrbPOA::shutdown()
{
  poa_lock.lock();
  omniOrbPOA* rp = theRootPOA;
  if( rp )  rp->incrRefCount();
  poa_lock.unlock();

  try {
    if( rp )  rp->destroy(1, 1);
  }
  catch(CORBA::OBJECT_NOT_EXIST&) {
    // That's okay -- someone else got there first.
  }

  if( rp )  CORBA::release(rp);
}


omniOrbPOA*
omniOrbPOA::getAdapter(const _CORBA_Octet* key, int keysize)
{
  const char* k = (const char*) key;
  const char* kend = k + keysize;

  omni_tracedmutex_lock sync(poa_lock);

  if( !theRootPOA )  return 0;
  omniOrbPOA* poa = theRootPOA;

  while( k < kend && *k == POA_NAME_SEP ) {

    k++;
    const char* name = k;

    while( k < kend && *k && *k != POA_NAME_SEP && *k != TRANSIENT_SUFFIX_SEP )
      k++;

    if( k == kend )  return 0;

    omniOrbPOA* child = poa->find_child(name, k);

    if( !child || child->pd_dying ) {
      if( poa->pd_adapterActivator ) {
	// We need to extract the name properly here.
	int namelen = k - name;
	char* thename = new char[namelen + 1];
	memcpy(thename, name, namelen);
	thename[namelen] = '\0';

	if( !(child = poa->attempt_to_activate_adapter(thename)) )
	  return 0;
      }
      else
	return 0;
    }

    poa = child;
  }

  if( k == kend )  return 0;

  if( *k == TRANSIENT_SUFFIX_SEP ) {
    // Check that <poa> is indeed a TRANSIENT poa, and that
    // the transient id matches.
    k++;
    if( k + TRANSIENT_SUFFIX_SIZE >= kend )  return 0;
    const char* t = (char*) poa->pd_poaId +
      poa->pd_poaIdSize - TRANSIENT_SUFFIX_SIZE - 1;
    int i = TRANSIENT_SUFFIX_SIZE;
    while( i-- )  if( *k++ != *t++ )  return 0;
  }
  if( *k )  return 0;

  poa->incrRefCount();
  return poa;
}

//////////////////////////////////////////////////////////////////////
////////////////////////// Internal helpers //////////////////////////
//////////////////////////////////////////////////////////////////////

void
omniOrbPOA::create_key(omniObjKey& key_out, const _CORBA_Octet* id, int idsize)
{
  key_out.set_size(pd_poaIdSize + idsize);
  CORBA::Octet* k = key_out.write_key();

  memcpy(k, (const char*) pd_poaId, pd_poaIdSize);
  memcpy(k + pd_poaIdSize, id, idsize);
}


void
omniOrbPOA::create_new_key(omniObjKey& key_out, const CORBA::Octet** id,
			   int* idsize)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(pd_lock, 1);

  key_out.set_size(pd_poaIdSize + SYS_ASSIGNED_ID_SIZE);
  CORBA::Octet* k = key_out.write_key();

  _CORBA_ULong idx = pd_oidIndex;
  if (omni::myByteOrder) {
    idx = (((idx & 0xff000000) >> 24) |
	   ((idx & 0x00ff0000) >> 8 ) |
	   ((idx & 0x0000ff00) << 8 ) |
	   ((idx & 0x000000ff) << 24));
  }

  memcpy(k, (const char*) pd_poaId, pd_poaIdSize);
  memcpy(k + pd_poaIdSize, (const CORBA::Octet*) &idx,
	 SYS_ASSIGNED_ID_SIZE);

  pd_oidIndex++;

  if( id )      *id = k + pd_poaIdSize;
  if( idsize )  *idsize = SYS_ASSIGNED_ID_SIZE;
}


omniOrbPOA*
omniOrbPOA::find_child(const char* name)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(poa_lock, 1);

  int count = pd_children.length();
  int bottom = 0;
  int top = count;

  while( bottom < top ){

    int middle = (bottom + top) / 2;

    int cmp = strcmp(name, pd_children[middle]->pd_name);

    if( cmp < 0 )       top = middle;
    else if( cmp > 0 )  bottom = middle + 1;
    else                return pd_children[middle];
  }

  return 0;
}


static inline int
tstrcmp(const char* start1, const char* end1,
	const char* start2, const char* end2)
{
  while( start1 != end1 && start2 != end2 && *start1 == *start2 )
    start1++, start2++;

  int c1 = (start1 == end1) ? 0 : *start1;
  int c2 = (start2 == end2) ? 0 : *start2;

  return c1 - c2;
}


omniOrbPOA*
omniOrbPOA::find_child(const char* name_start, const char* name_end)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(poa_lock, 1);

  int count = pd_children.length();
  int bottom = 0;
  int top = count;

  while( bottom < top ){

    int middle = (bottom + top) / 2;

    const char* cn = pd_children[middle]->pd_name;
    int cmp = tstrcmp(name_start, name_end, cn, cn + strlen(cn));

    if( cmp < 0 )       top = middle;
    else if( cmp > 0 )  bottom = middle + 1;
    else                return pd_children[middle];
  }

  return 0;
}


void
omniOrbPOA::insert_child(omniOrbPOA* child)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(poa_lock, 1);

  // We assume that the child does not already exist
  // in our list of children.  First determine the
  // insertion point in the sorted array using a
  // binary search.

  int count = pd_children.length();
  int bottom = 0;
  int top = count;

  while( bottom < top ) {

    int middle = (bottom + top) / 2;

    int cmp = strcmp(child->pd_name, pd_children[middle]->pd_name);

    if( cmp < 0 )       top = middle;
    else if( cmp > 0 )  bottom = middle + 1;
    else                OMNIORB_ASSERT(0);
  }

  OMNIORB_ASSERT(top == bottom);

  pd_children.length(count + 1);

  for( int i = count; i > bottom; i-- )
    pd_children[i] = pd_children[i - 1];

  pd_children[bottom] = child;
}


void
omniOrbPOA::lose_child(omniOrbPOA* child)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(poa_lock, 1);
  OMNIORB_ASSERT(child);

  int count = pd_children.length();
  int bottom = 0;
  int top = count;
  int middle = -1;

  while( bottom < top ){

    middle = (bottom + top) / 2;

    int cmp = strcmp(child->pd_name, pd_children[middle]->pd_name);

    if( cmp < 0 )       top = middle;
    else if( cmp > 0 )  bottom = middle + 1;
    else                break;
  }

  for( int i = middle; i < count - 1; i++ )
    pd_children[i] = pd_children[i + 1];

  pd_children.length(count - 1);
}


int
omniOrbPOA::adapter_name_is_valid(const char* name)
{
  if( !name )  return 0;

  while( *name )
    switch( *name ) {
    case POA_NAME_SEP:
    case TRANSIENT_SUFFIX_SEP:
      return 0;
    default:
      name++;
      break;
    }

  return 1;
}


void
omniOrbPOA::synchronise_request(omniLocalIdentity* lid)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);

  // Wait until the request can proceed, or discard it.

  while( pd_rq_state == (int) PortableServer::POAManager::HOLDING ) {
    if( omniORB::poaHoldRequestTimeout ) {
      unsigned long sec, nsec;
      omni_thread::get_time(&sec, &nsec, omniORB::poaHoldRequestTimeout);
      if( !pd_signal.timedwait(sec, nsec) ) {
	// We have to do startRequest() here, since the identity
	// will do endInvocation() when we pass through there.
	startRequest();
	omni::internalLock->unlock();
	OMNIORB_THROW(TRANSIENT,0, CORBA::COMPLETED_NO);
      }
    }
    else
      pd_signal.wait();
  }

  switch( pd_rq_state ) {
  case (int) PortableServer::POAManager::HOLDING:
    OMNIORB_ASSERT(0);

  case (int) PortableServer::POAManager::ACTIVE:
    break;

  case (int) PortableServer::POAManager::DISCARDING:
    // We have to do startRequest() here, since the identity
    // will do endInvocation() when we pass through there.
    startRequest();
    omni::internalLock->unlock();
    OMNIORB_THROW(TRANSIENT,0, CORBA::COMPLETED_NO);

  case (int) PortableServer::POAManager::INACTIVE:
    // We have to do startRequest() here, since the identity
    // will do endInvocation() when we pass through there.
    startRequest();
    omni::internalLock->unlock();
    // This came from Henning & Vinoski.  Not sure it is
    // very appropriate looking at the description of
    // CORBA::OBJ_ADAPTER.
    OMNIORB_THROW(OBJ_ADAPTER,0, CORBA::COMPLETED_NO);
  }

  // Check to see if the object has been deactivated while we've been
  // holding. If so, throw a TRANSIENT exception.
  //
  // Note that in some cases, such as dispatch through a default
  // servant or with a servant locator, the LocalIdentity is not in
  // the POA's list of identities, so deactivated() returns true, even
  // though "activated" is not really a meaningful concept. Rather
  // than treat this as a special case (and detect if the default
  // servant has changed under our feet, etc.), we always throw
  // TRANSIENT and let the caller retry. It's a very rare situation,
  // and if we've been holding, we aren't too concerned about
  // performance anyway.

  CORBA::Boolean deactivated;
  pd_lock.lock();
  deactivated = lid->deactivated();
  pd_lock.unlock();

  if (deactivated) {
    // We have to do startRequest() here, since the identity
    // will do endInvocation() when we pass through there.
    startRequest();
    omni::internalLock->unlock();
    OMNIORB_THROW(TRANSIENT,0, CORBA::COMPLETED_NO);
  }
}


void
omniOrbPOA::deactivate_objects(omniLocalIdentity* id)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);

  while( id ) {
    omni::deactivateObject(id->key(), id->keysize());
    id = id->nextInOAObjList();
  }
}


void
omniOrbPOA::complete_object_deactivation(omniLocalIdentity* id)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);

  while( id ) {
    id->deactivate();
    OMNIORB_ASSERT(id->is_idle());
    id = id->nextInOAObjList();
  }
}


void
omniOrbPOA::etherealise_objects(omniLocalIdentity* id,
				_CORBA_Boolean etherealise,
				PortableServer::ServantActivator_ptr sa)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 0);
  ASSERT_OMNI_TRACEDMUTEX_HELD(pd_lock, 0);

  while( id ) {
    OMNIORB_ASSERT(id->is_idle());

    omniLocalIdentity* next = id->nextInOAObjList();

    PortableServer::Servant servant = DOWNCAST(id->servant());

    if( sa && etherealise )
      // We have to do these etherealisations in a separate thread
      // to ensure they are properly serealised.
      add_object_to_etherealisation_queue(servant, sa, id->key(),
					  id->keysize(), 1);
    else
      servant->_remove_ref();

    id->die();
    id = next;
  }
}


static omniTaskQueue* etherealisation_queue = 0;


void
omniOrbPOA::add_object_to_etherealisation_queue(
				PortableServer::Servant servant,
				PortableServer::ServantActivator_ptr sa,
				const _CORBA_Octet* key, int keysize,
				int cleanup_in_progress, int detached)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 0);
  OMNIORB_ASSERT(servant && key);

  Etherealiser* e = new Etherealiser(servant, sa, this, key + pd_poaIdSize,
			     keysize - pd_poaIdSize, cleanup_in_progress);

  if( !detached )  detached_object();

  omni::internalLock->lock();
  if( !etherealisation_queue )  etherealisation_queue = new omniTaskQueue;
  e->set_is_last(!servant->_identities());
  etherealisation_queue->insert(e);
  omni::internalLock->unlock();

  // NB. remaining_activations is wrong at the mo' -- should only
  // refer to activations of the servant in *this* poa.  However,
  // I think it is much more sensible for it to indicate whether
  // or not there exist any other activations of this servant in
  // the ORB as a whole.  So I'm gonna stick with this just for
  // the moment.
  //  See CORBA 2.3 11.3.5.2
}


void
omniOrbPOA::dispatch_to_ds(GIOP_S& giop_s, const CORBA::Octet* key,
			   int keysize)
{
  pd_lock.lock();
  if( !pd_defaultServant ) {
    pd_lock.unlock();
    OMNIORB_THROW(OBJ_ADAPTER,0, CORBA::COMPLETED_NO);
  }

  pd_defaultServant->_add_ref();
  PortableServer::ServantBase_var servant(pd_defaultServant);
  pd_lock.unlock();

  omniLocalIdentity the_id(key, keysize);
  the_id.setServant((PortableServer::Servant) servant, this);

  omniORB::logs(10, "Dispatching through default servant");

  omni::internalLock->lock();
  the_id.dispatch(giop_s);
}


void
omniOrbPOA::dispatch_to_sa(GIOP_S& giop_s, const CORBA::Octet* key,
			   int keysize)
{
  // A bit of prep. outside the critical sections...
  int idsize = keysize - pd_poaIdSize;
  PortableServer::ObjectId oid(idsize);
  oid.length(idsize);
  memcpy(oid.NP_data(), key + pd_poaIdSize, idsize);
  CORBA::ULong hash = omni::hash(key, keysize);

  servant_activator_lock.lock();
  pd_lock.lock();
  if( pd_dying ) {
    pd_lock.unlock();
    servant_activator_lock.unlock();
    OMNIORB_THROW(OBJECT_NOT_EXIST,0, CORBA::COMPLETED_NO);
  }
  if( !pd_servantActivator ) {
    pd_lock.unlock();
    servant_activator_lock.unlock();
    OMNIORB_THROW(OBJ_ADAPTER,0, CORBA::COMPLETED_NO);
  }
  PortableServer::ServantActivator::_duplicate(pd_servantActivator);
  PortableServer::ServantActivator_var sa(pd_servantActivator);

  // We have to be sure that the object is not in the active object
  // map before we call the activator.  Thus we check that it is not
  // activated since the previous check here, and hold the lock to
  // make sure it is not concurrently incarnated by another
  // servant activator.
  //  However, we cannot be sure that it is not being activated by
  // another thread, not using a servant activator (since we cannot
  // hold the required locks whilst making the upcall).  The only
  // way to ensure this would be to put a place-holder in the object
  // table which says 'I'm about to be activated'.  Nasty.

  omni::internalLock->lock();
  omniLocalIdentity* id = omni::locateIdentity(key, keysize, hash);
  if( id && id->servant() ) {
    pd_lock.unlock();
    servant_activator_lock.unlock();
    id->dispatch(giop_s);
    return;
  }

  // We call enterAdapter() here to ensure that the POA can't
  // be fully destroyed before these objects have a chance to
  // be 'postinvoke'd.
  enterAdapter();
  omni::internalLock->unlock();
  pd_lock.unlock();

  PortableServer::Servant servant;

  try {
    servant = sa->incarnate(oid, this);
  }
  catch(PortableServer::ForwardRequest& fr) {
    servant_activator_lock.unlock();
    exitAdapter();
    throw omniORB::LOCATION_FORWARD(
			    CORBA::Object::_duplicate(fr.forward_reference));
  }
#ifndef HAS_Cplusplus_catch_exception_by_base
#define RETHROW_EXCEPTION(name)  \
  catch (CORBA::name& ex) {  \
    servant_activator_lock.unlock();  \
    exitAdapter();  \
    throw;  \
  }
  OMNIORB_FOR_EACH_SYS_EXCEPTION(RETHROW_EXCEPTION)
#undef RETHROW_EXCEPTION
#else
  catch(CORBA::SystemException&) {
    servant_activator_lock.unlock();
    exitAdapter();
    throw;
  }
#endif
  catch(...) {
    servant_activator_lock.unlock();
    exitAdapter();
    OMNIORB_THROW(OBJ_ADAPTER,0, CORBA::COMPLETED_NO);
  }
  servant_activator_lock.unlock();

  if( !servant ) {
    exitAdapter();
    omniORB::logs(5, "ServantActivator::incarnate() returned 0 (zero)!");
    OMNIORB_THROW(OBJ_ADAPTER,0, CORBA::COMPLETED_NO);
  }

  pd_lock.lock();
  if( pd_dying ) {
    pd_lock.unlock();
    add_object_to_etherealisation_queue(servant, sa, key, keysize, 1);
    exitAdapter();
    return;
  }
  omni::internalLock->lock();
  if( !pd_policy.multiple_id ) {
    // Check the servant is not already active in this poa.
    id = servant->_identities();
    while( id && id->adapter() != this )  id = id->servantsNextIdentity();
    if( id ) {
      exitAdapter(1);
      pd_lock.unlock();
      if( omniORB::trace(2) ) {
	omniORB::logger l;
	l << "A servant activator returned a servant which is already "
	  "activated\n as " << id << "\n to POA(" << (char*) pd_name << ")"
	  " which has the UNIQUE_ID policy.\n";
      }
      add_object_to_etherealisation_queue(servant, sa, key, keysize, 0);
      return;
    }
  }
  {
    omniObjKey akey(key, keysize);
    id = omni::activateObject(servant, this, akey);
  }
  if( !id ) {
    exitAdapter(1);
    pd_lock.unlock();
    add_object_to_etherealisation_queue(servant, sa, key, keysize, 0);
    return;
  }
  id->insertIntoOAObjList(&pd_activeObjList);
  pd_lock.unlock();
  exitAdapter(1, 1);

  id->dispatch(giop_s);
}


void
omniOrbPOA::dispatch_to_sl(GIOP_S& giop_s, const CORBA::Octet* key,
			   int keysize)
{
  pd_lock.lock();
  if( pd_dying ) {
    pd_lock.unlock();
    OMNIORB_THROW(OBJECT_NOT_EXIST,0, CORBA::COMPLETED_NO);
  }
  if( !pd_servantLocator ) {
    pd_lock.unlock();
    OMNIORB_THROW(OBJ_ADAPTER,0, CORBA::COMPLETED_NO);
  }
  PortableServer::ServantLocator::_duplicate(pd_servantLocator);
  PortableServer::ServantLocator_var sl(pd_servantLocator);

  // We call enterAdapter() here to ensure that the POA can't
  // be fully destroyed before these objects have a chance to
  // be 'postinvoke'd.
  omni::internalLock->lock();
  enterAdapter();
  omni::internalLock->unlock();
  pd_lock.unlock();

  int idsize = keysize - pd_poaIdSize;
  PortableServer::ObjectId oid(idsize);
  oid.length(idsize);
  memcpy(oid.NP_data(), key + pd_poaIdSize, idsize);
  PortableServer::Servant servant;
  PortableServer::ServantLocator::Cookie cookie = 0;
  try {
    servant = sl->preinvoke(oid, this, giop_s.operation(), cookie);
  }
#ifndef HAS_Cplusplus_catch_exception_by_base
#define RETHROW_EXCEPTION(name) catch(CORBA::name&) { exitAdapter(); throw; }
  OMNIORB_FOR_EACH_SYS_EXCEPTION(RETHROW_EXCEPTION)
#undef RETHROW_EXCEPTION
#else
  catch(CORBA::SystemException&) {
    exitAdapter();
    throw;
  }
#endif
  catch(PortableServer::ForwardRequest& fr) {
    exitAdapter();
    throw omniORB::LOCATION_FORWARD(
			    CORBA::Object::_duplicate(fr.forward_reference));
  }
  catch(...) {
    exitAdapter();
    OMNIORB_THROW(OBJ_ADAPTER,0, CORBA::COMPLETED_NO);
  }

  if( !servant ) {
    exitAdapter();
    omniORB::logs(5, "ServantLocator::preinvoke() returned 0 (zero)!");
    OMNIORB_THROW(OBJ_ADAPTER,0, CORBA::COMPLETED_NO);
  }

  omniLocalIdentity the_id(key, keysize);
  the_id.setServant((PortableServer::Servant) servant, this);

  // If the operation string stored in the GIOP_S is longer than the
  // GIOP_S's internal buffer, it will be deleted at the end of the
  // dispatch() call. We must therefore make a copy of it here, to
  // pass to postinvoke().
  CORBA::String_var operation(giop_s.operation());

  omni::internalLock->lock();
  try {
    the_id.dispatch(giop_s);
  }
  catch(...) {
    call_postinvoke(sl, oid, operation, cookie, servant);
    throw;
  }
  call_postinvoke(sl, oid, operation, cookie, servant);
}


void
omniOrbPOA::call_postinvoke(PortableServer::ServantLocator_ptr sl,
			    PortableServer::ObjectId& oid,
			    const char* op,
			    PortableServer::ServantLocator::Cookie cookie,
			    PortableServer::Servant servant)
{
  if( pd_policy.single_threaded )  pd_call_lock->lock();
  try {
    sl->postinvoke(oid, this, op, cookie, servant);
  }
  catch(...) {
    // The client is more interested in the request than the fact that
    // object etherealisation failed -- so if this fails we don't tell
    // the client.  Just log a message ...
    omniORB::logs(5, "ServantLocator::postinvoke() raised an exception.");
  }
  if( pd_policy.single_threaded )  pd_call_lock->unlock();
  exitAdapter();
}


omniOrbPOA*
omniOrbPOA::attempt_to_activate_adapter(const char* name)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(poa_lock, 1);
  OMNIORB_ASSERT(name);
  OMNIORB_ASSERT(pd_adapterActivator);

  // Record the fact that we are activating a child with
  // this name, so that when it is created it will be
  // put in the HOLDING state (regardless of what its
  // POAManager says).
  //  Check that some other thread isn't trying to activate
  // said POA.  If so wait until it is finished, and if it
  // suceeded, return that POA or fail if it failed.
  if( !start_adapteractivating_child_or_block(name) )
    return find_child(name);

  // ?? NB. We could implement the above without an dynamic allocations
  // by useing a queue, and allocating queue entries on the stack
  // (much like linux kernel wait queues).

  poa_lock.unlock();

  if( omniORB::trace(10) )
    omniORB::logf("Attempting to activate POA '%s' using an AdapterActivator",
		  name);

  CORBA::Boolean ret = 0;

  try {
    ret = pd_adapterActivator->unknown_adapter(this, name);
  }
  catch(...) {
    omniORB::logs(5,
		  "AdapterActivator::unknown_adapter() raised an exception!");
  }

  poa_lock.lock();

  finish_adapteractivating_child(name);

  if( ret == 0 )  return 0;
  omniOrbPOA* p = find_child(name);
  if( !p )  return 0;

  // <p> was not attached to its manager at creation time (to
  // prevent state changes before initialisation was complete),
  // so we do it here.
  p->pd_manager->gain_poa(p);

  return p;
}


int
omniOrbPOA::start_adapteractivating_child_or_block(const char* name)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(poa_lock, 1);

  if( is_adapteractivating_child(name) ) {
    do {
      adapteractivator_signal.wait();
    } while( is_adapteractivating_child(name) );
    return 0;
  }

  pd_adptrActvtnsInProgress.push_back(name);

  return 1;
}


void
omniOrbPOA::finish_adapteractivating_child(const char* name)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(poa_lock, 1);

  omnivector<const char*>::iterator i, last;
  i = pd_adptrActvtnsInProgress.begin();
  last = pd_adptrActvtnsInProgress.end();

  while( i != last && strcmp(*i, name) )  i++;

  OMNIORB_ASSERT(i != last);

  pd_adptrActvtnsInProgress.erase(i);

  adapteractivator_signal.broadcast();
}


int
omniOrbPOA::is_adapteractivating_child(const char* name)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(poa_lock, 1);

  omnivector<const char*>::iterator i, last;
  i = pd_adptrActvtnsInProgress.begin();
  last = pd_adptrActvtnsInProgress.end();

  while( i != last && strcmp(*i, name) )  i++;

  return i != last;
}

//////////////////////////////////////////////////////////////////////
////////////////////// omniOrbPOA::Etherealiser //////////////////////
//////////////////////////////////////////////////////////////////////

void
omniOrbPOA::Etherealiser::doit()
{
  // Call into application-supplied etherealisation code.
  try {
    omni_tracedmutex_lock sync(servant_activator_lock);
    pd_sa->etherealize(pd_oid, pd_poa, pd_servant, pd_cleanup, !pd_is_last);
  }
  catch(...) {
    if( omniORB::trace(5) )
      omniORB::logf("Servant etherealisation raised an exception!");
  }
  pd_poa->met_detached_object();
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

static void
transfer_and_check_policies(omniOrbPOA::Policies& pout,
			    const CORBA::PolicyList& pin)
{
  // We expect <pout> to be initialised with default values.

  // Keep track of which policies have been set, so we can detect
  // incompatibilities.
  omniOrbPOA::Policies seen;
  seen.single_threaded = 0;
  seen.transient = 0;
  seen.multiple_id = 0;
  seen.user_assigned_id = 0;
  seen.retain_servants = 0;
  seen.req_processing = 0;
  seen.implicit_activation = 0;

  // Check for policies which contradict one-another.

  for( CORBA::ULong i = 0; i < pin.length(); i++ )
    switch( pin[i]->policy_type() ) {

    case /*THREAD_POLICY_ID*/ 16:
      {
	PortableServer::ThreadPolicy_var p;
	p = PortableServer::ThreadPolicy::_narrow(pin[i]);
	if( seen.single_threaded && pout.single_threaded !=
	    (p->value() == PortableServer::SINGLE_THREAD_MODEL) )
	  throw PortableServer::POA::InvalidPolicy(i);
	pout.single_threaded =
	  p->value() == PortableServer::SINGLE_THREAD_MODEL;
	seen.single_threaded = 1;
	break;
      }

    case /*LIFESPAN_POLICY_ID*/ 17:
      {
	PortableServer::LifespanPolicy_var p;
	p = PortableServer::LifespanPolicy::_narrow(pin[i]);
	if( seen.transient &&
	    pout.transient != (p->value() == PortableServer::TRANSIENT) )
	  throw PortableServer::POA::InvalidPolicy(i);
	pout.transient = p->value() == PortableServer::TRANSIENT;
	seen.transient = 1;
	break;
      }

    case /*ID_UNIQUENESS_POLICY_ID*/ 18:
      {
	PortableServer::IdUniquenessPolicy_var p;
	p = PortableServer::IdUniquenessPolicy::_narrow(pin[i]);
	if( seen.multiple_id &&
	    pout.multiple_id != (p->value() == PortableServer::MULTIPLE_ID) )
	  throw PortableServer::POA::InvalidPolicy(i);
	pout.multiple_id = p->value() == PortableServer::MULTIPLE_ID;
	seen.multiple_id = 1;
	break;
      }

    case /*ID_ASSIGNMENT_POLICY_ID*/ 19:
      {
	PortableServer::IdAssignmentPolicy_var p;
	p = PortableServer::IdAssignmentPolicy::_narrow(pin[i]);
	if( seen.user_assigned_id &&
	    pout.user_assigned_id != (p->value() == PortableServer::USER_ID) )
	  throw PortableServer::POA::InvalidPolicy(i);
	pout.user_assigned_id = p->value() == PortableServer::USER_ID;
	seen.user_assigned_id = 1;
	break;
      }

    case /*IMPLICIT_ACTIVATION_POLICY_ID*/ 20:
      {
	PortableServer::ImplicitActivationPolicy_var p;
	p = PortableServer::ImplicitActivationPolicy::_narrow(pin[i]);
	if( seen.implicit_activation && pout.implicit_activation !=
	    (p->value() == PortableServer::IMPLICIT_ACTIVATION) )
	  throw PortableServer::POA::InvalidPolicy(i);
	pout.implicit_activation =
	  p->value() == PortableServer::IMPLICIT_ACTIVATION;
	seen.implicit_activation = 1;
	break;
      }

    case /*SERVANT_RETENTION_POLICY_ID*/ 21:
      {
	PortableServer::ServantRetentionPolicy_var p;
	p = PortableServer::ServantRetentionPolicy::_narrow(pin[i]);
	if( seen.retain_servants &&
	    pout.retain_servants != (p->value() == PortableServer::RETAIN) )
	  throw PortableServer::POA::InvalidPolicy(i);
	pout.retain_servants = p->value() == PortableServer::RETAIN;
	seen.retain_servants = 1;
	break;
      }

    case /*REQUEST_PROCESSING_POLICY_ID*/ 22:
      {
	PortableServer::RequestProcessingPolicy_var p;
	p = PortableServer::RequestProcessingPolicy::_narrow(pin[i]);
	if( seen.req_processing ) {
	  if( pout.req_processing == omniOrbPOA::RPP_ACTIVE_OBJ_MAP &&
	      p->value() != PortableServer::USE_ACTIVE_OBJECT_MAP_ONLY ||
	      pout.req_processing == omniOrbPOA::RPP_DEFAULT_SERVANT &&
	      p->value() != PortableServer::USE_DEFAULT_SERVANT ||
	      pout.req_processing == omniOrbPOA::RPP_SERVANT_MANAGER &&
	      p->value() != PortableServer::USE_SERVANT_MANAGER )
	    throw PortableServer::POA::InvalidPolicy(i);
	}
	switch( p->value() ) {
	case PortableServer::USE_ACTIVE_OBJECT_MAP_ONLY:
	  pout.req_processing = omniOrbPOA::RPP_ACTIVE_OBJ_MAP;
	  break;
	case PortableServer::USE_DEFAULT_SERVANT:
	  pout.req_processing = omniOrbPOA::RPP_DEFAULT_SERVANT;
	  break;
	case PortableServer::USE_SERVANT_MANAGER:
	  pout.req_processing = omniOrbPOA::RPP_SERVANT_MANAGER;
	  break;
	default:
	  throw PortableServer::POA::InvalidPolicy(i);
	}
	seen.req_processing = 1;
	break;
      }

    default:
      throw PortableServer::POA::InvalidPolicy(i);

    }

  // Check for incompatible policy combinations.

  if( (pout.req_processing == omniOrbPOA::RPP_ACTIVE_OBJ_MAP &&
       !pout.retain_servants) ||
      (pout.req_processing == omniOrbPOA::RPP_DEFAULT_SERVANT &&
       !pout.multiple_id) ) {
    // Find one of the policy objects which participated in
    // this flagrant flouting of the rules ...
    for( CORBA::ULong i = 0; i < pin.length(); i++ )
      if( pin[i]->policy_type() == /*REQUEST_PROCESSING_POLICY_ID*/ 22 )
	throw PortableServer::POA::InvalidPolicy(i);
  }

  if( pout.implicit_activation &&
      (pout.user_assigned_id || !pout.retain_servants) ) {
    for( CORBA::ULong i = 0; i < pin.length(); i++ )
      if( pin[i]->policy_type() == /*IMPLICIT_ACTIVATION_POLICY_ID*/ 20 )
	throw PortableServer::POA::InvalidPolicy(i);
  }
}


static void
generateUniqueId(CORBA::Octet* k)
{
  OMNIORB_ASSERT(k);

  static omni_mutex lock;
  omni_mutex_lock sync(lock);

  static CORBA::ULong hi = 0;
  static CORBA::ULong lo = 0;

  if( !hi && !lo ) {

    CORBA::Short pid;

#if !defined(__WIN32__) && !(defined(__VMS) && __VMS_VER < 70000000)
    // Use gettimeofday() to obtain the current time. Use this to
    // initialise the 32-bit field hi and med in the seed.
    // On unices, add the process id to med.
    // Initialise lo to 0.
    struct timeval v;
# ifndef __SINIX__
    gettimeofday(&v,0);
# else
    gettimeofday(&v);
# endif
    hi = v.tv_sec;
# if defined(UnixArchitecture) || defined(__VMS)
    pid = (CORBA::Short) getpid();
# else
    pid = (CORBA::Short) v.tv_usec;
    // without the process id, there is no guarantee that the keys generated
    // by this process do not clash with those generated by other processes.
# endif

#elif defined(__WIN32__)
    // Unique number on NT
    // Use _ftime() to obtain the current system time. 
# ifndef __BCPLUSPLUS__
    struct _timeb v;
    _ftime(&v);
    hi = v.time;
    pid = (CORBA::Short) _getpid();
# else
    struct timeb v;
    ftime(&v);
    hi = v.time;
    pid = (CORBA::Short) getpid();
# endif

#elif defined(__VMS) && __VMS_VER < 70000000
    // VMS systems prior to 7.0
    timeb v;
    ftime(&v);
    hi = v.time;
    pid = (CORBA::Short) getpid();
#endif

    // Byte-swap the pid, so that if the counter in the lower 16
    // bits of <lo> overflows it will not change the fastest
    // moving pid bit.

    pid = ((pid & 0xff00) >> 8) | ((pid & 0x00ff) << 8);

    lo = pid << 16;
  }

  CORBA::Octet* phi = (CORBA::Octet*) &hi;
  CORBA::Octet* plo = (CORBA::Octet*) &lo;

  *k++ = *phi++;
  *k++ = *phi++;
  *k++ = *phi++;
  *k++ = *phi++;
  *k++ = *plo++;
  *k++ = *plo++;
  *k++ = *plo++;
  *k++ = *plo++;

  lo++;
}


static void
destroyer_thread_fn(void* args)
{
  if( omniORB::trace(15) ) {
    omniORB::logger l; l << "POA destroyer thread started.\n";
  }

  OMNIORB_ASSERT(args);
  void** targs = (void**) args;

  omniOrbPOA* poa = (omniOrbPOA*) targs[0];
  CORBA::Boolean etherealise = (CORBA::Boolean) (unsigned long) targs[1];
  delete[] targs;

  poa->do_destroy(etherealise);
}
