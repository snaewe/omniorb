// -*- Mode: C++; -*-
//                            Package   : omniORB2
// object.cc                  Created on: 13/5/96
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
  Revision 1.18  1999/08/16 19:24:33  sll
  The ctor of ropeFactory_iterator now takes a pointer argument.

  Revision 1.17  1999/03/11 16:25:54  djr
  Updated copyright notice

  Revision 1.16  1999/01/07 16:12:12  djr
  Added support for the server side _interface() operation.

  Revision 1.15  1998/08/26 11:13:32  sll
  Minor updates to remove warnings when compiled with standard C++ compiler.

  Revision 1.14  1998/08/14 13:49:38  sll
  Added pragma hdrstop to control pre-compile header if the compiler feature
  is available.

  Revision 1.13  1998/04/07 19:50:35  sll
  Replace cerr with omniORB::log.

  Revision 1.12  1998/03/25 14:20:36  sll
  Temporary work-around for egcs compiler.

  $Log$
  Revision 1.18  1999/08/16 19:24:33  sll
  The ctor of ropeFactory_iterator now takes a pointer argument.

  Revision 1.17  1999/03/11 16:25:54  djr
  Updated copyright notice

  Revision 1.16  1999/01/07 16:12:12  djr
  Added support for the server side _interface() operation.

  Revision 1.15  1998/08/26 11:13:32  sll
  Minor updates to remove warnings when compiled with standard C++ compiler.

  Revision 1.14  1998/08/14 13:49:38  sll
  Added pragma hdrstop to control pre-compile header if the compiler feature
  is available.

  Revision 1.13  1998/04/07 19:50:35  sll
  Replace cerr with omniORB::log.

  Revision 1.11  1998/03/19 15:22:47  sll
  Fixed so that omniObject ctor would not reject zero length key.

  Revision 1.10  1998/02/27 13:59:30  sll
  Minor update to cope with the change of manager() to _objectManager() in
  class omniObject.

  Revision 1.9  1997/12/09 17:13:52  sll
  Added support for system exception handlers.
  Updated to use the rope factory interfaces.
  New member _realNarrow(), _real_is_a() to support late binding.

  Revision 1.8  1997/08/21 21:59:12  sll
  ~omniObject() now delete pd_iopprofile.

// Revision 1.7  1997/05/06  15:25:34  sll
// Public release.
//
  */

#include <omniORB2/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <ropeFactory.h>
#include <objectManager.h>
#include <excepthandler.h>
#include <omniORB2/proxyCall.h>
#include <bootstrap_i.h>


static CORBA::Object_ptr internal_get_interface(const char* repoId);


omniObject::omniObject(omniObjectManager* m)    // ctor for local object
{
  pd_repoId = 0;
  pd_repoIdsize = 0;
  pd_original_repoId = 0;
  pd_refCount = 0;
  pd_next = 0;
  pd_flags.proxy = 0;
  pd_flags.disposed = 0;
  pd_flags.forwardlocation = 0;
  pd_flags.transient_exception_handler = 0;
  pd_flags.commfail_exception_handler = 0;
  pd_flags.system_exception_handler = 0;
  pd_flags.existent_and_type_verified = 1;
  if( m != omniObject::nilObjectManager() ) {
    omniORB::generateNewKey(pd_objkey.native);
    pd_objkeysize = sizeof(pd_objkey.native);
    pd_manager = ((m) ? m : omniObjectManager::root());
    IOP::TaggedProfileList_var pl(new IOP::TaggedProfileList);
    ropeFactory_iterator iter(pd_manager->incomingRopeFactories());
    incomingRopeFactory* rp;
    while ((rp = (incomingRopeFactory*) iter())) {
      rp->getIncomingIOPprofiles((CORBA::Octet*)&pd_objkey.native,
				 pd_objkeysize,
				 *(pl.operator->()));
    }
    pd_iopprofile = pl._ptr;
    pl._ptr = 0;
  }
  else {
    // nil object
    pd_objkeysize = 0;
    pd_manager = m;
    pd_iopprofile = 0;
  }
  return;
}


