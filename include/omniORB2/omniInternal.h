// -*- Mode: C++; -*-
//                            Package   : omniORB2
// omniInternal.h             Created on: 6/2/96
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 1996, 1997 Olivetti & Oracle Research Laboratory
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
//	*** PROPRIETORY INTERFACE ***
//	

/*
  $Log$
  Revision 1.16  1998/01/20 16:46:18  sll
  Removed obsolute variable omni::traceLevel.

  Revision 1.15  1997/12/15 15:26:32  sll
  Added the missing dllimport/dllexport for WIN32.

  Revision 1.14  1997/12/12 19:59:19  sll
  Added version variable omniORB_x_y.

  Revision 1.13  1997/12/10 11:52:31  sll
  *** empty log message ***

  Revision 1.12  1997/12/09 20:45:54  sll
  Added support for system exception handlers.
  Added support for late bindings.

 * Revision 1.11  1997/09/20  16:23:53  dpg1
 * Added second argument, is_cxx_type to _widenFromTheMostDerivedIntf().
 * Added a new hash table of wrapped objects, and a mutex for it, for
 * LifeCycle support.
 *
  Revision 1.10  1997/08/26 15:25:26  sll
  Removed initFile.h include.

  Revision 1.9  1997/05/06 16:09:13  sll
  Public release.

 */

#ifndef __OMNIINTERNAL_H__
#define __OMNIINTERNAL_H__

#include <assert.h>
#include <iostream.h>

#include <stddef.h>

#if !defined(__atmos__) && !defined(_WIN32)
#include <strings.h>
#else
#include <string.h>
#endif
#include <omniORB2/CORBA_sysdep.h>
#include <omniORB2/CORBA_basetypes.h>
#include <omniORB2/seqtemplates.h>
#include <omniORB2/IOP.h>
#include <omniORB2/GIOP.h>
#include <omniORB2/IIOP.h>
#include <omnithread.h>

class Rope;
class GIOP_S;
class GIOP_C;
class GIOPobjectLocation;
class omniObject;
class initFile;
class omniORB;
class omniObjectManager;

// omniORB_x_y
//   Define this variable to trap the mismatch of the stub and the runtime
//   library. The two digits x,y should be the same as the shared library
//   major version number and minor version number. For example, for shared
//   library 2.4.0 the variable number should be omniORB_2.4. Notice that
//   the variable name stays the same with compatible shared library, e.g.
//   2.4.1.
//
extern const char* _OMNIORB2_NTDLL_ omniORB_2_4;

#include <omniORB2/rope.h>

struct omniObjectKey {
  _CORBA_ULong hi;
  _CORBA_ULong med;
  _CORBA_ULong lo;
};

class _OMNIORB2_NTDLL_ omni {

public:

#if SIZEOF_PTR == SIZEOF_LONG
  typedef long ptr_arith_t;
#elif SIZEOF_PTR == SIZEOF_INT
  typedef int ptr_arith_t;
#else
#error "No suitable type to do pointer arithmetic"
#endif

  static const _CORBA_Boolean myByteOrder;

  enum alignment_t { ALIGN_1 = 1, ALIGN_2 = 2, ALIGN_4 = 4, ALIGN_8 = 8 };
  static const alignment_t max_alignment;  // Maximum value of alignment_t

  static inline ptr_arith_t align_to(ptr_arith_t p,alignment_t align) {
    return (p + ((int) align - 1)) & ~((int) align - 1);
  }

  static _CORBA_Unbounded_Sequence_Octet myPrincipalID;

  static void objectIsReady(omniObject *obj);
  static void objectDuplicate(omniObject *obj);
  // Increment the reference count.

  static void objectRelease(omniObject *obj);
  // Decrement the reference count. The count must be >= 0, otherwise
  // throw a INV_OBJREF() exception.
  // If the reference count is 0, then
  //     Remove the object from one of the object table depends on
  //     whether it is a proxy or an implementation.
  //     if this is a proxy object, call the delete operator to remove
  //        the object
  //     else this is the implementation, then
  //          if the object has been marked to be disposed of, then
  //              call the delete operator
  //          else defer calling the delete operator until a subsequent
  //          call to BOA::dispose().

  static omniObject *locateObject(omniObjectManager*,omniObjectKey &k);
  static void disposeObject(omniObject *obj);
  // If the reference count of the object is 0, call the delete operator
  // to remove the object.
  // If the reference count of the object is not 0, then
  //    if this is a proxy object, simply returns.
  //    else this is the implementation, then
  //         *MARK the object as disposed but it defer calling the
  //          delete operator until the reference count drops to zero.


