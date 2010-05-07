// -*- Mode: C++; -*-
//                            Package   : omniORB2
// corbaBoa.h                 Created on: 23/7/99
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
//    Implementation of the BOA interface, built upon the POA interface.
//	

/*
  $Log$
  Revision 1.1.6.1  2003/03/23 21:03:52  dgrisby
  Start of omniORB 4.1.x development branch.

  Revision 1.1.4.3  2001/08/15 10:26:09  dpg1
  New object table behaviour, correct POA semantics.

  Revision 1.1.4.2  2001/05/29 17:03:49  dpg1
  In process identity.

  Revision 1.1.4.1  2001/04/18 17:18:18  sll
  Big checkin with the brand new internal APIs.
  These files were relocated and scoped with the omni namespace.

  Revision 1.2.2.1  2000/07/17 10:35:51  sll
  Merged from omni3_develop the diff between omni3_0_0_pre3 and omni3_0_0.

  Revision 1.3  2000/07/13 15:25:58  dpg1
  Merge from omni3_develop for 3.0 release.

  Revision 1.1.2.3  2000/06/02 14:20:15  dpg1
  Using boa_lock for the nil BOA's condition variable caused an
  assertion failure on exit.

  Revision 1.1.2.2  1999/09/28 10:54:33  djr
  Removed pretty-printing of object keys from object adapters.

  Revision 1.1.2.1  1999/09/22 14:26:45  djr
  Major rewrite of orbcore to support POA.

*/

#ifndef __CORBABOA_H__
#define __CORBABOA_H__

#ifndef __OMNI_OBJECTADAPTER_H__
#include <objectAdapter.h>
#endif

#ifdef _core_attr
# error "A local CPP macro _core_attr has already been defined."
#endif

#if defined(_OMNIORB_LIBRARY)
#     define _core_attr
#else
#     define _core_attr _OMNIORB_NTDLL_IMPORT
#endif

OMNI_NAMESPACE_BEGIN(omni)

class omniOrbBOA : public CORBA::BOA, public omniObjAdapter
{
public:
  omniOrbBOA(int is_nil);
  virtual ~omniOrbBOA();

  ////////////////
  // CORBA::BOA //
  ////////////////
  virtual void impl_is_ready(CORBA::ImplementationDef_ptr, CORBA::Boolean);
  virtual void impl_shutdown();
  virtual void destroy();

  virtual void obj_is_ready(omniOrbBoaServant*, CORBA::ImplementationDef_ptr);
  virtual void obj_is_ready(CORBA::Object_ptr, CORBA::ImplementationDef_ptr);
  virtual void dispose(CORBA::Object_ptr);

  virtual CORBA::Object_ptr create(const CORBA::ReferenceData&,
				   CORBA::_objref_InterfaceDef*,
				   CORBA::ImplementationDef_ptr);
  virtual CORBA::ReferenceData* get_id(CORBA::Object_ptr);
  virtual void change_implementation(CORBA::Object_ptr,
				     CORBA::ImplementationDef_ptr);
  virtual CORBA::Principal_ptr get_principal(CORBA::Object_ptr,
					     CORBA::Environment_ptr);
  virtual void deactivate_impl(CORBA::ImplementationDef_ptr);
  virtual void deactivate_obj(CORBA::Object_ptr);

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
  virtual void  dispatch(omniCallHandle&, omniLocalIdentity*);
  virtual void  dispatch(omniCallHandle&,
			 const _CORBA_Octet* key, int keysize);
  virtual void  dispatch(omniCallDescriptor&, omniLocalIdentity*);
  virtual int   objectExists(const _CORBA_Octet* key, int keysize);
  virtual void  lastInvocationHasCompleted(omniLocalIdentity* id);

  virtual void* _ptrToClass(int* cptr);
  static inline omniOrbBOA* _downcast(omniObjAdapter* a) {
    return a ? (omniOrbBOA*)a->_ptrToClass(&_classid) : 0;
  }
  static _core_attr int _classid;

  //////////////
  // Internal //
  //////////////
  static omniOrbBOA* theBOA();
  // Returns a reference to the BOA if it exists, or
  // zero (not nil) if not.

  inline void incrRefCount_locked() { pd_refCount++; }
  // Must hold <boa_lock>.

  inline omniObjTableEntry** activeObjList() { return &pd_activeObjList; }
  // Must hold <boa_lock>.

  void dispose(omniIdentity*);
  // Deactivates the given object, returning silently on failure.
  //  The caller must hold <omni::internalLock> and <boa_lock> on
  // entry.  Both are always released before this function returns.
  //  This function does not throw any exceptions.

  void synchronise_request();


private:
  enum { IDLE, ACTIVE, DESTROYED } pd_state;
  // Protected by <omni::internalLock>.

  int                              pd_refCount;
  // Protected by <boa_lock>.

  omniObjTableEntry*               pd_activeObjList;
  // A list of objects activated in this adapter.
  // Protected by <boa_lock>.

  int                              pd_nblocked;
  // The number of threads blocked in impl_is_ready().
  // Protected by <boa_lock> and <pd_signal>.

  int                              pd_nwaiting;
  // The number of threads blocked in
  // synchronise_request().
  //  Protected by <omni::internalLock>.

  omni_tracedcondition*            pd_state_signal;
  // Uses <omni::internalLock> as lock.  Signalled
  // on state change.
};


OMNI_NAMESPACE_END(omni)

#undef _core_attr

#endif  // __OMNIORB_BOA_H__
