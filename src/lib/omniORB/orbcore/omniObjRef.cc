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
  Revision 1.2.2.7  2000/12/05 17:39:31  dpg1
  New cdrStream functions to marshal and unmarshal raw strings.

  Revision 1.2.2.6  2000/11/07 18:44:03  sll
  Renamed omniObjRef::_hash and _is_equivalent to __hash and __is_equivalent
  to avoid name clash with the member functions of CORBA::Object.

  Revision 1.2.2.5  2000/11/03 19:12:07  sll
  Use new marshalling functions for byte, octet and char. Use get_octet_array
  instead of get_char_array and put_octet_array instead of put_char_array.

  Revision 1.2.2.4  2000/10/06 16:37:48  sll
  _invoke() can now cope with a call descriptor with no local call function.

  Revision 1.2.2.3  2000/10/03 17:37:08  sll
  Changed omniIOR synchronisation mutex from omni::internalLock to its own
  mutex.

  Revision 1.2.2.2  2000/09/27 18:40:38  sll
  Removed obsoluted _getRopeAndKey()
  New members _getIOR(), _marshal(), _unMarshal(), _toString,  _fromString(),
  _hash(), _is_equivalent().

  Revision 1.2.2.1  2000/07/17 10:35:56  sll
  Merged from omni3_develop the diff between omni3_0_0_pre3 and omni3_0_0.

  Revision 1.3  2000/07/13 15:25:56  dpg1
  Merge from omni3_develop for 3.0 release.

  Revision 1.1.2.6  2000/06/22 10:40:16  dpg1
  exception.h renamed to exceptiondefs.h to avoid name clash on some
  platforms.

  Revision 1.1.2.5  2000/03/01 17:57:41  dpg1
  New omniObjRef::_compatibleServant() function to support object
  references and servants written for languages other than C++.

  Revision 1.1.2.4  1999/11/08 09:45:17  djr
  Fixed bug in omniObjRef::_real_is_a().

  Revision 1.1.2.3  1999/10/27 17:32:14  djr
  omni::internalLock and objref_rc_lock are now pointers.

  Revision 1.1.2.2  1999/10/14 16:22:14  djr
  Implemented logging when system exceptions are thrown.

  Revision 1.1.2.1  1999/09/22 14:26:59  djr
  Major rewrite of orbcore to support POA.

*/

#include <omniORB4/CORBA.h>
#include <omniORB4/callDescriptor.h>
#include <localIdentity.h>
#include <remoteIdentity.h>
#include <objectAdapter.h>
#include <ropeFactory.h>
#include <excepthandler.h>
#include <exceptiondefs.h>
#include <objectStub.h>


CORBA::Boolean
omniObjRef::_compatibleServant(omniServant* svnt)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);

  if (svnt->_ptrToInterface(pd_intfRepoId))
    return 1;
  else
    return 0;
}


CORBA::Boolean
omniObjRef::_real_is_a(const char* repoId)
{
  if( !repoId )  return 0;

  if( _ptrToObjRef(repoId) )                   return 1;
  if( !strcmp(repoId, pd_mostDerivedRepoId) )  return 1;

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

      omniIOR* ior;

      {
	omni_tracedmutex_lock sync(*omniIOR::lock);
	ior = pd_ior->duplicateNoLock();
      }

      if( _localId() ) 
	objref = omni::createObjRef(repoId, _localId(),ior);
      else
	objref = omni::createObjRef(repoId,ior,1);

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

CORBA::Boolean 
omniObjRef::__is_equivalent(omniObjRef* o_obj)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 0);

  int m_islocal, o_islocal;
  const CORBA::Octet* m_key;
  const CORBA::Octet* o_key;
  int m_keysize, o_keysize;
  
  {
    omni_tracedmutex_lock sync(*omni::internalLock);

    m_key = pd_id->key();
    m_keysize = pd_id->keysize();

    o_key = o_obj->pd_id->key();
    o_keysize = o_obj->pd_id->keysize();

    if (m_keysize != o_keysize || 
	memcmp((void*)m_key,(void*)o_key,m_keysize) != 0)
      // Object keys do not match
      return 0;

    m_islocal = pd_localId ? 1 : 0;
    o_islocal = o_obj->pd_localId ? 1 : 0;

    if (m_islocal && o_islocal)
      return 1;
    else if (m_islocal != o_islocal)
      return 0;
    else
      return ((((omniRemoteIdentity*) pd_id)->rope()) == 
	      (((omniRemoteIdentity*) o_obj->pd_id)->rope()));
  }
}

