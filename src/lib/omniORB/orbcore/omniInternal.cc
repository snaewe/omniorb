// -*- Mode: C++; -*-
//                            Package   : omniORB
// omniInternal.cc            Created on: 25/2/99
//                            Author    : David Riddoch (djr)
//
//    Copyright (C) 1996, 1999 AT&T Research Cambridge
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
//    Implementation of methods defined in class omni.
//      

/*
  $Log$
  Revision 1.2.2.12  2001/07/13 15:29:59  sll
  Use the variable omniORB::maxServerThreadPoolSize to control invoker thread
  pool size.

  Revision 1.2.2.11  2001/05/31 16:18:13  dpg1
  inline string matching functions, re-ordered string matching in
  _ptrToInterface/_ptrToObjRef

  Revision 1.2.2.10  2001/05/29 17:03:52  dpg1
  In process identity.

  Revision 1.2.2.9  2001/05/10 15:08:37  dpg1
  _compatibleServant() replaced with _localServantTarget().
  createIdentity() now takes a target string.
  djr's fix to deactivateObject().

  Revision 1.2.2.8  2001/05/09 17:05:34  sll
  createIdentity() now can deal with it being called more than once with the
  same IOR.

  Revision 1.2.2.7  2001/04/18 18:18:06  sll
  Big checkin with the brand new internal APIs.

  Revision 1.2.2.6  2000/11/09 12:27:57  dpg1
  Huge merge from omni3_develop, plus full long long from omni3_1_develop.

  Revision 1.2.2.5  2000/11/03 19:14:03  sll
  Use _CORBA_Unbounded_Sequence_Octet instead of _CORBA_Unbounded_Sequence__Octet

  Revision 1.2.2.4  2000/10/09 16:22:47  sll
  Updated the usage of omniIOR duplicate and release to conform to the
  current locking requirement.

  Revision 1.2.2.3  2000/10/03 17:37:08  sll
  Changed omniIOR synchronisation mutex from omni::internalLock to its own
  mutex.

  Revision 1.2.2.2  2000/09/27 18:39:33  sll
  Updated to use omniIOR to store and pass the repository ID and IOP profiles
  of an IOR.

  Revision 1.1.2.16  2000/07/21 15:35:47  dpg1
  Incorrectly rejected object references with incompatible target and
  most-derived repoIds.

  Revision 1.1.2.15  2000/06/27 15:40:57  sll
  Workaround for Cygnus gcc's inability to recognise _CORBA_Octet*& and
  CORBA::Octet*& are the same type.

  Revision 1.1.2.14  2000/06/22 10:40:16  dpg1
  exception.h renamed to exceptiondefs.h to avoid name clash on some
  platforms.

  Revision 1.1.2.13  2000/04/27 10:51:13  dpg1
  Interoperable Naming Service

  stringToObject() and objectToString() moved to uri.cc.

  Revision 1.1.2.12  2000/04/13 17:48:58  djr
  Minor improvement -- reduces times when need to verify object's type.

  Revision 1.1.2.11  2000/03/01 17:57:41  dpg1
  New omniObjRef::_compatibleServant() function to support object
  references and servants written for languages other than C++.

  Revision 1.1.2.10  2000/02/22 12:25:38  dpg1
  A few things made `publicly' accessible so omniORBpy can get its hands
  on them.

  Revision 1.1.2.9  2000/01/27 16:31:33  djr
  String_member now initialised to empty string by default.

  Revision 1.1.2.8  2000/01/20 11:51:35  djr
  (Most) Pseudo objects now used omni::poRcLock for ref counting.
  New assertion check OMNI_USER_CHECK.

  Revision 1.1.2.7  1999/12/06 14:03:00  djr
  *** empty log message ***

  Revision 1.1.2.6  1999/10/29 13:18:18  djr
  Changes to ensure mutexes are constructed when accessed.

  Revision 1.1.2.5  1999/10/27 17:32:13  djr
  omni::internalLock and objref_rc_lock are now pointers.

  Revision 1.1.2.4  1999/10/14 16:22:13  djr
  Implemented logging when system exceptions are thrown.

  Revision 1.1.2.3  1999/09/27 08:48:33  djr
  Minor corrections to get rid of warnings.

  Revision 1.1.2.2  1999/09/24 15:01:35  djr
  Added module initialisers, and sll's new scavenger implementation.

  Revision 1.1.2.1  1999/09/22 14:26:57  djr
  Major rewrite of orbcore to support POA.

*/

