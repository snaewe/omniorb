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
  Revision 1.2.2.22  2001/10/17 16:44:07  dpg1
  Update DynAny to CORBA 2.5 spec, const Any exception extraction.

  Revision 1.2.2.21  2001/09/20 09:27:44  dpg1
  Remove assertion failure on exit if not all POAs are deleted.

  Revision 1.2.2.20  2001/09/19 17:26:51  dpg1
  Full clean-up after orb->destroy().

  Revision 1.2.2.19  2001/08/21 11:02:18  sll
  orbOptions handlers are now told where an option comes from. This
  is necessary to process DefaultInitRef and InitRef correctly.

  Revision 1.2.2.18  2001/08/20 10:43:57  sll
  Fixed minor bug detected by MSVC++

  Revision 1.2.2.17  2001/08/17 17:12:41  sll
  Modularise ORB configuration parameters.

  Revision 1.2.2.16  2001/08/17 15:00:48  dpg1
  Fixes for pre-historic compilers.

  Revision 1.2.2.15  2001/08/15 17:59:12  dpg1
  Minor POA bugs.

  Revision 1.2.2.14  2001/08/15 10:26:13  dpg1
  New object table behaviour, correct POA semantics.

  Revision 1.2.2.13  2001/08/03 17:41:24  sll
  System exception minor code overhaul. When a system exeception is raised,
  a meaning minor code is provided.

  Revision 1.2.2.12  2001/08/01 10:08:22  dpg1
  Main thread policy.

  Revision 1.2.2.11  2001/07/31 16:10:37  sll
  Added GIOP BiDir support.

  Revision 1.2.2.10  2001/06/29 16:24:48  dpg1
  Support re-entrancy in single thread policy POAs.

  Revision 1.2.2.9  2001/06/07 16:24:10  dpg1
  PortableServer::Current support.

  Revision 1.2.2.8  2001/05/31 16:18:14  dpg1
  inline string matching functions, re-ordered string matching in
  _ptrToInterface/_ptrToObjRef

  Revision 1.2.2.7  2001/05/29 17:03:52  dpg1
  In process identity.

  Revision 1.2.2.6  2001/04/18 18:18:05  sll
  Big checkin with the brand new internal APIs.

  Revision 1.2.2.5  2000/11/15 17:47:58  dpg1
  Typo in Monday's servant manager fix

  Revision 1.2.2.4  2000/11/13 12:39:54  dpg1
  djr's fix to exceptions in servant managers from omni3_develop

  Revision 1.2.2.3  2000/11/09 12:27:58  dpg1
  Huge merge from omni3_develop, plus full long long from omni3_1_develop.

  Revision 1.2.2.2  2000/09/27 18:02:56  sll
  Updated to use the new cdrStream abstraction.

  Revision 1.2.2.1  2000/07/17 10:35:56  sll
  Merged from omni3_develop the diff between omni3_0_0_pre3 and omni3_0_0.

  Revision 1.3  2000/07/13 15:25:56  dpg1
  Merge from omni3_develop for 3.0 release.

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

#include <omniORB4/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <poaimpl.h>
#include <omniORB4/IOP_S.h>
#include <omniORB4/callDescriptor.h>
#include <omniORB4/callHandle.h>
#include <omniORB4/objTracker.h>
#include <objectTable.h>
#include <inProcessIdentity.h>
#include <poamanager.h>
#include <exceptiondefs.h>
#include <poacurrentimpl.h>
#include <initialiser.h>
#include <orbOptions.h>
#include <orbParameters.h>
#include <initRefs.h>

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

OMNI_USING_NAMESPACE(omni)

////////////////////////////////////////////////////////////////////////////
//             Configuration options                                      //
////////////////////////////////////////////////////////////////////////////
  CORBA::ULong orbParameters::poaHoldRequestTimeout = 0;
//  This variable can be used to set a time-out for calls being held
//  in a POA which is in the HOLDING state.  It gives the time in
//  seconds after which a TRANSIENT exception will be thrown if the
//  POA is not transitioned to a different state.
//
//  Valid values = (n >= 0 in milliseconds)
//                  0 --> no time-out.


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
    registerNilCorbaObject(_the_nil_ptr);
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

OMNIORB_DEFINE_USER_EX_WITHOUT_MEMBERS(PortableServer::POA,
				       AdapterAlreadyExists,
	   "IDL:omg.org/PortableServer/POA/AdapterAlreadyExists" PS_VERSION)
OMNIORB_DEFINE_USER_EX_WITHOUT_MEMBERS(PortableServer::POA, AdapterInactive,
	   "IDL:omg.org/PortableServer/POA/AdapterInactive" PS_VERSION)
OMNIORB_DEFINE_USER_EX_WITHOUT_MEMBERS(PortableServer::POA, AdapterNonExistent,
	   "IDL:omg.org/PortableServer/POA/AdapterNonExistent" PS_VERSION)
OMNIORB_DEFINE_USER_EX_WITHOUT_MEMBERS(PortableServer::POA, NoServant,
	   "IDL:omg.org/PortableServer/POA/NoServant" PS_VERSION)
OMNIORB_DEFINE_USER_EX_WITHOUT_MEMBERS(PortableServer::POA,ObjectAlreadyActive,
	   "IDL:omg.org/PortableServer/POA/ObjectAlreadyActive" PS_VERSION)
OMNIORB_DEFINE_USER_EX_WITHOUT_MEMBERS(PortableServer::POA, ObjectNotActive,
	   "IDL:omg.org/PortableServer/POA/ObjectNotActive" PS_VERSION)
OMNIORB_DEFINE_USER_EX_WITHOUT_MEMBERS(PortableServer::POA,
				       ServantAlreadyActive,
	   "IDL:omg.org/PortableServer/POA/ServantAlreadyActive" PS_VERSION)
OMNIORB_DEFINE_USER_EX_WITHOUT_MEMBERS(PortableServer::POA, ServantNotActive,
	   "IDL:omg.org/PortableServer/POA/ServantNotActive" PS_VERSION)
OMNIORB_DEFINE_USER_EX_WITHOUT_MEMBERS(PortableServer::POA, WrongAdapter,
	   "IDL:omg.org/PortableServer/POA/WrongAdapter" PS_VERSION)
OMNIORB_DEFINE_USER_EX_WITHOUT_MEMBERS(PortableServer::POA, WrongPolicy,
	   "IDL:omg.org/PortableServer/POA/WrongPolicy" PS_VERSION)
OMNIORB_DEFINE_USER_EX_COMMON_FNS(PortableServer::POA, InvalidPolicy,
	   "IDL:omg.org/PortableServer/POA/InvalidPolicy" PS_VERSION)


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

void
PortableServer::POA::InvalidPolicy::operator>>= (cdrStream& _n) const
{
  index >>= _n;
}

void
PortableServer::POA::InvalidPolicy::operator<<= (cdrStream& _n)
{
  index <<= _n;
}


//////////////////////////////////////////////////////////////////////
///////////////////////////// omniOrbPOA /////////////////////////////
//////////////////////////////////////////////////////////////////////

#define CHECK_NOT_NIL()  \
 if( _NP_is_nil() )  _CORBA_invoked_nil_pseudo_ref()

#define CHECK_NOT_DYING()  \
 if( pd_dying )  OMNIORB_THROW(OBJECT_NOT_EXIST,OBJECT_NOT_EXIST_POANotInitialised, CORBA::COMPLETED_NO)

#define CHECK_NOT_DESTROYED()  \
 if( pd_destroyed )  OMNIORB_THROW(OBJECT_NOT_EXIST,OBJECT_NOT_EXIST_POANotInitialised, CORBA::COMPLETED_NO)

#define CHECK_NOT_NIL_OR_DESTROYED()  \
 if( _NP_is_nil() )  _CORBA_invoked_nil_pseudo_ref();  \
 if( pd_destroyed )  OMNIORB_THROW(OBJECT_NOT_EXIST,OBJECT_NOT_EXIST_POANotInitialised, CORBA::COMPLETED_NO)


static void transfer_and_check_policies(omniOrbPOA::Policies& pout,
					const CORBA::PolicyList& pin);


static omni_tracedmutex     poa_lock;

static omni_tracedcondition adapteractivator_signal(&poa_lock);
// Used to signal between threads when using an AdapterActivator
// to create a child POA.

