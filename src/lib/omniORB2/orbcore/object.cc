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
  Revision 1.18.4.2  1999/10/02 18:21:28  sll
  Added support to decode optional tagged components in the IIOP profile.
  Added support to negogiate with a firewall proxy- GIOPProxy to invoke
  remote objects inside a firewall.
  Added tagged component TAG_ORB_TYPE to identify omniORB IORs.

  Revision 1.18.4.1  1999/09/15 20:18:30  sll
  Updated to use the new cdrStream abstraction.
  Marshalling operators for NetBufferedStream and MemBufferedStream are now
  replaced with just one version for cdrStream.
  Derived class giopStream implements the cdrStream abstraction over a
  network connection whereas the cdrMemoryStream implements the abstraction
  with in memory buffer.

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
  Revision 1.18.4.2  1999/10/02 18:21:28  sll
  Added support to decode optional tagged components in the IIOP profile.
  Added support to negogiate with a firewall proxy- GIOPProxy to invoke
  remote objects inside a firewall.
  Added tagged component TAG_ORB_TYPE to identify omniORB IORs.

  Revision 1.18.4.1  1999/09/15 20:18:30  sll
  Updated to use the new cdrStream abstraction.
  Marshalling operators for NetBufferedStream and MemBufferedStream are now
  replaced with just one version for cdrStream.
  Derived class giopStream implements the cdrStream abstraction over a
  network connection whereas the cdrMemoryStream implements the abstraction
  with in memory buffer.

  Revision 1.19  1999/09/12 19:01:37  sll
  updated to use the new cdrstreams.

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
#include <giopStreamImpl.h>

static CORBA::Object_ptr internal_get_interface(const char* repoId);

omniObject::omniObject(omniObjectManager* m)
{
  pd_refCount = 0;
  pd_next = 0;
  pd_flags.proxy = 0;
  pd_flags.disposed = 0;
  pd_flags.forwardlocation = 0;
  pd_flags.transient_exception_handler = 0;
  pd_flags.commfail_exception_handler = 0;
  pd_flags.system_exception_handler = 0;
  pd_flags.existent_and_type_verified = 1;
  pd_objectInfo   = 0;
  pd_data.l.pd_manager = m;
  if( m != omniObject::nilObjectManager() ) {
    omniORB::generateNewKey(pd_data.l.pd_key);
    pd_data.l.pd_manager = ((m) ? m : omniObjectManager::root());
  }
}

void
omniObject::PR_setRepositoryID(const char* repoId)
{
  if (is_proxy())
    throw omniORB::fatalException(__FILE__,__LINE__,
	 "omniObject::PR_setRepostioryID called in the wrong state.");

  pd_repositoryID = repoId;
}

void
omniObject::PR_setKey(const omniObjectKey& k)
{
  if (is_proxy())
    throw omniORB::fatalException(__FILE__,__LINE__,
	 "omniObject::PR_setKey called in the wrong state.");

  pd_data.l.pd_key = k;
}

omniObject::omniObject(GIOPObjectInfo* objInfo,const char* use_as_repoID)
{
  pd_refCount = 0;
  pd_next = 0;
  pd_flags.disposed = 0;
  pd_flags.existent_and_type_verified = 0;
  pd_flags.forwardlocation = 0;
  pd_flags.transient_exception_handler = 0;
  pd_flags.commfail_exception_handler = 0;
  pd_flags.system_exception_handler = 0;
  pd_flags.proxy = 1;

  pd_repositoryID = objInfo->repositoryID();

  pd_objectInfo = objInfo;

  if (strcmp((const char*)pd_repositoryID,use_as_repoID) != 0) {
    pd_data.p.pd_use_as_repositoryID = CORBA::string_dup(use_as_repoID);
  }
  else {
    pd_data.p.pd_use_as_repositoryID = 0;
  }

  pd_data.p.pd_originalInfo = 0;

}


