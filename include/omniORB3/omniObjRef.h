// -*- Mode: C++; -*-
//                            Package   : omniORB
// omniObjRef.h               Created on: 22/2/99
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
//    Base class for object references.
//

/*
  $Log$
  Revision 1.1.2.2  1999/10/27 17:32:09  djr
  omni::internalLock and objref_rc_lock are now pointers.

  Revision 1.1.2.1  1999/09/24 09:51:48  djr
  Moved from omniORB2 + some new files.

*/

#ifndef __OMNIOBJREF_H__
#define __OMNIOBJREF_H__


class omni;
class omniInternal;
class omniObjKey;
class omniIdentity;
class omniLocalIdentity;
class omniRemoteIdentity;
class omniCallDescriptor;


class omniObjRef {
public:
  virtual void* _ptrToObjRef(const char* repoId) = 0;
  // The most derived class which overrides this virtual function will be
  // called to return a pointer to the object reference class identified by
  // <repoId>.  So for IDL type Echo, the returned pointer will be an
  // Echo_ptr.  All object reference types should support widening to
  // CORBA::Object_ptr.
  //  If this reference object cannot be widened to the class specified, a
  // null pointer will be returned.  No attempt is made to contact the object
  // or create a new object refence of a more derived type.  See _realNarrow.
  //  <repoId> must not be NULL.
  //  This function does not throw any exceptions.
  //  This function is thread-safe.

  int _getRopeAndKey(omniRopeAndKey& rak, _CORBA_Boolean* is_local=0) const;
  // Get the current value of the rope and key, and whether
  // the object is local or not.  <is_local> may be null, in
  // which case that value is not written.  Returns true if
  // this reference is a forwarded-location.
  //  This function is thread-safe.

  void _getTheKey(omniObjKey& key, int locked=0) const;
  // Get the current value of the key.
  //  This function is thread-safe.

  inline const char* _mostDerivedRepoId() const {
    return pd_mostDerivedRepoId;
  }
  // The interface repository id of the most derived interface
  // supported by the object.  May be the empty string '\0'.
  //  This function is thread-safe.

  inline IOP::TaggedProfileList* _iopProfiles() const {
    return pd_iopprofiles;
  }
  // This object retains the memory returned by this method.
  // The returned profiles must not be modified.  The returned
  // value is non-const for nasty dirty-hack type reasons!
  //  This function is thread-safe.

  _CORBA_Boolean _real_is_a(const char* repoId);
  // Returns 1 if the object is really an instance of the type identified
  // by the IR repository ID <repoId>.  If the stub for the object type is
  // linked into the executable, the ORB can fully determine the _is_a
  // relation without the need to contact the object remotely.  Otherwise,
  // contact the object implementation using _remote_is_a().
  //  This function is thread-safe.

  void* _realNarrow(const char* repoId);
  // If the actual type of the object can be widened to the requested
  // interface type identified by the IR repository ID <repoId>, return
  // a valid object reference.  Otherwise, return 0.  The return value is
  // of type void* and can be casted to the T_ptr type of the interface
  // T directly.  If necassary we contact the object itself to check the
  // inheritance relation.  <repoId> must be a type for which there exists
  // a proxy object factory.
  //  This function is thread-safe.
  //  Does not throw any exceptions.

  void _assertExistsAndTypeVerified();
  // If pd_flags.object_exists is zero, contact the object and verify
  // that it exists.
  //  If pd_flags.type_verified is zero, ask the object if it supports
  // the interface we are exporting.
  //  This function is thread-safe.

  _CORBA_Boolean _remote_is_a(const char* repoId);
  // Contacts to object implementation to determine whether it is an
  // instance of the type identified by <repoId>.  It is possible that
  // the implementation is a local servant of course.
  //  May throw system exceptions.
  //  This function is thread-safe.