omniObject::omniObject(const char *repoId,   // ctor for proxy object
	       Rope *r,
	       CORBA::Octet *key,
	       size_t keysize,
	       IOP::TaggedProfileList *profiles,
	       CORBA::Boolean release)
{
  if (!repoId || !r || !profiles)
    throw CORBA::BAD_PARAM(0,CORBA::COMPLETED_NO);
  // keysize may be >= 0, key may be nil.

  pd_repoIdsize = strlen(repoId)+1;
  pd_repoId = new char[pd_repoIdsize];
  if (!pd_repoId)
    throw CORBA::NO_MEMORY(0,CORBA::COMPLETED_NO);
  strcpy(pd_repoId,repoId);
  pd_original_repoId = 0;
  pd_rope = r;  // caller has already done a r->incrRefCount()
  pd_refCount = 0;
  pd_next = 0;
  pd_flags.disposed = 0;
  pd_flags.existent_and_type_verified = 0;
  pd_flags.forwardlocation = 0;
  pd_flags.transient_exception_handler = 0;
  pd_flags.commfail_exception_handler = 0;
  pd_flags.system_exception_handler = 0;
  pd_flags.proxy = 1;

  if (!release) {
    pd_iopprofile = 0;
    pd_objkey.foreign = 0;
    try {
      pd_iopprofile = new IOP::TaggedProfileList(*profiles);
      if (!pd_iopprofile)
	throw CORBA::NO_MEMORY(0,CORBA::COMPLETED_NO);
      pd_objkeysize = keysize;
      if (keysize) {
	pd_objkey.foreign = new CORBA::Octet[keysize];
	if (!pd_objkey.foreign)
	  throw CORBA::NO_MEMORY(0,CORBA::COMPLETED_NO);
	memcpy((void *)pd_objkey.foreign,(const void *)key,keysize);
      }
    }
    catch (...) {
      delete [] pd_repoId;
      if (pd_iopprofile) delete pd_iopprofile;
      if (pd_objkey.foreign) delete [] pd_objkey.foreign;
      throw;
    }
  }
  else {
    pd_objkeysize = keysize;
    pd_objkey.foreign =  key;
    pd_iopprofile = profiles;
  }
  return;

}

CORBA::Boolean
omniObject::getRopeAndKey(omniRopeAndKey& k) const
{
  omni_mutex_lock sync(omniObject::objectTableLock);
  CORBA::Boolean result = 0;
  if (is_proxy()) {
    k.rope(pd_rope);
    k.key(pd_objkey.foreign,(CORBA::ULong)pd_objkeysize);
    result = pd_flags.forwardlocation;
  }
  else {
    // This is a local object, we have to return a loopback rope in case 
    // the caller is using the rope to contact this object. One situation
    // in which this can occur is when the LOCATION FORWARDING code in
    // a proxy object's stub get this object as the new object reference
    // to forward the request to.
    k.rope(pd_manager->defaultLoopBack());
    k.key((CORBA::Octet*)&pd_objkey.native,sizeof(pd_objkey.native));
  }
  return result;
}

void
omniObject::setRopeAndKey(const omniRopeAndKey& l,
			  CORBA::Boolean keepIOP)
{
  omni_mutex_lock sync(omniObject::objectTableLock);
  if (is_proxy()) {
    if (pd_rope) {
      if (pd_rope != l.rope()) {
	pd_rope->decrRefCount();
	l.rope()->incrRefCount();
      }
    }
    pd_rope = l.rope();
    if (pd_objkeysize < l.keysize()) {
      delete [] pd_objkey.foreign;
      pd_objkey.foreign = new CORBA::Octet[l.keysize()];
    }
    pd_objkeysize = l.keysize();
    memcpy((void*)pd_objkey.foreign,(void*)l.key(),pd_objkeysize);
    if (!keepIOP) {
      IOP::TaggedProfileList_var np;
      Endpoint* addr = 0;
      (void) pd_rope->remote_is(addr);
      {
	const ropeFactory* rf;
	ropeFactory_iterator iter(globalOutgoingRopeFactories);
	while ((rf = iter())) {
	  if (rf->isOutgoing(addr)) {
	    np = new IOP::TaggedProfileList(1);
	    np->length(1);
	    rf->getType()->encodeIOPprofile(addr,pd_objkey.foreign,
					    pd_objkeysize,np[0]);
	    break;
	  }
	}
      }
      delete pd_iopprofile;
      pd_iopprofile = np._ptr; np._ptr = 0;
      pd_flags.forwardlocation = 0;
    }
    else {
      pd_flags.forwardlocation = 1;
    }
  }
  else {
    if (pd_refCount)
      throw CORBA::BAD_PARAM(0,CORBA::COMPLETED_NO);
    memcpy((void*)&pd_objkey.native,
	   (void*)l.key(),
	   sizeof(pd_objkey.native));
    if (!keepIOP) {
      IOP::TaggedProfileList_var np;
      np = new IOP::TaggedProfileList;
      {
	ropeFactory_iterator iter(pd_manager->incomingRopeFactories());
	incomingRopeFactory* rp;
	while ((rp = (incomingRopeFactory*) iter())) {
	  rp->getIncomingIOPprofiles((CORBA::Octet*)&pd_objkey.native,
				     pd_objkeysize,
				     *np._ptr);
	}
      }
      delete pd_iopprofile;
      pd_iopprofile = np._ptr; np._ptr = 0;
      pd_flags.forwardlocation = 0;
    }
    else {
      pd_flags.forwardlocation = 1;
    }
  }
}

