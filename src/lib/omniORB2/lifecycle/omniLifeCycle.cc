// -*- Mode: C++; -*-
//                            Package   : omniORB2
// omniLifeCycle.cc           Created on: 1997/0920
//                            Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 1997-1999 AT&T Laboratories Cambridge
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

// $Log$
// Revision 1.5  1999/06/28 13:22:45  dpg1
// Some functions moved from omniLC.h.
//
// Revision 1.4  1999/03/11 16:26:04  djr
// Updated copyright notice
//
// Revision 1.3  1998/04/07 19:41:25  sll
// Updated when omniORB is a namespace.
//
// Revision 1.2  1997/12/10 11:39:33  sll
// Updated life cycle runtime.
//
// Revision 1.1  1997/09/20  17:04:23  dpg1
// Initial revision
//
// Revision 1.1  1997/09/20  17:04:23  dpg1
// Initial revision
//

#include <omniORB2/omniLC.h>

// _wrap_proxy:

void
omniLC::
_wrap_proxy::_register_wrap(omniObject *obj) {
  {
    omniRopeAndKey l;
    obj->getRopeAndKey(l);
    _wrapped_key = *((omniObjectKey *)l.key());
  }
  omniObject::wrappedObjectTableLock.lock();
  omniLC::_wrap_proxy **p = (omniLC::_wrap_proxy **)
    (&omniObject::wrappedObjectTable[omniORB::hash(_wrapped_key)]);
  _next_wrap_proxy = *p;
  *p = this;
  omniObject::wrappedObjectTableLock.unlock();
}

void
omniLC::
_wrap_proxy::_unregister_wrap() {
  omniObject::wrappedObjectTableLock.lock();
  omniLC::_wrap_proxy **p = (omniLC::_wrap_proxy **)
    (&omniObject::wrappedObjectTable[omniORB::hash(_wrapped_key)]);
  while (*p) {
    if (*p == this) {
      *p = _next_wrap_proxy;
      break;
    }
    p = &((*p)->_next_wrap_proxy);
  }
  _next_wrap_proxy = 0;
  omniObject::wrappedObjectTableLock.unlock();
}

