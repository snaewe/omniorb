// -*- Mode: C++; -*-
//                            Package   : omniORB2
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
  Revision 1.21.4.1  1999/09/15 20:18:31  sll
  Updated to use the new cdrStream abstraction.
  Marshalling operators for NetBufferedStream and MemBufferedStream are now
  replaced with just one version for cdrStream.
  Derived class giopStream implements the cdrStream abstraction over a
  network connection whereas the cdrMemoryStream implements the abstraction
  with in memory buffer.

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

#include <omniORB2/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <ropeFactory.h>
#include <objectManager.h>
#include <bootstrap_i.h>

size_t  GIOP_Basetypes::max_giop_message_size = 2048 * 1024;

// Dynamic Object Loading
// 
static omniORB::loader::mapKeyToObject_t MapKeyToObjectFunction= 0;

void 
omniORB::loader::set(omniORB::loader::mapKeyToObject_t NewMapKeyToObject) 
{
  MapKeyToObjectFunction= NewMapKeyToObject;
}


// GIOP gateway
static omniORB::gateway::mapTargetAddressToObject_t MapTargetAddressToObjectFunction= 0;

void
omniORB::gateway::set(mapTargetAddressToObject_t f)
{
  MapTargetAddressToObjectFunction = f;
}

static void 
MarshallSystemException(GIOP_S *s,
			const GIOP_Basetypes::_SysExceptRepoID &id,
			const CORBA::SystemException &ex);


GIOP_S::GIOP_S(Strand *s)
{
  pd_state = GIOP_S::Idle;
  pd_cdrStream = giopStream::acquire(s);
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
GIOP_S::InitialiseReply(GIOP::ReplyStatusType status)
{
  if (!pd_invokeInfo.response_expected())
    throw terminateProcessing();

  if (pd_state != GIOP_S::WaitingForReply)
    throw omniORB::fatalException(__FILE__,__LINE__,
      "GIOP_S::InitialiseReply() entered with the wrong state.");

  pd_state = GIOP_S::ReplyIsBeingComposed;

  pd_cdrStream->outputReplyMessageBegin(pd_invokeInfo,status);
}

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
    InitialiseReply(GIOP::NO_EXCEPTION);
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
	  MarshallSystemException(this,GIOP_Basetypes::SysExceptRepoID:: exrepoid ,exvar); \
	}  \
      } \
    else { \
	throw; \
    } \
  } while (0)

