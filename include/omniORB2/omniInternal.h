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
  Revision 1.11  1997/09/20 16:23:53  dpg1
  Added second argument, is_cxx_type to _widenFromTheMostDerivedIntf().
  Added a new hash table of wrapped objects, and a mutex for it, for
  LifeCycle support.

 * Revision 1.10  1997/08/26  15:25:26  sll
 * Removed initFile.h include.
 *
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
  static const char*          myORBId;
  static const char*          myBOAId;
  static omni_mutex initLock;
  static _CORBA_Boolean orb_initialised;
  static _CORBA_Boolean boa_initialised;
  static initFile*      configFile;
  static _CORBA_ULong   traceLevel;

  enum alignment_t { ALIGN_1 = 1, ALIGN_2 = 2, ALIGN_4 = 4, ALIGN_8 = 8 };
  static const alignment_t max_alignment;  // Maximum value of alignment_t

  static inline ptr_arith_t align_to(ptr_arith_t p,alignment_t align) {
    return (p + ((int) align - 1)) & ~((int) align - 1);
  }

  static _CORBA_Unbounded_Sequence_Octet myPrincipalID;

  static void init(int &argc,char **argv,const char *orb_identifier);
  static void boaInit(int &argc,char **argv,const char *boa_identifier);

  static omniObject* resolveInitRef(const char* identifier);  
  static unsigned long listInitServices(char**& servicelist);

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

  static omniObject *locateObject(omniObjectKey &k);
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

  static Rope *iopProfilesToRope(const IOP::TaggedProfileList *profiles,
				 _CORBA_Octet *&objkey,
				 size_t &keysize);
  // Look at the IOP tagged profile list <profiles>, returns the most
  // most suitable Rope to talk to the object and its object key.
  // If the object is a local object, return 0 but still fills in the object
  // key value. If no suitable Rope can be found, throw an exception.

  static IOP::TaggedProfileList *objectToIopProfiles(omniObject *obj);
  // Returns a heap allocated IOP tagged profile list for the object.
  // If the object is local, the profile list contains the profile
  // for each of the supported IOPs.
  // If the object is a proxy, the profile list only contain one IOP profile,
  // which comes from the rope that is used to talk to the object.

  static omniObject * createObjRef(const char *mostDerivedRepoId,
				   const char *targetRepoId,
				   IOP::TaggedProfileList *profiles,
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
  
  static void  orbIsReady();
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

class _wrap_proxy;

class omniObject {

protected:

  omniObject();                    // ctor local object
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


public:

  void setRopeAndKey(const omniRopeAndKey& l,_CORBA_Boolean keepIOP=1);
  // Set new values to the rope and key. If keepIOP is true, keep the
  // original IOP profile. Otherwise update the profile as well.

  void resetRopeAndKey();
  // If this is a proxy object, reset the rope and key to the values
  // stored in the IOP profile.
  // This function has no effect on local objects and is silently ignored.

  _CORBA_Boolean getRopeAndKey(omniRopeAndKey& l) const;
  // Get the current value of the rope and key. If the values are the same
  // as those stored in the IOP profile, the return value is 0. Otherwise
  // the return value is 1.

  void assertObjectExistent();

  virtual _CORBA_Boolean dispatch(GIOP_S &,const char *operation,
				  _CORBA_Boolean response_expected);

  inline _CORBA_Boolean is_proxy() const { return pd_proxy; }

  inline const char *NP_IRRepositoryId() const { return pd_repoId; }

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

  inline IOP::TaggedProfileList * iopProfiles() const { 
    return pd_iopprofile; 
  }

  static omni_mutex          objectTableLock;
  static omniObject*         proxyObjectTable;
  static omniObject**        localObjectTable;
  static omni_mutex          wrappedObjectTableLock;
  static _wrap_proxy**       wrappedObjectTable;

private:
  union {
    _CORBA_Octet *foreign;
    omniObjectKey     native;
  }                             pd_objkey;
  size_t                        pd_objkeysize;
  char *                        pd_repoId;
  Rope *                        pd_rope;
  _CORBA_Boolean                pd_proxy;
  int                           pd_refCount;
  omniObject *                      pd_next;
  _CORBA_Boolean                pd_disposed;
  _CORBA_Boolean                pd_existentverified;
  _CORBA_Boolean               	pd_forwardlocation;
  
  IOP::TaggedProfileList *      pd_iopprofile;
  
  inline int getRefCount() const { return pd_refCount; }
  inline void setRefCount(int count) { pd_refCount = count; return; }

  friend void omni::objectIsReady(omniObject *obj);
  friend char * omni::objectToString(const omniObject *obj);
  friend void omni::objectDuplicate(omniObject *obj);
  friend omniObject *omni::locateObject(omniObjectKey &k);
  friend void omni::disposeObject(omniObject *obj);
  friend void omni::objectRelease(omniObject *obj);
  friend char *objectToString(const omniObject *obj);
  friend omniObject *stringToObject(const char *str);
  friend IOP::TaggedProfileList *omni::objectToIopProfiles(omniObject *obj);
};

#include <omniORB2/rope.h>
#include <omniORB2/bufferedStream.h>
#include <omniORB2/giopDriver.h>

#endif // __OMNIINTERNAL_H__