static omniOrbPOA* theRootPOA = 0;
static omniOrbPOA* theINSPOA  = 0;
// Protected by <poa_lock>.


omniOrbPOA::~omniOrbPOA()
{
  switch (pd_policy.threading) {
  case TP_ORB_CTRL:
    break;
  case TP_SINGLE_THREAD:
    delete pd_call_lock;
    break;
  case TP_MAIN_THREAD:
    delete pd_main_thread_sync.cond;
    delete pd_main_thread_sync.mu;
    break;
  };
}


PortableServer::POA_ptr
omniOrbPOA::create_POA(const char* adapter_name,
		       PortableServer::POAManager_ptr manager,
		       const CORBA::PolicyList& policies)
{
  CHECK_NOT_NIL();
  if( !adapter_name_is_valid(adapter_name) )
    OMNIORB_THROW(BAD_PARAM,BAD_PARAM_InvalidPOAName, CORBA::COMPLETED_NO);

  // Setup the default policies.
  Policies policy;
  policy.threading = TP_ORB_CTRL;
  policy.transient = 1;
  policy.multiple_id = 0;
  policy.user_assigned_id = 0;
  policy.retain_servants = 1;
  policy.req_processing = RPP_ACTIVE_OBJ_MAP;
  policy.implicit_activation = 0;
  policy.bidirectional_accept = 0;

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
  if( !adapter_name )  OMNIORB_THROW(BAD_PARAM,BAD_PARAM_InvalidPOAName,
				     CORBA::COMPLETED_NO);

  omni_tracedmutex_lock sync(poa_lock);

  omniOrbPOA* poa = find_child(adapter_name);

  if( poa && !poa->pd_dying ) {
    poa->incrRefCount();
    return poa;
  }

  if( !activate_it || !pd_adapterActivator )  throw AdapterNonExistent();

  poa = attempt_to_activate_adapter(adapter_name);

  if( poa && !poa->pd_dying ) {
    poa->incrRefCount();
    return poa;
  }

  throw AdapterNonExistent();
  return 0; // For dumb compilers
}


static void destroyer_thread_fn(void* poa_arg);


void
omniOrbPOA::destroy(CORBA::Boolean etherealize_objects,
		    CORBA::Boolean wait_for_completion)
{
  CHECK_NOT_NIL();
  if( wait_for_completion ) {

    omniCurrent* current = omniCurrent::get();
    if (current && current->callDescriptor()) {
      // In the context of an operation invocation.

      // This is interesting. It would be sufficient (but harder) to
      // only complain if in the context of an invocation in _this_
      // POA. However, the 2.4 spec says "...some POA belonging to the
      // same ORB as this POA...". Since we only ever support one ORB,
      // life is easy.

      OMNIORB_THROW(BAD_INV_ORDER,
		    BAD_INV_ORDER_WouldDeadLock,
		    CORBA::COMPLETED_NO);
    }
  }

  // Mark self as being in the process of destruction, sever links
  // with poa manager, destroy childer, deactivate all objects
  // (possibly waiting for completion of outstanding method
  // invocations) and then remove self from parent.

  {
    omni_tracedmutex_lock sync(pd_lock);

    if( pd_destroyed )  OMNIORB_THROW(OBJECT_NOT_EXIST,
				      OBJECT_NOT_EXIST_POANotInitialised,
				      CORBA::COMPLETED_NO);

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
    OMNIORB_THROW(OBJ_ADAPTER,OBJ_ADAPTER_NoServantManager,
		  CORBA::COMPLETED_NO);

  {
    // Check that <imgr> is a local object ...
    omni::internalLock->lock();
    int islocal = imgr->_identity()->inThisAddressSpace();
    omni::internalLock->unlock();
    if( !islocal )  OMNIORB_THROW(BAD_PARAM,BAD_PARAM_LocalObjectExpected,
				  CORBA::COMPLETED_NO);
  }

  omni_tracedmutex_lock sync(pd_lock);

  if( pd_servantActivator || pd_servantLocator )
    OMNIORB_THROW(BAD_INV_ORDER,
		  BAD_INV_ORDER_ServantManagerAlreadySet,
		  CORBA::COMPLETED_NO);

  if( pd_policy.retain_servants ) {
    pd_servantActivator = PortableServer::ServantActivator::_narrow(imgr);
    if( CORBA::is_nil(pd_servantActivator) ) {
      pd_servantActivator = 0;
      OMNIORB_THROW(OBJ_ADAPTER, OBJ_ADAPTER_NoServantManager,
		    CORBA::COMPLETED_NO);
    }
  } else {
    pd_servantLocator = PortableServer::ServantLocator::_narrow(imgr);
    if( CORBA::is_nil(pd_servantLocator) ) {
      pd_servantLocator = 0;
      OMNIORB_THROW(OBJ_ADAPTER, OBJ_ADAPTER_NoServantManager,
		    CORBA::COMPLETED_NO);
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
  if( !p_servant )
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_InvalidServant, CORBA::COMPLETED_NO);

  if( pd_policy.user_assigned_id || !pd_policy.retain_servants )
    throw WrongPolicy();

  omni_tracedmutex_lock sync(pd_lock);
  CHECK_NOT_DYING();
  omni_tracedmutex_lock sync2(*omni::internalLock);

  if( !pd_policy.multiple_id ) {
    // Check the servant's list of activations, to ensure that it
    // isn't already active in this POA.

    omnivector<omniObjTableEntry*>::const_iterator i, last;
    i    = p_servant->_activations().begin();
    last = p_servant->_activations().end();

    for (; i != last; i++) {
      if ((*i)->adapter() == this)
	throw ServantAlreadyActive();
    }
  }

  PortableServer::ObjectId* ret = new PortableServer::ObjectId();
  omniObjTableEntry* entry = 0;
  omniObjKey key;
  int idsize;
  const CORBA::Octet* oid;

  // We have to keep trying here, just in case someone has
  // activated an object with a system generated id, using
  // activate_object_with_id().

  do {
    create_new_key(key, &oid, &idsize);
    entry = omniObjTable::newEntry(key);

  } while( !entry );

  entry->setActive(p_servant, this);

  // *** FIXME: Move into setActive() ?
  p_servant->_add_ref();
  entry->insertIntoOAObjList(&pd_activeObjList);

  ret->length(idsize);
  memcpy(ret->NP_data(), oid, idsize);

  return ret;
}


void
omniOrbPOA::activate_object_with_id(const PortableServer::ObjectId& oid,
				    PortableServer::Servant p_servant)
{
  CHECK_NOT_NIL();
  if( !pd_policy.retain_servants )  throw WrongPolicy();

  if( !p_servant )
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_InvalidServant, CORBA::COMPLETED_NO);

  if( !pd_policy.user_assigned_id && oid.length() != SYS_ASSIGNED_ID_SIZE )
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_InvalidSystemId, CORBA::COMPLETED_NO);

  omniObjKey key;
  create_key(key, oid.NP_data(), oid.length());
  CORBA::ULong hashv = omni::hash(key.key(), key.size());
  omniObjTableEntry* entry;

  {
    // We need to check the object is not already activated. If there
    // is an ACTIVATING or ACTIVE entry in the object table, throw
    // ObjectAlreadyActive; if there is an entry in another state,
    // wait until it's ACTIVE or DEAD, then continue as before.
    omni_tracedmutex_lock isync(*omni::internalLock);

    entry = omniObjTable::locate(key.key(), key.size(), hashv,
				 omniObjTableEntry::ACTIVATING |
				 omniObjTableEntry::ACTIVE);
    if (entry) {
      if (entry->state() == omniObjTableEntry::ACTIVATING &&
	  omniORB::trace(5)) {
	omniORB::logger l;
	l << "Attempt to activate an object while it is already "
	  "being activated. Are you calling activate_object_with_id() "
	  "inside incarnate()?\n";
      }
      throw ObjectAlreadyActive();
    }
    // Put a place-holder in the object table so any other threads
    // know we're in the process of activating.
    entry = omniObjTable::newEntry(key, hashv);
    OMNIORB_ASSERT(entry);
  }

  omni_tracedmutex_lock sync(pd_lock);

  {
    omni_tracedmutex_lock isync(*omni::internalLock);

    // If we're dying, we have to remove the entry we just put in the
    // object table...
    if (pd_dying) {
      entry->setDead();
      OMNIORB_THROW(OBJECT_NOT_EXIST,
		    OBJECT_NOT_EXIST_POANotInitialised,
		    CORBA::COMPLETED_NO);
    }

    if( !pd_policy.multiple_id ) {
      // Check the servant's list of activations, to ensure that it
      // isn't already active in this POA.

      omnivector<omniObjTableEntry*>::const_iterator i, last;
      i    = p_servant->_activations().begin();
      last = p_servant->_activations().end();

      for (; i != last; i++) {
	if ((*i)->adapter() == this)
	  throw ServantAlreadyActive();
      }
    }
    entry->setActive(p_servant, this);
  }
  p_servant->_add_ref();
  entry->insertIntoOAObjList(&pd_activeObjList);
}


