// -*- Mode: C++; -*-
//                            Package   : omniORB
// omniObjRef.cc              Created on: 26/2/99
//                            Author    : David Riddoch (djr)
//
//    Copyright (C) 1996-1999 AT&T Research Cambridge
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
//

/*
  $Log$
  Revision 1.1.2.3  1999/10/27 17:32:14  djr
  omni::internalLock and objref_rc_lock are now pointers.

  Revision 1.1.2.2  1999/10/14 16:22:14  djr
  Implemented logging when system exceptions are thrown.

  Revision 1.1.2.1  1999/09/22 14:26:59  djr
  Major rewrite of orbcore to support POA.

*/

#include <omniORB3/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <omniORB3/omniObjRef.h>
#include <omniORB3/callDescriptor.h>
#include <omniORB3/omniServant.h>
#include <localIdentity.h>
#include <remoteIdentity.h>
#include <objectAdapter.h>
#include <ropeFactory.h>
#include <excepthandler.h>
#include <exception.h>


int
omniObjRef::_getRopeAndKey(omniRopeAndKey& rak, CORBA::Boolean* is_local) const
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 0);

  if( is_local )  *is_local = 0;

  int fwd;
  int use_loopback = 0;

  {
    omni::internalLock->lock();

    fwd = pd_flags.forward_location;
    rak.key(pd_id->key(), pd_id->keysize());

    if( is_local )  *is_local = pd_localId ? 1 : 0;

    if( pd_localId && pd_id == pd_localId )
      use_loopback = 1;
    else
      rak.rope(((omniRemoteIdentity*) pd_id)->rope());

    omni::internalLock->unlock();
  }

  if( use_loopback )  rak.rope(omniObjAdapter::defaultLoopBack());

  return fwd;
}


void
omniObjRef::_getTheKey(omniObjKey& key, int locked) const
{
  if( !locked )  omni::internalLock->lock();
  key.copy(pd_id->key(), pd_id->keysize());
  if( !locked )  omni::internalLock->unlock();
}


CORBA::Boolean
omniObjRef::_real_is_a(const char* repoId)
{
  if( !repoId )  return 0;

  if( _ptrToObjRef(repoId) )                   return 1;
  if( !strcmp(repoId, pd_mostDerivedRepoId) )  return 1;

  {
    omni::internalLock->lock();
    int tv = pd_flags.type_verified;
    omni::internalLock->unlock();
    if( tv )  return 0;
  }

  // Reach here because pd_flags.type_verified == 0, and we could not
  // verify the inheritance relationship using compile-time information.
  // Thus we ask our implementation if it is an instance of the given
  // type.

  return _remote_is_a(repoId);
}


void*
omniObjRef::_realNarrow(const char* repoId)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 0);
  OMNIORB_ASSERT(repoId && *repoId);

  // Attempt to narrow the reference using static type info.
  void* target = _ptrToObjRef(repoId);

  if( target ) {
    omni::duplicateObjRef(this);
  }
  else {
    // Either:
    //  1. This proxy object was created for an object type for which we
    //     do not have an authoritative answer to its inheritance relation.
    //  2. The object's actual most derived type has been changed to a
    //     more derived type after this object reference is generated.
    //     Since the type ID in the IOR is only considered as a hint,
    //     we cannot give up without asking the object about it.
    // Use _is_a() to query the object to find out if <repoId> is the
    // interface repo id of a base type of this object.

    if( _real_is_a(repoId) ) {
      // OK, the actual object does support the interface <repoId>.
      // However <repoId> is not necassarily a base type of our
      // most derived id -- since the implementation may have been
      // replaced by one of a more derived type.  In this case the
      // narrow will fail, since we can't be expected to know this.

      omniObjRef* objref;

      omni::internalLock->lock();

      if( _localId() )
	objref = omni::createObjRef(pd_mostDerivedRepoId, repoId, _localId());
      else
	objref = omni::createObjRef(pd_mostDerivedRepoId, repoId,
				    pd_iopprofiles, 0, 1);

      omni::internalLock->unlock();

      if( objref ) {
	target = objref->_ptrToObjRef(repoId);
	OMNIORB_ASSERT(target);
      }
    }
  }
  return target;
}


