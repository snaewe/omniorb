// -*- Mode: C++; -*-
//                            Package   : omniORB
// giopServer.cc              Created on: 26/3/96
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
//      Server side GIOP
//      
 
/*
  $Log$
  Revision 1.22.2.4  2000/11/03 19:12:05  sll
  Use new marshalling functions for byte, octet and char. Use get_octet_array
  instead of get_char_array and put_octet_array instead of put_char_array.

  Revision 1.22.2.3  2000/10/03 17:38:50  sll
  Fixed typo that cause user exception to be marshalled as system exception.

  Revision 1.22.2.2  2000/09/27 18:21:53  sll
  Use the new GIOP engine to handle a remote call.

  Revision 1.22.2.1  2000/07/17 10:35:53  sll
  Merged from omni3_develop the diff between omni3_0_0_pre3 and omni3_0_0.

  Revision 1.23  2000/07/13 15:25:57  dpg1
  Merge from omni3_develop for 3.0 release.

  Revision 1.21.6.11  2000/06/22 10:37:50  dpg1
  Transport code now throws omniConnectionBroken exception rather than
  CORBA::COMM_FAILURE when things go wrong. This allows the invocation
  code to distinguish between transport problems and COMM_FAILURES
  propagated from the server side.

  exception.h renamed to exceptiondefs.h to avoid name clash on some
  platforms.

  Revision 1.21.6.10  2000/05/05 16:59:44  dpg1
  Bug in HandleLocateRequest() when catching a LOCATION_FORWARD

  Revision 1.21.6.9  2000/04/27 10:48:00  dpg1
  Interoperable Naming Service

  Include initRefs.h instead of bootreap_i.h.

  Revision 1.21.6.8  2000/03/07 18:07:34  djr
  Fixed user-exceptions when can't catch by base class.

  Revision 1.21.6.7  1999/10/27 17:32:11  djr
  omni::internalLock and objref_rc_lock are now pointers.

  Revision 1.21.6.6  1999/10/18 11:27:39  djr
  Centralised list of system exceptions.

  Revision 1.21.6.5  1999/10/14 16:22:09  djr
  Implemented logging when system exceptions are thrown.

  Revision 1.21.6.4  1999/09/30 11:49:28  djr
  Implemented catching user-exceptions in GIOP_S for all compilers.

  Revision 1.21.6.3  1999/09/28 09:48:31  djr
  Check for zero length 'operation' string when unmarshalling GIOP
  Request header.  Fixed bug -- 'principal' field need not be zero
  terminated.

  Revision 1.21.6.2  1999/09/24 17:11:12  djr
  New option -ORBtraceInvocations and omniORB::traceInvocations.

  Revision 1.21.6.1  1999/09/22 14:26:49  djr
  Major rewrite of orbcore to support POA.

  Revision 1.21  1999/08/30 16:50:56  sll
  Added calls to Strand::Sync::clicksSet to control how long a call is
  allowed to progress or how long an idle connection is to stay open.

  Revision 1.20  1999/08/18 15:10:40  dpg1
  Now _really_ works correctly if the application defined loader returns
  a nil object reference.

  Revision 1.19  1999/08/17 14:49:20  sll
  Fixed bug introduced by the previous change. Now works correctly if the
  application defined loader returns a nil object reference.

  Revision 1.18  1999/08/14 16:38:17  sll
  Added support for python binding.
  Changed locate object code as locateObject no longer throws an exception.

  Revision 1.17  1999/06/26 18:06:16  sll
  In InitialiseReply, if the GIOP request has response expected set to false,
  raise terminateProcessing exception. Previous a fatalException is raised.
  In HandleRequest, catch terminateProcessing exception and orderly complete
  the current invocation.

  Revision 1.16  1999/03/11 16:25:53  djr
  Updated copyright notice

  Revision 1.15  1999/01/07 15:52:41  djr
  Changes needed due to changes to interface of Net/MemBufferedStream.

  Revision 1.14  1998/08/21 19:08:50  sll
  Added hook to recognise the key 'INIT' of the special bootstrapping
  agent. Dispatch the invocation to the agent if it has been initialised.

  Revision 1.13  1998/08/14 13:47:08  sll
  Added pragma hdrstop to control pre-compile header if the compiler feature
  is available.

  Revision 1.12  1998/04/07 19:34:26  sll
  Replace cerr with omniORB::log.

  Revision 1.11  1998/02/25 20:37:13  sll
  Added hooks for dynamic object loader.

  Revision 1.10  1997/12/23 19:24:27  sll
  Removed unnecessary token concatination.

  Revision 1.9  1997/12/09 17:48:37  sll
  Updated to use the new rope and strand interface.
  Adapted to coordinate with the incoming rope scavenger.
  New function omniORB::MaxMessageSize(size_t).
  Now unmarshal context properly.

  Revision 1.8  1997/08/21 21:56:00  sll
  Added system exception TRANSACTION_REQUIRED, TRANSACTION_ROLLEDBACK,
  INVALID_TRANSACTION, WRONG_TRANSACTION.

// Revision 1.7  1997/05/06  15:20:48  sll
// Public release.
//
  */