void
omniObject::resetRopeAndKey()
{
  omni_mutex_lock sync(omniObject::objectTableLock);
  if (is_proxy()) {
    if (pd_flags.forwardlocation) {
      Rope_var       _rope;
      CORBA::Octet*  _key;
      size_t         _keysize;
      omniObject*    _localobj;
      if ((_localobj = ropeFactory::iopProfilesToRope(pd_iopprofile,
						     _key,_keysize,_rope))) {
	// strangely enough, the object is now found in the local object
	// table. It may be the case that when this proxy object was
	// created, the local object has not been instantiated. The
	// ORB returns a rope so that a call to the object proceeds
	// via the network loopback.
	// We override this and change it to return a network
	// loopback.
	_rope = _localobj->_objectManager()->defaultLoopBack();
	_rope->incrRefCount();
	CORBA::Octet* k;
	CORBA::ULong ks;
	_localobj->getKey(k,ks);
	_keysize = sizeof(ks);
	_key  = new CORBA::Octet[_keysize];
	memcpy((void*)_key,(void*)k,_keysize);
      }

      if (pd_rope) {
	pd_rope->decrRefCount();
      }
      pd_rope = _rope;
      _rope._ptr = 0;
      delete [] pd_objkey.foreign;
      pd_objkey.foreign = _key;
      pd_objkeysize = _keysize;

      pd_flags.forwardlocation = 0;
      pd_flags.existent_and_type_verified = 0;
    }
  }
  else {
    // A local object, Not allowed to reset the Rope and Key
    // just ignored this command.
  }
}

void
omniObject::PR_IRRepositoryId(const char *ir)
{
  if (is_proxy()) 
    {
      if (!pd_original_repoId) {
	pd_original_repoId = pd_repoId;
	pd_repoId = 0;
      }
      pd_flags.existent_and_type_verified = 0;
    }
  else 
    {
      if (getRefCount()) {
	throw omniORB::fatalException(__FILE__,__LINE__,
				      "omniObject::PR_IRRepositoryId()- tried to set IR Id for an activated object");
      }
    }

  if (pd_repoId)
    delete [] pd_repoId;
  pd_repoIdsize = strlen(ir) + 1;
  pd_repoId = new char [pd_repoIdsize];
  if (!pd_repoId)
    throw CORBA::NO_MEMORY(0,CORBA::COMPLETED_NO);
  strcpy(pd_repoId,ir);
  return;
}

