// -*- Mode: C++; -*-
//                            Package   : omniORB2
// omniInternal.h             Created on: 6/2/96
//                            Author    : Sai Lai Lo (sll)
//
// Copyright (C) Olivetti Research Limited, 1996
//
// Description:
//	*** PROPRIETORY INTERFACE ***
//	

/*
  $Log$
  Revision 1.1  1997/01/08 17:28:30  sll
  Initial revision

 */

#ifndef __OMNIORB_H__
#define __OMNIORB_H__

#include <assert.h>
#include <iostream.h>

#include <stddef.h>
#include <strings.h>
#include <omniORB2/CORBA_sysdep.h>
#include <omniORB2/CORBA_basetypes.h>
#include <omniORB2/CORBA_templates.h>
#include <omniORB2/IOP.h>
#include <omniORB2/GIOP.h>
#include <omniORB2/IIOP.h>
#include <omnithread.h>

class Rope;
class GIOP_S;
class GIOP_C;
class omniObject;
class omniObjectKey;

class omniORB {

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

  static ptr_arith_t align_to(ptr_arith_t p,alignment_t align) {
    return (p + ((int) align - 1)) & ~((int) align - 1);
  }

  static _CORBA_Unbounded_Sequence_Octet myPrincipalID;

  static void init(int &argc,char **argv,const char *orb_identifier);
  static void boaInit(int &argc,char **argv,const char *boa_identifier);


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
  // Returns an object pointer identified by <repoId> and <profiles>.
  // If release is TRUE, the returned object assumes resposibility of
  // the heap allocated <profiles>.
  
  static void  orbIsReady();
  static size_t MaxMessageSize();
  // returns the ORB-wide limit on the size of GIOP message (excluding the
  // header).



  // This exception is thrown if a bug inside the omniORB2 runtime is
  // detected. The exact location in the source where the exception is
  // thrown is indicated by file() and line().
  // 
  class fatalException {
  public:
    fatalException(const char *file,int line,const char *errmsg) {
      pd_file = file;
      pd_line = line;
      pd_errmsg = errmsg;
    }
    ~fatalException() {}
    const char *file() const { return pd_file; }
    int line() const { return pd_line; }
    const char *errmsg() const { return pd_errmsg; }
  private:
    const char *pd_file;
    int         pd_line;
    const char *pd_errmsg;

    fatalException();
  };
};

struct omniObjectKey {
  _CORBA_ULong hi;
  _CORBA_ULong med;
  _CORBA_ULong lo;

  static const unsigned int hash_table_size;

  inline int hash() {
    return (hi + med + lo) % hash_table_size;
  }

  static void generateNewKey(omniObjectKey &k);
};

inline int operator==(const omniObjectKey &k1,const omniObjectKey &k2) { 
  return (k1.hi == k2.hi && 
	  k1.med == k2.med &&
	  k1.lo == k2.lo) ? 1 : 0; 
}

inline int operator!=(const omniObjectKey &k1,const omniObjectKey &k2) {
    return (k1.hi != k2.hi ||
	    k1.med != k2.med ||
	    k1.lo != k2.lo) ? 1 : 0;
}


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

  void  NP_objkey(const omniObjectKey &k);
  void  NP_rope(Rope *r);
  void  PR_IRRepositoryId(const char *s);

public:

  Rope *_rope() const { return pd_rope; }
  const void *objkey() const { return ((pd_proxy) ?
				 ((void *)pd_objkey.foreign) :
				 ((void *)&pd_objkey.native)); }
  const size_t objkeysize() const { return pd_objkeysize; }

  virtual _CORBA_Boolean dispatch(GIOP_S &,const char *operation,
				  _CORBA_Boolean response_expected);

  _CORBA_Boolean is_proxy() const { return pd_proxy; }

  const char *NP_IRRepositoryId() const { return pd_repoId; }

  virtual void *_widenFromTheMostDerivedIntf(const char *repoId) throw();
  // The most derived class which override this virtual function will be
  // called to return a pointer to the base class object that implements
  // the interface identified by the IR repository ID <repoId>.
  // If <repoID> is null, a pointer to the base class CORBA::Object will
  // be returned.
  // If the object cannot be widened to the interface specified, a
  // null pointer will be returned.
  // This function DO NOT throw any exception under any circumstance.

  IOP::TaggedProfileList * iopProfiles() const { 
    assert(pd_iopprofile != 0);
    return pd_iopprofile; 
  }

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
  
  IOP::TaggedProfileList *      pd_iopprofile;
  
  int getRefCount() const { return pd_refCount; }
  void setRefCount(int count) { pd_refCount = count; return; }

  friend void omniORB::objectIsReady(omniObject *obj);
  friend char * omniORB::objectToString(const omniObject *obj);
  friend void omniORB::objectDuplicate(omniObject *obj);
  friend omniObject *omniORB::locateObject(omniObjectKey &k);
  friend void omniORB::disposeObject(omniObject *obj);
  friend void omniORB::objectRelease(omniObject *obj);
  friend char *objectToString(const omniObject *obj);
  friend omniObject *stringToObject(const char *str);
  friend IOP::TaggedProfileList *omniORB::objectToIopProfiles(omniObject *obj);
};

#include <omniORB2/rope.h>
#include <omniORB2/bufferedStream.h>
#include <omniORB2/giopDriver.h>
#include <omniORB2/bufStream_templates.h>

#endif // __OMNIORB_H__