#include <omniORB4/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <ropeFactory.h>
#include <objectAdapter.h>
#include <omniORB4/omniServant.h>
#include <localIdentity.h>
#include <initRefs.h>
#include <exceptiondefs.h>


size_t  GIOP_Basetypes::max_giop_message_size = 2048 * 1024;


static void 
MarshallSystemException(GIOP_S *s,
			const CORBA::SystemException &ex);


GIOP_S::GIOP_S(Strand *s)
{
  pd_state = GIOP_S::Idle;
  pd_cdrStream = giopStream::acquireServer(s);
}


GIOP_S::~GIOP_S()
{
  if (pd_state == GIOP_S::Zombie)
    return;

  if (pd_state != GIOP_S::Idle) {
    pd_cdrStream->setTerminalError();
  }
  pd_state = GIOP_S::Zombie;
  pd_cdrStream->release();
}

void
GIOP_S::RequestReceived(CORBA::Boolean skip_msg)
{
  if (pd_state != GIOP_S::RequestIsBeingProcessed)
    throw omniORB::fatalException(__FILE__,__LINE__,
      "GIOP_S::RequestReceived() entered with the wrong state.");

  pd_cdrStream->inputMessageEnd(skip_msg);

  if (pd_cdrStream->terminalError()) {
    throw CORBA::COMM_FAILURE(0,CORBA::COMPLETED_NO);
  }
  pd_state = GIOP_S::WaitingForReply;
}

void
GIOP_S::InitialiseReply(const GIOP::ReplyStatusType status, giopMarshaller& m)
{
  if (!pd_invokeInfo.response_expected())
    throw terminateProcessing();

  if (pd_state != GIOP_S::WaitingForReply)
    throw omniORB::fatalException(__FILE__,__LINE__,
	  "GIOP_S::InitialiseReply() entered with the wrong state.");

  pd_state = GIOP_S::ReplyIsBeingComposed;

  // Setup callback in case the giopStream have to calculate the
  // message body size.
  pd_cdrStream->outputMessageBodyMarshaller(m);

  // Marshal reply header
  pd_cdrStream->outputReplyMessageBegin(pd_invokeInfo,status);

  // Marshal reply body 
  m.marshal(*pd_cdrStream);
}

class GIOP_S_null_marshaller : public giopMarshaller {
public:
  void marshal(cdrStream&) {}
};

void
GIOP_S::ReplyCompleted()
{
  if (!pd_invokeInfo.response_expected())
    {
      if (pd_state != GIOP_S::WaitingForReply)
	throw omniORB::fatalException(__FILE__,__LINE__,
	  "GIOP_S::ReplyCompleted() entered with the wrong state.");
      pd_state = GIOP_S::Idle;
      return;
    }
  else if (pd_state == GIOP_S::WaitingForReply) {
    // The call is a oneway but the remote end expect a response.
    // The stub is not supplying one. We make one here.
    GIOP_S_null_marshaller m;
    InitialiseReply(GIOP::NO_EXCEPTION,m);
  }

  if (pd_state != GIOP_S::ReplyIsBeingComposed)
    throw omniORB::fatalException(__FILE__,__LINE__,
	  "GIOP_S::ReplyCompleted() entered with the wrong state.");

  pd_cdrStream->outputMessageEnd();

  pd_state = GIOP_S::Idle;
}