  static char *objectToString(const omniObject *obj);
  // returns a heap allocated and stringified IOR representation
  // (ref CORBA 2 spec. 10.6.5)

  static omniObject *stringToObject(const char *str);
  // returns an object pointer identified by the stringified IOR representation
  // (ref CORBA 2 spec. 10.6.5)
  // returns 0 if this is a null object reference

  static omniObject * createObjRef(const char *mostDerivedRepoId,
				   const char *targetRepoId,
				   IOP::TaggedProfileList* profiles,
				   _CORBA_Boolean release);
  // Returns an object pointer identified by <mostDerivedRepoId> & <profiles>.
  // If release is TRUE, the returned object assumes resposibility of
  // the heap allocated <profiles>.
  // <mostDerivedRepoId> is the interface repository ID recorded in the
  // original IOR.
  // <targetRepoId> is the interface repository ID of the desired interface.
  // If <targetRepoId> is neither equal to <mostDerivedRepoId> nor the
  // latter is a derived interface of the former, a CORBA::MARSHAL exception
  // would be raised.
  // If <targetRepoId> == 0, then the desired interface is the pseudo object
  // CORBA::Object from which all interfaces derived.
};

class omniRopeAndKey {
public:
  inline omniRopeAndKey(Rope *r,_CORBA_Octet *k, _CORBA_ULong ksize) 
                 : pd_r(r), pd_keysize(0)
  {
    key(k,ksize);
  }

  inline omniRopeAndKey() : pd_r(0), pd_keysize(0) {}

  inline ~omniRopeAndKey() { 
    if (pd_keysize > sizeof(omniObjectKey)) {
      delete [] pd_keyptr;
    }
  }

  inline Rope* rope() const { return pd_r; }

  inline _CORBA_Octet* key() const { 
    if (pd_keysize <= sizeof(omniObjectKey)) {
      return (_CORBA_Octet*)&pd_key; 
    }
    else {
      return pd_keyptr; 
    }
  }
   
  inline _CORBA_ULong  keysize() const { return pd_keysize; }

  inline void rope(Rope* r) { pd_r = r; }
  inline void key(_CORBA_Octet* k, _CORBA_ULong ksize) {
    if (pd_keysize > sizeof(omniObjectKey)) delete [] pd_keyptr;
    pd_keysize = ksize;
    if (pd_keysize <= sizeof(omniObjectKey)) {
      memcpy((void *)&pd_key,(void*)k,pd_keysize);
    }
    else {
      pd_keyptr = new _CORBA_Octet[pd_keysize];
      memcpy((void*)pd_keyptr,(void*)k,pd_keysize);
    }
  }

private:
  Rope*             pd_r;
  _CORBA_ULong      pd_keysize;
  union {
    _CORBA_Octet*   pd_keyptr;
    omniObjectKey   pd_key;
  };

  omniRopeAndKey& operator=(const omniRopeAndKey&);
  omniRopeAndKey(const omniRopeAndKey&);
};

class omniObject {

protected:

  omniObject(omniObjectManager*p =0); // ctor local object
  omniObject(const char *repoId,   // ctor for proxy object
	 Rope *r,
	 _CORBA_Octet *key,
	 size_t keysize,
      	 IOP::TaggedProfileList *profiles,
	 _CORBA_Boolean release); 
  // If release TRUE, the object assumes management of the heap allocated
  // <key> and <profiles>.

  virtual ~omniObject();

  void  PR_IRRepositoryId(const char *s);
  // Set the IR repository ID of this object to <s>.
  // NOTE: this function is **not thread-safe**. It *should not* be called
  //       if there is any chance that the object might be accessed
  //       concurrently by another thread.
  // If this is a local object
  //    1. If omni::objectIsReady() has been called for this object,
  //       this function will throw a omniORB::fatalException().
  //    2. otherwise, <s> is recorded as the current IR repository ID.
  //
  // If this is a proxy object
  //    1. The IR repository ID of this object was initialised by the
  //       ctor for proxy objects. This value is preserved.
  //    2. <s> is then recorded as the current IR repository ID.
  //    3. An internal flag (pd_flags.existent_and_type_verified) is set to 0.
  //       This will cause assertObjectExistent() to check the type and
  //       verify the existent of the object before it performs the next
  //       invocation.

public:

  void setRopeAndKey(const omniRopeAndKey& l,_CORBA_Boolean keepIOP=1);
  // Set new values to the rope and key. If keepIOP is true, keep the
  // original IOP profile. Otherwise update the profile as well.
  // This function is thread-safe.

  void resetRopeAndKey();
  // If this is a proxy object, reset the rope and key to the values
  // stored in the IOP profile.
  // This function has no effect on local objects and is silently ignored.
  // This function is thread-safe.

