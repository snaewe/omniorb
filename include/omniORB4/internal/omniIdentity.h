// -*- Mode: C++; -*-
//                            Package   : omniORB2
// omniIdentity.h             Created on: 22/2/99
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
//    Base class for the identity of an object implementation (which
//    may be local or remote).
//

/*
  $Log$
  Revision 1.1.6.3  2007/04/14 17:56:52  dgrisby
  Identity downcasting mechanism was broken by VC++ 8's
  over-enthusiastic optimiser.

  Revision 1.1.6.2  2006/09/17 23:21:49  dgrisby
  Properly import identity_count for Windows DLL hell.

  Revision 1.1.6.1  2003/03/23 21:03:44  dgrisby
  Start of omniORB 4.1.x development branch.

  Revision 1.1.4.5  2001/09/19 17:26:46  dpg1
  Full clean-up after orb->destroy().

  Revision 1.1.4.4  2001/09/03 16:52:05  sll
  New signature for locateRequest. Now accept a calldescriptor argument.

  Revision 1.1.4.3  2001/08/15 10:26:09  dpg1
  New object table behaviour, correct POA semantics.

  Revision 1.1.4.2  2001/06/13 20:11:37  sll
  Minor update to make the ORB compiles with MSVC++.

  Revision 1.1.4.1  2001/04/18 17:18:15  sll
  Big checkin with the brand new internal APIs.
  These files were relocated and scoped with the omni namespace.

  Revision 1.2.2.3  2000/11/03 19:08:52  sll
  Use virtual dtor with gcc.

  Revision 1.2.2.2  2000/09/27 17:39:28  sll
  Updated include/omniORB3 to include/omniORB4

  Revision 1.2.2.1  2000/07/17 10:35:40  sll
  Merged from omni3_develop the diff between omni3_0_0_pre3 and omni3_0_0.

  Revision 1.3  2000/07/13 15:26:03  dpg1
  Merge from omni3_develop for 3.0 release.

  Revision 1.1.2.1  1999/09/22 14:26:26  djr
  Major rewrite of orbcore to support POA.

*/

#ifndef __OMNIIDENTITY_H__
#define __OMNIIDENTITY_H__

#ifdef _core_attr
# error "A local CPP macro _core_attr has already been defined."
#endif

#if defined(_OMNIORB_LIBRARY)
#     define _core_attr
#else
#     define _core_attr _OMNIORB_NTDLL_IMPORT
#endif

class omniCallDescriptor;
class omniObjRef;


class omniIdentity {
public:
  inline const _CORBA_Octet* key() const  { return pd_key.key();    }
  inline int keysize() const              { return pd_key.size();   }
  //  These functions are thread-safe.

  inline int is_equal(const _CORBA_Octet* key, int keysize) const {
    return pd_key.is_equal(key, keysize);
  }

  virtual void dispatch(omniCallDescriptor&) = 0;
  // Dispatch a call from an object reference.  Grabs a reference
  // to this identity, which it holds until the call returns.
  //  Must hold <omni::internalLock> on entry.  It is not held
  //  on exit.

  virtual void gainRef(omniObjRef* obj = 0) = 0;
  virtual void loseRef(omniObjRef* obj = 0) = 0;
  // Reference counting for identity objects. An identity may wish to
  // keep track of objrefs referring to it, so when objrefs gain/lose
  // a ref to an identity, the objref pointer is passed as an
  // argument. Other entities holding references to identities use the
  // default zero objref.
  //  Must hold <omni::internalLock>.

  virtual void locateRequest(omniCallDescriptor&) = 0;
  // If this returns normally, then the object exists.
  // Throws OBJECT_NOT_EXIST, or omniORB::LOCATION_FORWARD
  // otherwise.
  // Caller must hold <omni::internalLock>. On return or raised
  // exception, the lock is released.

  inline _CORBA_Boolean is_equivalent(const omniIdentity* id) {
    // Returns TRUE(1) if the two identity objects refer to the same CORBA
    // Object. This function does not raise any exceptions.

    // We rely on the concrete implementation of this abstract class to
    // supply its equivalent function. Only call the equivalent function
    // if this and the argument identity object are of the same derived
    // type.
    if (id->get_real_is_equivalent() == get_real_is_equivalent()) {
      return (get_real_is_equivalent())(this,id);
    }
    else
      return 0;
  }

  virtual _CORBA_Boolean inThisAddressSpace() = 0;
  // Return true if the identity represents an object in this address
  // space.


  static void waitForLastIdentity();
  // Block until all traced omniIdentity objects have been deleted.
  //  Must not hold <omni::internalLock>


  virtual void* ptrToClass(int* cptr);
  static inline omniIdentity* downcast(omniIdentity* i) {
    return (omniIdentity*)i->ptrToClass(&_classid);
  }
  static _core_attr int _classid;
  // Dynamic casting mechanism.

protected:
#ifndef __GNUG__
  inline
#else
  virtual
#endif
  ~omniIdentity() { }
  // Should only be destroyed by implementation of derived classes.
  // This doesn't need to be virtual, since it is only ever deleted by
  // the most derived type.  But gcc is rather anal and insists that a
  // class with virtual functions must have a virtual dtor.

  inline omniIdentity(omniObjKey& key)
    : pd_key(key, 1) {}
  // May consume <key> (if it is bigger than inline key buffer).

  inline omniIdentity(const _CORBA_Octet* key, int keysize)
    : pd_key(key, keysize) {}
  // Copies <key>.

  inline omniIdentity(_CORBA_Octet* key, int keysize)
    : pd_key(key, keysize) {}
  // Consumes <key>.

  inline omniIdentity() {}
  // No key. Used by dummy shutdown identity.


  static _core_attr int identity_count;
  // Count of active identity objects. When this goes to zero, all
  // outgoing invocations have completed.

  static void lastIdentityHasBeenDeleted();

public:
  typedef _CORBA_Boolean (*equivalent_fn)(const omniIdentity*,
					  const omniIdentity*);

protected:
  virtual equivalent_fn get_real_is_equivalent() const = 0;
  // return a pointer to the function that can compute whether 2 identity
  // objects of the same derived class are equivalent.

private:
  omniIdentity(const omniIdentity&);
  omniIdentity& operator = (const omniIdentity&);
  // Not implemented.

  omniObjKey pd_key;
  // Immutable.
};

#undef _core_attr

#endif  // __OMNIIDENTITY_H__