void
GIOP_S::dispatcher(Strand *s)
{
  GIOP_S gs(s);

  gs.pd_state = GIOP_S::RequestIsBeingProcessed;

  switch (gs.pd_cdrStream->inputRequestMessageBegin(gs.pd_invokeInfo)) {
  case GIOP::Request:
    {
      gs.HandleRequest();
      break;
    }
  case GIOP::LocateRequest:
    {
      gs.HandleLocateRequest();
      break;
    }
  case GIOP::CancelRequest:
    {
      gs.HandleCancelRequest();
      break;
    }
  default:
    {
      gs.pd_cdrStream->inputMessageEnd(0,1);
    }
  }
}

class GIOP_S_LocationForward_Marshaller : public giopMarshaller {
public:
  GIOP_S_LocationForward_Marshaller(CORBA::Object_var& d)
    : pd_d(d) {}

  void marshal(cdrStream& s) {
    CORBA::Object::_marshalObjRef(pd_d,s);
  }

private:
  CORBA::Object_var& pd_d;
};


#define CHECK_AND_MAYBE_MARSHALL_SYSTEM_EXCEPTION(exrepoid,exvar) do {\
    if (!pd_cdrStream->terminalError() && \
        (pd_state == RequestIsBeingProcessed || \
	 pd_state == WaitingForReply)) \
      { \
	if (pd_state == RequestIsBeingProcessed) { \
           RequestReceived(1); \
	} \
	if (!pd_invokeInfo.response_expected()) \
	{ \
	  pd_cdrStream->SendMsgErrorMessage(); \
          pd_state = GIOP_S::Idle; \
	} \
        else \
        { \
	  MarshallSystemException(this,exvar); \
	}  \
      } \
    else { \
	throw; \
    } \
  } while (0)


void
GIOP_S::HandleRequest()
{
  try {

    // Can we find the object in the local object table?

    if (pd_invokeInfo.keysize() < 0) {
      // possible with GIOP version 1.2 or above. The target object's 
      // identity is encoded in a GIOP::IORAddressingInfo struct. 
      // inputRequestHeader() has looked into it and discovered that it is 
      // not destined for an object in this address space.
      OMNIORB_THROW(OBJECT_NOT_EXIST,0, CORBA::COMPLETED_NO);
    }

    CORBA::ULong hash = omni::hash(pd_invokeInfo.key(), 
				   pd_invokeInfo.keysize());
    omni::internalLock->lock();
    omniLocalIdentity* id;
    id = omni::locateIdentity(pd_invokeInfo.key(),
			      pd_invokeInfo.keysize(), hash);

    if( id && id->servant() ) {
      id->dispatch(*this);
      return;
    }

    omni::internalLock->unlock();

    // Can we create a suitable object on demand?

    omniObjAdapter_var adapter(
	  omniObjAdapter::getAdapter(pd_invokeInfo.key(),
				     pd_invokeInfo.keysize()));

    if( adapter ) {
      adapter->dispatch(*this, pd_invokeInfo.key(), pd_invokeInfo.keysize());
      return;
    }

    // Or is it the bootstrap agent?

    if( pd_invokeInfo.keysize() == 4 && 
	!memcmp(pd_invokeInfo.key(), "INIT", 4) &&
	omniInitialReferences::invoke_bootstrap_agentImpl(*this) )
      return;

    // Oh dear.

    OMNIORB_THROW(OBJECT_NOT_EXIST,0, CORBA::COMPLETED_NO);

  }

#ifndef HAS_Cplusplus_catch_exception_by_base

  // We have to catch each type of system exception separately
  // here to support compilers which cannot catch more derived
  // types.

#define CATCH_AND_MAYBE_MARSHAL(name)  \
  catch (CORBA::name& ex) {  \
    CHECK_AND_MAYBE_MARSHALL_SYSTEM_EXCEPTION (name, ex);  \
  }

  OMNIORB_FOR_EACH_SYS_EXCEPTION(CATCH_AND_MAYBE_MARSHAL)

#undef CATCH_AND_MAYBE_MARSHAL

  catch(omniORB::StubUserException& ex) {
    MaybeMarshalUserException(ex.ex());
    delete ex.ex();  // ?? Possible memory leak?
  }

#else

  catch(CORBA::SystemException& ex) {
    if (!pd_cdrStream->terminalError() &&
        (pd_state == RequestIsBeingProcessed ||
	 pd_state == WaitingForReply))
      {
	if (pd_state == RequestIsBeingProcessed) {
           RequestReceived(1);
	}
	if (!pd_invokeInfo.response_expected())
	{
	  pd_cdrStream->SendMsgErrorMessage();
          pd_state = GIOP_S::Idle;
	}
	else {
	  MarshallSystemException(this,ex);
	}
      }
    else
      throw;
  }

  catch(CORBA::UserException& ex) {
    MaybeMarshalUserException(&ex);
  }

#endif

  catch(omniORB::LOCATION_FORWARD& ex) {
    // This is here to provide a convenient way to implement
    // location forwarding. The object implementation can throw
    // a location forward exception to re-direct the request
    // to another location.

    if( omniORB::traceInvocations )
      omniORB::logf("Implementation of \'%s\' generated LOCATION_FORWARD.",
		    pd_invokeInfo.operation());

    CORBA::Object_var release_it(ex.get_obj());

    if (!pd_cdrStream->terminalError() &&
        (pd_state == RequestIsBeingProcessed ||
	 pd_state == WaitingForReply)) {

      if (pd_state == RequestIsBeingProcessed) {
	RequestReceived(1);
      }

      if (!pd_invokeInfo.response_expected()) {
	pd_cdrStream->SendMsgErrorMessage();
	pd_state = GIOP_S::Idle;
      }
      else {

	if( CORBA::is_nil(ex.get_obj()) ) {
	  CORBA::UNKNOWN ex(0, CORBA::COMPLETED_MAYBE);
	  CHECK_AND_MAYBE_MARSHALL_SYSTEM_EXCEPTION (UNKNOWN,ex);
	}
	else {
	  // Build and send the location forward message...
	  GIOP_S_LocationForward_Marshaller m(release_it);
	  InitialiseReply(GIOP::LOCATION_FORWARD,m);
	
	  // All done...
	  ReplyCompleted();
	}
      }
    }
    else {
      CORBA::UNKNOWN ex(0, CORBA::COMPLETED_MAYBE);
      CHECK_AND_MAYBE_MARSHALL_SYSTEM_EXCEPTION (UNKNOWN,ex);
    }
  }
  catch (const terminateProcessing &) {
    if (pd_state == GIOP_S::WaitingForReply ||
	pd_state == GIOP_S::RequestIsBeingProcessed) {
      // XXXX Check if requestisbeingprocessed, should skip the rest of the
      // message.
      pd_state = GIOP_S::Idle;
    }
  }
  catch(...) {
    if( omniORB::traceLevel > 1 ) {
      omniORB::logger l;
      l << "WARNING -- method \'" << pd_invokeInfo.operation() << "\' raised an unexpected\n"
	" exception (not a CORBA exception).\n";
    }
    CORBA::UNKNOWN ex(0, CORBA::COMPLETED_MAYBE);
    CHECK_AND_MAYBE_MARSHALL_SYSTEM_EXCEPTION (UNKNOWN,ex);
  }
}


