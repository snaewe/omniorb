// -*- Mode: C++; -*-
//                            Package   : omniORB2
// objectTable.h              Created on: 2001/08/03
//                            Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2001 AT&T Research Cambridge
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
//    Definitions for object table entries
//

/*
  $Log$
  Revision 1.1.4.2  2005/01/06 23:08:25  dgrisby
  Big merge from omni4_0_develop.

  Revision 1.1.4.1  2003/03/23 21:03:44  dgrisby
  Start of omniORB 4.1.x development branch.

  Revision 1.1.2.3  2003/01/16 11:08:26  dgrisby
  Patches to support Digital Mars C++. Thanks Christof Meerwald.

  Revision 1.1.2.2  2001/08/22 13:31:31  dpg1
  Christof's fixes for gcc 3.0.1.

  Revision 1.1.2.1  2001/08/15 10:26:09  dpg1
  New object table behaviour, correct POA semantics.

*/

#ifndef __OMNIORB_OBJECTTABLE_H__
#define __OMNIORB_OBJECTTABLE_H__

#include <omniORB4/omniutilities.h>
#include <localIdentity.h>

class omniObjTable;

class omniObjTableEntry : public omniLocalIdentity {
public:
  enum State {
    ACTIVATING    = 1,
    // object is on its way to being active, but is not there yet.

    ACTIVE        = 2,
    // object is active and can be invoked upon.

    DEACTIVATING  = 4,
    // the application has asked for the object to be deactivated, and
    // we are waiting for outstanding invocations to complete. New
    // incoming calls are still dispatched in this state.

    DEACTIVATING_OA = 20,
    // the object is being deactivated because the object adapter is
    // being destroyed. At the time of OA destruction, the object was
    // idle.  However, as in DEACTIVATING, new incoming calls are
    // still dispatched. The difference is that when the last call
    // finishes, the thread doing the call does not continue with
    // etherealisation, since the OA destruction thread will do it.

    ETHEREALISING = 8,
    // all invocations have completed, and the object is ready to be
    // etherealised. When the object leaves this state, the
    // omniLocalIdentity is removed from the object table and deleted.

    DEAD          = 0,
    // Pseudo state to indicate the object table entry no longer
    // exists.

    ALL_STATES    = 15
  };

  static void* thisClassCompare(omniIdentity*, void*);

  ~omniObjTableEntry();

  inline omniObjTableEntry(omniObjKey& key,
			   classCompare_fn compare = thisClassCompare)
    : omniLocalIdentity(key, 0, 0, compare),
      pd_state(ACTIVATING),
      pd_nextInObjectTable(0),
      pd_nextInOAObjList(0),
      pd_prevInOAObjList(0),
      pd_cond(0),
      pd_waiters(0),
      pd_refCount(1)
  {}
  // May consume <key>.  Constructs an identity with ref count
  // of 1.

  inline omniObjTableEntry(const _CORBA_Octet* key, int keysize,
			   classCompare_fn compare = thisClassCompare)
    : omniLocalIdentity(key, keysize, 0, 0, compare),
      pd_state(ACTIVATING),
      pd_nextInObjectTable(0),
      pd_nextInOAObjList(0),
      pd_prevInOAObjList(0),
      pd_cond(0),
      pd_waiters(0),
      pd_refCount(1)
  {}
  // Copies <key>.  Constructs an identity with ref count
  // of 1.

  //
  // State changes
  // Each function changes to the requested state. The current state
  // must be the previous state in the order.
  //  Must hold <omni::internalLock>

  void setActive(omniServant* servant, _OMNI_NS(omniObjAdapter)* adapter);

  void setDeactivating();
  // Deactivating due to a deactivate_object call.

  void setDeactivatingOA();
  // Deactivating due to object adapter destruction. Sets state to
  // DEACTIVATING_OA or DEACTIVATING depending on whether object is
  // idle.

  void setEtherealising();
  // Remove from the servant's list of activations

  void setDead();
  // Remove from the object table. Can be called from any state except
  // DEAD.

  inline State state() { return pd_state; }

  State wait(_CORBA_ULong set);
  // Wait until the state matches one of the states in <set>, or
  // becomes DEAD.
  //  Must hold <omni::internalLock>.


  virtual void gainRef(omniObjRef* obj = 0);
  virtual void loseRef(omniObjRef* obj = 0);
  // Overrides omniIdentity.