void
omniOrbPOA::deactivate_object(const PortableServer::ObjectId& oid)
{
  // Once an object is targeted for deactivation, things happen in
  // this order:
  //
  //  o wait for requests to complete
  //     -- note that new requests can arrive in the mean time, so the
  //        deactivation may never actually complete!  This is
  //        necessary to prevent deadlocks in objects which do
  //        re-entrant calls to themselves.
  //  o remove from active object map
  //  o etherealise
  //
  // It is not possible to reactivate the same object until the
  // etherealisation stage has happened. Attempts to do so are blocked
  // until it is safe to continue.

  CHECK_NOT_NIL();
  if( !pd_policy.retain_servants )  throw WrongPolicy();

  omniObjKey key;
  create_key(key, oid.NP_data(), oid.length());
  CORBA::ULong hashv = omni::hash(key.key(), key.size());

  pd_lock.lock();
  CHECK_NOT_DESTROYED();
  omni::internalLock->lock();

  omniObjTableEntry* entry = omniObjTable::locate(key.key(),key.size(),hashv);

  if (!entry || entry->state() != omniObjTableEntry::ACTIVE) {
    if (omniORB::trace(10)) {
      if (entry->state() == omniObjTableEntry::ACTIVATING) {
	omniORB::logger l;
	l << "deactivate_object() races with a thread activating the "
	      "object. ObjectNotActive is thrown.\n";
      }
      else if (entry->state() != omniObjTableEntry::ACTIVE) {
	omniORB::logger l;
	l << "deactivate_object() races with another thread deactivating the "
	      "object. ObjectNotActive is thrown.\n";
      }
    }
    omni::internalLock->unlock();
    pd_lock.unlock();
    throw ObjectNotActive();
  }

  entry->setDeactivating();
  entry->removeFromOAObjList();

  if( entry->is_idle() ) {
    detached_object();
    pd_lock.unlock();
    lastInvocationHasCompleted(entry);
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
  if( pd_policy.user_assigned_id )  throw WrongPolicy();

  if( !intf )  intf = ""; // Null string is permitted.

  omniObjKey key;
  omniLocalIdentity* id;
  CORBA::ULong hash;

  pd_lock.lock();
  omni::internalLock->lock();

  // We need to be sure we use a new id ...

  do {
    create_new_key(key);
    hash = omni::hash(key.key(), key.size());
    id = omniObjTable::locate(key.key(), key.size(), hash);
  } while( id );

  pd_lock.unlock();

  omniObjRef* objref = omni::createLocalObjRef(intf,
					       CORBA::Object::_PD_repoId,
					       key.key(), key.size());
  omni::internalLock->unlock();

  OMNIORB_ASSERT(objref);

  return (CORBA::Object_ptr) objref->_ptrToObjRef(CORBA::Object::_PD_repoId);
}


CORBA::Object_ptr
omniOrbPOA::create_reference_with_id(const PortableServer::ObjectId& oid,
				     const char* intf)
{
  CHECK_NOT_NIL_OR_DESTROYED();
  if( !pd_policy.user_assigned_id && oid.length() != SYS_ASSIGNED_ID_SIZE )
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_InvalidSystemId, CORBA::COMPLETED_NO);

  if( !intf )  intf = ""; // Null string is permitted.

  omniObjKey key;
  create_key(key, oid.NP_data(), oid.length());

  omni::internalLock->lock();
  omniObjRef* objref = omni::createLocalObjRef(intf,
					       CORBA::Object::_PD_repoId,
					       key.key(), key.size());
  omni::internalLock->unlock();

  OMNIORB_ASSERT(objref);

  return (CORBA::Object_ptr) objref->_ptrToObjRef(CORBA::Object::_PD_repoId);
}

PortableServer::ObjectId*
omniOrbPOA::localId_to_ObjectId(omniIdentity* id)
{
  OMNIORB_ASSERT(id->inThisAddressSpace());
  OMNIORB_ASSERT(pd_poaIdSize == 0 ||
		 omni::strMatch((const char*) pd_poaId,
				(const char*) id->key()));
  int idsize = id->keysize() - pd_poaIdSize;
  OMNIORB_ASSERT(idsize >= 0);
  PortableServer::ObjectId* ret = new PortableServer::ObjectId(idsize);
  ret->length(idsize);
  memcpy(ret->NP_data(), id->key() + pd_poaIdSize, idsize);
  return ret;
}

void
omniOrbPOA::localId_to_ObjectId(omniIdentity* id,
				PortableServer::ObjectId& oid)
{
  OMNIORB_ASSERT(id->inThisAddressSpace());
  OMNIORB_ASSERT(pd_poaIdSize == 0 ||
		 omni::strMatch((const char*) pd_poaId,
				(const char*) id->key()));
  int idsize = id->keysize() - pd_poaIdSize;
  OMNIORB_ASSERT(idsize >= 0);
  oid.length(idsize);
  memcpy(oid.NP_data(), id->key() + pd_poaIdSize, idsize);
}



PortableServer::ObjectId*
omniOrbPOA::servant_to_id(PortableServer::Servant p_servant)
{
  CHECK_NOT_NIL_OR_DESTROYED();
  if( !p_servant )
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_InvalidServant, CORBA::COMPLETED_NO);

  if( !( (pd_policy.req_processing == RPP_DEFAULT_SERVANT) ||
	 (pd_policy.retain_servants && (!pd_policy.multiple_id ||
					pd_policy.implicit_activation)) ) )
    throw WrongPolicy();

  omni_tracedmutex_lock sync(pd_lock);

  if( pd_policy.req_processing == RPP_DEFAULT_SERVANT ) {

    if (p_servant == pd_defaultServant) {
      omniCurrent* current = omniCurrent::get();
      if (current) {
	omniCallDescriptor* call_desc = current->callDescriptor();

	if (call_desc &&
	    call_desc->poa() == this &&
	    call_desc->localId()->servant() == (omniServant*)p_servant) {

	  return localId_to_ObjectId(call_desc->localId());
	}
      }
      throw ServantNotActive();
    }
  }

  if (!pd_policy.retain_servants) throw WrongPolicy();

  omni_tracedmutex_lock sync2(*omni::internalLock);

  if( !pd_policy.multiple_id ) {
    // Search the servants activations, to see if it is activated in
    // this poa.

    omnivector<omniObjTableEntry*>::const_iterator i, last;
    i    = p_servant->_activations().begin();
    last = p_servant->_activations().end();

    for (; i != last; i++) {
      if ((*i)->adapter() == this)
	return localId_to_ObjectId(*i);
    }
  }

  if( !pd_policy.implicit_activation ) throw ServantNotActive();

  CHECK_NOT_DYING();

  // If we get here then we need to do an implicit activation.

  PortableServer::ObjectId* ret = new PortableServer::ObjectId();
  omniObjTableEntry* entry = 0;
  omniObjKey key;
  int idsize;
  const CORBA::Octet* oid;

  do {
    create_new_key(key, &oid, &idsize);
    entry = omniObjTable::newEntry(key);

  } while( !entry );

  entry->setActive(p_servant, this);

  // *** FIXME: Move into setActive() ?
  p_servant->_add_ref();
  entry->insertIntoOAObjList(&pd_activeObjList);

  ret->length(idsize);
  memcpy(ret->NP_data(), oid, idsize);

  return ret;
}