void
GIOP_S::HandleLocateRequest()
{
  RequestReceived();

  omniORB::logs(10, "Handling a GIOP LOCATE_REQUEST.");

  GIOP::LocateStatusType status = GIOP::UNKNOWN_OBJECT;

  CORBA::ULong hash = omni::hash(pd_invokeInfo.key(), pd_invokeInfo.keysize());
  omni::internalLock->lock();
  omniLocalIdentity* id;
  id = omni::locateIdentity(pd_invokeInfo.key(), 
			    pd_invokeInfo.keysize(), hash);
  if( id && id->servant() )  status = GIOP::OBJECT_HERE;
  omni::internalLock->unlock();

  if( status == GIOP::UNKNOWN_OBJECT ) {

    // We attempt to find the object adapter (activate it if necassary)
    // and ask it if the object exists, or if it has the *capability*
    // to activate such an object.  ie. is it able to do object loading
    // on demand?

    omniObjAdapter_var adapter(
	  omniObjAdapter::getAdapter(pd_invokeInfo.key(), 
				     pd_invokeInfo.keysize()));

    if( adapter ) {
      try {
	if( adapter->objectExists(pd_invokeInfo.key(),
				  pd_invokeInfo.keysize()) )
	  status = GIOP::OBJECT_HERE;
      }
      catch(omniORB::LOCATION_FORWARD& lf) {
	status = GIOP::OBJECT_FORWARD;

	pd_state = ReplyIsBeingComposed;

	CORBA::Object_var release_it(lf.get_obj());

	// XXX Should be changed to use giopMarshaller
	pd_cdrStream->outputLocateReplyMessageBegin(pd_invokeInfo,status);
	CORBA::Object::_marshalObjRef(release_it,*pd_cdrStream);
	pd_cdrStream->outputMessageEnd();
	
	pd_state = GIOP_S::Idle;
      }
      catch(...) {
	// status == GIOP::UNKNOWN_OBJECT
      }
    }
    else if( pd_invokeInfo.keysize() == 4 && 
	     !memcmp(pd_invokeInfo.key(), "INIT", 4) &&
	     omniInitialReferences::is_bootstrap_agentImpl_initialised() )
      status = GIOP::OBJECT_HERE;
  }

  if( status != GIOP::OBJECT_FORWARD ) {

    pd_state = GIOP_S::ReplyIsBeingComposed;

    // XXX Should be changed to use giopMarshaller
    pd_cdrStream->outputLocateReplyMessageBegin(pd_invokeInfo,status);
    pd_cdrStream->outputMessageEnd();

    pd_state = GIOP_S::Idle;
  }
}