#include <omniORB4/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <omniORB4/proxyFactory.h>
#include <omniORB4/omniServant.h>
#include <localIdentity.h>
#include <inProcessIdentity.h>
#include <remoteIdentity.h>
#include <objectAdapter.h>
#include <anonObject.h>
#include <initialiser.h>
#include <exceptiondefs.h>
#include <omniORB4/omniInterceptors.h>
#include <giopRope.h>
#include <invoker.h>


OMNI_USING_NAMESPACE(omni)

#if defined(HAS_Cplusplus_Namespace)
using omniORB::operator==;
#endif


const CORBA::Char                omni::myByteOrder = _OMNIORB_HOST_BYTE_ORDER_;
omni_tracedmutex*                omni::internalLock = 0;
omni_tracedmutex*                omni::poRcLock = 0;
_CORBA_Unbounded_Sequence_Octet  omni::myPrincipalID;
const omni::alignment_t          omni::max_alignment = ALIGN_8;

int                              omni::remoteInvocationCount = 0;
int                              omni::localInvocationCount = 0;

omni_tracedmutex*                omni::objref_rc_lock = 0;
// Protects omniObjRef reference counting.

OMNI_NAMESPACE_BEGIN(omni)

omniAsyncInvoker*                orbAsyncInvoker = 0;

// The local object table.  This is a dynamically resized
// open hash table.
static omniLocalIdentity**       objectTable = 0;
static _CORBA_ULong              objectTableSize = 0;
static int                       objectTableSizeI = 0;
static _CORBA_ULong              numObjectsInTable = 0;
static _CORBA_ULong              maxNumObjects = 0;
static _CORBA_ULong              minNumObjects = 0;

// Some sort of magic numbers that are supposed
// to be good for hash tables...
static int objTblSizes[] = {
  128 + 3,              // 2^7
  1024 + 9,             // 2^10
  8192 + 27,            // 2^13
  32768 + 3,            // 2^15
  65536 + 45,           // 2^16
  131072 + 9,
  262144 + 39,
  524288 + 39,
  1048576 + 9,          // 2^20
  2097152 + 5,
  4194304 + 3,
  8388608 + 33,
  16777216 + 27,
  33554432 + 9,         // 2^25
  67108864 + 71,
  134217728 + 39,
  268435456 + 9,
  536870912 + 5,
  1073741824 + 83       // 2^30 -- I'd be suprised if this is exceeded!
};

//////////////////////////////////////////////////////////////////////
//////////////////////////// omniInternal ////////////////////////////
//////////////////////////////////////////////////////////////////////

//: Just a few helper methods. Local to this file.

class omniInternal {
public:

  static void replaceImplementation(omniObjRef* objref,
				    omniIdentity* id,
				    omniLocalIdentity* local_id);
  // Removes the old identity from an object reference,
  // replacing it with a new one.  <local_id> must be nil if
  // the object implementation is not local.  If the new
  // identity is the same one as the old one, then this does
  // nothing.
  //  <objref->pd_flags.forward_location> and <type_verified>
  // are not modified by this function.
  //  Must hold <omni::internalLock>.

  static void removeAndDestroyDummyId(omniLocalIdentity* id);
  // Remove a dummy entry from the object table.  Consumes <id>.
  //  Must hold <omni::internalLock>.

  static void resizeObjectTable();
  // Does what it says on the can!
  //  Must hold <omni::internalLock>.

};


void
omniInternal::replaceImplementation(omniObjRef* objref, omniIdentity* id,
				    omniLocalIdentity* local_id)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);
  OMNIORB_ASSERT(objref);
  OMNIORB_ASSERT(id);

  omniLocalIdentity* old_lid = objref->_localId();
  omniIdentity* old_id = objref->_identity();

  if( old_id != id )  old_id->loseObjRef(objref);
  if( old_lid && old_lid != old_id )
    old_lid->omniLocalIdentity::loseObjRef(objref);

  objref->_setIdentity(id, local_id);

  if( old_id != id )  id->gainObjRef(objref);
  if( local_id && local_id != id )
    local_id->omniLocalIdentity::gainObjRef(objref);

  if( old_lid && !old_lid->servant() && !old_lid->localRefList() )
    removeAndDestroyDummyId(old_lid);
}