  _CORBA_Boolean _remote_non_existent();
  // Contacts to object implementation to determine whether it exists.
  // It is possible that the implementation is a local servant of course.
  // Returns 0 if the object exists, or 1 or throws OBJECT_NOT_EXIST if
  // not.  May throw other system exceptions.
  //  This function is thread-safe.

  void* _transientExceptionHandler(void*& cookie);
  // If a transientExceptionHandler_t has been installed for this object
  // by _transientExceptionHandler(void*,void*), returns this handler and its
  // associated opaque argument in cookie.
  // Otherwise return 0.
  //  This function is thread-safe.

  void _transientExceptionHandler(void* new_handler, void* cookie);
  // Set the transientExceptionHandler_t of this object.  By default,
  // i.e. when this function is not called for an object, the global
  // transientExceptionHandler_t will be invoked when a CORBA::TRANSIENT
  // exception is caught in a remote call from a proxy object.
  // The argument <cookie> is an opaque argument that will be passed
  // to the exception handler.
  //  This function is thread-safe.

  void* _commFailureExceptionHandler(void*& cookie);
  // If a commFailureExceptionHandler_t has been installed for this object
  // by _commFailureExceptionHandler(void*,void*), returns this handler and its
  // associated opaque argument in cookie.
  // Otherwise return 0.
  //  This function is thread-safe.

  void _commFailureExceptionHandler(void* new_handler, void* cookie);
  // Set the commFailureExceptionHandler_t of this object.  By default,
  // i.e. when this function is not called for an object, the global
  // commFailureExceptionHandler_t will be invoked when a CORBA::COMM_FAILURE
  // exception is caught in a remote call from a proxy object.
  // The argument <cookie> is an opaque argument that will be passed
  // to the exception handler.
  //  This function is thread-safe.

  void* _systemExceptionHandler(void*& cookie);
  // If a systemExceptionHandler_t has been installed for this object
  // by _systemExceptionHandler(void*,void*), returns this handler and its
  // associated opaque argument in cookie.
  // Otherwise return 0.
  //  This function is thread-safe.

  void _systemExceptionHandler(void* new_handler, void* cookie);
  // Set the systemExceptionHandler_t of this object.  By default,
  // i.e. when this function is not called for an object, the global
  // systemExceptionHandler_t will be invoked when a CORBA::SystemException
  // exception, other than CORBA::TRANSIENT and CORBA::COMM_FAILURE is caught
  // in a remote call from a proxy object.  The handlers for CORBA::TRANSIENT
  // and CORBA::COMM_FAILURE are installed their own install functions.
  // The argument <cookie> is an opaque argument that will be passed
  // to the exception handler.
  //  This function is thread-safe.

  inline omniObjRef** _addrOfLocalRefList() {
    ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);
    return &pd_nextInLocalRefList;
  }
  inline omniObjRef* _nextInLocalRefList() {
    ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);
    return pd_nextInLocalRefList;
  }
  // Used by omniLocalIdentity to insert/remove this from its local
  // ref list.

  inline omniIdentity* _identity() {
    ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);
    return pd_id;
  }
  // Must hold <omni::internalLock>.

  inline omniLocalIdentity* _localId() {
    ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);
    return pd_localId;
  }
  // Must hold <omni::internalLock>.

  inline int _isForwardLocation() const {
    ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);
    return pd_flags.forward_location;
  }
  //  Must hold <omni::internalLock>.

  inline _CORBA_Boolean _is_nil() { return !pd_id; }
  // This function is thread-safe.

  inline void _noExistentCheck() {
    pd_flags.type_verified = 1;
    pd_flags.object_exists = 1;
  }
  // This function instructs the ORB to skip the existence test
  // performed in _assertExistsAndTypeVerified().
  //  Really ought to only call this if you know no-one else
  // is accessing this reference, or holding <omni::internalLock>.

  void _invoke(omniCallDescriptor&, _CORBA_Boolean do_assert=1);
  // Does the client-side work of making a request.  Dispaches
  // the call to the object identity, and deals with exception
  // handlers, retries and location forwarding.


