// -*- Mode: C++; -*-
//                            Package   : omniORB
// inProcessIdentity.h        Created on: 16/05/2001
//                            Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2001 AT&T Laboratories Cambridge
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
//   Identity for objects in the caller's address space which cannot
//   be called directly. This can be because they are not activated
//   yet, because they are using DSI, or because they are in a
//   different language to the caller.

/*
 $Log$
 Revision 1.1.2.5  2001/09/19 17:26:46  dpg1
 Full clean-up after orb->destroy().

 Revision 1.1.2.4  2001/09/03 16:52:04  sll
 New signature for locateRequest. Now accept a calldescriptor argument.

 Revision 1.1.2.3  2001/08/22 13:31:31  dpg1
 Christof's fixes for gcc 3.0.1.

 Revision 1.1.2.2  2001/08/15 10:26:09  dpg1
 New object table behaviour, correct POA semantics.

 Revision 1.1.2.1  2001/05/29 17:03:49  dpg1
 In process identity.

*/

#ifndef __OMNIORB_INPROCESSIDENTITY_H__
#define __OMNIORB_INPROCESSIDENTITY_H__

#include <omniIdentity.h>

OMNI_NAMESPACE_BEGIN(omni)

class omniInProcessIdentity_RefHolder;

OMNI_NAMESPACE_END(omni)


class omniInProcessIdentity : public omniIdentity {
public:
  inline ~omniInProcessIdentity() {
    ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);
    if (--identity_count == 0)
      lastIdentityHasBeenDeleted();
  }

  inline omniInProcessIdentity(omniObjKey& key,
			       classCompare_fn compare = thisClassCompare)
    : omniIdentity(key, compare),
      pd_refCount(0)
    {
      ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);
      ++identity_count;
    }
  // May consume <key>.

  inline omniInProcessIdentity(const _CORBA_Octet* key, int keysize,
			       classCompare_fn compare = thisClassCompare)
    : omniIdentity(key, keysize, compare),
      pd_refCount(0)
    {
      ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);
      ++identity_count;
    }
  // Copies <key>.

  virtual void dispatch(omniCallDescriptor&);
  virtual void gainRef(omniObjRef* obj = 0);
  virtual void loseRef(omniObjRef* obj = 0);
  virtual omniIdentity::equivalent_fn get_real_is_equivalent() const;
  // Overrides omniIdentity.

  virtual void locateRequest(omniCallDescriptor&);
  // If this returns normally, then the object exists.
  // Throws OBJECT_NOT_EXIST, or omniORB::LOCATION_FORWARD
  // otherwise.
  // Caller must hold <internalLock>. On return or raised exception, the
  // lock is released.

  virtual _CORBA_Boolean inThisAddressSpace();
  // Override omniIdentity.

  static void* thisClassCompare(omniIdentity*, void*);

  static inline omniInProcessIdentity* downcast(omniIdentity* id)
  {
    return (omniInProcessIdentity*)(id->classCompare()
				    (id, (void*)thisClassCompare));
  }

private:
  friend class _OMNI_NS(omniInProcessIdentity_RefHolder);

  omniInProcessIdentity(const omniInProcessIdentity&);
  omniInProcessIdentity& operator = (const omniInProcessIdentity&);
  // Not implemented.

  int pd_refCount;

  static _CORBA_Boolean real_is_equivalent(const omniIdentity*,
					   const omniIdentity*);
};


#endif // __OMNIORB_INPROCESSIDENTITY_H__