void
omniInternal::removeAndDestroyDummyId(omniLocalIdentity* id)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);
  OMNIORB_ASSERT(id);

  omniORB::logs(15, "Removing dummy entry from object table -- no local refs");

  omniLocalIdentity** pid = objectTable +
    omni::hash(id->key(), id->keysize()) % objectTableSize;

  while( *pid != id ) {
    OMNIORB_ASSERT(*pid);
    pid = (*pid)->addrOfNextInObjectTable();
  }

  *pid = id->nextInObjectTable();
  --numObjectsInTable;

  id->finishedWithDummyId();
}


void
omniInternal::resizeObjectTable()
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);
  OMNIORB_ASSERT(numObjectsInTable > maxNumObjects ||
		 numObjectsInTable < minNumObjects && objectTableSizeI > 0);

  if( numObjectsInTable > maxNumObjects )  ++objectTableSizeI;
  else                                     --objectTableSizeI;

  _CORBA_ULong newsize = objTblSizes[objectTableSizeI];

  if( omniORB::trace(15) ) {
    omniORB::logf("Object table resizing from %lu to %lu",
		  (unsigned long) objectTableSize,
		  (unsigned long) newsize);
  }

  // Create and initialise new object table.
  omniLocalIdentity** newtable = new omniLocalIdentity* [newsize];
  CORBA::ULong i;
  for( i = 0; i < newsize; i++ )  newtable[i] = 0;

  // Move the objects across...
  for( i = 0; i < objectTableSize; i++ ) {
    omniLocalIdentity* id = objectTable[i];

    while( id ) {
      omniLocalIdentity* next = id->nextInObjectTable();

      _CORBA_ULong j = omni::hash(id->key(), id->keysize()) % newsize;
      *(id->addrOfNextInObjectTable()) = newtable[j];
      newtable[j] = id;

      id = next;
    }
  }

  // Replace the old table.
  delete[] objectTable;
  objectTable = newtable;
  objectTableSize = newsize;
  maxNumObjects = objectTableSize * 2 / 3;
  minNumObjects =
    objectTableSizeI ? (objTblSizes[objectTableSizeI - 1] / 3) : 0;
}

OMNI_NAMESPACE_END(omni)

//////////////////////////////////////////////////////////////////////
//////////////////////////////// omni ////////////////////////////////
//////////////////////////////////////////////////////////////////////

_CORBA_ULong
omni::hash(const CORBA::Octet* key, int keysize)
{
  //?? This is from Knuth.  We may be able to do better!

  _CORBA_ULong n = 0;

  while( keysize-- )  n = ((n << 5) ^ (n >> 27)) ^ *key++;

  return n;
}


omni_tracedmutex&
omni::nilRefLock()
{
  // We are safe just testing this here, as we guarentee that
  // it will be initialised during the static initialisation.
  // (Which is single-threaded).  If not by this method, then
  // by the static initialiser below.

  static omni_tracedmutex* nil_ref_lock = 0;

  if( !nil_ref_lock )  nil_ref_lock = new omni_tracedmutex;
  return *nil_ref_lock;
}


void
omni::duplicateObjRef(omniObjRef* objref)
{
  OMNIORB_ASSERT(objref);

  objref_rc_lock->lock();
  objref->pd_refCount++;
  objref_rc_lock->unlock();
}


