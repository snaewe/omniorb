// -*- Mode: C++; -*-
//                            Package   : omniORB2
// omniInternal.h             Created on: 6/2/96
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 1996-1999 AT&T Laboratories Cambridge
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
  Revision 1.29.4.2  1999/10/02 18:28:58  sll
  New internal function internal_init_BOA().
  New constant to identify omniORB in the TAG_ORB_TYPE component of an IOR.

  Revision 1.29.4.1  1999/09/15 20:18:14  sll
  Updated to use the new cdrStream abstraction.
  Marshalling operators for NetBufferedStream and MemBufferedStream are now
  replaced with just one version for cdrStream.
  Derived class giopStream implements the cdrStream abstraction over a
  network connection whereas the cdrMemoryStream implements the abstraction
  with in memory buffer.

  Revision 1.29  1999/08/16 19:33:07  sll
  New per-compilation unit initialiser class omniInitialiser.

  Revision 1.28  1999/08/14 16:37:31  sll
  Added support for the python binding.
  locateObject no longer throw an exception when the object is not found.
  It returns a nil pointer instead.

  Revision 1.27  1999/06/26 17:55:41  sll
  Added string.h for all platforms by default.

  Revision 1.26  1999/06/18 21:15:17  sll
  Updated copyright notice.

  Revision 1.25  1999/06/18 20:36:31  sll
  Replaced _LC_attr with _core_attr.

  Revision 1.24  1999/04/01 15:52:23  djr
  Updated version number to 2.8.
  Move an _LC_attr to the correct position in a declaration.

  Revision 1.23  1999/01/07 18:36:13  djr
  Changes to support split of omniORB library in two.

  Revision 1.22  1998/10/08 13:07:24  sll
  OpenVMS needs string.h (POSIX or System V) instead of strings.h (BSD).

  Revision 1.21  1998/08/21 19:27:08  sll
  New omniObject member noExistentCheck.

  Revision 1.20  1998/08/05 18:11:09  sll
  Updated version variable to omniORB_2_6 (previously omniORB_2_5).

  Revision 1.19  1998/04/07 19:53:38  sll
  Replace _OMNIORB2_NTDLL_ on the whole class omni with
  _OMNIORB_NTDLL_IMPORT on individual constants and variables.

  Revision 1.18  1998/02/27 14:00:17  sll
  Changed manager() to _objectManager() in class omniObject to avoid
  potential name clash with application code.

 * Revision 1.17  1998/01/27  16:06:45  ewc
 * Added support necessary for TypeCode and Any
 *
 * Revision 1.16  1998/01/20  16:46:18  sll
 * Removed obsolute variable omni::traceLevel.
 *
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
#include <stddef.h>

#if !defined(__atmos__) && !defined(_WIN32) && !defined(__VMS)
#include <strings.h>
#include <string.h>
#else
#include <string.h>
#endif
#include <omnithread.h>
#include <omniORB2/CORBA_sysdep.h>
#include <omniORB2/CORBA_basetypes.h>
#include <omniORB2/seqTemplatedecls.h>
#include <omniORB2/stringtypes.h>
#ifndef __IOP_hh_EXTERNAL_GUARD__
#include <omniORB2/IOP.h>
#define __IOP_hh_EXTERNAL_GUARD__
#endif
#include <omniORB2/GIOP.h>
#include <omniORB2/IIOP.h>
#include <omniORB2/templatedecls.h>

class Rope;
class GIOP_S;
class GIOP_C;
class GIOPObjectInfo;
class omniObject;
class initFile;
class omniObjectManager;

// Function prototype for Python support. Declared as a friend of
// omniObject.
extern void omniPy_objectIsReady(omniObject* obj);

// omniORB_x_y
//   Define this variable to trap the mismatch of the stub and the runtime
//   library. The two digits x,y should be the same as the shared library
//   major version number and minor version number. For example, for shared
//   library 2.5.0 the variable number should be omniORB_2_5. Notice that
//   the variable name stays the same with compatible shared library, e.g.
//   2.5.1.
//
extern _core_attr const char* omniORB_2_9;
extern _core_attr const _CORBA_ULong omniORB_TAG_ORB_TYPE; // ATT\x00


