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
  Revision 1.1.4.5  2001/08/22 13:31:31  dpg1
  Christof's fixes for gcc 3.0.1.

  Revision 1.1.4.4  2001/08/15 10:26:09  dpg1
  New object table behaviour, correct POA semantics.

  Revision 1.1.4.3  2001/06/13 20:11:37  sll
  Minor update to make the ORB compiles with MSVC++.

  Revision 1.1.4.2  2001/05/29 17:03:49  dpg1
  In process identity.

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
class omniCallHandle;

OMNI_NAMESPACE_BEGIN(omni)
class omniObjAdapter;
class omniLocalIdentity_RefHolder;
OMNI_NAMESPACE_END(omni)

class omniLocalIdentity : public omniIdentity {
public:
  inline ~omniLocalIdentity() {}

  inline omniLocalIdentity(omniObjKey& key,
			   omniServant* servant,
			   _OMNI_NS(omniObjAdapter)* adapter,
			   classCompare_fn compare = thisClassCompare)

    : omniIdentity(key, compare),
      pd_nInvocations(1),
      pd_servant(servant),
      pd_adapter(adapter),
      pd_deactivated(0)
    {}
  // May consume <key> (if it is bigger than inline key buffer).
  // Constructs an identity with ref count of 1.

  inline omniLocalIdentity(const _CORBA_Octet* key, int keysize,
			   omniServant* servant,
			   _OMNI_NS(omniObjAdapter)* adapter,
			   classCompare_fn compare = thisClassCompare)

    : omniIdentity(key, keysize, compare),
      pd_nInvocations(1),
      pd_servant(servant),
      pd_adapter(adapter),
      pd_deactivated(0)
    {}
  // Copies <key>.  Constructs an identity with ref count
  // of 1.

  virtual void dispatch(omniCallDescriptor&);
  virtual void gainRef(omniObjRef* obj = 0);
  virtual void loseRef(omniObjRef* obj = 0);
  virtual void locateRequest();
protected:
  virtual omniIdentity::equivalent_fn get_real_is_equivalent() const;
  // Override omniIdentity.

public:
  void dispatch(omniCallHandle&);
  // Dispatches a remote invocation.  Grabs a reference
  // to this identity, and dispatches the call to the
  // object adapter (releasing the reference before
  // returning).
  //  Must hold <omni::internalLock> on entry.  It is not held
  // on exit.

  inline omniServant*              servant() const { return pd_servant; }
  inline _OMNI_NS(omniObjAdapter)* adapter() const { return pd_adapter; }
  inline int                       is_idle() const { return !pd_nInvocations;}
  inline _CORBA_Boolean        deactivated() const { return pd_deactivated; }
  // For each of the above the ownership of the returned value
  // is the responsibility of this object.  No reference counts
  // are incremented.

  inline void setServant(omniServant* servant, _OMNI_NS(omniObjAdapter)* oa) {
    OMNIORB_ASSERT(!pd_servant);  OMNIORB_ASSERT(servant);
    OMNIORB_ASSERT(!pd_adapter && oa);
    pd_servant = servant;
    pd_adapter = oa;
  }

  virtual _CORBA_Boolean inThisAddressSpace();
  // Override omniIdentity.

  static void* thisClassCompare(omniIdentity*, void*);

  static inline omniLocalIdentity* downcast(omniIdentity* id)
  {
    return (omniLocalIdentity*)(id->classCompare()
				(id, (void*)thisClassCompare));
  }

protected:
  int pd_nInvocations;
  // This count gives the number of method calls in progress
  // on this object.  When it goes to zero, we check to see
  // if anyone is interested in such an event.
  //  To prevent this overhead when we are not interested in
  // invocations being finished, we add one to this value, so
  // that it never goes to zero.  The initial value is 1 for
  // this reason.  deactivate() decrements this value, so that
  // the adapter will be told when there are no invocations.

  omniServant* pd_servant;
  // Nil if this object is not yet incarnated, but once set
  // is immutable.
  //  The object adapter is responsible for managing the
  // etherealisation of the servant.

  _OMNI_NS(omniObjAdapter)* pd_adapter;
  // Nil if this object is not yet incarnated, but once set
  // is immutable.  We cannot have a pointer to the adapter
  // before the object is incarnated, since the adapter itself
  // may not yet exist either.
  //  We do not hold a reference to this adapter, since it will
  // (must!) outlive this object.

  static _CORBA_Boolean real_is_equivalent(const omniIdentity*,
					   const omniIdentity*);

  _CORBA_Boolean pd_deactivated;
  // True if this localIdentity is no longer active, and cannot be
  // used for invocations.

private:
  friend class _OMNI_NS(omniLocalIdentity_RefHolder);

  omniLocalIdentity(const omniLocalIdentity&);
  omniLocalIdentity& operator = (const omniLocalIdentity&);
  // Not implemented.
};


#endif  // __OMNIORB_LOCALIDENTITY_H__