void
omniObjRef::_assertExistsAndTypeVerified()
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 0);

  // NB. We don't lock here to protect <pd_flags> while reading.  This
  // just means that there could potentially be multiple threads doing
  // the following work simultaneously.  A waste, but not a disaster.
  // We do need to lock when writing of course.

  if( !pd_flags.type_verified ) {

    if( !_remote_is_a(pd_intfRepoId) ) {
      if( omniORB::traceLevel > 1 ) {
	omniORB::log <<
	  "omniORB: The object with the IR repository ID: " <<
	  pd_mostDerivedRepoId << "\n"
	  " returns FALSE to the query _is_a(\"" << pd_intfRepoId << "\").\n"
	  " A CORBA::INV_OBJREF is raised.\n";
	omniORB::log.flush();
      }
      OMNIORB_THROW(INV_OBJREF,0,CORBA::COMPLETED_NO);
    }
    {
      omni::internalLock->lock();
      pd_flags.type_verified = 1;
      pd_flags.object_exists = 1;
      omni::internalLock->unlock();
    }
    return;
  }

  if( !pd_flags.object_exists ) {

    _locateRequest();
    {
      omni::internalLock->lock();
      pd_flags.object_exists = 1;
      omni::internalLock->unlock();
    }

  }
}

//////////////////////////////////////////////////////////////////////

class omniObjRef_is_a_CallDesc : public omniCallDescriptor {
public:
  inline omniObjRef_is_a_CallDesc(LocalCallFn lcfn, const char* op,
				  int oplen, const char* i_1)
    : omniCallDescriptor(lcfn, op, oplen),
      a_1(i_1)
    {}

  virtual CORBA::ULong alignedSize(CORBA::ULong);
  virtual void marshalArguments(GIOP_C&);
  virtual void unmarshalReturnedValues(GIOP_C&);

  const char*    a_1;
  CORBA::Boolean result;
};


CORBA::ULong
omniObjRef_is_a_CallDesc::alignedSize(CORBA::ULong msgsize)
{
  msgsize = omni::align_to(msgsize, omni::ALIGN_4) + 4;
  msgsize += a_1 ? strlen(a_1) + 1 : 1;
  return msgsize;
}


void
omniObjRef_is_a_CallDesc::marshalArguments(GIOP_C& giop_client)
{
  CORBA::ULong len_1 = a_1 ? strlen(a_1) + 1 : 1;
  len_1 >>= giop_client;
  if( len_1 > 1 )
    giop_client.put_char_array((const CORBA::Char*) a_1, len_1);
  else {
    if( a_1 == 0 && omniORB::traceLevel > 1 )  _CORBA_null_string_ptr(0);
    CORBA::Char('\0') >>= giop_client;
  }
}


void
omniObjRef_is_a_CallDesc::unmarshalReturnedValues(GIOP_C& giop_client)
{
  result <<= giop_client;
}


static void
omniObjRef_is_a_lcfn(omniCallDescriptor* cd, omniServant* servant)
{
  omniObjRef_is_a_CallDesc* tcd = (omniObjRef_is_a_CallDesc*) cd;

  tcd->result = servant->_is_a(tcd->a_1);
}


CORBA::Boolean
omniObjRef::_remote_is_a(const char* a_repoId)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 0);

  omniObjRef_is_a_CallDesc call_desc(omniObjRef_is_a_lcfn,
				     "_is_a", sizeof("_is_a"), a_repoId);

  _invoke(call_desc, 0);
  return call_desc.result;
}

//////////////////////////////////////////////////////////////////////

class omniObjRef_non_existent_CallDesc : public omniCallDescriptor {
public:
  inline omniObjRef_non_existent_CallDesc(LocalCallFn lcfn,
					  const char* op, int oplen)
    : omniCallDescriptor(lcfn, op, oplen)
    {}

  virtual void unmarshalReturnedValues(GIOP_C&);

  CORBA::Boolean result;
};


void
omniObjRef_non_existent_CallDesc::unmarshalReturnedValues(GIOP_C& giop_client)
{
  result <<= giop_client;
}


static void
omniObjRef_non_existent_lcfn(omniCallDescriptor* cd, omniServant* servant)
{
  ((omniObjRef_non_existent_CallDesc*) cd)->result = servant->_non_existent();
}


CORBA::Boolean
omniObjRef::_remote_non_existent()
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 0);

  omniObjRef_non_existent_CallDesc call_desc(omniObjRef_non_existent_lcfn,
					     "_non_existent",
					     sizeof("_non_existent"));

  _invoke(call_desc, 0);
  return call_desc.result;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void*
omniObjRef::_transientExceptionHandler(void*& cookie)
{
  if (pd_flags.transient_exception_handler) {
    omniExHandlers_iterator iter;
    omniExHandlers* hp = iter.find(this);
    if (hp != 0) {
      cookie = hp->transient_cookie;
      void* result = hp->transient_hdr;
      return result;
    }
  }
  return 0;
}


