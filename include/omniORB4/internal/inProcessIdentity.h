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
  inline ~omniInProcessIdentity() {}

  inline omniInProcessIdentity(omniObjKey& key)
    : omniIdentity(key),
      pd_refCount(0)
    {}
  // May consume <key>.  Constructs an identity with ref count
  // of 1.  Initially has no implementation.

  inline omniInProcessIdentity(const _CORBA_Octet* key, int keysize)
    : omniIdentity(key, keysize)
    {}
  // Copies <key>.  Constructs an identity with ref count
  // of 1.  Initially has no implementation.

  virtual void dispatch(omniCallDescriptor&);
  virtual void gainObjRef(omniObjRef*);
  virtual void loseObjRef(omniObjRef*);
  virtual omniIdentity::equivalent_fn get_real_is_equivalent() const;
  // Overrides omniIdentity.

  virtual void locateRequest();
  // If this returns normally, then the object exists.
  // Throws OBJECT_NOT_EXIST, or omniORB::LOCATION_FORWARD
  // otherwise.
  // Caller must hold <internalLock>. On return or raised exception, the
  // lock is released.

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