void
GIOP_S::HandleRequest()
{
  omniObject_var obj;    

  // Note: If this is a one way invocation, we choose to return a 
  // MessageError Message instead of returning a Reply with System Exception
  // message because the other-end says do not send me a reply!

  try {

    if (pd_invokeInfo.keysize() == 0) {
      // This case is only possible with GIOP version 1.2 or
      // above. The target object's identity is encoded in a
      // GIOP::IORAddressingInfo struct. inputRequestHeader() has looked
      // into it and discovered that it is not destined for an object in
      // this address space.
      // Normally, we can just say object not exist here.
      // But our application may be a GIOP proxy which will relay the
      // request to the real target. We give such a proxy a chance to
      // to look at this request header and if it can return an omniObject
      // we'll use it to dispatch the call.
      if (MapTargetAddressToObjectFunction) {
	obj = MapTargetAddressToObjectFunction(pd_invokeInfo);
      }
    }
    else if (pd_invokeInfo.keysize() != sizeof(omniObjectKey)) {

      // This key did not come from this orb.

      // We make a special case for the bootstrapping agent.
      // The object key is "INIT". If the key match and we do have
      // the bootstrapping agent running, initialise obj to point to it. 
      if (pd_invokeInfo.keysize() == 4) {
	const char* k = (const char*) pd_invokeInfo.key();
	if (k[0] == 'I' && k[1] == 'N' && k[2] == 'I' && k[3] == 'T') {
	  obj = omniInitialReferences::singleton()->has_bootstrap_agentImpl();
	  if ((omniObject*)obj) omni::objectDuplicate(obj);
	}
      }
    }

    if (pd_invokeInfo.keysize() != sizeof(omniObjectKey) && !(omniObject*)obj){
      // We say we do not know this object.
      CORBA::OBJECT_NOT_EXIST ex(0,CORBA::COMPLETED_NO);
      CHECK_AND_MAYBE_MARSHALL_SYSTEM_EXCEPTION (OBJECT_NOT_EXIST,ex);
      return;
    }

    // In future, we have to partially decode the object key to
    // determine which object manager it belongs to.
    // For the moment, there is only one object manager- the rootObjectManager.

    if (! (omniObject*)obj) {
      obj = omni::locateObject(omniObjectManager::root(),
			       *(omniObjectKey*)pd_invokeInfo.key());
    }

    if (!obj)
      obj = omni::locatePyObject(omniObjectManager::root(),
				 *(omniObjectKey*)pd_invokeInfo.key());
 
    if (obj) {
      if (!obj->dispatch(*this,pd_invokeInfo.operation(),
			 pd_invokeInfo.response_expected())) {
	if (!obj->omniObject::dispatch(*this,pd_invokeInfo.operation(),
				       pd_invokeInfo.response_expected())) {
	  RequestReceived(1);
	  throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
	}
      }
    }
    else { // !obj
      if (pd_invokeInfo.response_expected() && MapKeyToObjectFunction) {
	// Cannot find the object in the object table. If the application
	// has registered a loader, do an upcall to locate the object.
	// If the returned value is not a nil object reference, send a
	// LOCATION_FORWARD message to the client to instruct it to retry
	// with the new object reference.
	//
	// Limitation: if this invocation is oneway, one cannot reply with
	//             a LOCATION_FORWARD message, in that case, just
	//             treat this as a normal OBJECT_NOT_EXIST and let
	//             the code below to deal with it.
	//
	CORBA::Object_var newDestination= MapKeyToObjectFunction(*(omniObjectKey*)pd_invokeInfo.key());
	if (!CORBA::is_nil(newDestination)) {
	  // Note that we have completed the object request
	  RequestReceived(1); 
	
	  // Build and send the location forward message...
	  InitialiseReply(GIOP::LOCATION_FORWARD);
	  CORBA::Object::marshalObjRef(newDestination, (cdrStream&)*this);
	
	  // All done...
	  ReplyCompleted(); return;
	}
      }
      
      // No application-defined loader, or the loader returned nil
      RequestReceived(1);
      if (!pd_invokeInfo.response_expected()) {
	// This is a one way invocation, we choose to return a MessageError
	// Message instead of returning a Reply with System Exception
	// message because the other-end says do not send me a reply!
	pd_cdrStream->SendMsgErrorMessage();
	pd_state = GIOP_S::Idle;
      }
      else {
	CORBA::OBJECT_NOT_EXIST ex(0,CORBA::COMPLETED_NO);
	MarshallSystemException(this,GIOP_Basetypes::SysExceptRepoID::OBJECT_NOT_EXIST,ex);
      }
    }
  }
  catch (const CORBA::OBJECT_NOT_EXIST &ex) {
    CHECK_AND_MAYBE_MARSHALL_SYSTEM_EXCEPTION (OBJECT_NOT_EXIST,ex);
  }
  catch (const CORBA::UNKNOWN &ex) {
    CHECK_AND_MAYBE_MARSHALL_SYSTEM_EXCEPTION (UNKNOWN,ex);
  }
  catch (const CORBA::BAD_PARAM &ex) {
    CHECK_AND_MAYBE_MARSHALL_SYSTEM_EXCEPTION (BAD_PARAM,ex);
  }
  catch (const CORBA::NO_MEMORY &ex) {
    CHECK_AND_MAYBE_MARSHALL_SYSTEM_EXCEPTION (NO_MEMORY,ex);
  }
  catch (const CORBA::IMP_LIMIT &ex) {
    CHECK_AND_MAYBE_MARSHALL_SYSTEM_EXCEPTION (IMP_LIMIT,ex);
  }
  catch (const CORBA::COMM_FAILURE &ex) {
    CHECK_AND_MAYBE_MARSHALL_SYSTEM_EXCEPTION (COMM_FAILURE,ex);
  }
  catch (const CORBA::INV_OBJREF &ex) {
    CHECK_AND_MAYBE_MARSHALL_SYSTEM_EXCEPTION (INV_OBJREF,ex);
  }
  catch (const CORBA::NO_PERMISSION &ex) {
    CHECK_AND_MAYBE_MARSHALL_SYSTEM_EXCEPTION (NO_PERMISSION,ex);
  }
  catch (const CORBA::INTERNAL &ex) {
    CHECK_AND_MAYBE_MARSHALL_SYSTEM_EXCEPTION (INTERNAL,ex);
  }
  catch (const CORBA::MARSHAL &ex) {
    CHECK_AND_MAYBE_MARSHALL_SYSTEM_EXCEPTION (MARSHAL,ex);
  }
  catch (const CORBA::INITIALIZE &ex) {
    CHECK_AND_MAYBE_MARSHALL_SYSTEM_EXCEPTION (INITIALIZE,ex);
  }
  catch (const CORBA::NO_IMPLEMENT &ex) {
    CHECK_AND_MAYBE_MARSHALL_SYSTEM_EXCEPTION (NO_IMPLEMENT,ex);
  }
  catch (const CORBA::BAD_TYPECODE &ex) {
    CHECK_AND_MAYBE_MARSHALL_SYSTEM_EXCEPTION (BAD_TYPECODE,ex);
  }
  catch (const CORBA::BAD_OPERATION &ex) {
    CHECK_AND_MAYBE_MARSHALL_SYSTEM_EXCEPTION (BAD_OPERATION,ex);
  }
  catch (const CORBA::NO_RESOURCES &ex) {
    CHECK_AND_MAYBE_MARSHALL_SYSTEM_EXCEPTION (NO_RESOURCES,ex);
  }
  catch (const CORBA::NO_RESPONSE &ex) {
    CHECK_AND_MAYBE_MARSHALL_SYSTEM_EXCEPTION (NO_RESPONSE,ex);
  }
  catch (const CORBA::PERSIST_STORE &ex) {
    CHECK_AND_MAYBE_MARSHALL_SYSTEM_EXCEPTION (PERSIST_STORE,ex);
  }
  catch (const CORBA::BAD_INV_ORDER &ex) {
    CHECK_AND_MAYBE_MARSHALL_SYSTEM_EXCEPTION (BAD_INV_ORDER,ex);
  }
  catch (const CORBA::TRANSIENT &ex) {
    CHECK_AND_MAYBE_MARSHALL_SYSTEM_EXCEPTION (TRANSIENT,ex);
  }
  catch (const CORBA::FREE_MEM &ex) {
    CHECK_AND_MAYBE_MARSHALL_SYSTEM_EXCEPTION (FREE_MEM,ex);
  }
  catch (const CORBA::INV_IDENT &ex) {
    CHECK_AND_MAYBE_MARSHALL_SYSTEM_EXCEPTION (INV_IDENT,ex);
  }
  catch (const CORBA::INV_FLAG &ex) {
    CHECK_AND_MAYBE_MARSHALL_SYSTEM_EXCEPTION (INV_FLAG,ex);
  }
  catch (const CORBA::INTF_REPOS &ex) {
    CHECK_AND_MAYBE_MARSHALL_SYSTEM_EXCEPTION (INTF_REPOS,ex);
  }
  catch (const CORBA::BAD_CONTEXT &ex) {
    CHECK_AND_MAYBE_MARSHALL_SYSTEM_EXCEPTION (BAD_CONTEXT,ex);
  }
  catch (const CORBA::OBJ_ADAPTER &ex) {
    CHECK_AND_MAYBE_MARSHALL_SYSTEM_EXCEPTION (OBJ_ADAPTER,ex);
  }
  catch (const CORBA::DATA_CONVERSION &ex) {
    CHECK_AND_MAYBE_MARSHALL_SYSTEM_EXCEPTION (DATA_CONVERSION,ex);
  }
  catch (const CORBA::TRANSACTION_REQUIRED &ex) {
    CHECK_AND_MAYBE_MARSHALL_SYSTEM_EXCEPTION (TRANSACTION_REQUIRED,ex);
  }
  catch (const CORBA::TRANSACTION_ROLLEDBACK &ex) {
    CHECK_AND_MAYBE_MARSHALL_SYSTEM_EXCEPTION (TRANSACTION_ROLLEDBACK,ex);
  }
  catch (const CORBA::INVALID_TRANSACTION &ex) {
    CHECK_AND_MAYBE_MARSHALL_SYSTEM_EXCEPTION (INVALID_TRANSACTION,ex);
  }
  catch (const CORBA::WRONG_TRANSACTION &ex) {
    CHECK_AND_MAYBE_MARSHALL_SYSTEM_EXCEPTION (WRONG_TRANSACTION,ex);
  }
  catch (const terminateProcessing &) {
    if (pd_state == GIOP_S::WaitingForReply ||
	pd_state == GIOP_S::RequestIsBeingProcessed) {
      // XXXX Check if requestisbeingprocessed, should skip the rest of the
      // message.
      pd_state = GIOP_S::Idle;
    }
  }
  catch (...) {
    CORBA::UNKNOWN ex(0,CORBA::COMPLETED_MAYBE);
    CHECK_AND_MAYBE_MARSHALL_SYSTEM_EXCEPTION (UNKNOWN,ex);
  }
  return;
}
#undef CHECK_AND_MAYBE_MARSHALL_SYSTEM_EXCEPTION