protected:
  virtual ~omniObjRef();
  // Must not hold <omni::internalLock>.

  omniObjRef();
  // Constructor for nil object references.  No operations should
  // be invoked on nil object references.  This should be tested
  // in the public interface.

  omniObjRef(const char* intfRepoId, const char* mostDerivedId,
	     IOP::TaggedProfileList* profiles,
	     omniIdentity* id, omniLocalIdentity* lid);
  // Constructed with an initial ref count of 1.

  void _locateRequest();
  // Issues a GIOP LocateRequest.  Throws OBJECT_NOT_EXIST, or
  // returns normally if the object does exist.
  // This function is thread-safe.


private:
  omniObjRef(const omniObjRef&);
  omniObjRef& operator = (const omniObjRef&);
  // Not implemented.


  //////////////////////////////////////////////////
  // Private methods - for use by class omni only //
  //////////////////////////////////////////////////
  friend class omni;
  friend class omniInternal;

  inline void _setIdentity(omniIdentity* id, omniLocalIdentity* lid) {
    ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);
    pd_id = id;
    pd_localId = lid;
  }
  // Must hold <omni::internalLock>.


  //////////////////
  // Data members //
  //////////////////

  int pd_refCount;
  // Protected by <omniInternal.cc:objref_rc_lock>.

  char* pd_mostDerivedRepoId;
  // The interface repository id that we have been told is the
  // most derived interface of the object implementation.  This
  // may be the same as the static type of this reference, or
  // it may be wrong (if the implementation has been replaced
  // by one of a more derived type), or it may be '\0' (if the
  // IOR contained no repoId).
  //  Immutable.

  char* pd_intfRepoId;
  // The repository id of the interface that this reference is
  // presenting.  That is, if this is a c++ <foo_ptr>, then this
  // will be the repository id for <foo>.  Thus for an anonymous
  // object reference, this is the CORBA::Object repoId.  Clearly
  // _ptrToObjRef(pd_intfRepoId) must be non-zero!
  //  This may or may not be equal to <pd_mostDerivedRepoId>,
  // but unlike <pd_mostDerivedRepoId> it may *not* be the empty
  // string.
  //  Immutable.

  IOP::TaggedProfileList* pd_iopprofiles;
  // Immutable.

  omniIdentity* pd_id;
  // An encapsulation of the current implementation of this
  // object.  Either a local object, or the location and
  // key of a remote object.
  //  Mutable.  Protected by <omni::internalLock>.

  omniLocalIdentity* pd_localId;
  // If this is a reference to a local object, this is a pointer
  // to the entry in the object table (which contains the key).
  // Nil otherwise.
  //  Mutable.  Protected by <omni::internalLock>.

  struct {
    unsigned forward_location            : 1;
    // True if we have had a LOCATION_FORWARD.  In this case the
    // implementation (pd_impl) is not the same as the one
    // referred to in the iop profiles.

    unsigned type_verified               : 1;
    // True if we believe that the object implementation supports
    // the interface <pd_intfRepoId>.  If this is zero then we
    // must contact the object to confirm that it supports this
    // interface before making any invocations.

    unsigned object_exists               : 1;
    // True if we have asked the object if it exists, and it says
    // yes.  This is set to zero in the constructor, so that we
    // will contact the object and check that it exists before the
    // very first invocation.

    unsigned transient_exception_handler : 1;
    unsigned commfail_exception_handler  : 1;
    unsigned system_exception_handler    : 1;
    // True if a per-object exception handler has been installed
    // for this reference.

  } pd_flags;
  // Mutable.  Protected by <omni::internalLock>.

  omniObjRef* pd_nextInLocalRefList;
  // Linked list of all references to a particular local object.
  // Protected by <omni::internalLock>.
};


#endif  // __OMNIOBJREF_H__