  //
  // Linked lists for object table and POA AOM.

  inline omniObjTableEntry* nextInObjectTable() {
    ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);
    return pd_nextInObjectTable;
  }
  inline omniObjTableEntry** addrOfNextInObjectTable() {
    ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);
    return &pd_nextInObjectTable;
  }

  inline omniObjTableEntry* nextInOAObjList() { return pd_nextInOAObjList; }
  inline void insertIntoOAObjList(omniObjTableEntry** p_head) {
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
  inline void reRootOAObjList(omniObjTableEntry** new_head) {
    OMNIORB_ASSERT(pd_prevInOAObjList);  OMNIORB_ASSERT(new_head);
    *pd_prevInOAObjList = 0;
    pd_prevInOAObjList = new_head;
    *new_head = this;
  }
  // Locking for these methods is the responsiblility of
  // the object adapter which owns this list.


  inline omnivector<omniObjRef*>& objRefs() {
    ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);
    return pd_objRefs;
  }
  // List of objrefs with references to this id.
  //  Must hold <omni::internalLock>


  static inline omniObjTableEntry* downcast(omniIdentity* id)
  {
    return (omniObjTableEntry*)(id->classCompare()
				(id, (void*)thisClassCompare));
  }

private:
  State pd_state;
  // State of this entry.
  //  Protected by <omni::internalLock>.

  omniObjTableEntry* pd_nextInObjectTable;
  // Object table is a hash table with chaining.  This is the linked
  // list of objects which hash to the same entry.

  omniObjTableEntry*  pd_nextInOAObjList;
  omniObjTableEntry** pd_prevInOAObjList;
  // Doubly linked list of all objects active in this adapter.
  // <pd_nextInOAObjList> is zero for last entry in list.  If
  // <pd_prevInOAObjList> is zero, then we are not in any list.
  // This needs to be doubly linked for fast removal -- since
  // it is likely to be a long list.
  //  Protected by the adapter which owns this object.

  omnivector<omniObjRef*> pd_objRefs;
  // List of objrefs holding a reference to this object table entry.
  // Used to avoid the overhead of creating new objrefs for common
  // operations like Servant::_this() and POA::*_to_reference().
  //  Protected by <omni::internalLock>.

  omni_tracedcondition* pd_cond;
  int                   pd_waiters;
  // Condition variable attached to <omni::internalLock> and count of
  // waiting threads. The condition variable is only constructed on
  // the first call to wait().

  int pd_refCount;
  //  Protected by <omni::internalLock>.

  friend class omniObjTable;
};

class omniObjTable {
public:
  // Static functions to locate and manipulate object table entries

  static omniObjTableEntry* locateActive(const _CORBA_Octet* key, int keysize,
					 _CORBA_ULong hash,
					 _CORBA_Boolean wait);
  // Search the object table for the given key. If an entry in the
  // ACTIVE or DEACTIVATING state is found, return it. If an entry in
  // the ACTIVATING state is found, and <wait> is true, wait until it
  // enters the ACTIVE state, then return it. In all other cases,
  // return zero.
  //  Must hold <omni::internalLock>.
  //  Does not throw any exceptions.

  static omniObjTableEntry*
  locate(const _CORBA_Octet* key, int keysize, _CORBA_ULong hash,
	 _CORBA_ULong set = omniObjTableEntry::ALL_STATES);
  // Search the object table for the given key. <set> is a mask
  // containing a set of states. If there is an entry in the object
  // table, but its state is not in the set, blocks until the state
  // changes to one in the set, or the entry is removed from the
  // table. Returns an entry in one of the required states, or zero if
  // there is (now) no entry for the key.
  //  Must hold <omni::internalLock>.
  //  Does not throw any exceptions.

  static omniObjTableEntry* newEntry(omniObjKey& key);
  static omniObjTableEntry* newEntry(omniObjKey& key, _CORBA_ULong hashv);
  // Create a new entry in the object table with the given key, in the
  // ACTIVATING state. Returns zero if there is already an entry with
  // the key. May consume <key> (if it is bigger than inline key
  // buffer).
  //  Must hold <omni::internalLock>.
  //  Does not throw any exceptions.


  static void resize();
  // Resize the object table if necessary. Does nothing if the table
  // does not need resizing.
  //  Must hold <omni::internalLock>.
};


#endif // __OMNIORB_OBJECTTABLE_H__