  _CORBA_Boolean getRopeAndKey(omniRopeAndKey& l) const;
  // Get the current value of the rope and key. If the values are the same
  // as those stored in the IOP profile, the return value is 0. Otherwise
  // the return value is 1.
  // This function is thread-safe.

  void getKey(_CORBA_Octet*& key,_CORBA_ULong& ksize) const {
    // This is a non-thread safe function to read the key of this object.
    // The object continues to own the storage of <key>.
    // If the key is modified concurrently by another thread's call to
    // setRopeAndKey(), the behaviour is underfine.
    if (is_proxy()) {
      key = pd_objkey.foreign;
    }
    else {
      key = (_CORBA_Octet*) &pd_objkey.native;
    }
    ksize = pd_objkeysize;
  }
  

  void assertObjectExistent();
  // If this is a local object, 
  //       returns.
  // If this is a proxy object
  //     if the internal flag (pd_flags.existent_and_type_verified) is 0
  //          contact the remote object to verify that it exists
  //          and is of the type identified by the IR repository ID
  //          given as the argument to the ctor of this object.
  //          Set pd_flags.existent_and_type_verified to 1
  //          returns
  //     else
  //          returns
  //
  // This function may throw a CORBA::SystemException if 
  //    1. the object does not exist - CORBA::OBJECT_NOT_EXIST
  //    2. non-transient communication failure - CORBA::COMM_FAILURE
  //    3. the object is of the wrong type - CORBA::INV_OBJREF
  //    4. other system errors.
  //
  // This function is thread-safe.

  virtual _CORBA_Boolean dispatch(GIOP_S &,const char *operation,
				  _CORBA_Boolean response_expected);
  // Normally, this function is overridden by a dispatch function in the
  // skeleton implementation stub. The function is called by the dispatcher
  // of GIOP_S.
  // This function is thread-safe.

  inline _CORBA_Boolean is_proxy() const { 
    return (_CORBA_Boolean)pd_flags.proxy; 
  }
  // Return 1 if this is a proxy object, 0 if this is a local object.
  // This function is thread-safe.

  inline const char *NP_IRRepositoryId() const { return pd_repoId; }
  // Return the IR repository ID of this object. The value is returned 
  // is either the value given to the ctor or set by the most recent
  // call to PR_IRRepositoryID().
  // This function is thread-safe.

  virtual void *_widenFromTheMostDerivedIntf(const char *type_id,
					     _CORBA_Boolean is_cxx_type_id=0);
  // The most derived class which override this virtual function will be
  // called to return a pointer to the base class object identified by
  // the type id <type_id>. 
  //
  // This function accepts two forms of type id:
  //   if <is_cxx_type_id> == 0
  //      <type_id> is a OMG IR repository ID. Furthermore, if <type_id> is
  //      null, a pointer to the base class CORBA::Object will be returned.
  //      The most derived class is guaranteed to support the widening to
  //      all of the base classes that implement the base IDL interfaces.
  //   if <is_cxx_type_id> == 1
  //      <type_id> is a C++ class name of the target base class. For example
  //      the base class CORBA::Object can be obtained by using 
  //      "CORBA::Object" as the <type_id>. 
  //      The most derived class may support the widening to a subset of its
  //      base classes. It may even support none at all.
  //
  // If the object cannot be widened to the class specified, a
  // null pointer will be returned.
  // This function DO NOT throw any exception under any circumstance.
  // This function is thread-safe.

  inline IOP::TaggedProfileList * iopProfiles() const { 
    // This function is thread-safe.
    return pd_iopprofile; 
  }

  _CORBA_Boolean _real_is_a(const char *repoId);
  // Returns 1 if the object is really an instance of the type identified
  // by the IR repository ID <repoId>. If the stub for the object type is
  // linked into the executable, the ORB can fully determine the _is_a
  // relation without the need to contact the object remotely. Otherwise,
  // the ORB will use the CORBA::Object::_is_a operation to query the
  // remote object to find out its actual type.
  //
  // This function is thread-safe.

  void* _realNarrow(const char* repoId);
  // If the actual type of the object can be widened to the requested interface
  // type identified by the IR repository ID <repoId>, return a valid
  // object reference. Otherwise, return 0. The return value is of type void*
  // and can be casted to the T_ptr type of the interface T directly.
  // The resulting T_ptr instance should be released using CORBA::release()
  // when it is no longer needed.
  // This function is thread-safe.