void
omni::releaseObjRef(omniObjRef* objref)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*internalLock, 0);
  OMNIORB_ASSERT(objref);

  objref_rc_lock->lock();
  int rc = --objref->pd_refCount;
  objref_rc_lock->unlock();

  if( rc > 0 )  return;

  if( rc < 0 ) {
    omniORB::logs(1,
      "ERROR -- trying to release an object with reference count <= 0.\n"
      " CORBA::release() may have been called too many times on an object\n"
      " reference.");
    return;
  }

  // If this is a reference to a local object, remove this reference
  // from the object's id's list of local references.
  //  NB. We have to be darn sure that no-one tries to copy this
  // reference from the local ref list.  The only place this is
  // done is in omni::createObjRef() -- which checks that it is
  // safe first.

  omni::internalLock->lock();

  omniLocalIdentity* lid = objref->_localId();

  if( objref->_identity() != lid )
    objref->_identity()->loseObjRef(objref);

  if( lid ) {

    lid->omniLocalIdentity::loseObjRef(objref);

    if( omniORB::trace(15) ) {
      omniORB::logger l;
      l << "Ref to: " << lid << " -- deleted.\n";
    }

    // If this id is a dummy entry (ie. the object is not activated)
    // and there are no more local references, we can remove it from
    // the object table.
    if( !lid->servant() && !lid->localRefList() )
      omniInternal::removeAndDestroyDummyId(lid);
  }

  internalLock->unlock();

  if( !lid && omniORB::trace(15) )
    omniORB::logf("ObjRef(%s) -- deleted.", objref->_mostDerivedRepoId());

  // Destroy the reference.
  delete objref;
}


omniServant*
omni::objRefToServant(omniObjRef* objref)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*internalLock, 1);
  OMNIORB_ASSERT(objref);

  omniLocalIdentity* lid = objref->_localId();
  if( !lid )  return 0;

  return lid ? lid->servant() : 0;
}


omniLocalIdentity*
omni::locateIdentity(const CORBA::Octet* key, int keysize, _CORBA_ULong hashv,
		     _CORBA_Boolean create_dummy)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*internalLock, 1);

  omniLocalIdentity** head = objectTable + hashv % objectTableSize;
  omniLocalIdentity* id = *head;

  while( id ) {
    if( id->is_equal(key, keysize) )  return id;

    id = id->nextInObjectTable();
  }

  if( !create_dummy )  return 0;

  if( ++numObjectsInTable > maxNumObjects ) {
    omniInternal::resizeObjectTable();
    head = objectTable + hashv % objectTableSize;
  }

  id = new omniLocalIdentity(key, keysize);
  *(id->addrOfNextInObjectTable()) = *head;
  *head = id;

  return id;
}


omniLocalIdentity*
omni::activateObject(omniServant* servant, omniObjAdapter* adapter,
		     omniObjKey& key)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*internalLock, 1);

  CORBA::ULong hashv = hash(key.key(), key.size());

  omniLocalIdentity* id = locateIdentity(key.key(), key.size(), hashv, 1);

  if( id->servant() )  return 0;

  id->setServant(servant, adapter);
  servant->_addIdentity(id);

  omniObjRef* objreflist = id->localRefList();

  if( omniORB::trace(10) ) {
    omniORB::logger l;
    l << "Activating: " << id;
    if( objreflist )  l << " (has local refs)";
    l << '\n';
  }

  while( objreflist ) {
    objreflist->pd_flags.object_exists = 1;
    if (servant->_ptrToInterface(objreflist->_localServantTarget())) {
      omniInternal::replaceImplementation(objreflist, id, id);
      objreflist->pd_flags.type_verified = 1;
    }
    else {
      OMNIORB_ASSERT(objreflist->_identity() != objreflist->_localId());
      objreflist->pd_flags.type_verified = 0;
    }
    objreflist = *(objreflist->_addrOfLocalRefList());
  }

  return id;
}