CORBA::Object_ptr
omniOrbPOA::servant_to_reference(PortableServer::Servant p_servant)
{
  CHECK_NOT_NIL_OR_DESTROYED();
  if( !p_servant )
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_InvalidServant, CORBA::COMPLETED_NO);

  omniCurrent* current = omniCurrent::get();

  if (current) {
    omniCallDescriptor* call_desc = current->callDescriptor();
    if (call_desc &&
	call_desc->localId()->servant() == (omniServant*)p_servant &&
	call_desc->poa() == this) {

      // Return reference associated with the current invocation
      omniObjRef* objref = omniOrbPOACurrent::real_get_reference(call_desc);
      return (CORBA::Object_ptr)
	                  objref->_ptrToObjRef(CORBA::Object::_PD_repoId);
    }
    // It's not clear from the spec what should happen if we're in the
    // context of an invocation on the given servant, but not in this
    // POA. It seems most sensible to carry on with the code below...
  }

  if( !( pd_policy.retain_servants &&
	 (!pd_policy.multiple_id || pd_policy.implicit_activation) ) )
    throw WrongPolicy();

  omni_tracedmutex_lock sync(pd_lock);
  omni_tracedmutex_lock sync2(*omni::internalLock);

  if( !pd_policy.multiple_id ) {
    // Search the servants identities, to see if it is
    // activated in this poa.

    omnivector<omniObjTableEntry*>::const_iterator i, last;
    i    = p_servant->_activations().begin();
    last = p_servant->_activations().end();

    for (; i != last; i++) {
      if ((*i)->adapter() == this) {
	omniObjTableEntry* entry = *i;

	OMNIORB_ASSERT(pd_poaIdSize == 0 ||
		       omni::ptrStrMatch(pd_poaId, (const char*)entry->key()));

	omniObjRef* objref =
	  omni::createLocalObjRef(p_servant->_mostDerivedRepoId(),
				  CORBA::Object::_PD_repoId, entry);
	OMNIORB_ASSERT(objref);
	return (CORBA::Object_ptr) objref->_ptrToObjRef(CORBA::Object
							::_PD_repoId);
      }
    }
  }
  if( !pd_policy.implicit_activation )  throw ServantNotActive();
  CHECK_NOT_DYING();

  // If we get here, then either the servant is not activated in
  // this POA, or we have the multiple id policy.  So do an
  // implicit activation.

  omniObjTableEntry* entry = 0;
  omniObjKey key;
  int idsize;
  const CORBA::Octet* oid;

  do {
    create_new_key(key, &oid, &idsize);
    entry = omniObjTable::newEntry(key);

  } while( !entry );

  entry->setActive(p_servant, this);

  // *** FIXME: Move into setActive() ?
  p_servant->_add_ref();
  entry->insertIntoOAObjList(&pd_activeObjList);

  omniObjRef* objref = omni::createLocalObjRef(p_servant->_mostDerivedRepoId(),
					       CORBA::Object::_PD_repoId,
					       entry);
  OMNIORB_ASSERT(objref);

  return (CORBA::Object_ptr) objref->_ptrToObjRef(CORBA::Object::_PD_repoId);
}


PortableServer::Servant
omniOrbPOA::reference_to_servant(CORBA::Object_ptr reference)
{
  CHECK_NOT_NIL_OR_DESTROYED();

  if( CORBA::is_nil(reference) )
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_InvalidObjectRef, CORBA::COMPLETED_NO);

  if( reference->_NP_is_pseudo() )  throw WrongAdapter();

  if( !pd_policy.retain_servants &&
      pd_policy.req_processing != RPP_DEFAULT_SERVANT )
    throw WrongPolicy();

  omni_tracedmutex_lock sync(pd_lock);
  omni_tracedmutex_lock sync2(*omni::internalLock);

  omniObjRef*   objref = reference->_PR_getobj();
  omniIdentity* id     = objref->_identity();

  if (!id->inThisAddressSpace() ||
      id->keysize() < pd_poaIdSize ||
      memcmp(id->key(), (const char*) pd_poaId, pd_poaIdSize))
    throw WrongAdapter();

  if( pd_policy.retain_servants ) {
    omniObjTableEntry* entry = omniObjTableEntry::downcast(id);

    if (!entry) {
      omniInProcessIdentity* ipid = omniInProcessIdentity::downcast(id);
      if (ipid) {
	// The objref has an inProcessIdentity. The servant might have
	// been activated since the reference was created, so check
	// the object table.
	CORBA::ULong hashv = omni::hash(id->key(), id->keysize());
	entry = omniObjTable::locate(id->key(), id->keysize(), hashv);
      }
    }

    if (entry && (entry->state() == omniObjTableEntry::ACTIVE ||
		  entry->state() == omniObjTableEntry::DEACTIVATING)) {

      omniServant* servant = entry->servant();
      OMNIORB_ASSERT(servant);
      PortableServer::Servant s = DOWNCAST(servant);
      s->_add_ref();
      return s;
    }
  }

  if( pd_policy.req_processing == RPP_DEFAULT_SERVANT && pd_defaultServant ) {
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
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_InvalidObjectRef, CORBA::COMPLETED_NO);

  if( reference->_NP_is_pseudo() )  throw WrongAdapter();

  omni_tracedmutex_lock sync(*omni::internalLock);

  omniObjRef*   objref = reference->_PR_getobj();
  omniIdentity* id     = objref->_identity();

  if (!id->inThisAddressSpace() ||
      id->keysize() < pd_poaIdSize ||
      memcmp(id->key(), (const char*) pd_poaId, pd_poaIdSize))
    throw WrongAdapter();

  return localId_to_ObjectId(id);
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
    omniObjTableEntry* entry = omniObjTable::locateActive(key.key(),
							  key.size(),
							  hash, 0);
    PortableServer::Servant s = 0;
    if( entry ) {
      OMNIORB_ASSERT(entry->servant());
      s = DOWNCAST(entry->servant());
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

  omniObjTableEntry* entry = omniObjTable::locateActive(key.key(),
							key.size(),
							hash, 0);
  if (!entry) {
    omni::internalLock->unlock();
    throw ObjectNotActive();
  }

  OMNIORB_ASSERT(entry->servant());

  omniObjRef* objref =
    omni::createLocalObjRef(entry->servant()->_mostDerivedRepoId(),
			    CORBA::Object::_PD_repoId, entry);
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

  if( omni::ptrStrMatch(repoId, PortableServer::POA::_PD_repoId) )
    return (PortableServer::POA_ptr) this;
  if( omni::ptrStrMatch(repoId, CORBA::Object::_PD_repoId) )
    return (CORBA::Object_ptr) this;

  return 0;
}


void
omniOrbPOA::_NP_incrRefCount()
{
  // Identical to incrRefCount().
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
omniOrbPOA::dispatch(omniCallHandle& handle, omniLocalIdentity* id)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);
  OMNIORB_ASSERT(id);  OMNIORB_ASSERT(id->servant());
  OMNIORB_ASSERT(id->adapter() == this);

  handle.poa(this);

  enterAdapter();

  if( pd_rq_state != (int) PortableServer::POAManager::ACTIVE )
    synchronise_request(id);

  startRequest();

  omni::internalLock->unlock();

  omni_optional_rlock sync(pd_call_lock,
			   pd_policy.threading != TP_SINGLE_THREAD,
			   pd_policy.threading != TP_SINGLE_THREAD);

  if (pd_policy.threading == TP_MAIN_THREAD) {
    omni_thread* self = omni_thread::self();
    if (!(self && self->id() == omni::mainThreadId))
      handle.mainThread(pd_main_thread_sync.mu, pd_main_thread_sync.cond);
  }

  if( omniORB::traceInvocations ) {
    omniORB::logger l;
    l << "Dispatching "
      << (handle.call_desc() ? "in process" : "remote")
      << " call '" << handle.operation_name()
      << "' to: " << id << '\n';
  }

  // Can we do a direct upcall?
  if (handle.call_desc() && handle.try_direct() &&
      handle.call_desc()->haslocalCallFn() &&
      id->servant()->
        _ptrToInterface(handle.call_desc()->objref()->_localServantTarget())) {

    handle.upcall(id->servant(), *handle.call_desc());
    return;
  }

  // Dispatch through the servant's _dispatch() function.
  if( !id->servant()->_dispatch(handle) ) {
    if( !id->servant()->omniServant::_dispatch(handle) ) {
      handle.SkipRequestBody();
      OMNIORB_THROW(BAD_OPERATION,
		    BAD_OPERATION_UnRecognisedOperationName,
		    CORBA::COMPLETED_NO);
    }
  }
}


