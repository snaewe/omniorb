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
  Revision 1.7.4.4  1999/11/04 20:20:22  sll
  GIOP engines can now do callback to the higher layer to calculate total
  message size if necessary.
  Where applicable, changed to use the new server side descriptor-based stub.

  Revision 1.7.4.3  1999/10/05 20:35:36  sll
  Added support to GIOP 1.2 to recognise all TargetAddress mode.
  Now handles NEEDS_ADDRESSING_MODE and LOC_NEEDS_ADDRESSING_MODE.

  Revision 1.7.4.2  1999/10/02 18:24:33  sll
  Reformatted trace messages.

  Revision 1.7.4.1  1999/09/15 20:18:29  sll
  Updated to use the new cdrStream abstraction.
  Marshalling operators for NetBufferedStream and MemBufferedStream are now
  replaced with just one version for cdrStream.
  Derived class giopStream implements the cdrStream abstraction over a
  network connection whereas the cdrMemoryStream implements the abstraction
  with in memory buffer.

*/

#include <omniORB2/CORBA.h>
#include <omniORB2/proxyCall.h>
#include <giopObjectInfo.h>

#define LOGMESSAGE(level,prefix,message) do {\
   if (omniORB::trace(level)) {\
     omniORB::logger log;\
	log << " " ## prefix ## ": " ## message ## "\n";\
   }\
} while (0)


//////////////////////////////////////////////////////////////////////
//////////////////////// OmniProxyCallWrapper ////////////////////////
//////////////////////////////////////////////////////////////////////

class OmniProxyCallArgumentsMarshaller : public giopMarshaller {
public:
  OmniProxyCallArgumentsMarshaller(giopStream& s,OmniProxyCallDesc& desc) : 
    pd_s(s), pd_desc(desc) {}

  void marshalData() {
    pd_desc.marshalArguments(pd_s);
  }

  size_t dataSize(size_t initialoffset) {
    cdrCountingStream s(initialoffset);
    pd_desc.marshalArguments(s);
    return s.total();
  }

private:
  giopStream&     pd_s;
  OmniProxyCallDesc& pd_desc;
};



void
OmniProxyCallWrapper::invoke(omniObject* o, OmniProxyCallDesc& call_desc)
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

      OmniProxyCallArgumentsMarshaller m((giopStream&)giop_client,call_desc);

      giop_client.InitialiseRequest(call_desc.operation(),
				    call_desc.operation_len(),0,1,m);

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
	  if( CORBA::is_nil(obj) ) {
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
	{
	  GIOP::AddressingDisposition v;
	  v <<= ((cdrStream&)giop_client);
	  invokeInfo->addrMode(v);
	  giop_client.RequestCompleted();
	  if (omniORB::trace(10)) {
	    omniORB::logger log("omniORB: ");
	    log << "OmniProxyCallWrapper: GIOP::NEEDS_ADDRESSING_MODE: "
		<< (int) v << " retry request.\n";
	  }
	}
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

class OmniOWProxyCallArgumentsMarshaller : public giopMarshaller {
public:
  OmniOWProxyCallArgumentsMarshaller(giopStream& s,OmniOWProxyCallDesc& desc) : 
    pd_s(s), pd_desc(desc) {}

  void marshalData() {
    pd_desc.marshalArguments(pd_s);
  }

  size_t dataSize(size_t initialoffset) {
    cdrCountingStream s(initialoffset);
    pd_desc.marshalArguments(s);
    return s.total();
  }

private:
  giopStream&     pd_s;
  OmniOWProxyCallDesc& pd_desc;
};

void
OmniProxyCallWrapper::one_way(omniObject* o, OmniOWProxyCallDesc& call_desc)
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

      OmniOWProxyCallArgumentsMarshaller m((giopStream&)giop_client,call_desc);

      giop_client.InitialiseRequest(call_desc.operation(),
				    call_desc.operation_len(),
				    1,0,m);

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

//////////////////////////////////////////////////////////////////////
////////////////////////// OmniProxyCallDesc /////////////////////////
//////////////////////////////////////////////////////////////////////

void
OmniProxyCallDesc::initialise(cdrStream&)
{
  // no-op
}

void
OmniProxyCallDesc::marshalArguments(cdrStream&)
{
  // no-op
}


void
OmniProxyCallDesc::unmarshalReturnedValues(cdrStream&)
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

void
OmniOWProxyCallDesc::initialise(cdrStream&)
{
  // no-op
}

void
OmniOWProxyCallDesc::marshalArguments(cdrStream&)
{
  // no-op
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void
OmniUpCallDesc::unmarshalArguments(cdrStream&) {
}

void
OmniUpCallDesc::marshalReturnedValues(cdrStream&) {
}

CORBA::Boolean
OmniUpCallDesc::marshalUserException(cdrStream& s) {
  return 0;
}

class OmniUpCallReturnedValuesMarshaller : public giopMarshaller {
public:
  OmniUpCallReturnedValuesMarshaller(giopStream& s,OmniUpCallDesc& desc) : 
    pd_s(s), pd_desc(desc) {}

  void marshalData() {
    pd_desc.marshalReturnedValues(pd_s);
  }

  size_t dataSize(size_t initialoffset) {
    cdrCountingStream s(initialoffset);
    pd_desc.marshalReturnedValues(s);
    return s.total();
  }

private:
  giopStream&     pd_s;
  OmniUpCallDesc& pd_desc;
};

class OmniUpCallUserExceptionMarshaller : public giopMarshaller {
public:
  OmniUpCallUserExceptionMarshaller(giopStream& s, OmniUpCallDesc& desc) : 
    pd_s(s), pd_desc(desc) {}

  void marshalData() {
    pd_desc.marshalUserException(pd_s);
  }

  size_t dataSize(size_t initialoffset) {
    cdrCountingStream s(initialoffset);
    (void) pd_desc.marshalUserException(s);
    return s.total();
  }

private:
  giopStream&     pd_s;
  OmniUpCallDesc& pd_desc;
};



static
void
doNoUserExceptionUpCall(GIOP_S& giop_s, OmniUpCallDesc& desc)
{
  cdrStream& s = (cdrStream&)giop_s;
  desc.unmarshalArguments(s);
  giop_s.RequestReceived();
  desc.doUpCall();
  if (!desc.is_oneway()) {
    OmniUpCallReturnedValuesMarshaller m((giopStream&)giop_s,desc);
    giop_s.InitialiseReply(GIOP::NO_EXCEPTION,m);
  }
  giop_s.ReplyCompleted();
}


void
OmniUpCallWrapper::upcall(GIOP_S& giop_s, OmniUpCallDesc& desc)
{
  if (desc.has_user_exceptions()) {
    try {
      doNoUserExceptionUpCall(giop_s,desc);
    }
    catch (CORBA::UserException& ex) {

      desc.setUserException(ex);

      cdrCountingStream c;
      if (desc.marshalUserException(c)) {
	OmniUpCallUserExceptionMarshaller m((giopStream&)giop_s,desc);
	giop_s.InitialiseReply(GIOP::USER_EXCEPTION,m);
	giop_s.ReplyCompleted();	
      }
      else {
	throw CORBA::UNKNOWN(0,CORBA::COMPLETED_MAYBE);
      }
    }
  }
  else {
    doNoUserExceptionUpCall(giop_s,desc);
  }
}