CORBA::Boolean
omniObject::dispatch(GIOP_S &_s,const char *_op,
		     CORBA::Boolean _response_expected)
{
  if (is_proxy()) {
    // If this code is ever called, it is certainly because of a bug.
    throw omniORB::fatalException(__FILE__,__LINE__,
	                          "omniObject::dispatch()- unexpected call to"
				  " this function inside a proxy object.");
  }
  
  if (strcmp(_op,"_is_a") == 0)
    {
      if (!_response_expected) {
	throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
      }
      CORBA::String_var id;
      {
	CORBA::ULong _len;
	_len <<= _s;
	if (!_len || _s.RdMessageUnRead() < _len)
	  throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);
	id = CORBA::string_alloc(_len-1);
	if (!((char *)id))
	  throw CORBA::NO_MEMORY(0,CORBA::COMPLETED_NO);
	_s.get_char_array((CORBA::Char *)((char *)id),_len);
      }
      _s.RequestReceived();
      CORBA::Boolean _result;
      _result = _real_is_a(id);
      size_t _msgsize = (size_t) GIOP_S::ReplyHeaderSize();
      _msgsize += 1;
      _s.InitialiseReply(GIOP::NO_EXCEPTION,(CORBA::ULong)_msgsize);
      _result >>= _s;
      _s.ReplyCompleted();
      return 1;
    }
  else if (strcmp(_op,"_non_existent") == 0) 
    {
      if (!_response_expected) {
	throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
      }
      _s.RequestReceived();
      CORBA::Boolean _result = 0;
      size_t _msgsize = (size_t) GIOP_S::ReplyHeaderSize();
      _msgsize += 1;
      _s.InitialiseReply(GIOP::NO_EXCEPTION,(CORBA::ULong)_msgsize);
      _result >>= _s;
      _s.ReplyCompleted();
      return 1;
    }
  else if (strcmp(_op,"_interface") == 0)
    {
      _s.RequestReceived();
      CORBA::Object_ptr result;
      result = internal_get_interface(NP_IRRepositoryId());
      size_t msgsize = (size_t) GIOP_S::ReplyHeaderSize();
      msgsize = CORBA::Object::NP_alignedSize(result, msgsize);
      _s.InitialiseReply(GIOP::NO_EXCEPTION, CORBA::ULong(msgsize));
      CORBA::Object::marshalObjRef(result, _s);
      _s.ReplyCompleted();
      return 1;
    }
  else if (strcmp(_op,"_implementation") == 0)
    {
      if( omniORB::traceLevel > 1 ) {
	omniORB::log <<
	  "omniORB: WARNING received GIOP request <get_implementation>.\n"
	  " This function is not yet supported.\n"
	  " CORBA::BAD_OPERATION was raised.\n";
	omniORB::log.flush();
      }
      throw CORBA::NO_IMPLEMENT(0, CORBA::COMPLETED_NO);
    }
  else
    return 0;

#ifdef NEED_DUMMY_RETURN
  return 0;
#endif
}


void
omniObject::assertObjectExistent()
{
  if (pd_flags.existent_and_type_verified)
    return;

  // Reach here only if this is a proxy object and this is the first
  // time this function is called after pd_flags.existent_and_type_verified
  // is set to 0.

  // We don't put mutual exclusion on pd_flags.existent_and_type_verified 
  // so potentially  we can have multiple threads doing the following
  // simulataneously. That is alright except perhaps a bit of redundent work.

  if (pd_original_repoId != 0) 
    {
      // If the repository ID has been updated (pd_original_repoid != 0),
      // we have to verify if the object does indeed support the interface
      // of this proxy object. The repository ID of this proxy object is
      // stored in pd_original_repoId.  Use _is_a() operation to query the
      // remote object to see if it supports the interface identified by
      // pd_original_repoId.
      if (!_real_is_a(pd_original_repoId)) {
	if (omniORB::traceLevel > 0) {
	  omniORB::log << "Warning: in omniObject::assertObjectExistent(), the object with the IR repository ID:\n"
	       << "         " << NP_IRRepositoryId() << " returns FALSE to the query\n"
	       << "         is_a(\"" << pd_original_repoId << "\").\n"
	       << "         A CORBA::INV_OBJREF is raised.\n";
	  omniORB::log.flush();
	}
	throw CORBA::INV_OBJREF(0,CORBA::COMPLETED_NO);
      }
      pd_flags.existent_and_type_verified = 1;
    }
  else
    {
      // Use GIOP LocateRequest to verify that the object exists.
      CORBA::ULong retries = 0;
#ifndef EGCS_WORKAROUND
AOE_again:
#else
while (1) {
#endif
      omniRopeAndKey rak;
      CORBA::Boolean fwd = getRopeAndKey(rak);
      CORBA::Boolean reuse = 0;
      try {
	GIOP_C _c(rak.rope());
	reuse = _c.isReUsingExistingConnection();
	CORBA::ULong _msgsize = GIOP_C::RequestHeaderSize(rak.keysize(),14);

	switch (_c.IssueLocateRequest(rak.key(),rak.keysize()))
	  {
	  case GIOP::OBJECT_HERE:
	    pd_flags.existent_and_type_verified = 1;
	    _c.RequestCompleted();
	    return;
	    break;	// dummy break
	  case GIOP::UNKNOWN_OBJECT:
	    _c.RequestCompleted();
	    throw CORBA::OBJECT_NOT_EXIST(0,CORBA::COMPLETED_NO);
	    break;        // dummy break
	  case GIOP::OBJECT_FORWARD:
	    {
	      {
		CORBA::Object_var obj = CORBA::Object::unmarshalObjRef(_c);
		_c.RequestCompleted();
		if (CORBA::is_nil(obj)) {
		  if (omniORB::traceLevel > 10) {
		    omniORB::log << "Received GIOP::OBJECT_FORWARD in LocateReply message that contains a nil object reference.\n";
		    omniORB::log.flush();
		  }
		  throw CORBA::COMM_FAILURE(0,CORBA::COMPLETED_NO);
		}
		omniRopeAndKey _0RL_r;
		obj->PR_getobj()->getRopeAndKey(_0RL_r);
		setRopeAndKey(_0RL_r);
	      }
	      if (omniORB::traceLevel > 10) {
		omniORB::log << "GIOP::OBJECT_FORWARD: retry request.\n";
		omniORB::log.flush();
	      }
	      break;
	    }
	  }
      }
      catch (const CORBA::COMM_FAILURE& ex) {
	if (reuse || fwd) {
	  if (fwd)
	    resetRopeAndKey();
	  CORBA::TRANSIENT ex2(ex.minor(),ex.completed());
	  if (!_omni_callTransientExceptionHandler(this,retries++,ex2))
	    throw ex2;
	}
	else {
	  if (!_omni_callCommFailureExceptionHandler(this,retries++,ex))
	    throw;
	}
      }
      catch (const CORBA::TRANSIENT& ex) {
	if (!_omni_callTransientExceptionHandler(this,retries++,ex))
	  throw;
      }
      catch (const CORBA::SystemException& ex) {
	if (!_omni_callSystemExceptionHandler(this,retries++,ex))
	  throw;
      }
#ifndef EGCS_WORKAROUND
      goto AOE_again;
#else
}
#endif
    }
  return;
}