void
omniOrbPOA::dispatch(omniCallHandle& handle,
		     const CORBA::Octet* key, int keysize)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 0);
  OMNIORB_ASSERT(key);
  OMNIORB_ASSERT(keysize >= pd_poaIdSize);
  //OMNIORB_ASSERT(!memcmp(key, (const char*) pd_poaId, pd_poaIdSize));

  handle.poa(this);

  // Check that the key is the right size (if system generated).
  if( !pd_policy.user_assigned_id &&
      keysize - pd_poaIdSize != SYS_ASSIGNED_ID_SIZE )

    OMNIORB_THROW(OBJECT_NOT_EXIST,
		  OBJECT_NOT_EXIST_NoMatch,
		  CORBA::COMPLETED_NO);

  switch( pd_policy.req_processing ) {
  case RPP_ACTIVE_OBJ_MAP:
    OMNIORB_THROW(OBJECT_NOT_EXIST,
		  OBJECT_NOT_EXIST_NoMatch,
		  CORBA::COMPLETED_NO);
    break;

  case RPP_DEFAULT_SERVANT:
    dispatch_to_ds(handle, key, keysize);
    break;

  case RPP_SERVANT_MANAGER:
    if( pd_policy.retain_servants )  dispatch_to_sa(handle, key, keysize);
    else                             dispatch_to_sl(handle, key, keysize);
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

  omni_optional_rlock sync(pd_call_lock,
			   pd_policy.threading != TP_SINGLE_THREAD,
			   pd_policy.threading != TP_SINGLE_THREAD);

  if( omniORB::traceInvocations ) {
    omniORB::logger l;
    l << "Dispatching local call \'" << call_desc.op() << "\' to "
      << id << '\n';
  }

  if (pd_policy.threading == TP_MAIN_THREAD) {
    omni_thread* self = omni_thread::self();
    if (!(self && self->id() == omni::mainThreadId)) {
      // Have to mess with thread switching. Leave it to
      // omniCallHandle::upcall()
      omniCallHandle handle(&call_desc,1);
      handle.poa(this);
      handle.localId(id);
      handle.mainThread(pd_main_thread_sync.mu, pd_main_thread_sync.cond);
      handle.upcall(id->servant(), call_desc);
      return;
    }
  }

  // Normal case -- do the call here
  call_desc.poa(this);
  _OMNI_NS(poaCurrentStackInsert) insert(&call_desc);
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
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);

  omniObjTableEntry* entry = omniObjTableEntry::downcast(id);
  OMNIORB_ASSERT(entry);
  // This function should only ever be called with a localIdentity
  // which is an objectTableEntry, since those are the only ones which
  // can be deactivated.

  if( omniORB::trace(15) ) {
    omniORB::logger l;
    l << "POA(" << (char*) pd_name << ") etherealising object.\n"
      << " id: " << id->servant()->_mostDerivedRepoId() << "\n";
  }

  entry->setEtherealising();

  omni::internalLock->unlock();

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
    add_object_to_etherealisation_queue(entry, sa, 0, 1);
  }
  else {
    omni::internalLock->lock();
    entry->setDead();
    omni::internalLock->unlock();
    servant->_remove_ref();
    met_detached_object();
  }
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
    pd_fullname = _CORBA_String_helper::alloc(fnlen);
    strcpy(pd_fullname, pd_parent->pd_fullname);
    strcat(pd_fullname, POA_NAME_SEP_STR);
    strcat(pd_fullname, name);

    pd_poaIdSize = 0;
    pd_poaId     = (const char*)"";
  }
  else if( pd_parent ) {
    int fnlen = strlen(parent->pd_fullname) + strlen(name) + 1;
    pd_fullname = _CORBA_String_helper::alloc(fnlen);
    strcpy(pd_fullname, parent->pd_fullname);
    strcat(pd_fullname, POA_NAME_SEP_STR);
    strcat(pd_fullname, name);

    pd_poaIdSize = fnlen + 1;
    if( policies.transient )  pd_poaIdSize += TRANSIENT_SUFFIX_SIZE + 1;
    pd_poaId = _CORBA_String_helper::alloc(pd_poaIdSize - 1);
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
    pd_poaId = _CORBA_String_helper::alloc(pd_poaIdSize - 1);
    ((char*) pd_poaId)[0] = TRANSIENT_SUFFIX_SEP;
    generateUniqueId((_CORBA_Octet*) ((char*) pd_poaId + 1));
    ((char*) pd_poaId)[pd_poaIdSize - 1] = '\0';
  }

  switch (policies.threading) {
  case TP_ORB_CTRL:
    break;
  case TP_SINGLE_THREAD:
    pd_call_lock = new omni_rmutex();
    break;
  case TP_MAIN_THREAD:
    pd_main_thread_sync.mu  = new omni_tracedmutex();
    pd_main_thread_sync.cond= new omni_tracedcondition(pd_main_thread_sync.mu);
    break;
  }

  // We assume that the policies given have been checked for validity.
  pd_policy = policies;
}


omniOrbPOA::omniOrbPOA()  // nil constructor
  : OMNIORB_BASE_CTOR(PortableServer::)POA(1),
    omniObjAdapter(1),
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

  omniObjTableEntry* obj_list = 0;

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
    pd_signal->broadcast();
    omni::internalLock->lock();
  }

  waitForAllRequestsToComplete(1);

  if( omniORB::trace(10) ) {
    omniORB::logger l;
    l << "Requests on POA(" << (char*) pd_name << ") completed.\n";
  }

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

  adapterDestroyed();
  CORBA::release(this);
}


