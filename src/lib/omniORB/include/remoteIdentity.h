// -*- Mode: C++; -*-
//                            Package   : omniORB2
// remoteIdentity.h           Created on: 16/6/99
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
//    Encapsulation of the identity of a remote object implementation.
//      
 
/*
  $Log$
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

OMNI_NAMESPACE_BEGIN(omni)

class omniRemoteIdentity_RefHolder;

OMNI_NAMESPACE_END(omni)

class omniRemoteIdentity : public omniIdentity {
public:
  inline omniRemoteIdentity(omniIOR* ior, 
			    const CORBA::Octet* key,
			    CORBA::ULong keysize,
			    _OMNI_NS(Rope)* rope)
    : omniIdentity(key,keysize),
      pd_refCount(0),
      pd_ior(ior),
      pd_rope(rope)
    {}
  // Consumes <ior> and <rope>.  Copies <key>.  Constructs an identity
  // with ref count of 0.

  virtual void dispatch(omniCallDescriptor&);
  virtual void gainObjRef(omniObjRef*);
  virtual void loseObjRef(omniObjRef*);
  virtual omniIdentity::equivalent_fn get_real_is_equivalent() const;
  // Overrides omniIdentity.

  inline _OMNI_NS(Rope)* rope() const { return pd_rope; }
  // This function is thread-safe.  Does not increment
  // the reference count of the rope.

  virtual void locateRequest();
  // If this returns normally, then the object exists.
  // Throws OBJECT_NOT_EXIST, or omniORB::LOCATION_FORWARD
  // otherwise.
  // Caller must hold <internalLock>. On return or raised exception, the
  // lock is released.



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

#endif  // __OMNIORB_REMOTEIDENTITY_H__
