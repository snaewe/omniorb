// -*- Mode: C++; -*-
//                            Package   : omniORB
// poaimpl.h                  Created on: 31/3/99
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
//    Internal implementation of the POA.
//

/*
  $Log$
  Revision 1.1.2.2  1999/09/24 10:28:52  djr
  Added POA_Helper and POA::the_children().

  Revision 1.1.2.1  1999/09/22 14:27:01  djr
  Major rewrite of orbcore to support POA.

*/

#ifndef __POAIMPL_H__
#define __POAIMPL_H__

#include <objectAdapter.h>
#include <poamanager.h>
#include <taskqueue.h>


#define PS_VERSION  ":2.3"


class omniOrbPOAManager;


class omniOrbPOA : public PortableServer::POA,
		   public omniObjAdapter
{
public:
  virtual ~omniOrbPOA();
  // Must not hold <omni::internalLock>.

  /////////////////////////
  // PortableServer::POA //
  /////////////////////////
  virtual PortableServer::POA_ptr create_POA(const char* adapter_name,
			     PortableServer::POAManager_ptr a_POAManager,
			     const CORBA::PolicyList& policies);
  virtual PortableServer::POA_ptr find_POA(const char* adapter_name,
					   CORBA::Boolean activate_it);
  virtual void destroy(CORBA::Boolean etherealize_objects,
		       CORBA::Boolean wait_for_completion);

  virtual char* the_name();
  virtual PortableServer::POA_ptr the_parent();
  virtual PortableServer::POAList* the_children();
  virtual PortableServer::POAManager_ptr the_POAManager();
  virtual PortableServer::AdapterActivator_ptr the_activator();
  virtual void the_activator(PortableServer::AdapterActivator_ptr aa);

  virtual PortableServer::ServantManager_ptr get_servant_manager();
  virtual void set_servant_manager(PortableServer::ServantManager_ptr imgr);

  virtual PortableServer::Servant get_servant();
  virtual void set_servant(PortableServer::Servant p_servant);

  virtual PortableServer::ObjectId* activate_object(
					    PortableServer::Servant p_servant);
  virtual void activate_object_with_id(const PortableServer::ObjectId& id,
				       PortableServer::Servant p_servant);
  virtual void deactivate_object(const PortableServer::ObjectId& oid);

  virtual CORBA::Object_ptr create_reference(const char* intf);
  virtual CORBA::Object_ptr create_reference_with_id(
				     const PortableServer::ObjectId& oid,
				     const char* intf);

  virtual
  PortableServer::ObjectId* servant_to_id(PortableServer::Servant p_servant);
  virtual
  CORBA::Object_ptr servant_to_reference(PortableServer::Servant p_servant);
  virtual
  PortableServer::Servant reference_to_servant(CORBA::Object_ptr reference);
  virtual
  PortableServer::ObjectId* reference_to_id(CORBA::Object_ptr reference);
  virtual
  PortableServer::Servant id_to_servant(const PortableServer::ObjectId& oid);
  virtual
  CORBA::Object_ptr id_to_reference(const PortableServer::ObjectId& oid);

  ////////////////////////////
  // Override CORBA::Object //
  ////////////////////////////

  virtual _CORBA_Boolean _non_existent();
  virtual void* _ptrToObjRef(const char* repoId);
  virtual void _NP_incrRefCount();
  virtual void _NP_decrRefCount();

  /////////////////////////////
  // Override omniObjAdapter //
  /////////////////////////////

  virtual void  incrRefCount();
  virtual void  decrRefCount();
  virtual void  dispatch(GIOP_S&, omniLocalIdentity*);
  virtual void  dispatch(GIOP_S&, const _CORBA_Octet* key, int keysize);
  virtual void  dispatch(omniCallDescriptor&, omniLocalIdentity*);
  virtual int   objectExists(const _CORBA_Octet* key, int keysize);
  virtual void  lastInvocationHasCompleted(omniLocalIdentity* id);
  virtual char* ppObject(omniLocalIdentity*);

  //////////////////////
  // omniORB Internal //
  //////////////////////

  enum {
    RPP_ACTIVE_OBJ_MAP = 0,
    RPP_DEFAULT_SERVANT = 1,
    RPP_SERVANT_MANAGER = 2
  };
  // Values for the req_processing field of Policies below.

  struct Policies {
    unsigned single_threaded        : 1;
    unsigned transient              : 1;
    unsigned multiple_id            : 1;
    unsigned user_assigned_id       : 1;
    unsigned retain_servants        : 1;
    unsigned req_processing         : 2;
    unsigned implicit_activation    : 1;
  };
  // The PERSISTENT lifespan policy is not currently supported.

  typedef _CORBA_Pseudo_Unbounded_Sequence<omniOrbPOA*, omniOrbPOA*> ChildSeq;


  omniOrbPOA(const char* name, omniOrbPOAManager* manager,
	     const Policies& policies, omniOrbPOA* parent);
  // Consumes <manager>.

  omniOrbPOA();
  // Constructor for nil POA object.

  void do_destroy(_CORBA_Boolean etherealize_objects);
  // The main work of destroying a POA.  Called by destroy(),
  // either directly or from a separate thread.  POA is
  // already marked dying at this stage.
  //  Must not hold any mutexes.

  /////////////////////////
  // POA Manager support //
  /////////////////////////

  void pm_change_state(omniOrbPOAManager::State new_state);
  // Changes the current POA state, returning immediately.
  // Must be called by the POAManager (pd_manager) only.

  void pm_waitForReqCmpltnOrSttChnge(omniOrbPOAManager::State state);
  // Blocks until all outstanding requests on objects in this POA
  // complete, or the state changes (due to another call on the
  // POA manager).

  void pm_deactivate(_CORBA_Boolean etherealize_objects);
  // Implements POAManager::deactivate().  Blocks until complete.


  void* servant__this(PortableServer::Servant servant, const char* repoId);
  // Used by PortableServer::ServantBase::_do_this() to implement
  // POA_foo::_this().

  static PortableServer::POA_ptr rootPOA(int init_if_none = 1);
  // Returns a reference to the root poa, initialising the
  // POA and omniObjAdapter if necessary.  if <init_if_none>
  // is zero, and the POA has not been initialised, then this
  // returns 0 (*not* POA::_nil()).
  //  This function is thread-safe.
  //?? Can this throw exceptions?  What if it fails?

  static void shutdown();
  // Destroys the root poa, waiting for completion.  If the
  // poa has not been initialised, or has already been destroyed
  // then this does nothing.
  //  Must not be in the context of a CORBA call.
  //  Not thread safe.

  static omniOrbPOA* getAdapter(const _CORBA_Octet* key, int keysize);
  // Attempts to find the adapter which generated the given key,
  // activating it if necassary.  Returns zero if the key is not
  // a valid POA key, or the POA cannot be found or activated.
  // Throws OBJ_ADAPTER if an adapter activator throws a system
  // exception.


private:
  void create_key(omniObjKey& key_out, const CORBA::Octet* id, int idsize);

  void create_new_key(omniObjKey& key_out, const CORBA::Octet** id = 0,
		      int* idsize = 0);
  // Must hold <pd_lock>.

  omniOrbPOA* find_child(const char* name);
  omniOrbPOA* find_child(const char* name_start, const char* name_end);
  // Does not increment the ref count of the returned POA.
  // Returns 0 if not found.
  //  Must hold <poa_lock>.

  void insert_child(omniOrbPOA* child);
  // Must hold <poa_lock>.

  void lose_child(omniOrbPOA* child);
  // Removes the child from our list of children, but does not
  // release the reference.
  //  Must hold <poa_lock>.

  int adapter_name_is_valid(const char* name);
  // Return true if <name> is a valid name for an adapter.

  void synchronise_request();
  // Must hold <omni::internalLock> on entry.  If the POA is
  // in the DISCARDING or INACTIVE state, <omni::internalLock>
  // is released, and a suitable exception is thrown.  If the
  // state is HOLDING, blocks until the state changes.  If it
  // is (or becomes) ACTIVE, just returns -- and the mutex is
  // still held on exit.

  void deactivate_objects(omniLocalIdentity* idle_objs);
  // Deactivate all objects in the given list.  Does not etherealise
  // (or detach) any of them.
  //  Must hold <omni::internalLock>.

  void complete_object_deactivation(omniLocalIdentity* id);
  // Call omniLocalIdentity::deactivate() on each object in the
  // list.  Assumes that all the objects have been deactivated
  // using deactivate_objects() above.  The objects must also
  // all be idle -- otherwise the POA will be asked to etherealise
  // them when they become idle.
  //  Must hold <omni::internalLock>.

  void etherealise_objects(omniLocalIdentity* id_list,
			   _CORBA_Boolean etherealise,
			   PortableServer::ServantActivator_ptr sa);
  // Etherealises the objects in the given list (if <etherealise>
  // is true).  The objects *must* all be idle.
  //  <sa> may be zero.

  void add_object_to_etherealisation_queue(PortableServer::Servant s,
				PortableServer::ServantActivator_ptr sa,
				const _CORBA_Octet* key, int keysize,
				int cleanup_in_progress, int detached=0);
  // Places the servant associated with the given object onto a queue
  // to be etherealised by the given ServantActivator.  This is done
  // in a separate thread.
  //  If <detached> then the object has already been 'detached' from
  // the adapter (so <pd_nOutstandingDeadObjects> is not incremented).
  //  Must not hold <omni::internalLock>.

  void dispatch_to_ds(GIOP_S&, const _CORBA_Octet*, int);
  void dispatch_to_sa(GIOP_S&, const _CORBA_Octet*, int);
  void dispatch_to_sl(GIOP_S&, const _CORBA_Octet*, int);
  // Called from dispatch(), to deal with dispatches using the
  // default servant, ServantActivator and ServantLocator
  // respectively.

  void call_postinvoke(PortableServer::ServantLocator_ptr sl,
		       PortableServer::ObjectId& oid,
		       const char* op,
		       PortableServer::ServantLocator::Cookie cookie,
		       PortableServer::Servant servant);
  // Calls the ServantLocator::postinvoke() method.  Used by
  // dispatch_to_sl() above.


  class Etherealiser : public omniTaskQueue::Task {
  public:
    inline Etherealiser(PortableServer::Servant s,
			PortableServer::ServantActivator_ptr sa,
			omniOrbPOA* poa, const _CORBA_Octet* id,
			int idsize, CORBA::Boolean cleanup)
      : pd_servant(s), pd_sa(sa),
	pd_poa(poa), pd_cleanup(cleanup) {
      pd_oid.length(idsize);
      memcpy(pd_oid.NP_data(), id, idsize);
    }
    inline void set_is_last(int il) { pd_is_last = il; }

    virtual void doit();

  private:
    PortableServer::Servant              pd_servant;
    int                                  pd_is_last;
    PortableServer::ServantActivator_ptr pd_sa;
    omniOrbPOA*                          pd_poa;
    PortableServer::ObjectId             pd_oid;
    CORBA::Boolean                       pd_cleanup;
  };
  friend class Etherealiser;


  int                                  pd_destroyed;
  // True if this POA has been destroyed.  This is set to 1 when
  // apparent destruction is complete.  ie. All objects have
  // been deactivated, and outstanding requests have completed.
  // However, etherealisation of objects may not yet have
  // occurred.  It is set to 2 when etherealisations are also
  // complete.  <pd_deathSignal> is signalled each time it
  // changes.
  //  NB. It is not always locked for reads, but since it is an
  // integer, reads should be atomic.
  //  Protected by <pd_lock>.

  int                                  pd_dying;
  // True if destroy() has been called for this POA, and so
  // it is in the process of being destroyed.
  //  Protected by <pd_lock>.

  int                                  pd_refCount;
  // Protected by <poa_lock>.

  CORBA::String_var                    pd_name;
  // Immutable.

  omniOrbPOA*                          pd_parent;
  // We don't own a reference to the parent -- it holds a
  // reference to us.  This is nil only for a POA which has
  // been destroyed, and the root poa.
  //  Mutable.  Protected by <pd_lock>.

  omniOrbPOAManager*                   pd_manager;
  // We hold a reference to this.
  //  Immutable.

  PortableServer::AdapterActivator_ptr pd_adapterActivator;
  // May be 0.
  //  Mutable.  Protected by <pd_lock>.

  PortableServer::ServantActivator_ptr pd_servantActivator;
  PortableServer::ServantLocator_ptr   pd_servantLocator;
  // At most one of these is non-zero, and then only if
  // pd_policy.req_processing == RPP_SERVANT_MANAGER.  (We
  // use 0, rather than the nil object to represent absence
  // of these).  Once set, they can never be changed.
  //  Mutable.  Protected by <pd_lock>.

  PortableServer::Servant              pd_defaultServant;
  // We hold a reference to this servant.  Only non-zero
  // if pd_policy.req_processing == RPP_DEFAULT_SERVANT,
  // and then only if the application has registered one.
  //  Mutable.  Protected by <pd_lock>.

  volatile int                         pd_rq_state;
  // Only changed in response to request by the POAManager, or
  // when this POA is destroyed.
  //  Mutable.  Protected by <omni::internalLock>.

  CORBA::String_var                    pd_fullname;
  // Name of this poa and all parents in \xff separated list.

  CORBA::String_var                    pd_poaId;
  int                                  pd_poaIdSize;
  // Representation of the poa name in an object key
  // (including terminating '\0').  The size of an
  // object key is this size + the size of the id.
  //  Immutable.

  ChildSeq                             pd_children;
  // The children of this POA, sorted in alphabetical order
  // of their adapter name.  We hold a reference to each
  // child.
  //  Mutable.  Protected by <poa_lock>.

  Policies                             pd_policy;
  // Immutable.

  omni_tracedmutex*                    pd_call_lock;
  // This lock is used to enforce the single threaded model policy.
  // if( pd_policy.single_threaded ) then a mutex is allocated.
  // Otherwise &pd_lock is used.  Note that the mutex is never
  // used in this case, but we do not want it to be zero, as we
  // need to dereference it to pass as a reference.

  omni_tracedmutex                     pd_lock;
  // Protects access to various members.

  omni_tracedcondition                 pd_deathSignal;
  // This CV is broadcast to when the destruction of this POA is
  // complete (or nearly complete).
  //  Uses <pd_lock> for mutex.

  _CORBA_ULong                         pd_oidIndex;
  // 32-bit value used as object id for POA allocated ids.
  // Used by create_new_key().
  //  Protected by <pd_lock>.

  omniLocalIdentity*                   pd_activeObjList;
  // A list of objects activated in this adapter.  Includes only
  // those in the active object map.
  //  Protected by <pd_lock>.
};


#endif  // __POAIMPL_H__