void
omniOrbPOA::pm_change_state(PortableServer::POAManager::State new_state)
{
  omni::internalLock->lock();
  pd_rq_state = (int) new_state;
  omni::internalLock->unlock();

  pd_signal->broadcast();
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
    pd_signal->wait();

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

  omniObjTableEntry* obj_list = 0;
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
    // Search the servants activations, to see if it is activated in
    // this poa.

    omnivector<omniObjTableEntry*>::const_iterator i, last;
    i    = p_servant->_activations().begin();
    last = p_servant->_activations().end();

    for (; i != last; i++) {
      if ((*i)->adapter() == this) {
	OMNIORB_ASSERT(pd_poaIdSize == 0 ||
		       omni::ptrStrMatch(pd_poaId, (const char*) (*i)->key()));

	omniObjRef* objref =
	  omni::createLocalObjRef(p_servant->_mostDerivedRepoId(),
				  repoId, *i);
	OMNIORB_ASSERT(objref);
	return objref->_ptrToObjRef(repoId);
      }
    }
  }

  if( !pd_policy.implicit_activation )  throw WrongPolicy();
  CHECK_NOT_DYING();

  // If we get here, then we have the implicit activation policy,
  // and either the servant is not activated in this POA, or we
  // have the multiple id policy.

  omniObjTableEntry* entry = 0;
  omniObjKey key;
  int idsize;
  const CORBA::Octet* oid;

  do {
    create_new_key(key, &oid, &idsize);
    entry = omniObjTable::newEntry(key);

  } while( !entry );

  entry->setActive(p_servant, this);

  // *** FIXME: Move into setActive() ?
  p_servant->_add_ref();
  entry->insertIntoOAObjList(&pd_activeObjList);

  omniObjRef* objref = omni::createLocalObjRef(p_servant->_mostDerivedRepoId(),
					       repoId, entry);
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
  policy.threading = omniOrbPOA::TP_ORB_CTRL;
  policy.transient = 1;
  policy.multiple_id = 0;
  policy.user_assigned_id = 0;
  policy.retain_servants = 1;
  policy.req_processing = omniOrbPOA::RPP_ACTIVE_OBJ_MAP;
  policy.implicit_activation = 1;
  policy.bidirectional_accept = 0;

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
    policy.threading           = omniOrbPOA::TP_ORB_CTRL;;
    policy.transient           = 0;
    policy.multiple_id         = 0;
    policy.user_assigned_id    = 1;
    policy.retain_servants     = 1;
    policy.req_processing      = omniOrbPOA::RPP_ACTIVE_OBJ_MAP;
    policy.implicit_activation = 1;
    policy.bidirectional_accept = 0;

    omni_tracedmutex_lock sync2(theRootPOA->pd_lock);

    if (theRootPOA->pd_dying)
      OMNIORB_THROW(OBJ_ADAPTER,
		    OBJ_ADAPTER_POANotInitialised,
		    CORBA::COMPLETED_NO);

    omniOrbPOAManager* manager = new omniOrbPOAManager();

    theINSPOA = new omniOrbPOA("omniINSPOA", manager, policy, (omniOrbPOA*)1);

    theRootPOA->insert_child(theINSPOA);

    manager->gain_poa(theINSPOA);
    theINSPOA->adapterActive();
  }
  theINSPOA->incrRefCount();
  return theINSPOA;
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
    if (omniORB::trace(15)) {
      omniORB::logger l;
      l << "POA for " << lid << " in HOLDING state; waiting...\n";
    }
    if( orbParameters::poaHoldRequestTimeout ) {
      unsigned long sec, nsec;
      omni_thread::get_time(&sec, &nsec,
			    orbParameters::poaHoldRequestTimeout/1000,
			    (orbParameters::poaHoldRequestTimeout%1000)*1000000);
      if( !pd_signal->timedwait(sec, nsec) ) {
	// We have to do startRequest() here, since the identity
	// will do endInvocation() when we pass through there.
	startRequest();
	omni::internalLock->unlock();
	OMNIORB_THROW(TRANSIENT, TRANSIENT_CallTimedout, CORBA::COMPLETED_NO);
      }
    }
    else
      pd_signal->wait();
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
    OMNIORB_THROW(TRANSIENT, TRANSIENT_POANoResource, CORBA::COMPLETED_NO);

  case (int) PortableServer::POAManager::INACTIVE:
    // We have to do startRequest() here, since the identity
    // will do endInvocation() when we pass through there.
    startRequest();
    omni::internalLock->unlock();

    // The spec says "When a POA manager is in the inactive state, the
    // associated POAs will reject new requests. The rejection
    // mechanism used is specific to the vendor. ... If the client is
    // co-resident in the same process, the ORB could raise the
    // OBJ_ADAPTER system exception, with standard minor code 1, ..."
    //
    // Not very clear. We choose to always throw OBJ_ADAPTER.

    OMNIORB_THROW(OBJ_ADAPTER,
		  OBJ_ADAPTER_POAUnknownAdapter,
		  CORBA::COMPLETED_NO);
  }

  // Check to see if the object has been deactivated while we've been
  // holding. If so, throw a TRANSIENT exception.

  CORBA::Boolean deactivated;
  pd_lock.lock();
  deactivated = lid->deactivated();
  pd_lock.unlock();

  if (deactivated) {
    // We have to do startRequest() here, since the identity
    // will do endInvocation() when we pass through there.
    startRequest();
    omni::internalLock->unlock();
    OMNIORB_THROW(TRANSIENT,
		  TRANSIENT_ObjDeactivatedWhileHolding,
		  CORBA::COMPLETED_NO);
  }
}


void
omniOrbPOA::deactivate_objects(omniObjTableEntry* entry)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);

  omniObjTableEntry* next;

  while( entry ) {
    while (entry->state() == omniObjTableEntry::ACTIVATING)
      entry->wait(omniObjTableEntry::ACTIVE |
		  omniObjTableEntry::DEACTIVATING |
		  omniObjTableEntry::ETHEREALISING);

    next = entry->nextInOAObjList();

    if (entry->state() == omniObjTableEntry::ACTIVE)
      entry->setDeactivating();

    if (!entry->is_idle()) {
      // Entry has outstanding invocations. When the last invocation
      // finishes, lastInvocationHasCompleted() will be called. We
      // detach the entry here, so the destroying thread doesn't try
      // to etherealise it.
      if (omniORB::trace(20)) {
	omniORB::logger l;
	l << entry << " is not idle, etherealise from other thread\n";
      }
      entry->removeFromOAObjList();
      detached_object();
    }
    entry = next;
  }
}


void
omniOrbPOA::complete_object_deactivation(omniObjTableEntry* entry)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);

  while( entry ) {
    if (entry->state() == omniObjTableEntry::DEACTIVATING)
      entry->setEtherealising();

    OMNIORB_ASSERT(entry->is_idle());
    entry = entry->nextInOAObjList();
  }
}


void
omniOrbPOA::etherealise_objects(omniObjTableEntry* entry,
				_CORBA_Boolean etherealise,
				PortableServer::ServantActivator_ptr sa)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 0);
  ASSERT_OMNI_TRACEDMUTEX_HELD(pd_lock, 0);

  while( entry ) {
    OMNIORB_ASSERT(entry->is_idle());

    omniObjTableEntry* next = entry->nextInOAObjList();

    PortableServer::Servant servant = DOWNCAST(entry->servant());

    if( sa && etherealise ) {
      // We have to do these etherealisations in a separate thread
      // to ensure they are properly serialised.
      add_object_to_etherealisation_queue(entry, sa, 0);
    }
    else {
      omni::internalLock->lock();
      entry->setDead();
      omni::internalLock->unlock();
      servant->_remove_ref();
    }
    entry = next;
  }
}


//////////////////////////////////////////////////////////////////////
////////////////////// Servant Activator queue ///////////////////////
//////////////////////////////////////////////////////////////////////

// Calls to incarnate() and etherealize() must be serialised. Calls to
// etherealize are queued to be performed by the queue's thread; calls
// to incarnate() are made by dispatch_to_sa(), but are synchronised
// with the queue's activities.

class omniServantActivatorTaskQueue : public omni_thread {
public:
  class Task;

  virtual ~omniServantActivatorTaskQueue();
  omniServantActivatorTaskQueue();

  inline void lock()   { pd_task_lock.lock(); }
  inline void unlock() { pd_task_lock.unlock(); }
  // Lock and unlock the task lock. This must come before all other
  // locks in the partial order of locks, since it is held when
  // calling into applcation code.

  void insert(Task*);
  // Add a task to the queue. The Task object is _not_ deleted after
  // it runs; if that's needed, the Task's doit() function should do
  // the delete.

  void die();

  class Task {
  public:
    inline Task() : pd_next(0) {}
    virtual ~Task();
    virtual void doit() = 0;
  private:
    friend class omniServantActivatorTaskQueue;
    Task(const Task&);
    Task& operator=(const Task&);

    Task* pd_next;
  };

private:
  virtual void* run_undetached(void*);

  omni_tracedmutex     pd_queue_lock;
  omni_tracedmutex     pd_task_lock;
  omni_tracedcondition pd_cond;
  Task*                pd_taskq;
  Task*                pd_taskqtail;
  int                  pd_dying;
};

omniServantActivatorTaskQueue::~omniServantActivatorTaskQueue() {}


omniServantActivatorTaskQueue::omniServantActivatorTaskQueue()
  : pd_cond(&pd_queue_lock),
    pd_taskq(0),
    pd_taskqtail(0),
    pd_dying(0)
{
  start_undetached();
}


void
omniServantActivatorTaskQueue::insert(Task* t)
{
  OMNIORB_ASSERT(t);

  omni_tracedmutex_lock sync(pd_queue_lock);

  int signal = !pd_taskq;

  t->pd_next = 0;
  if( pd_taskq ) {
    pd_taskqtail->pd_next = t;
    pd_taskqtail = t;
  }
  else
    pd_taskq = pd_taskqtail = t;

  if( signal )  pd_cond.signal();
}

void
omniServantActivatorTaskQueue::die()
{
  pd_queue_lock.lock();
  pd_dying = 1;
  int signal = !pd_taskq;
  pd_queue_lock.unlock();

  if( signal )  pd_cond.signal();
}

void*
omniServantActivatorTaskQueue::run_undetached(void*)
{
  while( 1 ) {
    pd_queue_lock.lock();
    while( !pd_taskq ) {
      if( pd_dying ) {
        pd_queue_lock.unlock();
	omniORB::logs(15, "Servant Activator task queue exit");
        return 0;
      }
      pd_cond.wait();
    }
    Task* t = pd_taskq;
    pd_taskq = pd_taskq->pd_next;

    pd_queue_lock.unlock();

    try {
      omni_tracedmutex_lock sync(pd_task_lock);
      t->doit();
    }
    catch(...) {}
  }
  return 0;
}

omniServantActivatorTaskQueue::Task::~Task()  {}


