// -*- Mode: C++; -*-
//                            Package   : omniORB2
// localIdentity.h            Created on: 16/6/99
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
//    Encapsulation of the identity of a local object implementation.
//      
 
/*
  $Log$
  Revision 1.1.4.1  2001/04/18 17:18:16  sll
  Big checkin with the brand new internal APIs.
  These files were relocated and scoped with the omni namespace.

  Revision 1.2.2.1  2000/07/17 10:35:55  sll
  Merged from omni3_develop the diff between omni3_0_0_pre3 and omni3_0_0.

  Revision 1.3  2000/07/13 15:25:57  dpg1
  Merge from omni3_develop for 3.0 release.

  Revision 1.1.2.3  2000/06/02 16:09:59  dpg1
  If an object is deactivated while its POA is in the HOLDING state,
  clients which were held now receive a TRANSIENT exception when the POA
  becomes active again.

  Revision 1.1.2.2  1999/10/27 17:32:12  djr
  omni::internalLock and objref_rc_lock are now pointers.

  Revision 1.1.2.1  1999/09/22 14:26:53  djr
  Major rewrite of orbcore to support POA.

*/

#ifndef __OMNIORB_LOCALIDENTITY_H__
#define __OMNIORB_LOCALIDENTITY_H__

#include <omniIdentity.h>


class omniServant;
class omniObjRef;

OMNI_NAMESPACE_BEGIN(omni)
class omniObjAdapter;
class omniLocalIdentity_RefHolder;
OMNI_NAMESPACE_END(omni)

class omniLocalIdentity : public omniIdentity {
public:
  inline ~omniLocalIdentity() {}

  inline omniLocalIdentity(omniObjKey& key)
    : omniIdentity(key),
      pd_nInvocations(1),
      pd_servant(0),
      pd_adapter(0),
      pd_servantsNextIdentity(0),
      pd_nextInObjectTable(0),
      pd_localRefs(0),
      pd_nextInOAObjList(0),
      pd_prevInOAObjList(0)
    {}
  // May consume <key>.  Constructs an identity with ref count
  // of 1.  Initially has no implementation.

  inline omniLocalIdentity(const _CORBA_Octet* key, int keysize)
    : omniIdentity(key, keysize),
      pd_nInvocations(1),
      pd_servant(0),
      pd_adapter(0),
      pd_servantsNextIdentity(0),
      pd_nextInObjectTable(0),
      pd_localRefs(0),
      pd_nextInOAObjList(0),
      pd_prevInOAObjList(0)
    {}
  // Copies <key>.  Constructs an identity with ref count
  // of 1.  Initially has no implementation.

  virtual void dispatch(omniCallDescriptor&);
  virtual void gainObjRef(omniObjRef*);
  virtual void loseObjRef(omniObjRef*);
  virtual void locateRequest();
  virtual omniIdentity::equivalent_fn get_real_is_equivalent() const;
  // Override omniIdentity.

  void dispatch(_OMNI_NS(IOP_S)&);
  // Dispatches a remote invocation.  Grabs a reference
  // to this identity, and dispatches the call to the
  // object adapter (releasing the reference before
  // returning).
  //  Must hold <omni::internalLock on entry.  It is not held
  // on exit.

  void finishedWithDummyId();
  // Called when the ORB no longer needs a dummy id, because
  // there are no more local references to it. This must really
  // be a dummy id!
  //  Must hold <omni::internalLock>.