void
omniLC::
_wrap_proxy::_reset_wraps(omniObject *obj) {
  omniObjectKey k;
  {
    omniRopeAndKey l;
    obj->getRopeAndKey(l);
    k = *((omniObjectKey *)l.key());
  }
  omniObject::wrappedObjectTableLock.lock();
  omniLC::_wrap_proxy **p = (omniLC::_wrap_proxy **)
    (&omniObject::wrappedObjectTable[omniORB::hash(k)]);
  while (*p) {
#ifdef HAS_Cplusplus_Namespace
    // operator== is defined in the omniORB namespace
    if (omniORB::operator==((*p)->_wrapped_key,k)) {
#else
    // operator== is defined in the global namespace
    if ((*p)->_wrapped_key == k) {
#endif
      (*p)->_reset_proxy();
      *p = (*p)->_next_wrap_proxy;
    }
    else
      p = &((*p)->_next_wrap_proxy);
  }
  omniObject::wrappedObjectTableLock.unlock();
}


// LifeCycleInfo_i:

omniLC::
LifeCycleInfo_i::LifeCycleInfo_i(_wrap_home *w, CORBA::Object_ptr h)
  : wrap(w)
{
  home = CORBA::Object::_duplicate(h);
}

void
omniLC::
LifeCycleInfo_i::reportMove(CORBA::Object_ptr obj) {
  wrap->_move(obj);
}

void
omniLC::
LifeCycleInfo_i::reportRemove() {
  wrap->_remove();
  CORBA::BOA::getBOA()->dispose(this);
}

CORBA::Object_ptr
omniLC::
LifeCycleInfo_i::homeObject() {
  return CORBA::Object::_duplicate(home);
}


// _lc_sk:

omniLC::
_lc_sk::_lc_sk() {
  _linfo = omniLifeCycleInfo::_nil();
}

void
omniLC::
_lc_sk::_set_linfo(omniLifeCycleInfo_ptr li) {
  if (CORBA::is_nil(_linfo)) {
    _linfo = omniLifeCycleInfo::_duplicate(li);
  }
  else {
    // _set_linfo called after a call to _this() or called twice
    throw omniORB::fatalException(__FILE__,__LINE__,
				  "_set_linfo() must only be called once,"
				  " before the first call to _this().");
  }
}

omniLifeCycleInfo_ptr
omniLC::
_lc_sk::_get_linfo() {
  return _linfo;
}


// Proxy call wrapper:

CORBA::Boolean
OmniLCProxyCallWrapper::invoke(omniObject* o,
			       OmniProxyCallDesc& call_desc,
			       omniLC::_wrap_proxy *wp)
{
  CORBA::ULong retries = 0;

#ifndef EGCS_WORKAROUND
_again:
#else
  while(1) {
#endif
    if (omniORB::verifyObjectExistsAndType)
      o->assertObjectExistent();
    omniRopeAndKey ropeAndKey;
    o->getRopeAndKey(ropeAndKey);
    CORBA::Boolean reuse = 0;

    try{
      // Get a GIOP driven strand
      GIOP_C giop_client(ropeAndKey.rope());
      reuse = giop_client.isReUsingExistingConnection();

      // Calculate the size of the message.
      CORBA::ULong message_size =
	GIOP_C::RequestHeaderSize(ropeAndKey.keysize(),
				  call_desc.operation_len());

      message_size = call_desc.alignedSize(message_size);

      giop_client.InitialiseRequest(ropeAndKey.key(), ropeAndKey.keysize(),
				    call_desc.operation(),
				    call_desc.operation_len(),
				    message_size, 0);

      // Marshal the arguments to the operation.
      call_desc.marshalArguments(giop_client);

      // Wait for the reply.
      switch(giop_client.ReceiveReply()){
      case GIOP::NO_EXCEPTION:
	// Unmarshal the result and out/inout arguments.
	call_desc.unmarshalReturnedValues(giop_client);

	giop_client.RequestCompleted();
	return 1;

      case GIOP::USER_EXCEPTION:
	{
	  if( !call_desc.has_user_exceptions() ) {
	    giop_client.RequestCompleted(1);
	    throw CORBA::UNKNOWN(0, CORBA::COMPLETED_MAYBE);
	  }

	  // Retrieve the Interface Repository ID of the exception.
	  CORBA::ULong repoIdLen;
	  repoIdLen <<= giop_client;
	  CORBA::String_var repoId(CORBA::string_alloc(repoIdLen - 1));
	  giop_client.get_char_array((CORBA::Char*)(char*)repoId,
				     repoIdLen);

	  call_desc.userException(giop_client, repoId);
	  // Never get here - this must throw either a user exception
	  // or CORBA::MARSHAL.
	}

      case GIOP::SYSTEM_EXCEPTION:
	giop_client.RequestCompleted(1);
	throw omniORB::fatalException(__FILE__,__LINE__,
				      "GIOP::SYSTEM_EXCEPTION should not be"
				      " returned by GIOP_C::ReceiveReply()");

      case GIOP::LOCATION_FORWARD:
	{
	  CORBA::Object_var obj(CORBA::Object::unmarshalObjRef(giop_client));
	  giop_client.RequestCompleted();
	  if( CORBA::is_nil(obj) ){
	    if( omniORB::traceLevel > 10 ){
	      omniORB::log << "Received GIOP::LOCATION_FORWARD message that"
		" contains a nil object reference.\n";
	      omniORB::log.flush();
	    }
	    throw CORBA::COMM_FAILURE(0, CORBA::COMPLETED_NO);
	  }
	  giop_client.~GIOP_C();
	  wp->_forward_to(obj);
	  if( omniORB::traceLevel > 10 ){
	    omniORB::log << "GIOP::LOCATION_FORWARD: retry request.\n";
	    omniORB::log.flush();
	  }
	}
	return 0;

      default:
	throw omniORB::fatalException(__FILE__,__LINE__,
				      "GIOP_C::ReceiveReply"
				      " returned an invalid code");
      }
    }
    catch(const CORBA::COMM_FAILURE& ex){
      if (reuse) {
	CORBA::TRANSIENT ex2(ex.minor(), ex.completed());
	if( !_omni_callTransientExceptionHandler(o, retries++, ex2) )
	  throw ex2;
      }
      else if (wp->_forwarded()) {
	wp->_reset_proxy();
	CORBA::TRANSIENT ex2(ex.minor(), ex.completed());
	if (_omni_callTransientExceptionHandler(o, retries++, ex2))
	  return 0;
	else
	  throw ex2;
      }
      else {
	if (!_omni_callCommFailureExceptionHandler(o, retries++, ex))
	  throw;
      }
    }
    catch(const CORBA::TRANSIENT& ex){
      if( !_omni_callTransientExceptionHandler(o, retries++, ex) )
	throw;
    }
    catch(const CORBA::OBJECT_NOT_EXIST& ex){
      if (wp->_forwarded()) {
	wp->_reset_proxy();
	CORBA::TRANSIENT ex2(ex.minor(), ex.completed());
	if (_omni_callTransientExceptionHandler(o, retries++, ex2))
	  return 0;
	else
	  throw ex2;
      }
      else {
	if (!_omni_callSystemExceptionHandler(o, retries++, ex))
	  throw;
      }
    }
    catch(const CORBA::SystemException& ex){
      if( !_omni_callSystemExceptionHandler(o, retries++, ex) )
	throw;
    }

#ifndef EGCS_WORKAROUND
    goto _again;
#else
  }
#endif
}


CORBA::Boolean
OmniLCProxyCallWrapper::one_way(omniObject* o,
				OmniOWProxyCallDesc& call_desc,
				omniLC::_wrap_proxy *wp)
{
  CORBA::ULong retries = 0;

#ifndef EGCS_WORKAROUND
_again:
#else
  while(1) {
#endif
    if (omniORB::verifyObjectExistsAndType)
      o->assertObjectExistent();
    omniRopeAndKey ropeAndKey;
    o->getRopeAndKey(ropeAndKey);
    CORBA::Boolean reuse = 0;

    try{
      // Get a GIOP driven strand
      GIOP_C giop_client(ropeAndKey.rope());
      reuse = giop_client.isReUsingExistingConnection();

      // Calculate the size of the message.
      CORBA::ULong message_size =
	GIOP_C::RequestHeaderSize(ropeAndKey.keysize(),
				  call_desc.operation_len());

      message_size = call_desc.alignedSize(message_size);

      giop_client.InitialiseRequest(ropeAndKey.key(), ropeAndKey.keysize(),
				    call_desc.operation(),
				    call_desc.operation_len(),
				    message_size, 1);

      // Marshal the arguments to the operation.
      call_desc.marshalArguments(giop_client);

      // Wait for the reply.
      switch(giop_client.ReceiveReply()){
      case GIOP::NO_EXCEPTION:
	giop_client.RequestCompleted();
	return 1;

      case GIOP::USER_EXCEPTION:
      case GIOP::SYSTEM_EXCEPTION:
      case GIOP::LOCATION_FORWARD:
	giop_client.RequestCompleted(1);
	throw omniORB::fatalException(__FILE__,__LINE__,
				      "GIOP_C::ReceiveReply() returned"
				      " unexpected code on oneway");

      default:
	throw omniORB::fatalException(__FILE__,__LINE__,
				      "GIOP_C::ReceiveReply"
				      " returned an invalid code");
      }
    }
    catch(const CORBA::COMM_FAILURE& ex){
      if (reuse) {
	CORBA::TRANSIENT ex2(ex.minor(), ex.completed());
	if( !_omni_callTransientExceptionHandler(o, retries++, ex2) )
	  throw ex2;
      }
      else if (wp->_forwarded()) {
	wp->_reset_proxy();
	CORBA::TRANSIENT ex2(ex.minor(), ex.completed());
	if (_omni_callTransientExceptionHandler(o, retries++, ex2))
	  return 0;
	else
	  throw ex2;
      }
      else {
	if (!_omni_callCommFailureExceptionHandler(o, retries++, ex))
	  throw;
      }
    }
    catch(const CORBA::TRANSIENT& ex){
      if( !_omni_callTransientExceptionHandler(o, retries++, ex) )
	throw;
    }
    catch(const CORBA::SystemException& ex){
      if( !_omni_callSystemExceptionHandler(o, retries++, ex) )
	throw;
    }

#ifndef EGCS_WORKAROUND
    goto _again;
#else
  }
#endif
}


// Versions with context:

CORBA::Boolean
OmniLCProxyCallWrapper::invoke(omniObject* o,
			       OmniProxyCallDescWithContext& call_desc,
			       omniLC::_wrap_proxy *wp)
{
  CORBA::ULong retries = 0;

#ifndef EGCS_WORKAROUND
_again:
#else
  while(1) {
#endif
    if (omniORB::verifyObjectExistsAndType)
      o->assertObjectExistent();
    omniRopeAndKey ropeAndKey;
    o->getRopeAndKey(ropeAndKey);
    CORBA::Boolean reuse = 0;

    try{
      // Get a GIOP driven strand
      GIOP_C giop_client(ropeAndKey.rope());
      reuse = giop_client.isReUsingExistingConnection();

      // Calculate the size of the message.
      CORBA::ULong message_size =
	GIOP_C::RequestHeaderSize(ropeAndKey.keysize(),
				  call_desc.operation_len());

      message_size = call_desc.alignedSize(message_size);

      if( call_desc.contexts_expected() )
	message_size = CORBA::Context::NP_alignedSize(call_desc.context(),
				      call_desc.contexts_expected(),
				      call_desc.num_contexts_expected(),
				      message_size);

      giop_client.InitialiseRequest(ropeAndKey.key(), ropeAndKey.keysize(),
				    call_desc.operation(),
				    call_desc.operation_len(),
				    message_size, 0);

      // Marshal the arguments to the operation.
      call_desc.marshalArguments(giop_client);
      if( call_desc.contexts_expected() )
	CORBA::Context::marshalContext(call_desc.context(),
				       call_desc.contexts_expected(),
				       call_desc.num_contexts_expected(),
				       giop_client);

      // Wait for the reply.
      switch(giop_client.ReceiveReply()){
      case GIOP::NO_EXCEPTION:
	// Unmarshal the result and out/inout arguments.
	call_desc.unmarshalReturnedValues(giop_client);

	giop_client.RequestCompleted();
	return 1;

      case GIOP::USER_EXCEPTION:
	{
	  if( !call_desc.has_user_exceptions() ) {
	    giop_client.RequestCompleted(1);
	    throw CORBA::UNKNOWN(0, CORBA::COMPLETED_MAYBE);
	  }

	  // Retrieve the Interface Repository ID of the exception.
	  CORBA::ULong repoIdLen;
	  repoIdLen <<= giop_client;
	  CORBA::String_var repoId(CORBA::string_alloc(repoIdLen - 1));
	  giop_client.get_char_array((CORBA::Char*)(char*)repoId,
				     repoIdLen);

	  call_desc.userException(giop_client, repoId);
	  // Never get here - this must throw either a user exception
	  // or CORBA::MARSHAL.
	}

      case GIOP::SYSTEM_EXCEPTION:
	giop_client.RequestCompleted(1);
	throw omniORB::fatalException(__FILE__,__LINE__,
				      "GIOP::SYSTEM_EXCEPTION should not be"
				      " returned by GIOP_C::ReceiveReply()");

      case GIOP::LOCATION_FORWARD:
	{
	  CORBA::Object_var obj(CORBA::Object::unmarshalObjRef(giop_client));
	  giop_client.RequestCompleted();
	  if( CORBA::is_nil(obj) ){
	    if( omniORB::traceLevel > 10 ){
	      omniORB::log << "Received GIOP::LOCATION_FORWARD message that"
		" contains a nil object reference.\n";
	      omniORB::log.flush();
	    }
	    throw CORBA::COMM_FAILURE(0, CORBA::COMPLETED_NO);
	  }
	  giop_client.~GIOP_C();
	  wp->_forward_to(obj);
	  if( omniORB::traceLevel > 10 ){
	    omniORB::log << "GIOP::LOCATION_FORWARD: retry request.\n";
	    omniORB::log.flush();
	  }
	}
	return 0;

      default:
	throw omniORB::fatalException(__FILE__,__LINE__,
				      "GIOP_C::ReceiveReply"
				      " returned an invalid code");
      }
    }
    catch(const CORBA::COMM_FAILURE& ex){
      if (reuse) {
	CORBA::TRANSIENT ex2(ex.minor(), ex.completed());
	if( !_omni_callTransientExceptionHandler(o, retries++, ex2) )
	  throw ex2;
      }
      else if (wp->_forwarded()) {
	wp->_reset_proxy();
	CORBA::TRANSIENT ex2(ex.minor(), ex.completed());
	if (_omni_callTransientExceptionHandler(o, retries++, ex2))
	  return 0;
	else
	  throw ex2;
      }
      else {
	if (!_omni_callCommFailureExceptionHandler(o, retries++, ex))
	  throw;
      }
    }
    catch(const CORBA::TRANSIENT& ex){
      if( !_omni_callTransientExceptionHandler(o, retries++, ex) )
	throw;
    }
    catch(const CORBA::OBJECT_NOT_EXIST& ex){
      if (wp->_forwarded()) {
	wp->_reset_proxy();
	CORBA::TRANSIENT ex2(ex.minor(), ex.completed());
	if (_omni_callTransientExceptionHandler(o, retries++, ex2))
	  return 0;
	else
	  throw ex2;
      }
      else {
	if (!_omni_callSystemExceptionHandler(o, retries++, ex))
	  throw;
      }
    }
    catch(const CORBA::SystemException& ex){
      if( !_omni_callSystemExceptionHandler(o, retries++, ex) )
	throw;
    }

#ifndef EGCS_WORKAROUND
    goto _again;
#else
  }
#endif
}


CORBA::Boolean
OmniLCProxyCallWrapper::one_way(omniObject* o,
				OmniOWProxyCallDescWithContext& call_desc,
				omniLC::_wrap_proxy *wp)
{
  CORBA::ULong retries = 0;

#ifndef EGCS_WORKAROUND
_again:
#else
  while(1) {
#endif
    if (omniORB::verifyObjectExistsAndType)
      o->assertObjectExistent();
    omniRopeAndKey ropeAndKey;
    o->getRopeAndKey(ropeAndKey);
    CORBA::Boolean reuse = 0;

    try{
      // Get a GIOP driven strand
      GIOP_C giop_client(ropeAndKey.rope());
      reuse = giop_client.isReUsingExistingConnection();

      // Calculate the size of the message.
      CORBA::ULong message_size =
	GIOP_C::RequestHeaderSize(ropeAndKey.keysize(),
				  call_desc.operation_len());

      message_size = call_desc.alignedSize(message_size);
      if( call_desc.contexts_expected() )
	message_size = CORBA::Context::NP_alignedSize(call_desc.context(),
				      call_desc.contexts_expected(),
				      call_desc.num_contexts_expected(),
				      message_size);

      giop_client.InitialiseRequest(ropeAndKey.key(), ropeAndKey.keysize(),
				    call_desc.operation(),
				    call_desc.operation_len(),
				    message_size, 1);

      // Marshal the arguments to the operation.
      call_desc.marshalArguments(giop_client);
      if( call_desc.contexts_expected() )
	CORBA::Context::marshalContext(call_desc.context(),
				       call_desc.contexts_expected(),
				       call_desc.num_contexts_expected(),
				       giop_client);

      // Wait for the reply.
      switch(giop_client.ReceiveReply()){
      case GIOP::NO_EXCEPTION:
	giop_client.RequestCompleted();
	return 1;

      case GIOP::USER_EXCEPTION:
      case GIOP::SYSTEM_EXCEPTION:
      case GIOP::LOCATION_FORWARD:
	giop_client.RequestCompleted(1);
	throw omniORB::fatalException(__FILE__,__LINE__,
				      "GIOP_C::ReceiveReply() returned"
				      " unexpected code on oneway");

      default:
	throw omniORB::fatalException(__FILE__,__LINE__,
				      "GIOP_C::ReceiveReply"
				      " returned an invalid code");
      }
    }
    catch(const CORBA::COMM_FAILURE& ex){
      if (reuse) {
	CORBA::TRANSIENT ex2(ex.minor(), ex.completed());
	if( !_omni_callTransientExceptionHandler(o, retries++, ex2) )
	  throw ex2;
      }
      else if (wp->_forwarded()) {
	wp->_reset_proxy();
	CORBA::TRANSIENT ex2(ex.minor(), ex.completed());
	if (_omni_callTransientExceptionHandler(o, retries++, ex2))
	  return 0;
	else
	  throw ex2;
      }
      else {
	if (!_omni_callCommFailureExceptionHandler(o, retries++, ex))
	  throw;
      }
    }
    catch(const CORBA::TRANSIENT& ex){
      if( !_omni_callTransientExceptionHandler(o, retries++, ex) )
	throw;
    }
    catch(const CORBA::SystemException& ex){
      if( !_omni_callSystemExceptionHandler(o, retries++, ex) )
	throw;
    }

#ifndef EGCS_WORKAROUND
    goto _again;
#else
  }
#endif
}