void
GIOP_S::HandleLocateRequest()
{
  RequestReceived();

  omniObject_var obj;    

  GIOP::LocateStatusType status = GIOP::UNKNOWN_OBJECT;

  try {

    if (pd_invokeInfo.keysize() == 0) {
      // This case is only possible with GIOP version 1.2 or
      // above. The target object's identity is encoded in a
      // GIOP::IORAddressingInfo struct. inputRequestHeader() has looked
      // into it and discovered that it is not destined for an object in
      // this address space.
      // Normally, we can just say object not exist here.
      // But our application may be a GIOP proxy which will relay the
      // request to the real target. We give such a proxy a chance to
      // to look at this request header and if it can return an omniObject
      // we'll use it to dispatch the call.
      if (MapTargetAddressToObjectFunction) {
	obj = MapTargetAddressToObjectFunction(pd_invokeInfo);
      }
    }
    else if (pd_invokeInfo.keysize() != sizeof(omniObjectKey)) {

      // This key did not come from this orb.

      // We make a special case for the bootstrapping agent.
      // The object key is "INIT". If the key match and we do have
      // the bootstrapping agent running, initialise obj to point to it. 
      if (pd_invokeInfo.keysize() == 4) {
	const char* k = (const char*) pd_invokeInfo.key();
	if (k[0] == 'I' && k[1] == 'N' && k[2] == 'I' && k[3] == 'T') {
	  obj = omniInitialReferences::singleton()->has_bootstrap_agentImpl();
	  if ((omniObject*)obj) omni::objectDuplicate(obj);
	}
      }
    }

    if (! (omniObject*)obj) {
      if (pd_invokeInfo.keysize() == sizeof(omniObjectKey)){
	// In future, we have to partially decode the object key to
	// determine which object manager it belongs to.
	// For the moment, there is only one object manager- rootObjectManager.
	if ((obj = omni::locateObject(omniObjectManager::root(),
				      *(omniObjectKey*)pd_invokeInfo.key()))) {
	  status = GIOP::OBJECT_HERE;
	}
	else if ((obj = omni::locatePyObject(omniObjectManager::root(),
				      *(omniObjectKey*)pd_invokeInfo.key()))) {
	  status = GIOP::OBJECT_HERE;
	}
	else if (MapKeyToObjectFunction) {
	  // Cannot find the object in the object table. If the application
	  // has registered a loader, do an upcall to locate the object.
	  // If the return value is not a nil object reference, reply with
	  // OBJECT_FORWARD and the new object reference.
	  CORBA::Object_var newDestination = MapKeyToObjectFunction(*(omniObjectKey*)pd_invokeInfo.key());
	  if (!CORBA::is_nil(newDestination)) {
	    status = GIOP::OBJECT_FORWARD;

	    pd_state = GIOP_S::ReplyIsBeingComposed;

	    pd_cdrStream->outputLocateReplyMessageBegin(pd_invokeInfo,status);
	    CORBA::Object::marshalObjRef(newDestination,*pd_cdrStream);
	    pd_cdrStream->outputMessageEnd();

	    pd_state = GIOP_S::Idle;
	  }
	}
      }
    }
    else {
      status = GIOP::OBJECT_HERE;
    }

  }
  catch(...) {
    status = GIOP::UNKNOWN_OBJECT;
  }

  if (status != GIOP::OBJECT_FORWARD) {

    pd_state = GIOP_S::ReplyIsBeingComposed;

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

static
void 
MarshallSystemException(GIOP_S *s,
			const GIOP_Basetypes::_SysExceptRepoID &id,
			const CORBA::SystemException &ex)
{
  s->InitialiseReply(GIOP::SYSTEM_EXCEPTION);

  CORBA::ULong exsize = id.len + 
    GIOP_Basetypes::SysExceptRepoID::versionLen + 1;

  cdrStream& m = (cdrStream&)*s;
  exsize >>= m;
  m.put_char_array(id.id, id.len);
  m.put_char_array((CORBA::Char*)
		   GIOP_Basetypes::SysExceptRepoID::version,
		   GIOP_Basetypes::SysExceptRepoID::versionLen + 1);
  ex.minor() >>= m;
  operator>>= ((CORBA::ULong)ex.completed(),m);
  s->ReplyCompleted();
  return;
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