void
omniObjRef::_transientExceptionHandler(void* new_handler,void* cookie)
{
  omniExHandlers_iterator iter;
  omniExHandlers* hp = iter.find_or_create(this);
  hp->transient_cookie = cookie;
  hp->transient_hdr = new_handler;
  pd_flags.transient_exception_handler = 1;
}


void*
omniObjRef::_commFailureExceptionHandler(void*& cookie)
{
  if (pd_flags.commfail_exception_handler) {
    omniExHandlers_iterator iter;
    omniExHandlers* hp = iter.find(this);
    if (hp != 0) {
      cookie = hp->commfail_cookie;
      void* result = hp->commfail_hdr;
      return result;
    }
  }
  return 0;
}


void
omniObjRef::_commFailureExceptionHandler(void* new_handler, void* cookie)
{
  omniExHandlers_iterator iter;
  omniExHandlers* hp = iter.find_or_create(this);
  hp->commfail_cookie = cookie;
  hp->commfail_hdr = new_handler;
  pd_flags.commfail_exception_handler = 1;
}


void*
omniObjRef::_systemExceptionHandler(void*& cookie)
{
  if (pd_flags.system_exception_handler) {
    omniExHandlers_iterator iter;
    omniExHandlers* hp = iter.find(this);
    if (hp != 0) {
      cookie = hp->sysexcpt_cookie;
      void* result = hp->sysexcpt_hdr;
      return result;
    }
  }
  return 0;
}


void
omniObjRef::_systemExceptionHandler(void* new_handler,void* cookie)
{
  omniExHandlers_iterator iter;
  omniExHandlers* hp = iter.find_or_create(this);
  hp->sysexcpt_cookie = cookie;
  hp->sysexcpt_hdr = new_handler;
  pd_flags.system_exception_handler = 1;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

omniObjRef::~omniObjRef()
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 0);

  if( pd_refCount ) {
    if( omniORB::traceLevel > 0 ) {
      omniORB::log <<
	"omniORB: ERROR -- an object reference has been explicity deleted.\n"
	" This is not legal, and will probably lead to a crash. Good luck!\n";
      omniORB::log.flush();
    }
  }

  if( pd_intfRepoId != pd_mostDerivedRepoId )
    delete[] pd_intfRepoId;
  if( pd_mostDerivedRepoId )  delete[] pd_mostDerivedRepoId;
  if( pd_iopprofiles )        delete pd_iopprofiles;
}


omniObjRef::omniObjRef()
  : pd_refCount(0),
    pd_mostDerivedRepoId(0),
    pd_intfRepoId(0),
    pd_iopprofiles(0),
    pd_id(0),
    pd_localId(0),
    pd_nextInLocalRefList(0)
{
  // Nil objref.
}


omniObjRef::omniObjRef(const char* intfRepoId, const char* mostDerivedId,
		       IOP::TaggedProfileList* profiles,
		       omniIdentity* id, omniLocalIdentity* lid)
  : pd_refCount(1),
    pd_iopprofiles(profiles),
    pd_id(id),
    pd_localId(lid),
    pd_nextInLocalRefList(0)
{
  OMNIORB_ASSERT(intfRepoId);
  OMNIORB_ASSERT(mostDerivedId);
  OMNIORB_ASSERT(id);

  pd_intfRepoId = new char[strlen(intfRepoId) + 1];
  strcpy(pd_intfRepoId, intfRepoId);
  if( strcmp(intfRepoId, mostDerivedId) ) {
    pd_mostDerivedRepoId = new char[strlen(mostDerivedId) + 1];
    strcpy(pd_mostDerivedRepoId, mostDerivedId);
  }
  else
    pd_mostDerivedRepoId = pd_intfRepoId;

  pd_flags.forward_location = 0;
  pd_flags.type_verified = 1;
  pd_flags.object_exists = (id == lid);
  pd_flags.transient_exception_handler = 0;
  pd_flags.commfail_exception_handler = 0;
  pd_flags.system_exception_handler = 0;
}