#include <omniORB2/rope.h>


struct omniObjectKey {
  _CORBA_ULong hi;
  _CORBA_ULong med;
  _CORBA_ULong lo;
};

//////////////////////////////////////////////////////////////////////
//////////////////////////////// omni ////////////////////////////////
//////////////////////////////////////////////////////////////////////

class omni {

public:

#if SIZEOF_PTR == SIZEOF_LONG
  typedef long ptr_arith_t;
#elif SIZEOF_PTR == SIZEOF_INT
  typedef int ptr_arith_t;
#else
#error "No suitable type to do pointer arithmetic"
#endif

  static _core_attr const _CORBA_Char myByteOrder;

  enum alignment_t { ALIGN_1 = 1, ALIGN_2 = 2, ALIGN_4 = 4, ALIGN_8 = 8 };
  static _core_attr const alignment_t max_alignment;
  // Maximum value of alignment_t

  static inline ptr_arith_t align_to(ptr_arith_t p,alignment_t align) {
    return (p + ((int) align - 1)) & ~((int) align - 1);
  }

  static _core_attr _CORBA_Unbounded_Sequence_Octet myPrincipalID;

  static void objectIsReady(omniObject* obj);
  static void objectDuplicate(omniObject* obj);
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
  static omniObject *locatePyObject(omniObjectManager*,omniObjectKey &k);
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

  // XXX
  static omniObject * createObjRef(const char *mostDerivedRepoId,
				   const char *targetRepoId,
				   IOP::TaggedProfileList* profiles);
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
  //
  // If an exception occurs, <profiles> is freed.

  static void internal_init_BOA();
  // This internal function make sure that the BOA is started.
  // The application may be a pure client and do not initialise a BOA.
  // On the otherhand, on occasions the ORB has to create internal
  // CORBA objects to response to local and remote requests. The
  // ctor of these internal objects call this function to ensure
  // that the BOA is started.

};

//////////////////////////////////////////////////////////////////////
///////////////////////////// omniObject /////////////////////////////
//////////////////////////////////////////////////////////////////////

class omniObject {

protected:

  omniObject(omniObjectManager* p = 0); // ctor local object

  omniObject(GIOPObjectInfo* objInfo,const char* use_as_repoID); // ctor proxy

  virtual ~omniObject();

  void PR_setRepositoryID(const char* repoID);
  void PR_setKey(const omniObjectKey& k);


public:


  GIOPObjectInfo* getInvokeInfo(_CORBA_Boolean& location_forwarded);
  // Returns a GIOPObjectInfo with all the information necessary to do a
  // GIOP request.  If the info does not correspond to that contained in
  // the object's IOR, set <original> to 1. Typically this is because a
  // setInvokeInfo has been called. Otherwise returns 0.  The returned
  // object should be released by calling its _release() method.  This
  // function is atomic and thread-safe.
  // 

  void setInvokeInfo(GIOPObjectInfo*,_CORBA_Boolean keepIOP=1);
  // Use the argument GIOPObjectInfo for future invocations.
  // Future invocations will make use the new info. Typically this function
  // is used to update the object state with the info contained in a
  // LOCATION FORWARD message.
  // Ownership of argument GIOPObjectInfo now belongs to this object.
  // This function is atomic and thread-safe.

  void resetInvokeInfo();
  // Reset the state of the object to be the same as described in its IOR.
  // This function is atomic and thread-safe.

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

  inline const char* NP_IRRepositoryId() const { return pd_repositoryID; }
  // Return the IR repository ID of this object. This is the ID that
  // is recorded in the IOR of this object.
  //  This function is thread-safe.