class omniEtherealiser : public omniServantActivatorTaskQueue::Task {
public:
  inline omniEtherealiser(omniObjTableEntry* entry,
			  PortableServer::ServantActivator_ptr sa,
			  omniOrbPOA* poa, const _CORBA_Octet* id,
			  int idsize, CORBA::Boolean cleanup)
    : pd_entry(entry), pd_sa(sa),
      pd_poa(poa), pd_cleanup(cleanup)
  {
    pd_oid.length(idsize);
    memcpy(pd_oid.NP_data(), id, idsize);
  }
  inline void set_is_last(int il) { pd_is_last = il; }

  virtual void doit();

private:
  omniObjTableEntry*                   pd_entry;
  int                                  pd_is_last;
  PortableServer::ServantActivator_ptr pd_sa;
  omniOrbPOA*                          pd_poa;
  PortableServer::ObjectId             pd_oid;
  CORBA::Boolean                       pd_cleanup;
};

void
omniEtherealiser::doit()
{
  // Call into application-supplied etherealisation code.
  try {
    pd_sa->etherealize(pd_oid, pd_poa, DOWNCAST(pd_entry->servant()),
		       pd_cleanup, !pd_is_last);
  }
  catch(...) {
    if( omniORB::trace(5) )
      omniORB::logf("Servant etherealisation raised an exception!");
  }
  omni::internalLock->lock();
  pd_entry->setDead();
  omni::internalLock->unlock();

  pd_poa->met_detached_object();
  delete this;
}

static omniServantActivatorTaskQueue* servant_activator_queue = 0;


void
omniOrbPOA::add_object_to_etherealisation_queue(omniObjTableEntry* entry,
				PortableServer::ServantActivator_ptr sa,
				int cleanup_in_progress, int detached)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 0);
  OMNIORB_ASSERT(entry);

  omniEtherealiser* e = new omniEtherealiser(entry, sa, this,
					     entry->key() + pd_poaIdSize,
					     entry->keysize() - pd_poaIdSize,
					     cleanup_in_progress);

  if( !detached )  detached_object();

  omni::internalLock->lock();

  if( !servant_activator_queue )
    servant_activator_queue = new omniServantActivatorTaskQueue;

  e->set_is_last(entry->servant()->_activations().empty());

  servant_activator_queue->insert(e);

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
omniOrbPOA::dispatch_to_ds(omniCallHandle& handle,
			   const CORBA::Octet* key, int keysize)
{
  pd_lock.lock();
  if( !pd_defaultServant ) {
    pd_lock.unlock();
    OMNIORB_THROW(OBJ_ADAPTER, OBJ_ADAPTER_NoDefaultServant,
		  CORBA::COMPLETED_NO);
  }

  pd_defaultServant->_add_ref();
  PortableServer::ServantBase_var servant(pd_defaultServant);
  pd_lock.unlock();

  omniLocalIdentity the_id(key, keysize,
			   (PortableServer::Servant)servant, this);

  omniORB::logs(10, "Dispatching through default servant");

  omni::internalLock->lock();
  the_id.dispatch(handle);
}


void
omniOrbPOA::dispatch_to_sa(omniCallHandle& handle,
			   const CORBA::Octet* key, int keysize)
{
  // A bit of prep. outside the critical sections...
  int idsize = keysize - pd_poaIdSize;
  PortableServer::ObjectId oid(idsize);
  oid.length(idsize);
  memcpy(oid.NP_data(), key + pd_poaIdSize, idsize);
  CORBA::ULong hash = omni::hash(key, keysize);

  omniObjTableEntry* entry;

  // We need to check the object is not already activated. If there
  // is an ACTIVE or DEACTIVATING entry in the object table,
  // dispatch to it; if there is an entry in another state, wait
  // until it's ACTIVE or DEAD, then continue as before.
  omni::internalLock->lock();

  entry = omniObjTable::locate(key, keysize, hash,
			       omniObjTableEntry::ACTIVE |
			       omniObjTableEntry::DEACTIVATING);
  if (entry) {
    if (omniORB::trace(10)) {
      omniORB::logger l;
      l << "No need to incarnate " << entry << ".\n";
    }
    entry->dispatch(handle);
    return;
  }

  // Put a place-holder in the object table so any other threads
  // know we're in the process of activating.
  omniObjKey okey(key, keysize);
  entry = omniObjTable::newEntry(okey, hash);
  OMNIORB_ASSERT(entry);

  if( !servant_activator_queue )
    servant_activator_queue = new omniServantActivatorTaskQueue;

  enterAdapter();
  omni::internalLock->unlock();

  pd_lock.lock();
  if( pd_dying || !pd_servantActivator ) {
    omni::internalLock->lock();
    entry->setDead();
    exitAdapter(1,1);
    omni::internalLock->unlock();
    pd_lock.unlock();

    if (pd_dying)
      OMNIORB_THROW(OBJECT_NOT_EXIST,
		    OBJECT_NOT_EXIST_POANotInitialised,
		    CORBA::COMPLETED_NO);
    else
      OMNIORB_THROW(OBJ_ADAPTER,
		    OBJ_ADAPTER_NoServantManager,
		    CORBA::COMPLETED_NO);
  }
  PortableServer::ServantActivator::_duplicate(pd_servantActivator);
  PortableServer::ServantActivator_var sa(pd_servantActivator);

  pd_lock.unlock();

  PortableServer::Servant servant;

  servant_activator_queue->lock();

  try {
    servant = sa->incarnate(oid, this);
  }
  catch(PortableServer::ForwardRequest& fr) {
    servant_activator_queue->unlock();
    omni::internalLock->lock();
    entry->setDead();
    exitAdapter(1,1);
    omni::internalLock->unlock();
    throw omniORB::LOCATION_FORWARD(
	               CORBA::Object::_duplicate(fr.forward_reference), 0);
  }
#ifndef HAS_Cplusplus_catch_exception_by_base
#define RETHROW_EXCEPTION(name)  \
  catch (CORBA::name& ex) {  \
    servant_activator_queue->unlock();  \
    omni::internalLock->lock(); \
    entry->setDead(); \
    exitAdapter(1,1); \
    omni::internalLock->unlock(); \
    throw;  \
  }
  OMNIORB_FOR_EACH_SYS_EXCEPTION(RETHROW_EXCEPTION)
#undef RETHROW_EXCEPTION
#else
  catch(CORBA::SystemException&) {
    servant_activator_queue->unlock();
    omni::internalLock->lock();
    entry->setDead();
    exitAdapter(1,1);
    omni::internalLock->unlock();
    throw;
  }
#endif
  catch(...) {
    servant_activator_queue->unlock();
    omni::internalLock->lock();
    entry->setDead();
    exitAdapter(1,1);
    omni::internalLock->unlock();
    OMNIORB_THROW(UNKNOWN, UNKNOWN_UserException, CORBA::COMPLETED_NO);
  }
  servant_activator_queue->unlock();

  if( !servant ) {
    omni::internalLock->lock();
    entry->setDead();
    exitAdapter(1,1);
    omni::internalLock->unlock();
    omniORB::logs(5, "ServantActivator::incarnate() returned 0 (zero)!");

    // The 2.4.2 spec incorrectly says we should throw OBJ_ADAPTER
    // with standard minor code 3, but that's used for "no default
    // servant". The correct code is 2 -- see OMG issue 4306.
    OMNIORB_THROW(OBJ_ADAPTER, OBJ_ADAPTER_NoServant, CORBA::COMPLETED_NO);
  }

  pd_lock.lock();
  omni::internalLock->lock();
  if( !pd_policy.multiple_id ) {
    // Check the servant is not already active in this poa.

    omnivector<omniObjTableEntry*>::const_iterator i, last;
    i    = servant->_activations().begin();
    last = servant->_activations().end();

    for (; i != last; i++) {
      if ((*i)->adapter() == this) {
	entry->setDead();
	exitAdapter(1,1);
	omni::internalLock->unlock();
	pd_lock.unlock();

	if( omniORB::trace(2) ) {
	  omniORB::logger l;
	  l << "A servant activator returned a servant which is already"
	    " activated\n as " << (*i) << "\n to POA(" << (char*) pd_name << ")"
	    " which has the UNIQUE_ID policy.\n";
	}
	// The 2.4.2 spec doesn't give a standard minor code to use
	// here, so we use an omniORB specific one.
	OMNIORB_THROW(OBJ_ADAPTER,
		      OBJ_ADAPTER_ServantAlreadyActive,
		      CORBA::COMPLETED_NO);
      }
    }
  }
  entry->setActive(servant, this);
  entry->insertIntoOAObjList(&pd_activeObjList);

  if( pd_dying ) {
    // After all that, we're dying and we have to schedule the object
    // for etherealisation.

    pd_lock.unlock();
    entry->setDeactivating();
    entry->removeFromOAObjList();
    entry->setEtherealising();
    omni::internalLock->unlock();
    add_object_to_etherealisation_queue(entry, sa, 0);
    exitAdapter();
    OMNIORB_THROW(OBJECT_NOT_EXIST,
		  OBJECT_NOT_EXIST_POANotInitialised,
		  CORBA::COMPLETED_NO);
  }

  pd_lock.unlock();
  exitAdapter(1, 1);

  entry->dispatch(handle);
}