omniLocalIdentity*
omni::deactivateObject(const CORBA::Octet* key, int keysize)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*internalLock, 1);

  omniLocalIdentity** pid = objectTable + hash(key, keysize) % objectTableSize;

  while( *pid ) {
    if( (*pid)->is_equal(key, keysize) )
      break;

    pid = (*pid)->addrOfNextInObjectTable();
  }

  omniLocalIdentity* id = *pid;
  if( !(id && id->servant()) )  return 0;

  omniObjRef* objreflist = id->localRefList();

  if( objreflist ) {

    if( omniORB::trace(10) ) {
      omniORB::logger l;
      l << "Deactivating: " << id << " (has local refs).\n";
    }

    // Replace the object table entry with a dummy entry.

    omniLocalIdentity* newid = new omniLocalIdentity(id->key(), id->keysize());

    *newid->addrOfNextInObjectTable() = id->nextInObjectTable();
    *pid = newid;
    *id->addrOfNextInObjectTable() = 0;

    // Go through each local reference, and replace its
    // implementation with a loop-back (if not already
    // using one).

    omniIdentity* remote_id = 0;

    while( objreflist ) {
      omniObjRef* next = *(objreflist->_addrOfLocalRefList());
      objreflist->pd_flags.object_exists = 0;

      if( !remote_id ) {
	int has_remote_id = objreflist->_identity() != objreflist->_localId();

	if( has_remote_id )
	  remote_id = objreflist->_identity();
	else {
	  remote_id = createInProcessIdentity(newid->key(),newid->keysize());
	}
      }
      omniInternal::replaceImplementation(objreflist, remote_id, newid);
      objreflist = next;
    }
    OMNIORB_ASSERT(id->localRefList() == 0);
  }
  else {
    if( omniORB::trace(10) ) {
      omniORB::logger l;
      l << "Deactivating: " << id << '\n';
    }

    // No local refs to the id -- just remove it from the table.
    *pid = id->nextInObjectTable();

    if( --numObjectsInTable < minNumObjects )
      omniInternal::resizeObjectTable();
  }

  // Remove <id> from the servants list of identities.
  id->servant()->_removeIdentity(id);

  return id;
}


omniIdentity*
omni::createIdentity(omniIOR* ior,omniLocalIdentity*& local_id,
		     const char* target, CORBA::Boolean locked) {

  omniIOR_var holder(ior); // Place the ior inside a var. If ever
                           // any function we called results in an
                           // exception being thrown, the ior is released
                           // by the var, hence fullifilling the semantics
                           // of this function.
                           // If this function completed normally, make
                           // sure that _retn() is called on the var so
                           // that the ior is not released incorrectly!

  if (local_id) {
    // We are told this is a local object, check to see if we can just
    // use the localIdentity or we have to use an in process identity
    if (local_id->servant() && local_id->servant()->_ptrToInterface(target)) {
      return local_id;
    }
    else {
      // Return in process identity
      return createInProcessIdentity(local_id->key(),local_id->keysize());
    }
  }

  // Reach here means that we have to look into the IOR to find out
  // what identity object to return.

  omniIdentity* result = 0;
  {
    omniInterceptors::createIdentity_T::info_T info(ior,local_id,
						    result,locked);
    omniORB::getInterceptors()->createIdentity.visit(info);
  }
  if (result) {
    holder._retn();
    return result;
  }

  const IOP::TaggedProfileList& profiles = ior->iopProfiles();

  if (ior->addr_selected_profile_index() < 0) {

    // Pick the first TAG_INTERNET_IOP profile

    CORBA::ULong total = profiles.length();
    CORBA::ULong index;
    for (index = 0; index < total; index++) {
      if ( profiles[index].tag == IOP::TAG_INTERNET_IOP ) break;
    }
    if (index == total) {
      return 0;
    }
    ior->addr_selected_profile_index(index);
  }

  omniIOR::IORInfo* info = ior->getIORInfo();
  // getIORInfo() has the side effect of decoding the selected
  // TAG_INTERNET_IOP profile and any IOP::TAG_MULTIPLE_COMPONENTS
  // if that has not been done already.
  //
  // We use this function to trigger the decoding, instead of say
  // calling omniIOR::decodeIOPprofile() first, because createIdentity
  // may be called with the same ior more than once. It is highly
  // undesirable if the IOP profile is decoded in each of these calls.
  // Not only is this inefficent but doing so would create a thread safety
  // nightmare.

  CORBA::Boolean is_local = 0;
  Rope* rope;

  if (_OMNI_NS(giopRope)::selectRope(ior->getIORInfo()->addresses(),
				     rope,is_local) == 0) {
    return 0;
  }

  _CORBA_Unbounded_Sequence_Octet object_key;

  IIOP::unmarshalObjectKey(profiles[ior->addr_selected_profile_index()],
			   object_key);

  if (is_local) {

    CORBA::ULong hashv = hash(object_key.get_buffer(),object_key.length());
    omni_optional_lock sync(*internalLock,locked,locked);
    // If the identity does not exist in the local object table, this will
    // insert a dummy entry
    local_id = locateIdentity(object_key.get_buffer(),object_key.length(),
			      hashv, 1);

    if (local_id->servant() && local_id->servant()->_ptrToInterface(target)) {
      return local_id;
    }
    else {
      // Return in process identity
      return createInProcessIdentity(local_id->key(),local_id->keysize());
    }
  }
  else {
    holder._retn();
    result = new omniRemoteIdentity(ior,
				    object_key.get_buffer(),
				    object_key.length(),
				    rope);
    return result;
  }
}