void
omniObjRef::_invoke(omniCallDescriptor& call_desc, CORBA::Boolean do_assert)
{
  int retries = 0;
  int fwd;

  if( _is_nil() )  _CORBA_invoked_nil_objref();

#ifndef EGCS_WORKAROUND
 _again:
#else
  while(1) {
#endif
    if( omniORB::verifyObjectExistsAndType && do_assert )
      _assertExistsAndTypeVerified();

    try{

      omni::internalLock->lock();
      fwd = pd_flags.forward_location;
      _identity()->dispatch(call_desc);
      return;

    }
    catch(CORBA::COMM_FAILURE& ex) {
      if( fwd ) {
	omni::revertToOriginalProfile(this);
	CORBA::TRANSIENT ex2(ex.minor(), ex.completed());
	if( !_omni_callTransientExceptionHandler(this, retries++, ex2) )
	  throw ex2;
      }
      else if( !_omni_callCommFailureExceptionHandler(this, retries++, ex) )
	throw;
    }
    catch(CORBA::TRANSIENT& ex) {
      if( !_omni_callTransientExceptionHandler(this, retries++, ex) )
	throw;
    }
    catch(CORBA::OBJECT_NOT_EXIST& ex) {
      if( fwd ){
	omni::revertToOriginalProfile(this);
	CORBA::TRANSIENT ex2(ex.minor(), ex.completed());
	if( !_omni_callTransientExceptionHandler(this, retries++, ex2) )
	  throw ex2;
      }
      else if( !_omni_callSystemExceptionHandler(this, retries++, ex) )
	throw;
    }
    catch(CORBA::SystemException& ex) {
      if( !_omni_callSystemExceptionHandler(this, retries++, ex) )
	throw;
    }
    catch(omniORB::LOCATION_FORWARD& ex) {
      if( CORBA::is_nil(ex.get_obj()) ) {
	CORBA::COMM_FAILURE ex2(0, CORBA::COMPLETED_NO);
	if( omniORB::traceLevel > 10 ){
	  omniORB::log << "Received GIOP::LOCATION_FORWARD message that"
	    " contains a nil object reference.\n";
	  omniORB::log.flush();
	}
	if( !_omni_callCommFailureExceptionHandler(this, retries++, ex2) )
	  throw ex2;
      }
      omni::locationForward(this, ex.get_obj()->_PR_getobj());
    }

#ifndef EGCS_WORKAROUND
    goto _again;
#else
  }
#endif
}


void
omniObjRef::_locateRequest()
{
  int retries = 0;
  int fwd;

  if( _is_nil() )  _CORBA_invoked_nil_objref();

#ifndef EGCS_WORKAROUND
 _again:
#else
  while(1) {
#endif

    try{

      omni::internalLock->lock();
      if( pd_id == pd_localId ) {
	// Its a local object, and we know its here.
	omni::internalLock->unlock();
	return;
      }
      fwd = pd_flags.forward_location;
      ((omniRemoteIdentity*) _identity())->locateRequest();
      return;

    }
    catch(CORBA::COMM_FAILURE& ex) {
      if( fwd ) {
	omni::revertToOriginalProfile(this);
	CORBA::TRANSIENT ex2(ex.minor(), ex.completed());
	if( !_omni_callTransientExceptionHandler(this, retries++, ex2) )
	  throw ex2;
      }
      else if( !_omni_callCommFailureExceptionHandler(this, retries++, ex) )
	throw;
    }
    catch(CORBA::TRANSIENT& ex) {
      if( !_omni_callTransientExceptionHandler(this, retries++, ex) )
	throw;
    }
    catch(CORBA::OBJECT_NOT_EXIST& ex) {
      if( fwd ){
	omni::revertToOriginalProfile(this);
	CORBA::TRANSIENT ex2(ex.minor(), ex.completed());
	if( !_omni_callTransientExceptionHandler(this, retries++, ex2) )
	  throw ex2;
      } else if( !_omni_callSystemExceptionHandler(this, retries++, ex) )
	throw;
    }
    catch(CORBA::SystemException& ex) {
      if( !_omni_callSystemExceptionHandler(this, retries++, ex) )
	throw;
    }
    catch(omniORB::LOCATION_FORWARD& ex) {
      if( CORBA::is_nil(ex.get_obj()) ) {
	if( omniORB::traceLevel > 10 ){
	  omniORB::log << "Received GIOP::LOCATION_FORWARD message that"
	    " contains a nil object reference.\n";
	  omniORB::log.flush();
	}
	CORBA::COMM_FAILURE ex2(0, CORBA::COMPLETED_NO);
	if( !_omni_callCommFailureExceptionHandler(this, retries++, ex2) )
	  throw ex2;
      }
      omni::locationForward(this, ex.get_obj()->_PR_getobj());
    }

#ifndef EGCS_WORKAROUND
    goto _again;
#else
  }
#endif
}