CORBA::Boolean
omniObject::_real_is_a(const char *repoId)
{
  if (!repoId)
    return 0;

  if (!is_proxy() || pd_original_repoId == 0) {
    if (_widenFromTheMostDerivedIntf(repoId) ||
	strcmp((const char*)CORBA::Object::repositoryID,repoId) == 0)
      return 1;
    else
      return 0;
  }

  // Reach here because is_proxy() == 1 and pd_original_repoId != 0
  // This is the case when the repository ID of the object is not the
  // same as the original repository ID of this proxy object.
  // That is, we don't know the exact type of this object but some time
  // in the past we were told this object is of type pd_original_repoId.
  // We have to use the _is_a operation to query the remote object to see
  // see if it really is an instance of the type identified by <repoId>.

  if (omniORB::traceLevel > 10) {
    omniORB::log << "Info: omniORB uses CORBA::Object::_is_a operation to check if "
	 << NP_IRRepositoryId() << " is a " << repoId << "\n";
    omniORB::log.flush();
  }

  CORBA::ULong   _retries = 0;
#ifndef EGCS_WORKAROUND
ISA_again:
#else
while(1) {
#endif
  omniRopeAndKey _r;
  CORBA::Boolean _fwd = getRopeAndKey(_r);
  CORBA::Boolean _reuse = 0;
  CORBA::Boolean _result;
  try {
    GIOP_C _c(_r.rope());
    _reuse = _c.isReUsingExistingConnection();
    CORBA::ULong _msgsize = GIOP_C::RequestHeaderSize(_r.keysize(),6);
    _msgsize = omni::align_to(_msgsize,omni::ALIGN_4);
    _msgsize += 4 + strlen((const char *)repoId) + 1;
    _c.InitialiseRequest(_r.key(),_r.keysize(),(char *)"_is_a",6,_msgsize,0);
    {
      CORBA::ULong _len = strlen((const char *)repoId)+1;
      _len >>= _c;
      _c.put_char_array((const CORBA::Char *)((const char*)repoId),_len);
    }
    switch (_c.ReceiveReply())
    {
      case GIOP::NO_EXCEPTION:
      {
        _result <<= _c;
        _c.RequestCompleted();
        return _result;
        break;
      }
      case GIOP::USER_EXCEPTION:
      {
        _c.RequestCompleted(1);
        throw CORBA::UNKNOWN(0,CORBA::COMPLETED_MAYBE);
        break;
      }
      case GIOP::SYSTEM_EXCEPTION:
      {
        _c.RequestCompleted(1);
        throw omniORB::fatalException(__FILE__,__LINE__,"GIOP::SYSTEM_EXCEPTION should not be returned by GIOP_C::ReceiveReply()");
      }
      case GIOP::LOCATION_FORWARD:
      {
        {
          CORBA::Object_var obj = CORBA::Object::unmarshalObjRef(_c);
          _c.RequestCompleted();
          if (CORBA::is_nil(obj)) {
            if (omniORB::traceLevel > 10) {
              omniORB::log << "Received GIOP::LOCATION_FORWARD message that contains a nil object reference.\n";
	      omniORB::log.flush();
            }
            throw CORBA::COMM_FAILURE(0,CORBA::COMPLETED_NO);
          }
          omniRopeAndKey _0RL_r;
          obj->PR_getobj()->getRopeAndKey(_0RL_r);
          setRopeAndKey(_0RL_r);
        }
        if (omniORB::traceLevel > 10) {
          omniORB::log << "GIOP::LOCATION_FORWARD: retry request.\n";
	  omniORB::log.flush();
        }
	break;
      }
    }
  }
  catch (const CORBA::COMM_FAILURE& ex) {
    if (_reuse || _fwd) {
      if (_fwd)
        resetRopeAndKey();
      CORBA::TRANSIENT ex2(ex.minor(),ex.completed());
      if (!_omni_callTransientExceptionHandler(this,_retries++,ex2))
	throw ex2;
    }
    else {
      if (!_omni_callCommFailureExceptionHandler(this,_retries++,ex))
	throw;
    }
  }
  catch (const CORBA::TRANSIENT& ex) {
    if (!_omni_callTransientExceptionHandler(this,_retries++,ex))
      throw;
  }
  catch (const CORBA::SystemException& ex) {
    if (!_omni_callSystemExceptionHandler(this,_retries++,ex))
      throw;
  }
#ifndef EGCS_WORKAROUND
  goto ISA_again;
#else
}
#endif
#ifdef NEED_DUMMY_RETURN
  {
    // never reach here! Dummy return to keep some compilers happy.
    CORBA::Boolean _result = 0;
    return _result;
  }
#endif
}