  virtual void* _widenFromTheMostDerivedIntf(const char* type_id,
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

  omniObjectManager* _objectManager() const { return pd_data.l.pd_manager; }
  // This function should only be called for local object.
  // Returns the object manager of this object.
  // Calling this function for a proxy object would result in undefined
  // behaviour.

  void noExistentCheck() { pd_flags.existent_and_type_verified = 1; }
  // This function instructs the ORB to skip the existence test performed
  // in assertObjectExistent().

  static omni_mutex          objectTableLock;
  static omniObject*         proxyObjectTable;
  static omniObject**        localObjectTable;
  static omniObject**        localPyObjectTable;
  static omni_mutex          wrappedObjectTableLock;
  static void**              wrappedObjectTable;

  // This function is not thread-safe and should be called once only.

  static omniObjectManager*  nilObjectManager();

  //protected:
  void getKey(omniObjectKey&) const;

private:

  int                  pd_refCount;
  omniObject*          pd_next;

  struct {
    _CORBA_UShort proxy                       : 1;
    _CORBA_UShort disposed                    : 1;
    _CORBA_UShort existent_and_type_verified  : 1;
    _CORBA_UShort forwardlocation             : 1;
    _CORBA_UShort transient_exception_handler : 1;
    _CORBA_UShort commfail_exception_handler  : 1;
    _CORBA_UShort system_exception_handler    : 1;
  } pd_flags;

  _CORBA_String_member    pd_repositoryID;
  // Repository ID encoded in the IOR

  GIOPObjectInfo*         pd_objectInfo;

  union {
    struct  {
      char*                   pd_use_as_repositoryID;
      // Repository ID of the interface this object is used as.
      // If this value does not equals pd_use_as_repositoryID, the object 
      // has to be contacted to verify its true type before the first
      // invocation.
      GIOPObjectInfo*         pd_originalInfo;
    } p;
    struct {
      omniObjectKey           pd_key;
      omniObjectManager*      pd_manager;
    } l;
  } pd_data;

  inline int getRefCount() const     { return pd_refCount;  }
  inline void setRefCount(int count) { pd_refCount = count; }

  friend void omni::objectIsReady(omniObject* obj);
  friend void omni::objectDuplicate(omniObject* obj);
  friend omniObject* omni::locateObject(omniObjectManager*,omniObjectKey &k);
  friend omniObject* omni::locatePyObject(omniObjectManager*,omniObjectKey &k);
  friend void omni::disposeObject(omniObject* obj);
  friend void omni::objectRelease(omniObject* obj);
  friend class omni_objectRef_initialiser;

  friend void omniPy_objectIsReady(omniObject* obj);
};


// Singletons created per compilation unit. The attach method is called
// when the ORB is initialised. The detach method is called when the ORB
// is destroyed.
class omniInitialiser {
public:
  virtual void attach() = 0;
  virtual void detach() = 0;
};

class omniObject_var {
public:
  inline omniObject_var() : pd_obj(0) { }
  inline omniObject_var(omniObject* p) : pd_obj(p) {}
  inline ~omniObject_var() {
    if (pd_obj) omni::objectRelease(pd_obj); pd_obj = 0;
  }
  inline omniObject_var(const omniObject_var& p) : pd_obj(p.pd_obj) {
    if (pd_obj) {
      omni::objectDuplicate(pd_obj);
    }
  }	
  inline omniObject_var& operator= (omniObject* p) {
    if (pd_obj) omni::objectRelease(pd_obj);
    pd_obj = p;
    return *this;
  }
  inline omniObject_var& operator= (const omniObject_var& p) {
    if (pd_obj) omni::objectRelease(pd_obj);
    pd_obj = p.pd_obj;
    if (pd_obj) {
      omni::objectDuplicate(pd_obj);
    }
    return *this;
  }
  
  inline omniObject* operator->() const { return pd_obj; }

  inline operator omniObject*() const { return pd_obj; }

  inline omniObject* retn() { omniObject* p = pd_obj; pd_obj = 0; return p; }

private:
  omniObject* pd_obj;
};

#include <omniORB2/cdrStream.h>
#include <omniORB2/seqTemplatedefns.h>
#include <omniORB2/giopStream.h>
#include <omniORB2/giopDriver.h>


template <class T>
class _CORBA_ConstrType_Variable_Var;

#endif // __OMNIINTERNAL_H__