void
GIOP_S::HandleCancelRequest()
{
  // XXX Not supported yet!!!
  // For the moment, silently ignore this message

  RequestReceived(1);
  pd_state = GIOP_S::Idle;
}

class GIOP_S_UserException_Marshaller : public giopMarshaller {
public:
  GIOP_S_UserException_Marshaller(const CORBA::UserException &ex)
    : pd_ex(ex) {}

  void marshal(cdrStream& s) {
    int repoid_size;
    const char* repoid = pd_ex._NP_repoId(&repoid_size);

    CORBA::ULong(repoid_size) >>= s;
    s.put_octet_array((const CORBA::Octet*) repoid, repoid_size);
    pd_ex._NP_marshal(s);
  }
  
private:
  const CORBA::UserException& pd_ex;
};

void
GIOP_S::MaybeMarshalUserException(void* pex)
{
  CORBA::UserException& ex = * (CORBA::UserException*) pex;

  int i, repoid_size;
  const char* repoid = ex._NP_repoId(&repoid_size);

  const char*const* user_exns = pd_invokeInfo.user_exceptions();
  int n_user_exns = pd_invokeInfo.n_user_exceptions();

  // Could turn this into a binary search (list is sorted).
  // Usually a short list though -- probably not worth it.
  for( i = 0; i < n_user_exns; i++ )
    if( !strcmp(user_exns[i], repoid) ) {

      GIOP_S_UserException_Marshaller m(ex);
      InitialiseReply(GIOP::USER_EXCEPTION,m);
      ReplyCompleted();
      break;
    }

  if( i == n_user_exns ) {
    if( omniORB::trace(1) ) {
      omniORB::logger l;
      l << "WARNING -- method \'" << pd_invokeInfo.operation() 
	<< "\' on: " << pd_invokeInfo.key()
	<< "\n raised the exception: " << repoid << '\n';
    }
    CORBA::UNKNOWN ex(0, CORBA::COMPLETED_MAYBE);
    CHECK_AND_MAYBE_MARSHALL_SYSTEM_EXCEPTION (UNKNOWN, ex);
  }
}

class GIOP_S_SystemException_Marshaller : public giopMarshaller {
public:
  GIOP_S_SystemException_Marshaller(const CORBA::SystemException &ex)
    : pd_ex(ex) {}

  void marshal(cdrStream& s) {
    int repoid_size;
    const char* repoid = pd_ex._NP_repoId(&repoid_size);

    CORBA::ULong(repoid_size) >>= s;
    s.put_octet_array((const CORBA::Octet*) repoid, repoid_size);
    pd_ex.minor() >>= s;
    CORBA::ULong(pd_ex.completed()) >>= s;
  }
  
private:
  const CORBA::SystemException& pd_ex;
};

static
void 
MarshallSystemException(GIOP_S *s,
			const CORBA::SystemException &ex)
{
  GIOP_S_SystemException_Marshaller m(ex);
  s->InitialiseReply(GIOP::SYSTEM_EXCEPTION,m);
  s->ReplyCompleted();
}

size_t
omniORB::MaxMessageSize()
{
  return GIOP_Basetypes::max_giop_message_size;
}

void
omniORB::MaxMessageSize(size_t newvalue)
{
  GIOP_Basetypes::max_giop_message_size = newvalue;
}
