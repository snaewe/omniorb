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

/*
 $Log$
 Revision 1.7.4.2  1999/10/02 18:24:31  sll
 Reformatted trace messages.

 Revision 1.7.4.1  1999/09/15 20:18:23  sll
 Updated to use the new cdrStream abstraction.
 Marshalling operators for NetBufferedStream and MemBufferedStream are now
 replaced with just one version for cdrStream.
 Derived class giopStream implements the cdrStream abstraction over a
 network connection whereas the cdrMemoryStream implements the abstraction
 with in memory buffer.

 Revision 1.7  1999/06/28 10:49:16  sll
 Added missing check for verifyObjectExistsAndType in oneway invoke.

 Revision 1.6  1999/06/26 18:03:51  sll
 Added check for verifyObjectExistsAndType setting before doing
 assertObjectExistent().

 Revision 1.5  1999/05/20 18:37:34  sll
 These proxyCall functions support context. This is separated from
 the equivalent version in the orbcore library. The orbcore version
 only support calls without context.

 Revision 1.4  1999/04/21 13:11:18  djr
 Added support for contexts.

*/

#include <omniORB2/CORBA.h>
#include <omniORB2/proxyCall.h>

#define LOGMESSAGE(level,prefix,message) do {\
   if (omniORB::trace(level)) {\
     omniORB::logger log("omniORB: ");\
	log << prefix ## ": " ## message ## "\n";\
   }\
} while (0)


//////////////////////////////////////////////////////////////////////
//////////////////////// OmniProxyCallWrapper ////////////////////////
//////////////////////////////////////////////////////////////////////

void
OmniProxyCallWrapper::invoke(omniObject* o, 
			     OmniProxyCallDescWithContext& call_desc)
{
  CORBA::ULong retries = 0;

  while(1) {

    if (call_desc.doAssertObjectExistence()) o->assertObjectExistent();

    CORBA::Boolean fwd;
    GIOPObjectInfo* invokeInfo = o->getInvokeInfo(fwd);

    try{
      // Get a GIOP driven strand
      GIOP_C giop_client(invokeInfo);

      call_desc.initialise((cdrStream&)giop_client);

      giop_client.InitialiseRequest(call_desc.operation(),
				    call_desc.operation_len(),0,1);

      // Marshal the arguments to the operation.
      call_desc.marshalArguments((cdrStream&)giop_client);
      if( call_desc.contexts_expected() )
	CORBA::Context::marshalContext(call_desc.context(),
				       call_desc.contexts_expected(),
				       call_desc.num_contexts_expected(),
				       giop_client);

      // Wait for the reply.
      GIOP::ReplyStatusType rc;
      switch((rc = giop_client.ReceiveReply())){
      case GIOP::NO_EXCEPTION:
	// Unmarshal the result and out/inout arguments.
	call_desc.unmarshalReturnedValues((cdrStream&)giop_client);
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
	  cdrStream& s = giop_client;
	  repoIdLen <<= s;
	  CORBA::String_var repoId = CORBA::string_alloc(repoIdLen - 1);
	  s.get_char_array((CORBA::Char*)(const char*)repoId,repoIdLen);

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
      case GIOP::LOCATION_FORWARD_PERM:
	{
	  CORBA::Object_var obj =
	    CORBA::Object::unmarshalObjRef((cdrStream&)giop_client);
	  giop_client.RequestCompleted();
	  if( CORBA::is_nil(obj) ){
	    LOGMESSAGE(10,"OmniProxyCallWrapper","Received GIOP::LOCATION_FORWARD message that"
		       " contains a nil object reference.");
	    throw CORBA::COMM_FAILURE(0, CORBA::COMPLETED_NO);
	  }
	  GIOPObjectInfo* newinfo = obj->PR_getobj()->getInvokeInfo(fwd);
	  o->setInvokeInfo(newinfo,
			   (rc == GIOP::LOCATION_FORWARD_PERM) ? 0 : 1);
	  LOGMESSAGE(10,"OmniProxyCallWrapper","GIOP::LOCATION_FORWARD: retry request.");
	}
      break;

      case GIOP::NEEDS_ADDRESSING_MODE:
	giop_client.RequestCompleted();
	throw CORBA::TRANSIENT(0,CORBA::COMPLETED_NO);
	break; // redundent.

      default:
	throw omniORB::fatalException(__FILE__,__LINE__,
				      "GIOP_C::ReceiveReply"
				      " returned an invalid code");
      }
    }
    catch(const CORBA::COMM_FAILURE& ex){
      if( fwd ){
	o->resetInvokeInfo();
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
	o->resetInvokeInfo();
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
  }
}

void
OmniProxyCallWrapper::one_way(omniObject* o,
			      OmniOWProxyCallDescWithContext& call_desc)
{
  CORBA::ULong retries = 0;

  while (1) {

    if (call_desc.doAssertObjectExistence()) o->assertObjectExistent();

    CORBA::Boolean fwd; 
    GIOPObjectInfo* invokeInfo = o->getInvokeInfo(fwd);

    try{
      // Get a GIOP driven strand
      GIOP_C giop_client(invokeInfo);

      call_desc.initialise((cdrStream&)giop_client);

      giop_client.InitialiseRequest(call_desc.operation(),
				    call_desc.operation_len(),
				    1,0);

      // Marshal the arguments to the operation.
      call_desc.marshalArguments((cdrStream&)giop_client);
      if( call_desc.contexts_expected() )
	CORBA::Context::marshalContext(call_desc.context(),
				       call_desc.contexts_expected(),
				       call_desc.num_contexts_expected(),
				       giop_client);

      // Wait for the reply.
      switch(giop_client.ReceiveReply()){
      case GIOP::NO_EXCEPTION:
	giop_client.RequestCompleted();
	return;

      case GIOP::USER_EXCEPTION:
      case GIOP::SYSTEM_EXCEPTION:
      case GIOP::LOCATION_FORWARD:
      case GIOP::LOCATION_FORWARD_PERM:
      case GIOP::NEEDS_ADDRESSING_MODE:
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
      if( fwd ){
	o->resetInvokeInfo();
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

  }
}
