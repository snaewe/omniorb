// -*- Mode: C++; -*-
//                            Package   : omniORB2
// omniServant.h              Created on: 22/2/99
//                            Author    : David Riddoch (djr)
//
//    Copyright (C) 1996, 1999 AT&T Research Cambridge
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
//    Base class for object implementations (servants).
//      
 
/*
  $Log$
  Revision 1.2.2.3  2001/04/18 17:50:43  sll
  Big checkin with the brand new internal APIs.
  Scoped where appropriate with the omni namespace.

  Revision 1.2.2.2  2000/09/27 17:12:47  sll
  New member _upcall().

  Revision 1.2.2.1  2000/07/17 10:35:35  sll
  Merged from omni3_develop the diff between omni3_0_0_pre3 and omni3_0_0.

  Revision 1.3  2000/07/13 15:26:05  dpg1
  Merge from omni3_develop for 3.0 release.

  Revision 1.1.2.2  1999/10/27 17:32:09  djr
  omni::internalLock and objref_rc_lock are now pointers.

  Revision 1.1.2.1  1999/09/24 09:51:48  djr
  Moved from omniORB2 + some new files.

*/

#ifndef __OMNISERVANT_H__
#define __OMNISERVANT_H__

class omniObjRef;
class omniIdentity;

//: Base class for all Servants.


class omniServant {
protected:
  inline omniServant() : pd_identities(0) {}
  inline omniServant(const omniServant&) : pd_identities(0) {}
  inline omniServant& operator = (const omniServant&) { return *this; }

public:
  virtual ~omniServant();

  virtual void* _ptrToInterface(const char* repoId);
  // The most derived class which overrides this virtual function will be
  // called to return a pointer to the base class object identified by
  // the repository id <repoId>. Thus for IDL interface foo, this method
  // should return a (_impl_foo*).  If this object does not support the
  // given interface, then it should return 0.  For the CORBA::Object
  // repoId this method should return non-zero, but the value will not be
  // used (it need not be a valid pointer).
  //  This default version returns non zero only for CORBA::Object, so
  // is suitable for DSI servants.
  //  <repoId> must not be null.
  //  This function must not throw any exceptions.
  //  This function must be thread-safe.

  virtual void* _downcast();
  // This is a hook which more derived servant types may use to obtain
  // a pointer to the more derived type.  The default version returns
  // zero.
  //  This is intended to be used by servant base classes
  // (eg PortableServer::ServantBase and omniOrbBoaServant), not by
  // skeletons.

  virtual const char* _mostDerivedRepoId();
  // The most derived skeleton type should return the repo id of
  // this servant.  This cannot be done for DSI servants, since
  // the _primary_interface() method can only be called in the
  // context of serving a CORBA request.  Thus this default
  // version returns "", indicating that it is not known.
  //  This function must not throw any exceptions.
  //  This function must be thread-safe.

  virtual _CORBA_Boolean _is_a(const char* logical_type_id);
  // May be overridden by more derived servants.  This default
  // version calls the _ptrToInterface() method and returns
  // true if this method returns nonzero.

  virtual _CORBA_Boolean _non_existent();
  // May be overridden by more derived servants.  This default
  // version returns 0.

  virtual omniObjRef* _do_get_interface() = 0;
  // Must be overridden by more derived servants to implement the
  // GIOP _get_interface operation.  The returned reference really
  // ought to be of type CORBA::InterfaceDef_ptr.

  virtual _CORBA_Boolean _dispatch(_OMNI_NS(IOP_S)&);
  // This is the point at which the ORB makes an upcall into the object
  // implementation when making method invocations from the wire.  This
  // function is called by the dispatcher of GIOP_S.  It should return
  // false if the operation is not implemented by this servant, true
  // otherwise.  The implementation of this function may throw system
  // exceptions, which will be passed back to the client side.
  //  For static servants this function is overriden by a dispatch function
  // in the skeleton implementation stub.  The version here implements the
  // standard CORBA::Object functions -- and is called if the most derived
  // version returns false.
  //  This function is thread-safe.  Overriding methods should also be
  // thread-safe if this servant is to be activated in a multi-threaded
  // object adapter.

  void _addIdentity(omniLocalIdentity*);
  // Add an identity to this servant's list.
  //  Must hold <omni::internalLock>.

  void _removeIdentity(omniLocalIdentity*);
  // Remove an identity from this servant's list.
  //  Must hold <omni::internalLock>.

  inline omniLocalIdentity* _identities() const {
    ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);
    return pd_identities;
  }
  // Must hold <omni::internalLock>.

protected:
  void _upcall(_OMNI_NS(IOP_S)&, omniCallDescriptor&);
  // Called by the dispatcher in the skeleton implementation stub to drive the 
  // upcall into the implementation code.

private:
  omniLocalIdentity* pd_identities;
  // A linked list of the object identities that this servant is
  // incarnating.  If it is not incarnating any objects, this will
  // be nil.  Note that when an object is deactivated the identity
  // will be removed from this list.  However, the identity may
  // still hold a reference to this servant until it is destroyed.
};

#endif  // __OMNISERVANT_H__