omniIdentity*
omni::createInProcessIdentity(const _CORBA_Octet* key,int keysize){
  return new omniInProcessIdentity(key,keysize);
}


omniIdentity*
omni::createLoopBackIdentity(omniIOR* ior,const _CORBA_Octet* key,int keysize){
  Rope* rope = omniObjAdapter::defaultLoopBack();
  rope->incrRefCount();
  return new omniRemoteIdentity(ior,key,keysize,rope);
}

omniObjRef*
omni::createObjRef(const char* targetRepoId,
		   omniIOR* ior,
		   CORBA::Boolean locked,
		   omniIdentity* id,
		   omniLocalIdentity* local_id)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*internalLock, locked);
  OMNIORB_ASSERT(targetRepoId);
  OMNIORB_ASSERT(ior);

  if (!id) {
    ior->duplicate();  // consumed by createIdentity
    id = omni::createIdentity(ior,local_id,targetRepoId,locked);
    if ( !id ) {
      ior->release();
      return 0;
    }
  }

  proxyObjectFactory* pof = proxyObjectFactory::lookup(ior->repositoryID());

  if( pof && !pof->is_a(targetRepoId) &&
      !omni::ptrStrMatch(targetRepoId, CORBA::Object::_PD_repoId) ) {

    // We know that <mostDerivedRepoId> is not derived from
    // <targetRepoId>. 

    OMNIORB_ASSERT(id != local_id);
    // createIdentity() should have made sure that the local id is
    // compatible with the target type.

    pof = 0;
  }

  // Once we reach here:
  // if (pof != 0)
  //    pof points to the proxy factory that is an exact match to
  //    the interface identified by the ior's repository ID and it has been
  //    verified that the interface identified by <targetRepoId> is
  //    equal to or is a base class of the ior's repository ID.
  // else
  //    there is no proxy factory linked into this executable that
  //    matches the interface identified by the ior's repository ID.
  // or
  //    there _is_ a proxy factory for ior's repository ID, but we
  //    know that it is not derived from <targetRepoId>. We must
  //    contact the object, in case it actually supports an interface
  //    derived from both the ior's type and <targetRepoId>.

  int target_intf_not_confirmed = 0;

  if( !pof ) {
    pof = proxyObjectFactory::lookup(targetRepoId);
    OMNIORB_ASSERT(pof);
    // The assertion above will fail if your application attempts to
    // create an object reference while another thread is shutting
    // down the ORB.

    if( !omni::ptrStrMatch(targetRepoId, CORBA::Object::_PD_repoId) )
      target_intf_not_confirmed = 1;
  }

  if( omniORB::trace(10) ) {
    omniORB::logger l;
    l << "Creating ref to " << ((id == local_id) ? "local" : "remote")
      << ": " << id << "\n"
      " target id      : " << targetRepoId << "\n"
      " most derived id: " << (const char*)ior->repositoryID() << "\n";
  }

  // Now create the object reference itself.

  omniObjRef* objref = pof->newObjRef(ior, id, local_id);
  if( target_intf_not_confirmed )  objref->pd_flags.type_verified = 0;

  {
    omni_optional_lock sync(*internalLock, locked, locked);
    if (id != local_id)
      id->gainObjRef(objref);
    if (local_id)
      local_id->gainObjRef(objref);
  }

  return objref;
}

