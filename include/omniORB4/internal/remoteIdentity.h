// -*- Mode: C++; -*-
//                            Package   : omniORB2
// remoteIdentity.h           Created on: 16/6/99
//                            Author    : David Riddoch (djr)
//
//    Copyright (C) 2002-2007 Apasphere Ltd
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
//    Encapsulation of the identity of a remote object implementation.
//      
 
/*
  $Log$
  Revision 1.1.6.3  2009/05/06 16:16:01  dgrisby
  Update lots of copyright notices.

  Revision 1.1.6.2  2007/04/14 17:56:52  dgrisby
  Identity downcasting mechanism was broken by VC++ 8's
  over-enthusiastic optimiser.

  Revision 1.1.6.1  2003/03/23 21:03:42  dgrisby
  Start of omniORB 4.1.x development branch.

  Revision 1.1.4.6  2002/11/29 14:03:41  dgrisby
  Rearrange declarations to make Code Warrior happy.

  Revision 1.1.4.5  2001/09/19 17:26:47  dpg1
  Full clean-up after orb->destroy().

  Revision 1.1.4.4  2001/09/03 16:52:05  sll
  New signature for locateRequest. Now accept a calldescriptor argument.

  Revision 1.1.4.3  2001/08/22 13:31:31  dpg1
  Christof's fixes for gcc 3.0.1.

  Revision 1.1.4.2  2001/08/15 10:26:10  dpg1
  New object table behaviour, correct POA semantics.

  Revision 1.1.4.1  2001/04/18 17:18:14  sll
  Big checkin with the brand new internal APIs.
  These files were relocated and scoped with the omni namespace.

  Revision 1.2.2.3  2000/10/03 17:41:45  sll
  Make sure object key is copied in ctor.

  Revision 1.2.2.2  2000/09/27 17:46:29  sll
  New data member pd_ior and the new ctor signature to match.

  Revision 1.2.2.1  2000/07/17 10:35:40  sll
  Merged from omni3_develop the diff between omni3_0_0_pre3 and omni3_0_0.

  Revision 1.3  2000/07/13 15:26:03  dpg1
  Merge from omni3_develop for 3.0 release.

  Revision 1.1.2.1  1999/09/22 14:26:26  djr
  Major rewrite of orbcore to support POA.

*/

#ifndef __OMNIORB_REMOTEIDENTITY_H__
#define __OMNIORB_REMOTEIDENTITY_H__

#include <omniIdentity.h>

#ifdef _core_attr
# error "A local CPP macro _core_attr has already been defined."
#endif

#if defined(_OMNIORB_LIBRARY)
#     define _core_attr
#else
#     define _core_attr _OMNIORB_NTDLL_IMPORT
#endif

OMNI_NAMESPACE_BEGIN(omni)

class omniRemoteIdentity_RefHolder;

OMNI_NAMESPACE_END(omni)

class omniRemoteIdentity : public omniIdentity {
public:
  inline omniRemoteIdentity(omniIOR* ior, 
			    const CORBA::Octet* key,
			    CORBA::ULong keysize,
			    _OMNI_NS(Rope)* rope)
    : omniIdentity(key, keysize),
      pd_refCount(0),
      pd_ior(ior),
      pd_rope(rope)
    {
      ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);
      ++identity_count;
    }
  // Consumes <ior> and <rope>.  Copies <key>.  Constructs an identity
  // with ref count of 0.

  virtual void dispatch(omniCallDescriptor&);
  virtual void gainRef(omniObjRef* obj = 0);
  virtual void loseRef(omniObjRef* obj = 0);
  virtual omniIdentity::equivalent_fn get_real_is_equivalent() const;
  // Overrides omniIdentity.

  inline _OMNI_NS(Rope)* rope() const { return pd_rope; }
  // This function is thread-safe.  Does not increment
  // the reference count of the rope.

  virtual void locateRequest(omniCallDescriptor&);
  // If this returns normally, then the object exists.
  // Throws OBJECT_NOT_EXIST, or omniORB::LOCATION_FORWARD
  // otherwise.
  // Caller must hold <internalLock>. On return or raised exception, the
  // lock is released.

  virtual _CORBA_Boolean inThisAddressSpace();
  // Override omniIdentity.


  virtual void* ptrToClass(int* cptr);
  static inline omniRemoteIdentity* downcast(omniIdentity* i) {
    return (omniRemoteIdentity*)i->ptrToClass(&_classid);
  }
  static _core_attr int _classid;
  // Dynamic casting mechanism.

private:
  friend class _OMNI_NS(omniRemoteIdentity_RefHolder);

  ~omniRemoteIdentity();

  omniRemoteIdentity(const omniRemoteIdentity&);
  omniRemoteIdentity& operator = (const omniRemoteIdentity&);
  // Not implemented.


  int              pd_refCount;
  omniIOR*         pd_ior;
  _OMNI_NS(Rope)*  pd_rope;
  // Immutable.

  static _CORBA_Boolean real_is_equivalent(const omniIdentity*,
					   const omniIdentity*);

};

#undef _core_attr

#endif  // __OMNIORB_REMOTEIDENTITY_H__