void*
omniObject::_realNarrow(const char* repoId)
{
  void* target = _widenFromTheMostDerivedIntf(repoId);

  if (target) {
    // Must increment the reference count because the return value
    // will be freed by CORBA::release().
    omni::objectDuplicate(this);
  }
  else {
    if (is_proxy()) {
      // Either: 
      //  1. This proxy object was created for an object type that we do not
      //     have authoritative answer to its inheritance relation.
      //  2. The object's actual most derived type has been changed to a
      //     more derived type after this object reference is generated.
      //     Since the type ID in the IOR is only considered as a hint,
      //     we cannot give up without asking the object about it.
      // Use _is_a() to query the object to find out if <repoId> is the
      // IR repositoryID of a base type of this object.
      if (_real_is_a(repoId)) {
	// OK, it is a base type
	try {
	  omniObject* omniobj = omni::createObjRef(NP_IRRepositoryId(),
						   repoId,
						   iopProfiles(),0);
	  target = omniobj->_widenFromTheMostDerivedIntf(repoId);
	}
	catch(...) {}
      }
    }
    
  }
  return target;
}



void* 
omniObject::_transientExceptionHandler(void*& cookie) 
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
omniObject::_transientExceptionHandler(void* new_handler,void* cookie) 
{
  omniExHandlers_iterator iter;
  omniExHandlers* hp = iter.find_or_create(this);
  hp->transient_cookie = cookie;
  hp->transient_hdr = new_handler;
  pd_flags.transient_exception_handler = 1;
  return;
}

void* 
omniObject::_commFailureExceptionHandler(void*& cookie) 
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
omniObject::_commFailureExceptionHandler(void* new_handler,void* cookie) 
{
  omniExHandlers_iterator iter;
  omniExHandlers* hp = iter.find_or_create(this);
  hp->commfail_cookie = cookie;
  hp->commfail_hdr = new_handler;
  pd_flags.commfail_exception_handler = 1;
  return;
}