omniObjRef*
omni::createObjRef(const char* mostDerivedRepoId,
		   const char* targetRepoId,
		   omniLocalIdentity* local_id)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*internalLock, 1);
  OMNIORB_ASSERT(targetRepoId);
  OMNIORB_ASSERT(local_id);

  omniIdentity* id = 0;

  // See if a suitable reference exists in the local ref list.
  // Suitable means having the same most-derived-intf-repo-id, and
  // also supporting the <targetRepoId>.
  {
    omniObjRef* objref = local_id->localRefList();

    while( objref ) {

      if( omni::ptrStrMatch(mostDerivedRepoId, objref->_mostDerivedRepoId()) &&
	  objref->_ptrToObjRef(targetRepoId) ) {

	omniORB::logs(15, "createObjRef -- reusing reference from local"
		      " ref list.");

	// We just need to check that the ref count is not zero here,
	// 'cos if it is then the objref is about to be deleted!
	// See omni::releaseObjRef().

	objref_rc_lock->lock();
	int dying = objref->pd_refCount == 0;
	if( !dying )  objref->pd_refCount++;
	objref_rc_lock->unlock();

	if( !dying ) {
	  return objref;
	}
      }

      if( objref->_identity() != objref->_localId() )
	// If there is a id available, just keep a
	// note of it in case we need it.
	id = objref->_identity();

      objref = objref->_nextInLocalRefList();
    }
  }
  
  // Reach here if we have to create a new objref.
  omniIOR* ior = new omniIOR(mostDerivedRepoId,local_id);
  return createObjRef(targetRepoId,ior,1,id,local_id);
}


void
omni::revertToOriginalProfile(omniObjRef* objref)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*internalLock, 0);
  OMNIORB_ASSERT(objref);

  omniORB::logs(10, "Reverting object reference to original profile");

  omniIOR_var ior = objref->_getIOR();

  omni_tracedmutex_lock sync(*internalLock);

  // We might have already been reverted... We check here
  // rather than sooner, so as to avoid holding <internalLock>
  // longer than necassary.
  if( !objref->pd_flags.forward_location ) {
    return;
  }

  ior->duplicate(); // consumed by createIdentity
  omniLocalIdentity* local_id = 0;
  omniIdentity* id = omni::createIdentity(ior, local_id,
					  objref->_localServantTarget(), 1);
  if( !id ) {
    OMNIORB_THROW(INV_OBJREF,0, CORBA::COMPLETED_NO);
  }

  // For efficiency lets just assume that it exists.  We are
  // about to retry anyway -- so we'll soon find out!
  objref->pd_flags.forward_location = 0;
  objref->pd_flags.type_verified = 1;
  objref->pd_flags.object_exists = 1;

  omniInternal::replaceImplementation(objref, id, local_id);
}


void
omni::locationForward(omniObjRef* objref, omniObjRef* new_location,
		      CORBA::Boolean permanent)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*internalLock, 0);
  OMNIORB_ASSERT(objref);
  OMNIORB_ASSERT(new_location);

  omniORB::logs(10, "GIOP::LOCATION_FORWARD -- retry request.");

  // I suppose it is possible that a local servant was stupid
  // enough to re-direct us to itself!  If this happened it is
  // just possible that new_location == objref (if the most-
  // derived and interface types were the same).

  if( new_location == objref ) {
    releaseObjRef(new_location);
    return;
  }

  {
    omni_tracedmutex_lock sync(*internalLock);

    // We assume that the new object exists, and that it supports
    // the correct interface.  If it doesn't exist we'll get an
    // OBJECT_NOT_EXIST exception, revert to the original and try
    // again.  At worst we will keep trying again and again with
    // exponential backoff.
    //  If it supports the wrong interface then this is not our
    // fault.  It should show up as a BAD_OPERATION or something.

    objref->pd_flags.forward_location = 1;
    objref->pd_flags.object_exists = 1;
    objref->pd_flags.type_verified = 1;

    // <new_location>'s identity may well be sufficient
    // for our needs -- but if local we do need to check
    // that the servant supports the correct c++ type
    // interface.

    omniIdentity*      nl_id  = new_location->_identity();
    omniLocalIdentity* nl_lid = new_location->_localId();

    if( nl_id == nl_lid &&
	nl_lid->servant()->_ptrToInterface(objref->_localServantTarget()) ) {

      omniInternal::replaceImplementation(objref, nl_lid, nl_lid);
    }
    else {
      // This means that we have been forwarded to an implementation
      // that we cannot connect to directly. So:
      // 
      //   o  it is remote
      //   o  it is local but doesn't yet exist
      //   o  it is local & exists but doesn't support our interface
      //
      // In all of these cases we need to use a remote id, and set
      // type_verified to 0 to ensure we re-check the type before
      // using it.

      if ( nl_id == nl_lid ) {
	// local & exists but doesn't support our interface

	// Create in process identity
	nl_id = createInProcessIdentity(nl_id->key(),nl_id->keysize());
	objref->pd_flags.type_verified = 0;
      }
      omniInternal::replaceImplementation(objref, nl_id, nl_lid);
    }

    if (permanent) {
      // This location forwarding is permanent, replace the IOR of this
      // object reference with the new one. If this object reference is
      // later passed to another server, the new IOR will be transferred.
      omni_tracedmutex_lock sync(*omniIOR::lock);

      new_location->pd_ior->duplicateNoLock();
      objref->pd_ior->releaseNoLock();
      objref->pd_ior = new_location->pd_ior;
      objref->pd_flags.forward_location = 0;
    }
  }
  releaseObjRef(new_location);
}


