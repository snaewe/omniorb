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

#if !defined(__atmos__) && !defined(_WIN32) && !defined(__VMS)
#include <strings.h>
#include <string.h>
#else
#include <string.h>
#endif
#include <omnithread.h>
#include <omniORB3/CORBA_sysdep.h>
#include <omniORB3/CORBA_basetypes.h>
#include <omniORB3/seqtemplates.h>
#include <omniORB3/GIOP.h>
#include <omniORB3/IIOP.h>
#include <omniORB3/omniObjKey.h>
#include <omniORB3/tracedthread.h>


// Forward declarations.
class Rope;
class GIOP_S;
class GIOP_C;
class omniObjRef;
class omniServant;
class omniIdentity;
class omniLocalIdentity;
class omniObjAdapter;
class NetBufferedStream;
class MemBufferedStream;


//
// omniORB_x_y
//   Define this variable to trap the mismatch of the stub and the runtime
//   library. The two digits x,y should be the same as the shared library
//   major version number and minor version number. For example, for shared
//   library 2.5.0 the variable number should be omniORB_2_5. Notice that
//   the variable name stays the same with compatible shared library, e.g.
//   2.5.1.
//
extern _core_attr const char* omniORB_3_0;


#include <omniORB3/rope.h>


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

  enum alignment_t { ALIGN_1 = 1, ALIGN_2 = 2, ALIGN_4 = 4, ALIGN_8 = 8 };


  static _core_attr const _CORBA_Char                myByteOrder;
  static _core_attr omni_tracedmutex*                internalLock;
  static _core_attr omni_tracedmutex                 nilRefLock;
  static _core_attr _CORBA_Unbounded_Sequence__Octet myPrincipalID;
  static _core_attr const alignment_t                max_alignment;
  // Maximum value of alignment_t

  static _core_attr int                              remoteInvocationCount;
  static _core_attr int                              localInvocationCount;


  static inline ptr_arith_t align_to(ptr_arith_t p, alignment_t align) {
    return (p + ((int) align - 1)) & ~((int) align - 1);
  }

  static _CORBA_ULong hash(const _CORBA_Octet* key, int keysize);
  // Computes a hash of the object key.  The caller must ensure
  // that the returned value is bounded to the required range.

  static inline char* allocString(int len) { return new char[len + 1]; }
  // Allocate a string -- as CORBA::string_alloc(), except that
  // we don't initialise to empty string.
  //  <len> does not include nul terminator.

  static inline void freeString(char* s) { delete[] s; }
  // As CORBA::string_free().

  static void duplicateObjRef(omniObjRef*);
  // Thread safe.

  static void releaseObjRef(omniObjRef*);
  // Must not hold <internalLock>.

  static omniServant* objRefToServant(omniObjRef* obj);
  // Returns the servant associated with the given reference in
  // the object table.  Returns 0 if this is not a reference to
  // a local object, or the object is not incarnated.  Does not
  // increment the reference count of the servant.
  //  Must hold <internalLock>.

  static omniLocalIdentity* locateIdentity(const _CORBA_Octet* key,
					   int keysize, _CORBA_ULong hash,
					   _CORBA_Boolean create_dummy = 0);
  // Searches the servant table for the given key.  Returns the identity
  // if found.  If not and <create_dummy> is true, it creates a dummy
  // identity and puts it in the table -- and returns that.  Does not
  // increment the ref count of the id if found.
  //  Must hold <internalLock>.

  static omniLocalIdentity* activateObject(omniServant* servant,
					   omniObjAdapter* adapter,
					   omniObjKey& key);
  // Insert the given servant into the object table.  Returns 0 if
  // an object has already been activated with the given key.  May
  // consume <key>.  On success, consumes <servant> and returns the
  // identity of the object.
  //  NB. The adapter *must* outlive all its objects -- so a
  // reference to it is not needed.
  //  Must hold <internalLock>.
  //  This function does not throw any exceptions.

  static omniLocalIdentity* deactivateObject(const _CORBA_Octet* key,
					     int keysize);
  // Deactivate the object with the given key.  Returns 0
  // if the given object was not active, otherwise returns
  // the identity.  The caller should decrement the reference
  // count of the returned id when finished with it.
  //  Must hold <internalLock>.
  //  This function does not throw any exceptions.

  // Each of the reference creating functions below return a
  // reference which supports the c++ type interface give by
  // the repository id <targetRepoId> -- it must be a type for
  // which we have static information.  <mostDerivedRepoId> is
  // the interface repository ID recorded in the original IOR.
  // This may be the empty string (but *not* null).
  //  If <targetRepoId> is neither equal to <mostDerivedRepoId>
  // nor the latter is a derived interface of the former, these
  // methods return 0.

  static omniObjRef* createObjRef(const char* mostDerivedRepoId,
				  const char* targetRepoId,
				  IOP::TaggedProfileList* profiles,
				  _CORBA_Boolean release_profiles,
				  _CORBA_Boolean locked);
  // Returns an object reference identified by <profiles>.  If the
  // object is not local, and a suitable outgoing rope could not
  // be instantiated, then 0 is returned.
  //  If <release_profiles> is true then <profiles> is consumed,
  // even in the case of error.
  //  <locked> =>> hold <internalLock>.

  static omniObjRef* createObjRef(const char* mostDerivedRepoId,
				  const char* targetRepoId,
				  omniLocalIdentity* id,
				  IOP::TaggedProfileList* profiles = 0,
				  _CORBA_Boolean release_profiles = 0,
				  _CORBA_Octet* key = 0);
  // Return a reference to the given local object (which may or
  // may not have an entry in the local object table).  May
  // return an existing reference if a suitable one exists, or
  // may create a new one.
  //  <profiles> and <key> may be passed if the calling context
  // has dynamically allocated versions available.  This does not
  // change the semantics in any way.  If given, <key> is consumed.
  // If <profiles> is given it is consumed if <release_profiles> is
  // true.  
  //  Must hold <internalLock>.

  static void revertToOriginalProfile(omniObjRef* objref);
  // Reset the implementation of the reference to that stored
  // in the IOP profile.  Throws INV_OBJREF if cannot instantiate
  // a suitable rope.  (This is unlikely to happen, since we
  // suceeded when we made the ref the first time around -- but
  // someone may have been fiddling with the rope factories).
  //  Must not hold <internalLock>.

  static void locationForward(omniObjRef* obj, omniObjRef* new_location);
  // This function implements location forwarding.  The implementation
  // of <obj> is replaced by that in <new_location> (subject to the
  // usual type checks).  <new_location> is released before returning.
  //  <new_location> must not be nil.
  //  Must not hold <internalLock>.
  //?? Error behaviour?

  static int stringToObject(omniObjRef*& objref, const char* ior);
  // Converts the given IOR to an object reference.  Returns 0
  // on error.  If the IOR is a nil reference, <objref> is zero.
  //  Does not throw any exceptions.

  static char* objectToString(omniObjRef*);
  // Returns a stringified IOR for the object reference given.
  //  Does not throw any exceptions.

  static void assertFail(const char* file, int line, const char* exp);

};