CORBA::ULong
omniObjRef::__hash(CORBA::ULong maximum)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 0);

  const CORBA::Octet* key;
  int keysize;

  {
    omni_tracedmutex_lock sync(*omni::internalLock);
    key = pd_id->key();
    keysize = pd_id->keysize();

    return CORBA::ULong(omni::hash(key, keysize) % maximum);
  }
}


//////////////////////////////////////////////////////////////////////
CORBA::Boolean
omniObjRef::_remote_is_a(const char* a_repoId)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 0);

  omni_is_a_CallDesc call_desc("_is_a", sizeof("_is_a"), a_repoId);

  _invoke(call_desc, 0);
  return call_desc.result;
}

//////////////////////////////////////////////////////////////////////
CORBA::Boolean
omniObjRef::_remote_non_existent()
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 0);

  omni_non_existent_CallDesc call_desc("_non_existent",
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

  if (pd_ior) {
    pd_ior->release();
  }
}


omniObjRef::omniObjRef()
  : pd_refCount(0),
    pd_mostDerivedRepoId(0),
    pd_intfRepoId(0),
    pd_ior(0),
    pd_id(0),
    pd_localId(0),
    pd_nextInLocalRefList(0)
{
  // Nil objref.
}


omniObjRef::omniObjRef(const char* intfRepoId, omniIOR* ior,
		       omniIdentity* id, omniLocalIdentity* lid)
  : pd_refCount(1),
    pd_ior(ior),
    pd_id(id),
    pd_localId(lid),
    pd_nextInLocalRefList(0)
{
  OMNIORB_ASSERT(intfRepoId);
  OMNIORB_ASSERT(ior);
  OMNIORB_ASSERT(id);

  pd_intfRepoId = new char[strlen(intfRepoId) + 1];
  strcpy(pd_intfRepoId, intfRepoId);

  if( strcmp(intfRepoId, ior->repositoryID) ) {
    pd_mostDerivedRepoId = new char[strlen(ior->repositoryID) + 1];
    strcpy(pd_mostDerivedRepoId, ior->repositoryID);
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

  while(1) {

    if( omniORB::verifyObjectExistsAndType && do_assert )
      _assertExistsAndTypeVerified();

    try{

      omni::internalLock->lock();
      fwd = pd_flags.forward_location;
      omniIdentity* id = _identity();

      if (!call_desc.haslocalCallFn() && _localId()) {
	// This is a local object so a call to id->dispatch will cause
	// the local call function in the call descriptor to be invoked.
	// But we do not have a local call function! We have to create
	// a remoteIdentity with the loop back transport to dispatch the call.
	// The remoteIdentity will be deleted automatically when the
	// call finishes.
	//
	// This is not the normal case but could happen in rare circumstance,
	// such as a DII call descriptor invoking on a local object.
	Rope* rope = omniObjAdapter::defaultLoopBack();
	rope->incrRefCount();
	omniIOR* ior = _getIOR();
	id = new omniRemoteIdentity(ior, rope);
      }

      id->dispatch(call_desc);
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
      omni::locationForward(this,ex.get_obj()->_PR_getobj(),ex.is_permanent());
    }

  }
}


omniIOR* 
omniObjRef::_getIOR()
{
  omni_tracedmutex_lock sync(*omniIOR::lock);
  // Must hold mutex before reading pd_ior.
  return pd_ior->duplicateNoLock();
}

void
omniObjRef::_marshal(omniObjRef* objref, cdrStream& s)
{

  if (!objref || objref->_is_nil()) {
    ::operator>>= ((CORBA::ULong)1,s);
    s.marshalOctet('\0');
    ::operator>>= ((CORBA::ULong) 0,s);
    return;
  }

  omniIOR_var ior;
  {
    omni_tracedmutex_lock sync(*omniIOR::lock);
    // Must hold mutex before reading pd_ior.
    ior =  objref->pd_ior->duplicateNoLock();
  }

  s.marshalRawString(ior->repositoryID);
  (IOP::TaggedProfileList&)ior->iopProfiles >>= s;
}

char*
omniObjRef::_toString(omniObjRef* objref)
{
  cdrMemoryStream buf(CORBA::ULong(0),1);
  buf.marshalOctet(omni::myByteOrder);
  _marshal(objref,buf);

  // turn the encapsulation into a hex string with "IOR:" prepended
  buf.rewindInputPtr();
  size_t s = buf.bufSize();
  CORBA::Char * data = (CORBA::Char *)buf.bufPtr();

  char *result = new char[4+s*2+1];
  result[4+s*2] = '\0';
  result[0] = 'I';
  result[1] = 'O';
  result[2] = 'R';
  result[3] = ':';
  for (int i=0; i < (int)s; i++) {
    int j = 4 + i*2;
    int v = (data[i] & 0xf0);
    v = v >> 4;
    if (v < 10)
      result[j] = '0' + v;
    else
      result[j] = 'a' + (v - 10);
    v = ((data[i] & 0xf));
    if (v < 10)
      result[j+1] = '0' + v;
    else
      result[j+1] = 'a' + (v - 10);
  }
  return result;
}


omniObjRef*
omniObjRef::_unMarshal(const char* repoId, cdrStream& s)
{
  CORBA::String_var id;
  IOP::TaggedProfileList_var profiles;

  id = IOP::IOR::unmarshaltype_id(s);
  
  profiles = new IOP::TaggedProfileList();
  (IOP::TaggedProfileList&)profiles <<= s;

  if (profiles->length() == 0 && strlen(id) == 0) {
    // This is a nil object reference
    return 0;
  }
  else {
    // It is possible that we reach here with the id string = '\0'.
    // That is alright because the actual type of the object will be
    // verified using _is_a() at the first invocation on the object.
    //
    // Apparently, some ORBs such as ExperSoft's do that. Furthermore,
    // this has been accepted as a valid behaviour in GIOP 1.1/IIOP 1.1.
    // 
    omniIOR* ior = new omniIOR(id._retn(),profiles._retn());
    omniObjRef* objref = omni::createObjRef(repoId,ior,0);
    return objref;
  }
}

omniObjRef*
omniObjRef::_fromString(const char* str)
{
  size_t s = (str ? strlen(str) : 0);
  if (s<4)
    OMNIORB_THROW(MARSHAL,0,CORBA::COMPLETED_NO);
  const char *p = str;
  if (p[0] != 'I' ||
      p[1] != 'O' ||
      p[2] != 'R' ||
      p[3] != ':')
    OMNIORB_THROW(MARSHAL,0,CORBA::COMPLETED_NO);

  s = (s-4)/2;  // how many octets are there in the string
  p += 4;

  cdrMemoryStream buf((CORBA::ULong)s,0);

  for (int i=0; i<(int)s; i++) {
    int j = i*2;
    CORBA::Octet v;
    
    if (p[j] >= '0' && p[j] <= '9') {
      v = ((p[j] - '0') << 4);
    }
    else if (p[j] >= 'a' && p[j] <= 'f') {
      v = ((p[j] - 'a' + 10) << 4);
    }
    else if (p[j] >= 'A' && p[j] <= 'F') {
      v = ((p[j] - 'A' + 10) << 4);
    }
    else
      OMNIORB_THROW(MARSHAL,0,CORBA::COMPLETED_NO);

    if (p[j+1] >= '0' && p[j+1] <= '9') {
      v += (p[j+1] - '0');
    }
    else if (p[j+1] >= 'a' && p[j+1] <= 'f') {
      v += (p[j+1] - 'a' + 10);
    }
    else if (p[j+1] >= 'A' && p[j+1] <= 'F') {
      v += (p[j+1] - 'A' + 10);
    }
    else
      OMNIORB_THROW(MARSHAL,0,CORBA::COMPLETED_NO);
    buf.marshalOctet(v);
  }

  buf.rewindInputPtr();
  CORBA::Boolean b = buf.unmarshalBoolean();
  buf.setByteSwapFlag(b);

  return _unMarshal(CORBA::Object::_PD_repoId,buf);
}

void
omniObjRef::_locateRequest()
{
  int retries = 0;
  int fwd;

  if( _is_nil() )  _CORBA_invoked_nil_objref();

  while(1) {

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
      omni::locationForward(this,ex.get_obj()->_PR_getobj(),ex.is_permanent());
    }

  }
}