void* 
omniObject::_systemExceptionHandler(void*& cookie) 
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
omniObject::_systemExceptionHandler(void* new_handler,void* cookie) 
{
  omniExHandlers_iterator iter;
  omniExHandlers* hp = iter.find_or_create(this);
  hp->sysexcpt_cookie = cookie;
  hp->sysexcpt_hdr = new_handler;
  pd_flags.system_exception_handler = 1;
  return;
}



omniObject::~omniObject()
{
  if (pd_refCount) {
    // A dtor should not be called if the reference count is not 0
    assert(0);
  }
  if (is_proxy() && pd_rope) {
    pd_rope->decrRefCount();
    pd_rope = 0;
  }
  if (pd_repoId)
    delete [] pd_repoId;
  pd_repoId = 0;
  if (pd_original_repoId)
    delete [] pd_original_repoId;
  pd_original_repoId = 0;
  if (is_proxy() && pd_objkey.foreign) {
    delete [] pd_objkey.foreign;
    pd_objkey.foreign = 0;
  }
  if (pd_iopprofile) {
    delete pd_iopprofile;
    pd_iopprofile = 0;
  }
  if (pd_flags.transient_exception_handler |
      pd_flags.commfail_exception_handler |
      pd_flags.system_exception_handler) {
    omniExHandlers_iterator iter;
    iter.remove(this);
  }
  return;
}


class nilObjectManager : public omniObjectManager {
public:
  ropeFactoryList* incomingRopeFactories() {
    throw omniORB::fatalException(__FILE__,__LINE__,
				  "Invalid call to nilObjectManger");
#ifdef NEED_DUMMY_RETURN
    return 0;
#endif    
  }
  Rope* defaultLoopBack() {
    throw omniORB::fatalException(__FILE__,__LINE__,
				  "Invalid call to nilObjectManger");
#ifdef NEED_DUMMY_RETURN
    return 0;
#endif
  }
};

static nilObjectManager _nilObjectManager;

omniObjectManager*
omniObject::nilObjectManager()
{
  return &_nilObjectManager;
}


//////////////////////////////////////////////////////////////////////
/////////////////////// internal_get_interface ///////////////////////
//////////////////////////////////////////////////////////////////////

class OmniORBGetInterfaceCallDesc : public OmniProxyCallDesc {
public:
  inline OmniORBGetInterfaceCallDesc(const char* _search_id)
    : OmniProxyCallDesc("lookup_id", 10),
      arg_search_id(_search_id)  {}

  virtual CORBA::ULong alignedSize(CORBA::ULong size_in);
  virtual void marshalArguments(GIOP_C&);
  virtual void unmarshalReturnedValues(GIOP_C&);

  inline CORBA::Object_ptr result() { return pd_result; }

private:
  const char* arg_search_id;
  CORBA::Object_ptr pd_result;
};


CORBA::ULong
OmniORBGetInterfaceCallDesc::alignedSize(CORBA::ULong msgsize)
{
  msgsize = omni::align_to(msgsize,omni::ALIGN_4);
  return msgsize + 4 + (arg_search_id ? strlen(arg_search_id) + 1 : 1);
}


void
OmniORBGetInterfaceCallDesc::marshalArguments(GIOP_C& giop_client)
{
  CORBA::String_member s;
  s._ptr = (char*) arg_search_id;
  s >>= giop_client;
  s._ptr = 0;
}


void
OmniORBGetInterfaceCallDesc::unmarshalReturnedValues(GIOP_C& giop_client)
{
  pd_result = CORBA::Object::unmarshalObjRef(giop_client);
}


static CORBA::Object_ptr
internal_get_interface(const char* repoId)
{
  // Obtain the object reference for the interface repository.
  CORBA::Object_var repository = omniInitialReferences::singleton()
    ->get("InterfaceRepository");
  if( CORBA::is_nil(repository) )
    throw CORBA::INTF_REPOS(0, CORBA::COMPLETED_NO);

  // Make a call to the interface repository.
  OmniORBGetInterfaceCallDesc call_desc(repoId);
  OmniProxyCallWrapper::invoke(repository->PR_getobj(), call_desc);
  return call_desc.result();
}


