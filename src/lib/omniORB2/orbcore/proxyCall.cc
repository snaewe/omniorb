// -*- Mode: C++; -*-
//                            Package   : omniORB2
// proxyCall.cc               Created on: 12/1998
//                            Author    : David Riddoch (djr)
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
//  The implementation of OmniProxyCallWrapper, and default implementations
//  for OmniProxyCallDesc and OmniOWProxyCallDesc.
//
//   OmniProxyCallWrapper provides the implementation of client proxy
//  calls. The details specific to an individual call are provided by
//  an instance of a class derived from OmniProxyCallDesc (defined in
//  the stubs).
//

#include <omniORB2/CORBA.h>
#include <omniORB2/proxyCall.h>


//////////////////////////////////////////////////////////////////////
//////////////////////// OmniProxyCallWrapper ////////////////////////
//////////////////////////////////////////////////////////////////////

void
OmniProxyCallWrapper::invoke(omniObject* o, OmniProxyCallDesc& call_desc)
{
  CORBA::ULong retries = 0;

#ifndef EGCS_WORKAROUND
_again:
#else
  while(1) {
#endif
    o->assertObjectExistent();
    omniRopeAndKey ropeAndKey;
    CORBA::Boolean fwd = o->getRopeAndKey(ropeAndKey);
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
	return;

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
	  omniRopeAndKey _r;
	  obj->PR_getobj()->getRopeAndKey(_r);
	  o->setRopeAndKey(_r);
	}
      if( omniORB::traceLevel > 10 ){
	omniORB::log << "GIOP::LOCATION_FORWARD: retry request.\n";
	omniORB::log.flush();
      }
      break;

      default:
	throw omniORB::fatalException(__FILE__,__LINE__,
				      "GIOP_C::ReceiveReply"
				      " returned an invalid code");
      }
    }
    catch(const CORBA::COMM_FAILURE& ex){
      if( reuse || fwd ){
	if( fwd )  o->resetRopeAndKey();
	CORBA::TRANSIENT ex2(ex.minor(), ex.completed());
	if( !_omni_callTransientExceptionHandler(o, retries++, ex2) )
	  throw ex2;
      }else{
	if( !_omni_callCommFailureExceptionHandler(o, retries++, ex) )
	  throw;
      }
    }
    catch(const CORBA::TRANSIENT& ex){
      if( !_omni_callTransientExceptionHandler(o, retries++, ex) )
	throw;
    }
    catch(const CORBA::OBJECT_NOT_EXIST& ex){
      if( fwd ){
	o->resetRopeAndKey();
	CORBA::TRANSIENT ex2(ex.minor(), ex.completed());
	if( !_omni_callTransientExceptionHandler(o, retries++, ex2) )
	  throw ex2;
      }else{
	if( !_omni_callSystemExceptionHandler(o, retries++, ex) )
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


void
OmniProxyCallWrapper::one_way(omniObject* o, OmniOWProxyCallDesc& call_desc)
{
  CORBA::ULong retries = 0;

#ifndef EGCS_WORKAROUND
_again:
#else
  while(1) {
#endif
    o->assertObjectExistent();
    omniRopeAndKey ropeAndKey;
    CORBA::Boolean fwd = o->getRopeAndKey(ropeAndKey);
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
	return;

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
      if( reuse || fwd ){
	if( fwd )  o->resetRopeAndKey();
	CORBA::TRANSIENT ex2(ex.minor(), ex.completed());
	if( !_omni_callTransientExceptionHandler(o, retries++, ex2) )
	  throw ex2;
      }else{
	if( !_omni_callCommFailureExceptionHandler(o, retries++, ex) )
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

//////////////////////////////////////////////////////////////////////
////////////////////////// OmniProxyCallDesc /////////////////////////
//////////////////////////////////////////////////////////////////////

CORBA::ULong
OmniProxyCallDesc::alignedSize(CORBA::ULong size_in)
{
  return size_in;
}


void
OmniProxyCallDesc::marshalArguments(GIOP_C&)
{
  // no-op
}


void
OmniProxyCallDesc::unmarshalReturnedValues(GIOP_C&)
{
  // no-op
}


void
OmniProxyCallDesc::userException(GIOP_C&, const char* repoId)
{
  // Shouldn't really ever be called - but it doesn't matter
  // if it is.
  throw CORBA::MARSHAL(0, CORBA::COMPLETED_MAYBE);
}

//////////////////////////////////////////////////////////////////////
///////////////////////// OmniOWProxyCallDesc ////////////////////////
//////////////////////////////////////////////////////////////////////

CORBA::ULong
OmniOWProxyCallDesc::alignedSize(CORBA::ULong size_in)
{
  return size_in;
}


void
OmniOWProxyCallDesc::marshalArguments(GIOP_C&)
{
  // no-op
}