  inline void deactivate() {
    ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);
    OMNIORB_ASSERT(pd_nInvocations > 0);
    --pd_nInvocations;
  }
  // This should only be called after doing omni::deactivateObject().
  //  Must hold <omni::internalLock>.

  void die();
  // May be called after the object has been deactivated (above),
  // and all outstanding requests have completed.
  // Frees any resources it was using, and delete's itself.
  //  Must not hold <omni::internalLock>.

  inline omniServant*       servant() const  { return pd_servant;       }
  inline _OMNI_NS(omniObjAdapter)* adapter() const  { return pd_adapter; }
  inline int                is_idle() const  { return !pd_nInvocations; }
  // For each of the above the ownership of the returned value
  // is the responsibility of this object.  No reference counts
  // are incremented.

  inline omniObjRef* localRefList() const {
    ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);
    return pd_localRefs;
  }

  inline omniLocalIdentity* nextInObjectTable() {
    ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);
    return pd_nextInObjectTable;
  }
  inline omniLocalIdentity** addrOfNextInObjectTable() {
    ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);
    return &pd_nextInObjectTable;
  }

  inline omniLocalIdentity* servantsNextIdentity() {
    ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);
    return pd_servantsNextIdentity;
  }
  inline omniLocalIdentity** addrOfServantsNextIdentity() {
    ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);
    return &pd_servantsNextIdentity;
  }

  inline omniLocalIdentity* nextInOAObjList() { return pd_nextInOAObjList; }
  inline void insertIntoOAObjList(omniLocalIdentity** p_head) {
    OMNIORB_ASSERT(!pd_nextInOAObjList && !pd_prevInOAObjList);
    OMNIORB_ASSERT(p_head);
    pd_nextInOAObjList = *p_head;
    pd_prevInOAObjList = p_head;
    *p_head = this;
    if( pd_nextInOAObjList )
      pd_nextInOAObjList->pd_prevInOAObjList = &pd_nextInOAObjList;
  }
  inline void removeFromOAObjList() {
    OMNIORB_ASSERT(pd_prevInOAObjList);
    *pd_prevInOAObjList = pd_nextInOAObjList;
    if( pd_nextInOAObjList )
      pd_nextInOAObjList->pd_prevInOAObjList = pd_prevInOAObjList;
    pd_nextInOAObjList = 0;
    pd_prevInOAObjList = 0;
  }
  inline void reRootOAObjList(omniLocalIdentity** new_head) {
    OMNIORB_ASSERT(pd_prevInOAObjList);  OMNIORB_ASSERT(new_head);
    *pd_prevInOAObjList = 0;
    pd_prevInOAObjList = new_head;
    *new_head = this;
  }
  inline CORBA::Boolean deactivated() {
    return pd_adapter && !pd_prevInOAObjList;
  }
  // Locking for these methods is the responsiblility of
  // the object adapter which owns this list.

  inline void setServant(omniServant* servant, _OMNI_NS(omniObjAdapter)* oa) {
    OMNIORB_ASSERT(!pd_servant);  OMNIORB_ASSERT(servant);
    OMNIORB_ASSERT(!pd_adapter && oa);
    pd_servant = servant;
    pd_adapter = oa;
  }

private:
  friend class _OMNI_NS(omniLocalIdentity_RefHolder);

  omniLocalIdentity(const omniLocalIdentity&);
  omniLocalIdentity& operator = (const omniLocalIdentity&);
  // Not implemented.


  int                    pd_nInvocations;
  // This count gives the number of method calls in progress
  // on this object.  When it goes to zero, we check to see
  // if anyone is interested in such an event.
  //  To prevent this overhead when we are not interested in
  // invocations being finished, we add one to this value, so
  // that it never goes to zero.  The initial value is 1 for
  // this reason.  deactivate() decrements this value, so that
  // the adapter will be told when there are no invocations.

  omniServant*           pd_servant;
  // Nil if this object is not yet incarnated, but once set
  // is immutable.
  //  The object adapter is responsible for managing the
  // etherealisation of the servant.

  _OMNI_NS(omniObjAdapter)*        pd_adapter;
  // Nil if this object is not yet incarnated, but once set
  // is immutable.  We cannot have a pointer to the adapter
  // before the object is incarnated, since the adapter itself
  // may not yet exist either.
  //  We do not hold a reference to this adapter, since it will
  // (must!) outlive this object.

  omniLocalIdentity*     pd_servantsNextIdentity;
  // Linked list of a particular servant's identities.  An
  // identity is only in this list whilst it is in the
  // active object map.

  omniLocalIdentity*     pd_nextInObjectTable;
  // Local object table is a hash table with chaining.  This
  // is the linked list of objects which hash to the same
  // entry.

  omniObjRef*            pd_localRefs;
  // Pointer to a linked list of local references to this object.
  // Protected by <omni::internalLock>.

  omniLocalIdentity*     pd_nextInOAObjList;
  omniLocalIdentity**    pd_prevInOAObjList;
  // Doubly linked list of all objects active in this adapter.
  // <pd_nextInOAObjList> is zero for last entry in list.  If
  // <pd_prevInOAObjList> is zero, then we are not in any list.
  //  This needs to be doubly linked for fast removal -- since
  // it is likely to be a long list.
  //  Protected by the adapter which owns this object.

  static _CORBA_Boolean real_is_equivalent(const omniIdentity*,
					   const omniIdentity*);
};


#endif  // __OMNIORB_LOCALIDENTITY_H__