void
omniOrbPOA::dispatch_to_sl(omniCallHandle& handle,
			   const CORBA::Octet* key, int keysize)
{
  pd_lock.lock();
  if( pd_dying ) {
    pd_lock.unlock();
    OMNIORB_THROW(OBJECT_NOT_EXIST,
		  OBJECT_NOT_EXIST_POANotInitialised,
		  CORBA::COMPLETED_NO);
  }
  if( !pd_servantLocator ) {
    pd_lock.unlock();
    OMNIORB_THROW(OBJ_ADAPTER,
		  OBJ_ADAPTER_NoServantManager,
		  CORBA::COMPLETED_NO);
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
    servant = sl->preinvoke(oid, this, handle.operation_name(), cookie);
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
			    CORBA::Object::_duplicate(fr.forward_reference),0);
  }
  catch(...) {
    exitAdapter();
    OMNIORB_THROW(UNKNOWN, UNKNOWN_UserException, CORBA::COMPLETED_NO);
  }

  if( !servant ) {
    exitAdapter();
    omniORB::logs(5, "ServantLocator::preinvoke() returned 0 (zero)!");

    // The 2.4.2 spec incorrectly says we should throw OBJ_ADAPTER
    // with standard minor code 3, but that's used for "no default
    // servant". The correct code is 2 -- see OMG issue 4306.
    OMNIORB_THROW(OBJ_ADAPTER, OBJ_ADAPTER_NoServant, CORBA::COMPLETED_NO);
  }

  omniLocalIdentity the_id(key, keysize,
			   (PortableServer::Servant)servant, this);

  // Create postinvoke hook
  SLPostInvokeHook postinvokeHook(this, sl, oid, handle.operation_name(),
				  cookie, servant);
  handle.postinvoke_hook(&postinvokeHook);

  omni::internalLock->lock();
  the_id.dispatch(handle);
}

void
omniOrbPOA::
SLPostInvokeHook::
postinvoke()
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 0);

  try {
    pd_sl->postinvoke(pd_oid, pd_poa, pd_op, pd_cookie, pd_servant);
  }
  catch (...) {
    pd_poa->exitAdapter();
    throw;
  }
  pd_poa->exitAdapter();
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

  // ?? NB. We could implement the above without any dynamic
  // allocations by using a queue, and allocating queue entries on the
  // stack (much like linux kernel wait queues).

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
  seen.threading = 0;
  seen.transient = 0;
  seen.multiple_id = 0;
  seen.user_assigned_id = 0;
  seen.retain_servants = 0;
  seen.req_processing = 0;
  seen.implicit_activation = 0;
  seen.bidirectional_accept = 0;

  // Check for policies which contradict one-another.

  for( CORBA::ULong i = 0; i < pin.length(); i++ )
    switch( pin[i]->policy_type() ) {

    case /*THREAD_POLICY_ID*/ 16:
      {
	PortableServer::ThreadPolicy_var p;
	p = PortableServer::ThreadPolicy::_narrow(pin[i]);

	if( seen.threading ) {
	  if ( pout.threading == omniOrbPOA::TP_ORB_CTRL &&
	       p->value() != PortableServer::ORB_CTRL_MODEL ||
	       pout.threading == omniOrbPOA::TP_SINGLE_THREAD &&
	       p->value() != PortableServer::SINGLE_THREAD_MODEL ||
	       pout.threading == omniOrbPOA::TP_MAIN_THREAD &&
	       p->value() != PortableServer::MAIN_THREAD_MODEL )
	    throw PortableServer::POA::InvalidPolicy(i);
	}
	switch( p->value() ) {
	case PortableServer::ORB_CTRL_MODEL:
	  pout.threading = omniOrbPOA::TP_ORB_CTRL;
	  break;
	case PortableServer::SINGLE_THREAD_MODEL:
	  pout.threading = omniOrbPOA::TP_SINGLE_THREAD;
	  break;
	case PortableServer::MAIN_THREAD_MODEL:
	  pout.threading = omniOrbPOA::TP_MAIN_THREAD;
	  break;
	default:
	  throw PortableServer::POA::InvalidPolicy(i);
	}
	seen.threading = 1;
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

    case /*BiDirPolicy::BIDIRECTIONAL_POLICY_TYPE*/ 37:
      {
	BiDirPolicy::BidirectionalPolicy_var p;
	p = BiDirPolicy::BidirectionalPolicy::_narrow(pin[i]);
	if (seen.bidirectional_accept) {
	  throw PortableServer::POA::InvalidPolicy(i);
	}
	seen.bidirectional_accept = 1;
	if (p->value() == BiDirPolicy::BOTH) {
	  pout.bidirectional_accept = 1;
	}
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

  if (!pout.retain_servants &&
      !(pout.req_processing == omniOrbPOA::RPP_DEFAULT_SERVANT ||
	pout.req_processing == omniOrbPOA::RPP_SERVANT_MANAGER)) {

    for( CORBA::ULong i = 0; i < pin.length(); i++ )
      if( pin[i]->policy_type() == /*SERVANT_RETENTION_POLICY_ID*/ 21 )
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

  omni::internalLock->lock();
  omniServantActivatorTaskQueue* queue = servant_activator_queue;
  omni::internalLock->unlock();

  if (queue) {
    void* v;
    servant_activator_queue->die();
    servant_activator_queue->join(&v);
  }
  servant_activator_queue = 0;
}

OMNI_NAMESPACE_BEGIN(omni)

/////////////////////////////////////////////////////////////////////////////
//            Handlers for Configuration Options                           //
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
class poaHoldRequestTimeoutHandler : public orbOptions::Handler {
public:

  poaHoldRequestTimeoutHandler() :
    orbOptions::Handler("poaHoldRequestTimeout",
			"poaHoldRequestTimeout = n >= 0 in msec",
			1,
			"-ORBpoaHoldRequestTimeout < n >= 0 in msec >") {}

  void visit(const char* value,orbOptions::Source) throw (orbOptions::BadParam) {

    CORBA::ULong v;
    if (!orbOptions::getULong(value,v)) {
      throw orbOptions::BadParam(key(),value,
				 orbOptions::expect_non_zero_ulong_msg);
    }
    orbParameters::poaHoldRequestTimeout = v;
  }

  void dump(orbOptions::sequenceString& result) {
    orbOptions::addKVULong(key(),orbParameters::poaHoldRequestTimeout,
			   result);
  }
};

static poaHoldRequestTimeoutHandler poaHoldRequestTimeoutHandler_;


/////////////////////////////////////////////////////////////////////////////
//            Module initialiser                                           //
/////////////////////////////////////////////////////////////////////////////

static CORBA::Object_ptr resolveRootPOAFn(){ return omniOrbPOA::rootPOA(); }
static CORBA::Object_ptr resolveINSPOAFn() { return omniOrbPOA::omniINSPOA(); }

class omni_poa_initialiser : public omniInitialiser {
public:

  omni_poa_initialiser() {
    orbOptions::singleton().registerHandler(poaHoldRequestTimeoutHandler_);
    omniInitialReferences::registerPseudoObjFn("RootPOA",    resolveRootPOAFn);
    omniInitialReferences::registerPseudoObjFn("omniINSPOA", resolveINSPOAFn);
  }

  void attach() {
  }
  void detach() { }
};


static omni_poa_initialiser initialiser;

omniInitialiser& omni_poa_initialiser_ = initialiser;

OMNI_NAMESPACE_END(omni)

