// -*- Mode: C++; -*-
//                            Package   : omniORB2
// omniInternal.h             Created on: 6/2/96
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2003-2009 Apasphere Ltd
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

#ifndef __OMNIINTERNAL_H__
#define __OMNIINTERNAL_H__

#include <stddef.h>

#if !defined(__atmos__) && !defined(_WIN32) && !defined(__VMS) && !defined(__rtems__) && !defined(__vxWorks__)
#include <strings.h>
#include <string.h>
#else
#include <string.h>
#endif
#include <omniORB4/CORBA_sysdep.h>
#include <omnithread.h>
#include <omniORB4/finalCleanup.h>
#include <omniORB4/CORBA_basetypes.h>
#include <omniORB4/seqTemplatedecls.h>
#include <omniORB4/templatedecls.h>
#include <omniORB4/valueTemplatedecls.h>
#include <omniORB4/stringtypes.h>
#include <omniORB4/wstringtypes.h>
#ifndef __IOP_hh_EXTERNAL_GUARD__
#include <omniORB4/IOP.h>
#define __IOP_hh_EXTERNAL_GUARD__
#endif
#include <omniORB4/GIOP.h>
#include <omniORB4/IIOP.h>
#include <omniORB4/omniObjKey.h>
#include <omniORB4/tracedthread.h>
#include <omniORB4/userexception.h>


class omniObjRef;
class omniServant;
class omniIOR;
class omniIORHints;
class omniObjTableEntry;
class omniLocalIdentity;
class omniRemoteIdentity;
class omniIdentity;

OMNI_NAMESPACE_BEGIN(omni)

class omniObjAdapter;

//
// omniORB_x_y
//   Define this variable to trap the mismatch of the stub and the runtime
//   library. The two digits x,y should be the same as the shared library
//   major version number and minor version number. For example, for shared
//   library 2.5.0 the variable number should be omniORB_2_5. Notice that
//   the variable name stays the same with compatible shared library, e.g.
//   2.5.1.
//
extern _core_attr const char* omniORB_4_2;
extern _dyn_attr  const char* omniORB_4_2_dyn;
extern _core_attr const _CORBA_ULong omniORB_TAG_ORB_TYPE; // ATT\x00

class Strand;
class Rope;
class IOP_S;
class IOP_C;

OMNI_NAMESPACE_END(omni)

//////////////////////////////////////////////////////////////////////
//////////////////////////////// omni ////////////////////////////////
//////////////////////////////////////////////////////////////////////

_CORBA_MODULE omni

