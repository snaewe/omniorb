// -*- Mode: C++; -*-
//                            Package   : omniORB2
// object.cc                  Created on: 13/5/96
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
  Revision 1.8  1997/08/21 21:59:12  sll
  ~omniObject() now delete pd_iopprofile.

// Revision 1.7  1997/05/06  15:25:34  sll
// Public release.
//
  */

#include <omniORB2/CORBA.h>

omniObject::omniObject()                     // ctor for local object
{
  omniORB::generateNewKey(pd_objkey.native);
  pd_objkeysize = sizeof(pd_objkey.native);
  pd_repoId = 0;
  pd_rope = 0;
  pd_proxy = 0;
  pd_refCount = 0;
  pd_next = 0;
  pd_disposed = 0;
  pd_existentverified = 1;
  pd_forwardlocation = 0;
  pd_iopprofile = omni::objectToIopProfiles(this);
  return;
}


omniObject::omniObject(const char *repoId,   // ctor for proxy object
	       Rope *r,
	       CORBA::Octet *key,
	       size_t keysize,
	       IOP::TaggedProfileList *profiles,
	       CORBA::Boolean release)
{
  if (!repoId || !r || !key || !keysize || !profiles)
    throw CORBA::BAD_PARAM(0,CORBA::COMPLETED_NO);

  pd_repoId = new char[strlen(repoId)+1];
  if (!pd_repoId)
    throw CORBA::NO_MEMORY(0,CORBA::COMPLETED_NO);
  strcpy(pd_repoId,repoId);

  pd_rope = r;  // caller has already done a r->incrRefCount()
  pd_proxy = 1;
  pd_refCount = 0;
  pd_next = 0;
  pd_disposed = 0;
  pd_existentverified = 0;
  pd_forwardlocation = 0;

  if (!release) {
    pd_iopprofile = 0;
    pd_objkey.foreign = 0;
    try {
      pd_iopprofile = new IOP::TaggedProfileList(*profiles);
      if (!pd_iopprofile)
	throw CORBA::NO_MEMORY(0,CORBA::COMPLETED_NO);
      pd_objkeysize = keysize;
      pd_objkey.foreign = new CORBA::Octet[keysize];
      if (!pd_objkey.foreign)
	throw CORBA::NO_MEMORY(0,CORBA::COMPLETED_NO);
      memcpy((void *)pd_objkey.foreign,(const void *)key,keysize);
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
  CORBA::Boolean result = 0;
  omniObject::objectTableLock.lock();
  if (is_proxy()) {
    k.rope(pd_rope);
    k.key(pd_objkey.foreign,(CORBA::ULong)pd_objkeysize);
    result = pd_forwardlocation;
  }
  else {
    k.rope(0);
    k.key((CORBA::Octet*)&pd_objkey.native,sizeof(pd_objkey.native));
  }
  omniObject::objectTableLock.unlock();
  return result;
}

void
omniObject::setRopeAndKey(const omniRopeAndKey& l,
			  CORBA::Boolean keepIOP)
{
  omniObject::objectTableLock.lock();
  try {
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
	IOP::TaggedProfileList * np = new IOP::TaggedProfileList(1);
	if (!np)
	  throw CORBA::NO_MEMORY(0,CORBA::COMPLETED_NO);
	try {
	  pd_rope->iopProfile((CORBA::Char *)pd_objkey.foreign,
			      pd_objkeysize,
			      ((IOP::TaggedProfileList &) *np)[0]);
	}	
	catch (...) {
	  delete np;
	  throw;
	}
	delete pd_iopprofile;
	pd_iopprofile = np;
	pd_forwardlocation = 0;
      }
      else {
	pd_forwardlocation = 1;
      }
    }
    else {
      if (pd_refCount)
	throw CORBA::BAD_PARAM(0,CORBA::COMPLETED_NO);
      memcpy((void*)&pd_objkey.native,
	     (void*)l.key(),
	     sizeof(pd_objkey.native));
      if (!keepIOP) {
	IOP::TaggedProfileList *np = new IOP::TaggedProfileList;
	if (!np)
	  throw CORBA::NO_MEMORY(0,CORBA::COMPLETED_NO);
	try {
	  Rope_iterator next(&Anchor::incomingAnchor);
	  Rope *r;
	  while ((r = next())) {
	    np->length(np->length()+1);
	    r->iopProfile((CORBA::Char *)&pd_objkey.native,
			  pd_objkeysize,
			  ((IOP::TaggedProfileList &)*np)[np->length()-1]);
	  }
	}
	catch (...) {
	  delete np;
	  throw;
	}
	delete pd_iopprofile;
	pd_iopprofile = np;
	pd_forwardlocation = 0;
      }
      else {
	pd_forwardlocation = 1;
      }
    }
  }
  catch(...) {
    omniObject::objectTableLock.unlock();
    throw;
  }
  omniObject::objectTableLock.unlock();
}

