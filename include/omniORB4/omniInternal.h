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
  Revision 1.2.2.14  2002/02/13 16:04:01  dpg1
  Rearrange includes so omnithread sees system dependencies.

  Revision 1.2.2.13  2001/09/24 14:26:01  dpg1
  Safer static translation unit counts for omnithread and final clean-up.

  Revision 1.2.2.12  2001/09/19 17:26:43  dpg1
  Full clean-up after orb->destroy().

  Revision 1.2.2.11  2001/08/15 10:26:08  dpg1
  New object table behaviour, correct POA semantics.

  Revision 1.2.2.10  2001/08/01 10:08:19  dpg1
  Main thread policy.

  Revision 1.2.2.9  2001/05/31 16:18:11  dpg1
  inline string matching functions, re-ordered string matching in
  _ptrToInterface/_ptrToObjRef

  Revision 1.2.2.8  2001/05/29 17:03:48  dpg1
  In process identity.

  Revision 1.2.2.7  2001/05/10 15:08:37  dpg1
  _compatibleServant() replaced with _localServantTarget().
  createIdentity() now takes a target string.
  djr's fix to deactivateObject().

  Revision 1.2.2.6  2001/04/18 17:50:44  sll
  Big checkin with the brand new internal APIs.
  Scoped where appropriate with the omni namespace.

  Revision 1.2.2.5  2000/11/15 17:03:38  sll
  Added include codeSets.h.

  Revision 1.2.2.4  2000/11/03 18:58:47  sll
  Unbounded sequence of octet got a new type name.

  Revision 1.2.2.3  2000/10/27 15:42:03  dpg1
  Initial code set conversion support. Not yet enabled or fully tested.

  Revision 1.2.2.2  2000/09/27 17:21:02  sll
  Updated to use the new cdrStream abstraction.
  Replace include/omniORB3 with include/omniORB4.
  Changed the signatures of the createObjRef() functions to accept the
  omniIOR* argument.
  Removed the obsoluted omniRopeAndKey.

  Revision 1.2.2.1  2000/07/17 10:35:35  sll
  Merged from omni3_develop the diff between omni3_0_0_pre3 and omni3_0_0.

  Revision 1.3  2000/07/13 15:26:05  dpg1
  Merge from omni3_develop for 3.0 release.

  Revision 1.1.2.10  2000/06/27 16:15:08  sll
  New classes: _CORBA_String_element, _CORBA_ObjRef_Element,
  _CORBA_ObjRef_tcDesc_arg to support assignment to an element of a
  sequence of string and a sequence of object reference.

  Revision 1.1.2.9  2000/04/27 10:36:48  dpg1
  Interoperable Naming Service

  stringToObject() and objectToString() moved to omniURI.h.

  Revision 1.1.2.8  2000/02/22 12:25:37  dpg1
  A few things made `publicly' accessible so omniORBpy can get its hands
  on them.

  Revision 1.1.2.7  2000/02/08 13:25:23  djr
  Added a couple of missing _core_attr.

  Revision 1.1.2.6  2000/01/27 16:31:32  djr
  String_member now initialised to empty string by default.

  Revision 1.1.2.5  2000/01/20 11:51:33  djr
  (Most) Pseudo objects now used omni::poRcLock for ref counting.
  New assertion check OMNI_USER_CHECK.

  Revision 1.1.2.4  1999/10/29 13:18:09  djr
  Changes to ensure mutexes are constructed when accessed.

  Revision 1.1.2.3  1999/10/27 17:32:09  djr
  omni::internalLock and objref_rc_lock are now pointers.

  Revision 1.1.2.2  1999/09/24 15:01:27  djr
  Added module initialisers, and sll's new scavenger implementation.

  Revision 1.1.2.1  1999/09/24 09:51:45  djr
  Moved from omniORB2 + some new files.

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

#include <stddef.h>

#if !defined(__atmos__) && !defined(_WIN32) && !defined(__VMS) && !defined(__rtems__)
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
extern _core_attr const char* omniORB_4_0;
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


#if SIZEOF_PTR == SIZEOF_LONG
  typedef long ptr_arith_t;
#elif SIZEOF_PTR == SIZEOF_INT
  typedef int ptr_arith_t;
#else
#error "No suitable type to do pointer arithmetic"
#endif

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
    return (p + ((int) align - 1)) & ~((int) align - 1);
  }

  _CORBA_MODULE_FN _CORBA_ULong hash(const _CORBA_Octet* key, int keysize);
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

  _CORBA_MODULE_FN omniIdentity* createLoopBackIdentity(omniIOR* ior,
							const _CORBA_Octet* k,
							int keysize);
  // Returns an instance of omniRemoteIdentity to contact a local
  // object identified by the call arguments. <ior> is consumed.

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
						 omniObjTableEntry* entry);
  // Return a reference to the specified activated local object.
  //  Must hold <internalLock>.

  _CORBA_MODULE_FN omniObjRef* createLocalObjRef(const char* mostDerivedRepoId,
						 const char* targetRepoId,
						 const _CORBA_Octet* key,
						 int keysize);
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
#include <omniORB4/omniObjRef.h>
#include <omniORB4/omniIOR.h>
#include <omniORB4/proxyFactory.h>
#include <omniORB4/omniServant.h>


//??
template <class T>
class _CORBA_ConstrType_Variable_Var;


#endif // __OMNIINTERNAL_H__