omniObject::~omniObject()
{
  if (pd_refCount) {
    // A dtor should not be called if the reference count is not 0
    assert(0);
  }

  if (pd_flags.transient_exception_handler |
      pd_flags.commfail_exception_handler |
      pd_flags.system_exception_handler) {
    omniExHandlers_iterator iter;
    iter.remove(this);
  }

  if (pd_objectInfo) pd_objectInfo->release();

  if (pd_flags.proxy) {
    if (pd_data.p.pd_use_as_repositoryID) 
      CORBA::string_free(pd_data.p.pd_use_as_repositoryID);
    if (pd_data.p.pd_originalInfo)
      pd_data.p.pd_originalInfo->release();
  }
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
  
  cdrStream& s = (cdrStream&)_s;

  if (strcmp(_op,"_is_a") == 0)
    {
      if (!_response_expected) {
	throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
      }
      CORBA::String_var id;
      {
	CORBA::ULong _len;
	_len <<= s;
	if (!_len || !s.checkInputOverrun(1,_len))
	  throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);
	id = CORBA::string_alloc(_len-1);
	if (!((const char *)id))
	  throw CORBA::NO_MEMORY(0,CORBA::COMPLETED_NO);
	s.get_char_array((CORBA::Char *)((const char *)id),_len);
      }
      _s.RequestReceived();
      CORBA::Boolean _result;
      _result = _real_is_a(id);
      _s.InitialiseReply(GIOP::NO_EXCEPTION);
      _result >>= s;
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
      _s.InitialiseReply(GIOP::NO_EXCEPTION);
      _result >>= s;
      _s.ReplyCompleted();
      return 1;
    }
  else if (strcmp(_op,"_interface") == 0)
    {
      _s.RequestReceived();
      CORBA::Object_ptr result;
      result = internal_get_interface(NP_IRRepositoryId());
      _s.InitialiseReply(GIOP::NO_EXCEPTION);
      CORBA::Object::marshalObjRef(result, s);
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

  if (pd_data.p.pd_use_as_repositoryID != 0) 
    {
      // If the repository ID has been updated,
      // we have to verify if the object does indeed support the interface
      // of this proxy object. The repository ID of this proxy object is
      // stored in pd_use_as_repositoryID. Use _is_a() operation to query the
      // remote object to see if it supports the interface identified by
      // pd_use_as_repositoryID.
      if (!_real_is_a(pd_data.p.pd_use_as_repositoryID)) {
	if (omniORB::traceLevel > 0) {
	  omniORB::log << "Warning: in omniObject::assertObjectExistent(), the object with the IR repository ID:\n"
	       << "         " << NP_IRRepositoryId() << " returns FALSE to the query\n"
	       << "         is_a(\"" << pd_data.p.pd_use_as_repositoryID << "\").\n"
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

      while (1) {

	CORBA::Boolean fwd;
	GIOPObjectInfo* invokeInfo = getInvokeInfo(fwd);

	try {

	  GIOP_C _c(invokeInfo);

	  GIOP::LocateStatusType rc;

	  switch ((rc = _c.IssueLocateRequest()))
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
	    case GIOP::OBJECT_FORWARD_PERM:
	      {
		CORBA::Object_var obj =
		  CORBA::Object::unmarshalObjRef((cdrStream&)_c);
		_c.RequestCompleted();
		if( CORBA::is_nil(obj) ){
		  if( omniORB::traceLevel > 10 ){
		    omniORB::log << "Received GIOP::LOCATION_FORWARD message that"
		      " contains a nil object reference.\n";
		    omniORB::log.flush();
		  }
		  throw CORBA::COMM_FAILURE(0, CORBA::COMPLETED_NO);
		}
		GIOPObjectInfo* newinfo = obj->PR_getobj()->getInvokeInfo(fwd);
		setInvokeInfo(newinfo,
			      (rc == GIOP::OBJECT_FORWARD_PERM) ? 0 : 1);
		break;
	      }

	    default:
	      throw omniORB::fatalException(__FILE__,__LINE__,
					    "omniObject::assertObjectExistent "
					    " returned an invalid code");
	    }
	}
	catch (const CORBA::COMM_FAILURE& ex) {
	  if (fwd) {
	    resetInvokeInfo();
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
      }
    }
}

class omniIsACallDesc  : public OmniProxyCallDesc
{
public:
  inline omniIsACallDesc(const char* _op, size_t _op_len, const char* p) :
    OmniProxyCallDesc(_op, _op_len), arg_x(p)  {
    skipAssertObjectExistence();
  }

  virtual void marshalArguments(cdrStream& s) {
    CORBA::ULong _len = (((const char*) arg_x) ? 
			 strlen((const char*) arg_x) + 1 : 1);
    _len >>= s;
    if (_len > 1)
      s.put_char_array((const CORBA::Char *)((const char*) arg_x),_len);
    else {
      if ((const char*) arg_x == 0 && omniORB::traceLevel > 1)
        _CORBA_null_string_ptr(0);
      CORBA::Char('\0') >>= s;
    }
  }

  virtual void unmarshalReturnedValues(cdrStream& s) {
    pd_result <<= s;
  }

  inline CORBA::Boolean result() { return pd_result; }

private:
  const char* arg_x;
  CORBA::Boolean pd_result;
};


CORBA::Boolean
omniObject::_real_is_a(const char *repoId)
{
  if (!repoId)
    return 0;

  if (!is_proxy() || pd_data.p.pd_use_as_repositoryID == 0) {
    if (_widenFromTheMostDerivedIntf(repoId) ||
	strcmp((const char*)CORBA::Object::repositoryID,repoId) == 0)
      return 1;
    else
      return 0;
  }

  // Reach here because is_proxy() == 1 and pd_use_as_repositoryID != 0
  // This is the case when the repository ID of the object is not the
  // same as the original repository ID of this proxy object.
  // That is, we don't know the exact type of this object but some time
  // in the past we were told this object is of type pd_use_as_repositoryID.
  // We have to use the _is_a operation to query the remote object to see
  // see if it really is an instance of the type identified by <repoId>.

  if (omniORB::traceLevel > 10) {
    omniORB::log << "Info: omniORB uses CORBA::Object::_is_a operation to check if "
	 << NP_IRRepositoryId() << " is a " << repoId << "\n";
    omniORB::log.flush();
  }

  omniIsACallDesc _call_desc("_is_a",6,repoId);
  OmniProxyCallWrapper::invoke(this, _call_desc);
  return _call_desc.result();
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
	  GIOPObjectInfo* objInfo;
	  if (!(objInfo = pd_data.p.pd_originalInfo))
	    objInfo = pd_objectInfo;
	  omniObject* omniobj = omni::createObjRef(NP_IRRepositoryId(),repoId,
			new IOP::TaggedProfileList(*objInfo->iopProfiles()));
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

  virtual void marshalArguments(cdrStream&);
  virtual void unmarshalReturnedValues(cdrStream&);

  inline CORBA::Object_ptr result() { return pd_result; }

private:
  const char* arg_search_id;
  CORBA::Object_ptr pd_result;
};


void
OmniORBGetInterfaceCallDesc::marshalArguments(cdrStream& s)
{
  CORBA::String_member m;
  m._ptr = (char*) arg_search_id;
  m >>= s;
  m._ptr = 0;
}


void
OmniORBGetInterfaceCallDesc::unmarshalReturnedValues(cdrStream& s)
{
  pd_result = CORBA::Object::unmarshalObjRef(s);
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

//////////////////////////////////////////////////////////////////////
/////////////////////// GIOPObjectInfo         ///////////////////////
//////////////////////////////////////////////////////////////////////

GIOPObjectInfo::GIOPObjectInfo() : iopProfiles_(0), 
                                   orb_type_(0), tag_components_(0),
                                   opaque_data_(0),
                                   pd_refcount(1) {}

GIOPObjectInfo::~GIOPObjectInfo() {
  if (iopProfiles_) { delete iopProfiles_; iopProfiles_ = 0; }
  if (tag_components_) { delete tag_components_; tag_components_ = 0; }
  if (opaque_data_) { 
    for (CORBA::ULong index = 0; index < opaque_data_->length(); index++) {
      (*opaque_data_)[index].destructor((*opaque_data_)[index].data);
    }
    delete opaque_data_;	
    opaque_data_ = 0;
  }
}

GIOPObjectInfo*
omniObject::getInvokeInfo(CORBA::Boolean& location_forwarded)
{
  omni_mutex_lock sync(omniObject::objectTableLock);

  if (!is_proxy() && !pd_objectInfo) {

    if (omniORB::trace(15)) {
      omniORB::logger log("omniORB");
      log << "getInvokeInfo: invoke on local object\n";
    }

    // We create a GIOPObjectInfo if we haven't got one yet.
    pd_objectInfo = new GIOPObjectInfo();
    pd_objectInfo->repositoryID_ = pd_repositoryID;

    IOP::TaggedProfileList_var pl(new IOP::TaggedProfileList);
    ropeFactory_iterator iter(pd_data.l.pd_manager->incomingRopeFactories());
    incomingRopeFactory* rp;
    while ((rp = (incomingRopeFactory*) iter())) {
      rp->getIncomingIOPprofiles((CORBA::Octet*)&pd_data.l.pd_key,
				 sizeof(pd_data.l.pd_key),
				 *(pl.operator->()));
    }
    pd_objectInfo->iopProfiles_ = pl._retn();
    // We also have to return a loopback rope in case 
    // the caller is using the rope to contact this object. One situation
    // in which this can occur is when the LOCATION FORWARDING code in
    // a proxy object's stub get this object as the new object reference
    // to forward the request to.
    pd_objectInfo->rope_ = pd_data.l.pd_manager->defaultLoopBack();
    pd_objectInfo->rope_->incrRefCount();
    pd_objectInfo->version_ = giopStreamImpl::maxVersion()->version();
    pd_objectInfo->object_key_.length(sizeof(pd_data.l.pd_key));
    memcpy((void*)pd_objectInfo->object_key_.get_buffer(),
	   (void*)&pd_data.l.pd_key,sizeof(pd_data.l.pd_key));
  }
  else {
    location_forwarded = pd_flags.forwardlocation;
  }
  pd_objectInfo->duplicateNoLock();
  return pd_objectInfo;
}

void
omniObject::setInvokeInfo(GIOPObjectInfo* g, CORBA::Boolean keepIOP)
{
  GIOPObjectInfo *old,*org;
  CORBA::Boolean delold = 0;
  CORBA::Boolean delorg = 0;
  {
    omni_mutex_lock sync(omniObject::objectTableLock);

    if (!is_proxy()) return;

    old = pd_objectInfo;
    pd_objectInfo = g;

    if (keepIOP) {
      pd_flags.forwardlocation = 1;
      if (!pd_data.p.pd_originalInfo) {
	pd_data.p.pd_originalInfo = old;
      }
      else {
	delold = (old->releaseNoLock() <= 0) ? 1 : 0;
      }
    }
    else {
      pd_flags.forwardlocation = 0;
      delold = (old->releaseNoLock() <= 0) ? 1 : 0;
      if ((org = pd_data.p.pd_originalInfo)) {
	delorg = (pd_data.p.pd_originalInfo->releaseNoLock() <= 0) ? 1 : 0;
	pd_data.p.pd_originalInfo = 0;
      }
    }
  }
  // delete the objects only after the sync lock is released. This prevents
  // any deadlock if the object recursively delete other GIOPObjectInfos
  // stored in GIOPObjectInfo::opaque_sequence.
  if (delold) delete old;
  if (delorg) delete org;
}

void
omniObject::resetInvokeInfo()
{
  GIOPObjectInfo* g;
  CORBA::Boolean delg = 0;
  {
    omni_mutex_lock sync(omniObject::objectTableLock);

    if (!is_proxy()) return;

    if (pd_flags.forwardlocation) {
      g = pd_objectInfo;
      delg = ((g->releaseNoLock() <= 0) ? 1 : 0);
      pd_objectInfo = pd_data.p.pd_originalInfo;
      pd_data.p.pd_originalInfo = 0;
      pd_flags.forwardlocation = 0;
      pd_flags.existent_and_type_verified = 0;
    }
  }
  if (delg) delete g;
}

void
GIOPObjectInfo::duplicate()
{
  omni_mutex_lock sync(omniObject::objectTableLock);
  duplicateNoLock();
}

void
GIOPObjectInfo::duplicateNoLock()
{

  if (pd_refcount <= 0)
    throw omniORB::fatalException(__FILE__,__LINE__,
			    "GIOPObjectInfo::duplicate() -ve ref count.");
  pd_refcount++;
}

void
GIOPObjectInfo::release()
{
  CORBA::Boolean del;
  {
    omni_mutex_lock sync(omniObject::objectTableLock);
    del = ((releaseNoLock() <= 0) ? 1 : 0);
  }
  if (del) delete this;
}

int
GIOPObjectInfo::releaseNoLock()
{
  if (pd_refcount <= 0)
    throw omniORB::fatalException(__FILE__,__LINE__,
			    "GIOPObjectInfo::release() -ve ref count.");
  return --pd_refcount;
}

void
omniObject::getKey(omniObjectKey& k) const
{
  assert(!is_proxy());

  memcpy((void*)&k,(void*)&pd_data.l.pd_key,sizeof(k));
}