//////////////////////////////////////////////////////////////////////
/////////////////////////// omniRopeAndKey ///////////////////////////
//////////////////////////////////////////////////////////////////////

class omniRopeAndKey {
public:
  inline omniRopeAndKey(Rope* r, const _CORBA_Octet* k, int ksize)
    : pd_rope(r), pd_key(k, ksize)
    { r->incrRefCount(); }

  inline omniRopeAndKey() : pd_rope(0)  {}

  inline ~omniRopeAndKey() { if( pd_rope )  pd_rope->decrRefCount(); }

  inline Rope* rope() const { return pd_rope; }
  inline const _CORBA_Octet* key() const { 
    return pd_key.key();
  }
  inline _CORBA_ULong keysize() const { return pd_key.size(); }

  inline void rope(Rope* r) { pd_rope = r; r->incrRefCount(); }
  inline void key(const _CORBA_Octet* k, int ksize) { pd_key.copy(k, ksize); }

private:
  omniRopeAndKey& operator=(const omniRopeAndKey&);
  omniRopeAndKey(const omniRopeAndKey&);
  // Not implemented.

  Rope*      pd_rope;
  omniObjKey pd_key;
};

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

#if 1
#define OMNIORB_ASSERT(e)  \
  if( !(e) )  omni::assertFail(__FILE__,__LINE__, #e)
#else
#define OMNIORB_ASSERT(e)
#endif

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

#include <omniORB3/bufferedStream.h>
#include <omniORB3/giopDriver.h>
#include <omniORB3/omniObjRef.h>
#include <omniORB3/proxyFactory.h>
#include <omniORB3/omniServant.h>


//??
template <class T>
class _CORBA_ConstrType_Variable_Var;


#endif // __OMNIINTERNAL_H__