  void* _transientExceptionHandler(void*& cookie);
  // If a transientExceptionHandler_t has been installed for this object
  // by _transientExceptionHandler(void*,void*), returns this handler and its
  // associated opaque argument in cookie.
  // Otherwise return 0.
  // This function is thread-safe.

  void _transientExceptionHandler(void* new_handler,void* cookie);
  // Set the transientExceptionHandler_t of this object.  By default,
  // i.e. when this function is not called for an object, the global
  // transientExceptionHandler_t will be invoked when a CORBA::TRANSIENT
  // exception is caught in a remote call from a proxy object.
  // The argument <cookie> is an opaque argument that will be passed
  // to the exception handler.
  // This function is thread-safe.

  void* _commFailureExceptionHandler(void*& cookie);
  // If a commFailureExceptionHandler_t has been installed for this object
  // by _commFailureExceptionHandler(void*,void*), returns this handler and its
  // associated opaque argument in cookie.
  // Otherwise return 0.
  // This function is thread-safe.

  void _commFailureExceptionHandler(void* new_handler,void* cookie);
  // Set the commFailureExceptionHandler_t of this object.  By default,
  // i.e. when this function is not called for an object, the global
  // commFailureExceptionHandler_t will be invoked when a CORBA::COMM_FAILURE
  // exception is caught in a remote call from a proxy object.
  // The argument <cookie> is an opaque argument that will be passed
  // to the exception handler.
  // This function is thread-safe.

  void* _systemExceptionHandler(void*& cookie);
  // If a systemExceptionHandler_t has been installed for this object
  // by _systemExceptionHandler(void*,void*), returns this handler and its
  // associated opaque argument in cookie.
  // Otherwise return 0.
  // This function is thread-safe.

  void _systemExceptionHandler(void* new_handler,void* cookie);
  // Set the systemExceptionHandler_t of this object.  By default,
  // i.e. when this function is not called for an object, the global
  // systemExceptionHandler_t will be invoked when a CORBA::SystemException
  // exception, other than CORBA::TRANSIENT and CORBA::COMM_FAILURE is caught 
  // in a remote call from a proxy object. The handlers for CORBA::TRANSIENT
  // and CORBA::COMM_FAILURE are installed their own install functions.
  // The argument <cookie> is an opaque argument that will be passed
  // to the exception handler.
  // This function is thread-safe.

  omniObjectManager* manager() const { return pd_manager; }
  // This function should only be called for local object.
  // Returns the object manager of this object.
  // Calling this function for a proxy object would result in undefined
  // behaviour.

  static omni_mutex          objectTableLock;
  static omniObject*         proxyObjectTable;
  static omniObject**        localObjectTable;
  static omni_mutex          wrappedObjectTableLock;
  static void**              wrappedObjectTable;

  static void                globalInit();
  // This function is not thread-safe and should be called once only.

  static omniObjectManager*  nilObjectManager();

private:
  union {
    _CORBA_Octet *foreign;
    omniObjectKey     native;
  }                             pd_objkey;
  size_t                        pd_objkeysize;
  char *                        pd_repoId;
  size_t                        pd_repoIdsize;
  char *                        pd_original_repoId;
  union {
    Rope *                      pd_rope;
    omniObjectManager*          pd_manager;
  };
  int                           pd_refCount;
  omniObject *                      pd_next;

  struct {
    _CORBA_UShort              proxy                       : 1;
    _CORBA_UShort              disposed                    : 1;
    _CORBA_UShort              existent_and_type_verified  : 1;
    _CORBA_UShort              forwardlocation             : 1;
    _CORBA_UShort              transient_exception_handler : 1;
    _CORBA_UShort              commfail_exception_handler  : 1;
    _CORBA_UShort              system_exception_handler    : 1;
  } pd_flags;
  
  IOP::TaggedProfileList *      pd_iopprofile;
  
  inline int getRefCount() const { return pd_refCount; }
  inline void setRefCount(int count) { pd_refCount = count; return; }

  friend void omni::objectIsReady(omniObject *obj);
  friend void omni::objectDuplicate(omniObject *obj);
  friend omniObject *omni::locateObject(omniObjectManager*,omniObjectKey &k);
  friend void omni::disposeObject(omniObject *obj);
  friend void omni::objectRelease(omniObject *obj);
  friend char* omni::objectToString(const omniObject *obj);
  friend omniObject* omni::stringToObject(const char *str);
  friend omniObject * omni::createObjRef(const char *mostDerivedRepoId,
					 const char *targetRepoId,
					 IOP::TaggedProfileList *profiles,
					 _CORBA_Boolean release);
};

#include <omniORB2/bufferedStream.h>
#include <omniORB2/giopDriver.h>

#endif // __OMNIINTERNAL_H__