// omni::stringToObject() and omni::objectToString are replaced by
// functions in omniURI::


void
omni::assertFail(const char* file, int line, const char* expr)
{
  if( omniORB::trace(1) ) {
    omniORB::logger l;
    l << "Assertion failed.  This indicates a bug in the application using\n"
      "omniORB, or maybe in omniORB itself. e.g. using the ORB after it has\n"
      "been shut down.\n"
      " file: " << file << "\n"
      " line: " << line << "\n"
      " info: " << expr << '\n';
  }
  throw omniORB::fatalException(file, line, expr);
}


void
omni::ucheckFail(const char* file, int line, const char* expr)
{
  if( omniORB::trace(1) ) {
    omniORB::logger l;
    l << "Application check failed. This indicates a bug in the application\n"
      " using omniORB.  See the comment in the source code for more info.\n"
      " file: " << file << "\n"
      " line: " << line << "\n"
      " info: " << expr << '\n';
  }
}

/////////////////////////////////////////////////////////////////////////////
//            Module initialiser                                           //
/////////////////////////////////////////////////////////////////////////////

OMNI_NAMESPACE_BEGIN(omni)

class omni_omniInternal_initialiser : public omniInitialiser {
public:

  void attach() {
    OMNIORB_ASSERT(!objectTable);  OMNIORB_ASSERT(!omni::internalLock);

    omni::internalLock   = new omni_tracedmutex;
    omni::poRcLock       = new omni_tracedmutex;
    omni::objref_rc_lock = new omni_tracedmutex;

    numObjectsInTable = 0;
    minNumObjects = 0;

    if( omniORB::objectTableSize ) {
      objectTableSize = omniORB::objectTableSize;
      maxNumObjects = 1ul << 31;
    }
    else {
      objectTableSizeI = 0;
      objectTableSize = objTblSizes[objectTableSizeI];
      maxNumObjects = objectTableSize * 2 / 3;
    }

    objectTable = new omniLocalIdentity* [objectTableSize];
    for( CORBA::ULong i = 0; i < objectTableSize; i++ )  objectTable[i] = 0;

    orbAsyncInvoker = new omniAsyncInvoker(omniORB::maxServerThreadPoolSize);
  }

  void detach() {
    if (orbAsyncInvoker) {
      delete orbAsyncInvoker;
      orbAsyncInvoker = 0;
    }
  }
};

static omni_omniInternal_initialiser initialiser;

omniInitialiser& omni_omniInternal_initialiser_ = initialiser;

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

class static_initialiser {
public:
  inline static_initialiser() {
    // Ensure that nil_ref_lock is initialised during
    // static initialisation.
    omni::nilRefLock();
  }
  static static_initialiser the_instance;
};

OMNI_NAMESPACE_END(omni)