void
omniObject::resetRopeAndKey()
{
  omniObject::objectTableLock.lock();
  try {
    if (is_proxy()) {
      if (pd_forwardlocation) {
	Rope*          _rope;
	CORBA::Octet*  _key;
	size_t         _keysize;
	_rope = omni::iopProfilesToRope(pd_iopprofile,_key,_keysize);
	if (pd_rope) {
	  pd_rope->decrRefCount();
	}
	pd_rope = _rope;
	if (pd_objkeysize < _keysize) {
	  delete [] pd_objkey.foreign;
	  pd_objkey.foreign = new CORBA::Octet[_keysize];
	}
	pd_objkeysize = _keysize;
	memcpy((void*)pd_objkey.foreign,(void*)_key,_keysize);
	pd_forwardlocation = 0;
	pd_existentverified = 0;
      }
    }
    else {
      // A local object, Not allowed to reset the Rope and Key
      // just ignored this command.
    }
  }
  catch(...) {
    omniObject::objectTableLock.unlock();
    throw;
  }
  omniObject::objectTableLock.unlock();

}

void
omniObject::PR_IRRepositoryId(const char *ir)
{
  if (pd_refCount || pd_proxy)
    throw omniORB::fatalException(__FILE__,__LINE__,
      "omniObject::PR_IRRepositoryId()- tried to set IR Id for a proxy or an activated object");
  if (pd_repoId)
    delete [] pd_repoId;
  pd_repoId = new char [strlen(ir)+1];
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
	                          "omniObject::dispatch()- unexpected call to this function inside a proxy object.");
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
      if (_widenFromTheMostDerivedIntf(id))
	_result = 1;
      else
	_result = 0;
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
      if (omniORB::traceLevel > 0) {
	cerr << "Warning: received GIOP request - get_interface, this function is not supported yet.\n" 
	     << "         A CORBA::BAD_OPERATION is raised."
	     << endl;
      }
      throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
    }
  else if (strcmp(_op,"_implementation") == 0)
    {
      if (omniORB::traceLevel > 0) {
	cerr << "Warning: received GIOP request - get_implementation, this function is not supported yet.\n" 
	     << "         A CORBA::BAD_OPERATION is raised."
	     << endl;
      }
      throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
    }
  else
    return 0;

  // For MSVC++ 4.2:
  return 0;
}


void
omniObject::assertObjectExistent()
{
  if (pd_existentverified)
    return;

  // We don't put mutual exclusion on pd_existentverified so potentially
  // we can have multiple threads doing the following simulataneously.
  // That is alright except perhaps a bit of redundent work.
  omniRopeAndKey rak;
  CORBA::Boolean fwd = getRopeAndKey(rak);

  try {
    GIOP_C _c(rak.rope());
    CORBA::ULong _msgsize = GIOP_C::RequestHeaderSize(rak.keysize(),14);

    switch (_c.IssueLocateRequest(rak.key(),rak.keysize()))
      {
      case GIOP::OBJECT_HERE:
	pd_existentverified = 1;
	_c.RequestCompleted();
	break;
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
		cerr << "Received GIOP::OBJECT_FORWARD in LocateReply message that contains a nil object reference." << endl;
	      }
	      throw CORBA::COMM_FAILURE(0,CORBA::COMPLETED_NO);
	    }
	    omniRopeAndKey __r;
	    obj->PR_getobj()->getRopeAndKey(__r);
	    setRopeAndKey(__r);
	    _c.~GIOP_C();
	  }
	  if (omniORB::traceLevel > 10) {
	    cerr << "GIOP::OBJECT_FORWARD: retry request." << endl;
	  }
	  assertObjectExistent();
	}
      }
  }
  catch (const CORBA::COMM_FAILURE& ex) {
    if (fwd) {
      resetRopeAndKey();
      throw CORBA::TRANSIENT(0,CORBA::COMPLETED_NO);
    }
    throw;
  }
}


omniObject::~omniObject()
{
  if (pd_refCount) {
    // A dtor should not be called if the reference count is not 0
    assert(0);
  }
  if (pd_rope)
    pd_rope->decrRefCount();
  pd_rope = 0;
  if (pd_repoId)
    delete [] pd_repoId;
  pd_repoId = 0;
  if (pd_proxy && pd_objkey.foreign) {
    delete [] pd_objkey.foreign;
    pd_objkey.foreign = 0;
  }
  if (pd_iopprofile) {
    delete pd_iopprofile;
    pd_iopprofile = 0;
  }
  return;
}