_CORBA_MODULE_BEG

  typedef omni_ptr_arith_t ptr_arith_t;

  enum alignment_t { ALIGN_1 = 1, ALIGN_2 = 2, ALIGN_4 = 4, ALIGN_8 = 8 };

  _CORBA_MODULE_VAR _core_attr const _CORBA_Char                myByteOrder;
  _CORBA_MODULE_VAR _core_attr omni_tracedmutex*                internalLock;
  _CORBA_MODULE_VAR _core_attr omni_tracedmutex*                objref_rc_lock;

  _CORBA_MODULE_VAR _core_attr omni_tracedmutex*                poRcLock;
  // Psuedo-object ref count lock.

  _CORBA_MODULE_VAR _core_attr _CORBA_Unbounded_Sequence_Octet  myPrincipalID;
  _CORBA_MODULE_VAR _core_attr const alignment_t                max_alignment;
  // Maximum value of alignment_t

  _CORBA_MODULE_VAR _core_attr int remoteInvocationCount;
  _CORBA_MODULE_VAR _core_attr int localInvocationCount;
  // These are updated whilst internalLock is held.  However it is
  // suggested that they may be read without locking, since integer
  // reads are likely to be atomic.

  _CORBA_MODULE_VAR _core_attr int mainThreadId;
  // id of the main thread. 0 by default. Can be changed by calling
  // omniORB::setMainThread().


  _CORBA_MODULE_FN inline ptr_arith_t align_to(ptr_arith_t p,
					       alignment_t align) {
    return (p + ((ptr_arith_t) align - 1)) & ~((ptr_arith_t) align - 1);
  }

  _CORBA_MODULE_FN inline _CORBA_ULong hash(const _CORBA_Octet* key,
					    int keysize)
  {
    _CORBA_ULong n = 0;
    while( keysize-- )  n = ((n << 5) ^ (n >> 27)) ^ *key++;
    return n;
  }
  // Computes a hash of the object key.  The caller must ensure
  // that the returned value is bounded to the required range.

  _CORBA_MODULE_FN omni_tracedmutex& nilRefLock();
  // This is needed to ensure that the mutex is constructed by the
  // time it is first used.  This can occur at static initialisation
  // if a _var type is declared at global scope.

  _CORBA_MODULE_FN void duplicateObjRef(omniObjRef*);
  // Thread safe.

  _CORBA_MODULE_FN void releaseObjRef(omniObjRef*);
  // Must not hold <internalLock>.


  // Each of the reference creating functions below return a
  // reference which supports the c++ type interface give by
  // the repository id <targetRepoId> -- it must be a type for
  // which we have static information.  <mostDerivedRepoId> is
  // the interface repository ID recorded in the original IOR.
  // This may be the empty string (but *not* null).

  _CORBA_MODULE_FN omniIdentity* createIdentity(omniIOR* ior,
						const char* target,
						_CORBA_Boolean locked);
  // Create an identity object that can be used to invoke operations
  // on the CORBA object identified by <ior>. If the object is local
  // and activated, the servant is checked for compatibility with
  // <target>. If they are compatible, the localIdentity is returned;
  // otherwise, an inProcessIdentity is used.
  //
  // <ior> is always consumed even if the function returns 0.
  //
  // <locked> => hold <internalLock>.

  _CORBA_MODULE_FN omniIdentity* createInProcessIdentity(const _CORBA_Octet* k,
							 int keysize);
  // Returns an omniIdentity to contact an object in this address
  // space which is unsuitable for contact through a localIdentity.

  _CORBA_MODULE_FN omniObjRef* createObjRef(const char* targetRepoId,
					    omniIOR* ior,
					    _CORBA_Boolean locked,
					    omniIdentity* id = 0);
  // Returns an object reference identified by <ior>.  If <id> is not 0, it
  // is a readily available identity object.
  // Return 0 if a type error is detected and the object reference cannot be
  // created.
  //  <ior> is always consumed even if the function returns 0.
  //  <locked> => hold <internalLock>.

  _CORBA_MODULE_FN omniObjRef* createLocalObjRef(const char* mostDerivedRepoId,
						 const char* targetRepoId,
						 omniObjTableEntry* entry,
						 const omniIORHints& hints);
  // Return a reference to the specified activated local object.
  //  Must hold <internalLock>.

  _CORBA_MODULE_FN omniObjRef* createLocalObjRef(const char* mostDerivedRepoId,
						 const char* targetRepoId,
						 const _CORBA_Octet* key,
						 int keysize,
						 const omniIORHints& hints);
  // Return a reference to the local object with the given key, which
  // may or may not be active.
  //  Must hold <internalLock>.


  _CORBA_MODULE_FN void revertToOriginalProfile(omniObjRef* objref);
  // Reset the implementation of the reference to that stored
  // in the IOP profile.  Throws INV_OBJREF if cannot instantiate
  // a suitable rope.  (This is unlikely to happen, since we
  // suceeded when we made the ref the first time around -- but
  // someone may have been fiddling with the rope factories).
  //  Must not hold <internalLock>.

  _CORBA_MODULE_FN void locationForward(omniObjRef* obj,
					omniObjRef* new_location,
					_CORBA_Boolean permanent);
  // This function implements location forwarding.  The implementation
  // of <obj> is replaced by that in <new_location> (subject to the
  // usual type checks).  <new_location> is released before returning.
  //  <new_location> must not be nil.
  // From GIOP 1.2 onwards, location forward can either be temporary or
  // permanent. This is indicated by the permanent flag.
  //  Must not hold <internalLock>.
  //?? Error behaviour?

  _CORBA_MODULE_FN inline _CORBA_Boolean strMatch(const char* a,
						  const char* b)
  {
    do {
      if (*a != *b) return 0;
    } while (*a++ && *b++);
    return 1;
  }

  _CORBA_MODULE_FN inline _CORBA_Boolean ptrStrMatch(const char* a,
						     const char* b)
  {
    if (a==b) return 1;
    do {
      if (*a != *b) return 0;
    } while (*a++ && *b++);
    return 1;
  }
  // Surprisingly, having these inline string matching functions gives
  // a noticable performance improvement over using strcmp().
  // ptrStrMatch() is used in cases where it's likely that the string
  // pointers are the same; strMatch() is for cases where they
  // definitely aren't, and saves a little bit of inline code.


  _CORBA_MODULE_FN void assertFail(const char* file, int line, const char* exp);
  _CORBA_MODULE_FN void ucheckFail(const char* file, int line, const char* exp);

_CORBA_MODULE_END


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

#if 1

# define OMNIORB_ASSERT(e)  \
  do{ if( !(e) )  omni::assertFail(__FILE__,__LINE__, #e); }while(0)

# define OMNIORB_USER_CHECK(e)  \
  do{ if( !(e) )  omni::ucheckFail(__FILE__,__LINE__, #e); }while(0)

#else
# define OMNIORB_ASSERT(e)
# define OMNIORB_USER_CHECK(e)
#endif

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

#include <omniORB4/codeSets.h>
#include <omniORB4/cdrStream.h>
#include <omniORB4/seqTemplatedefns.h>
#include <omniORB4/valueTemplatedefns.h>
#include <omniORB4/omniObjRef.h>
#include <omniORB4/omniServer.h>
#include <omniORB4/proxyFactory.h>
#include <omniORB4/omniServant.h>


//??
template <class T>
class _CORBA_ConstrType_Variable_Var;


#endif // __OMNIINTERNAL_H__
