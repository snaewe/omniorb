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

#include <scavenger.h>
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


static void 
MarshallSystemException(GIOP_S *s,
			const GIOP_Basetypes::_SysExceptRepoID &id,
			const CORBA::SystemException &ex);


GIOP_S::GIOP_S(Strand *s)
  : NetBufferedStream(s,1,0,0)
{
  pd_state = GIOP_S::Idle;
  pd_operation = &pd_op_buffer[0];
  pd_principal = &pd_pr_buffer[0];
  pd_max_message_size = NetBufferedStream::MaxMessageSize() - 
                           sizeof(MessageHeader::HeaderType) -
                           sizeof(CORBA::ULong);
  if (pd_max_message_size > omniORB::MaxMessageSize()) {
    pd_max_message_size = omniORB::MaxMessageSize();
  }
  return;
}


GIOP_S::~GIOP_S()
{
  if (pd_state == GIOP_S::Zombie)
    return;

  if (pd_operation != &pd_op_buffer[0]) {
    delete [] pd_operation;
    pd_operation = &pd_op_buffer[0];
  }
  if (pd_principal != &pd_pr_buffer[0]) {
    delete [] pd_principal;
    pd_principal = &pd_pr_buffer[0];
  }

  if (pd_state != GIOP_S::Idle) {
    setStrandIsDying();
  }
  pd_state = GIOP_S::Zombie;
  return;
}

void
GIOP_S::RequestReceived(CORBA::Boolean skip_msg)
{
  if (pd_state != GIOP_S::RequestIsBeingProcessed)
    throw omniORB::fatalException(__FILE__,__LINE__,
				  "GIOP_S::RequestReceived() entered with the wrong state.");				  
  if (skip_msg)
    {
      skip(RdMessageUnRead(),1);
    }
  else
    {
      if (RdMessageUnRead())
	{
	  // The value in the message size field in the GIOP header is larger
	  // than the actual body size, i.e there is garbage at the end.
	  //
	  // The default behaviour is to just silently skip the unread part.
	  // The problem with this behaviour is that the header message size
	  // may actually be garbage value, caused by a bug in the client
	  // code. This thread may forever block on the strand as it tries
	  // to read more data from it. In this case the client won't send
	  // anymore as it thinks it has marshalled in all the data.
	  //
	  // It is really a sloppy IIOP implementation (through strictly
	  // speaking not a violation of the specification) to have a
	  // message size value in the header that doesn't match with
	  // a body that contains only the marshalled data.
	  //
	  // If omniORB::strictIIOP non-zero, we expect incoming calls to
	  // be well behaved and rejects anything that is not.
	  if (omniORB::traceLevel >= 15) {
	    omniORB::log << "GIOP_S::RequestReceived: garbage left at the end of message.\n";
	    omniORB::log.flush();
	  }
	  if (!omniORB::strictIIOP) {
	    skip(RdMessageUnRead(),1);
	  }
	  else {
	    setStrandIsDying();
	    throw CORBA::COMM_FAILURE(0,CORBA::COMPLETED_NO);
	  }
	}
      else {
	skip(0,1);
      }
    }

  WrLock();
  // Strictly speaking, we do not need to have exclusive write access to
  // the strand so early. However, WrLock() has the side effect of resetting
  // the heartbeat boolean of the strand and hence tell the strand scavenger
  // to leave it. Therefore, we call WrLock() here and check if the strand
  // has been killed asynchronously by the scavenger. If it is the case
  // (as indicated by strandIsDying() == 1), we do not proceed any further.
  if (strandIsDying()) {
    throw CORBA::COMM_FAILURE(0,CORBA::COMPLETED_NO);
  }

  pd_state = GIOP_S::WaitingForReply;
  return;
}

size_t 
GIOP_S::ReplyHeaderSize()
{
  // Compute the header size, this includes the GIOP Message header and
  // the Reply message header.

  CORBA::ULong msgsize = sizeof(MessageHeader::Reply) + sizeof(CORBA::ULong);

  // ReplyHeader.service_context
  msgsize += sizeof(CORBA::ULong); // XXX 0 length service context
  
  // ReplyHeader.request_id
  // msgsize = omni::align_to(msgsize,omni::ALIGN_4);
  msgsize += sizeof(CORBA::ULong);

  // ReplyHeader.reply_status
  // msgsize = omni::align_to(msgsize,omni::ALIGN_4);
  msgsize += sizeof(CORBA::ULong);

  return msgsize;
}

void
GIOP_S::InitialiseReply(const GIOP::ReplyStatusType status,
			const size_t  msgsize)
{
  if (!pd_response_expected)
    throw terminateProcessing();

  if (pd_state != GIOP_S::WaitingForReply)
    throw omniORB::fatalException(__FILE__,__LINE__,
				  "GIOP_S::InitialiseReply() entered with the wrong state.");

  size_t bodysize = msgsize-sizeof(MessageHeader::Reply)-sizeof(CORBA::ULong);
  if (bodysize > MaxMessageSize())
    throw CORBA::MARSHAL(0,CORBA::COMPLETED_YES);

  // 
  // WrLock();
  //   Strictly speaking, we only need to have exclusive write access to
  //   the strand from this point on. Hence it is appropriate to acquire the
  //   write lock here. 
  //   However, WrLock() also has the side effect of resetting the heartbeat
  //   boolean of the strand and hence tell the strand scavenger to leave it.
  //   For this purpose, we have to call WrLock before we do an upcall to the
  //   implementation class to ensure that the invocation is not discrupted
  //   by the scavenger killing the strand under our feet.

  pd_state = GIOP_S::ReplyIsBeingComposed;


  // Marshall the GIOP Message Header

  WrMessageSize(msgsize);
  put_char_array((CORBA::Char*) MessageHeader::Reply,
		 sizeof(MessageHeader::Reply),
		 omni::ALIGN_1, 1, 1);

  operator>>= ((CORBA::ULong)bodysize,*this);

  // Marshall the Reply Header
  // XXX should marshall service context here,
  // XXX for the moment, just marshall a 0 length service context
  operator>>= ((CORBA::ULong) 0,*this);

  operator>>= (pd_request_id,*this);
  operator>>= ((CORBA::ULong) status,*this);
  return;
}

void
GIOP_S::ReplyCompleted()
{
  if (!pd_response_expected)
    {
      if (pd_state != GIOP_S::WaitingForReply)
	throw omniORB::fatalException(__FILE__,__LINE__,
				      "GIOP_S::ReplyCompleted() entered with the wrong state.");
      pd_state = GIOP_S::Idle;
      return;
    }

  if (pd_state != GIOP_S::ReplyIsBeingComposed)
    throw omniORB::fatalException(__FILE__,__LINE__,
				  "GIOP_S::ReplyCompleted() entered with the wrong state.");

  flush(1);

  if (WrMessageSpaceLeft())
    throw omniORB::fatalException(__FILE__,__LINE__,
				  "GIOP_S::ReplyCompleted() reported wrong reply message size.");

  if (pd_operation != &pd_op_buffer[0]) {
    delete [] pd_operation;
    pd_operation = &pd_op_buffer[0];
  }
  if (pd_principal != &pd_pr_buffer[0]) {
    delete [] pd_principal;
    pd_principal = &pd_pr_buffer[0];
  }
  
  pd_state = GIOP_S::Idle;

  WrUnlock();
}

void
GIOP_S::dispatcher(Strand *s)
{
  GIOP_S gs(s);

  gs.pd_state = GIOP_S::RequestIsBeingProcessed;

  gs.RdMessageSize(sizeof(MessageHeader::HeaderType)+sizeof(CORBA::ULong),0);

  MessageHeader::HeaderType hdr;
  gs.get_char_array((CORBA::Char *)hdr, sizeof(MessageHeader::HeaderType),
		    omni::ALIGN_1, 1);

  gs.clicksSet(StrandScavenger::serverCallTimeLimit());

  switch (hdr[7])
    {
    case GIOP::Request:
      {
	if (hdr[0] != MessageHeader::Request[0] ||
	    hdr[1] != MessageHeader::Request[1] ||
	    hdr[2] != MessageHeader::Request[2] ||
	    hdr[3] != MessageHeader::Request[3] ||
	    hdr[4] != MessageHeader::Request[4] ||
	    hdr[5] != MessageHeader::Request[5])
	  {
	    // Wrong header
	    gs.SendMsgErrorMessage();
	    gs.setStrandIsDying();
	    throw CORBA::COMM_FAILURE(0,CORBA::COMPLETED_MAYBE);
	  }
	gs.RdMessageSize(sizeof(CORBA::ULong),hdr[6]);
	gs.HandleRequest(hdr[6]);
	break;
      }
    case GIOP::LocateRequest:
      {
	if (hdr[0] != MessageHeader::LocateRequest[0] ||
	    hdr[1] != MessageHeader::LocateRequest[1] ||
	    hdr[2] != MessageHeader::LocateRequest[2] ||
	    hdr[3] != MessageHeader::LocateRequest[3] ||
	    hdr[4] != MessageHeader::LocateRequest[4] ||
	    hdr[5] != MessageHeader::LocateRequest[5])
	  {
	    // Wrong header
	    gs.SendMsgErrorMessage();
	    gs.setStrandIsDying();
	    throw CORBA::COMM_FAILURE(0,CORBA::COMPLETED_MAYBE);
	  }
	gs.RdMessageSize(sizeof(CORBA::ULong),hdr[6]);
	gs.HandleLocateRequest(hdr[6]);
	break;
      }
    case GIOP::CancelRequest:
      {
	if (hdr[0] != MessageHeader::CancelRequest[0] ||
	    hdr[1] != MessageHeader::CancelRequest[1] ||
	    hdr[2] != MessageHeader::CancelRequest[2] ||
	    hdr[3] != MessageHeader::CancelRequest[3] ||
	    hdr[4] != MessageHeader::CancelRequest[4] ||
	    hdr[5] != MessageHeader::CancelRequest[5])
	  {
	    // Wrong header
	    gs.SendMsgErrorMessage();
	    gs.setStrandIsDying();
	    throw CORBA::COMM_FAILURE(0,CORBA::COMPLETED_MAYBE);
	  }
	gs.RdMessageSize(sizeof(CORBA::ULong),hdr[6]);
	gs.HandleCancelRequest(hdr[6]);
	break;
      }
    case GIOP::MessageError:
      {
	if (hdr[0] != MessageHeader::Request[0] ||
	    hdr[1] != MessageHeader::Request[1] ||
	    hdr[2] != MessageHeader::Request[2] ||
	    hdr[3] != MessageHeader::Request[3] ||
	    hdr[4] != MessageHeader::Request[4] ||
	    hdr[5] != MessageHeader::Request[5])
	  {
	    // Wrong header
	    gs.SendMsgErrorMessage();
	    gs.setStrandIsDying();
	    throw CORBA::COMM_FAILURE(0,CORBA::COMPLETED_MAYBE);
	  }
	gs.HandleMessageError();
	break;
      }
    case GIOP::CloseConnection:
      {
	if (hdr[0] != MessageHeader::Request[0] ||
	    hdr[1] != MessageHeader::Request[1] ||
	    hdr[2] != MessageHeader::Request[2] ||
	    hdr[3] != MessageHeader::Request[3] ||
	    hdr[4] != MessageHeader::Request[4] ||
	    hdr[5] != MessageHeader::Request[5])
	  {
	    // Wrong header
	    gs.SendMsgErrorMessage();
	    gs.setStrandIsDying();
	    throw CORBA::COMM_FAILURE(0,CORBA::COMPLETED_MAYBE);
	  }
	gs.HandleCloseConnection();
	break;
      }
    default:
      {
	// Wrong header or invalid message type
	gs.SendMsgErrorMessage();
	gs.setStrandIsDying();
	throw CORBA::COMM_FAILURE(0,CORBA::COMPLETED_NO);
      }
    }

  gs.clicksSet(StrandScavenger::inIdleTimeLimit());

  return;
}

#define CHECK_AND_MAYBE_MARSHALL_SYSTEM_EXCEPTION(exrepoid,exvar) do {\
    if (!strandIsDying() && (pd_state == RequestIsBeingProcessed || \
	pd_state == WaitingForReply)) \
      { \
	if (pd_state == RequestIsBeingProcessed) { \
           RequestReceived(1); \
	} \
	if (!pd_response_expected) \
	{ \
	  SendMsgErrorMessage(); \
          ReplyCompleted(); \
	} \
        else \
        { \
	  MarshallSystemException(this,SysExceptRepoID:: exrepoid ,exvar); \
	}  \
      } \
    else { \
	throw; \
    } \
  } while (0)

void
GIOP_S::HandleRequest(CORBA::Boolean byteorder)
{
  CORBA::ULong msgsize;
  omniObject *obj = 0;    

  try {
    // This try block catches any exception that might raise during
    // the processing of the request header
    msgsize <<= *this;
    if (msgsize > MaxMessageSize()) {
      SendMsgErrorMessage();
      setStrandIsDying();
      throw CORBA::COMM_FAILURE(0,CORBA::COMPLETED_NO);
    }

    RdMessageSize(msgsize,byteorder);  // Set the size of the message body.
  
    // XXX Do not support any service context yet, 
    // XXX For the moment, skips the service context
    CORBA::ULong svcccount;
    CORBA::ULong svcctag;
    CORBA::ULong svcctxtsize;
    svcccount <<= *this;
    while (svcccount-- > 0) {
      svcctag <<= *this;
      svcctxtsize <<= *this;
      skip(svcctxtsize);
    };

    pd_request_id <<= *this;

    pd_response_expected <<= *this;

    CORBA::ULong keysize;
    keysize <<= *this;
    if (keysize == sizeof(omniObjectKey)) {
      get_char_array((CORBA::Char*) &pd_objkey, keysize);
    }
    else {
      // This key did not come from this orb.
      // silently skip the key. Initialise pd_objkey to all zeros and
      // let the call to locateObject() below to raise the proper exception
      pd_objkey = omniORB::nullkey();
      
      // However, we make a special case for the bootstrapping agent.
      // The object key is "INIT". If the key match and we do have
      // the bootstrapping agent running, initialise obj to point to it. 
      if (keysize == 4) {
	CORBA::Char k[4];
	get_char_array(k, 4);
	if (k[0] == 'I' && k[1] == 'N' && k[2] == 'I' && k[3] == 'T') {
	  obj = omniInitialReferences::singleton()->has_bootstrap_agentImpl();
	}
      }
      else {
	skip(keysize);
      }
    }

    CORBA::ULong octetlen;
    octetlen <<= *this;
    if (octetlen > OMNIORB_GIOPDRIVER_GIOP_S_INLINE_BUF_SIZE)
      {
	// Do not blindly allocate a buffer for the operation string
	// a poison packet might set this to a huge value
	if (octetlen > RdMessageUnRead()) {
	  throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);
	}
	CORBA::Octet *p = new CORBA::Octet[octetlen];
	if (!p)
	  throw CORBA::NO_MEMORY(0,CORBA::COMPLETED_NO);
	pd_operation = p;
      }
    get_char_array((CORBA::Octet*) pd_operation, octetlen);

    octetlen <<= *this;
    if (octetlen > OMNIORB_GIOPDRIVER_GIOP_S_INLINE_BUF_SIZE)
      {
	// Do not blindly allocate a buffer for the principal octet vector
	// a poison packet might set this to a huge value
	if (octetlen > RdMessageUnRead()) {
	  throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);
	}
	CORBA::Octet *p = new CORBA::Octet[octetlen];
	if (!p)
	  throw CORBA::NO_MEMORY(0,CORBA::COMPLETED_NO);
	pd_principal = p;
      }
    get_char_array((CORBA::Octet*) pd_principal, octetlen);
  }
  catch (const CORBA::MARSHAL &ex) {
    RequestReceived(1);
    SendMsgErrorMessage();
    pd_state = GIOP_S::Idle;
    return;
  }
  catch (const CORBA::NO_MEMORY &ex) {
    RequestReceived(1);
    MarshallSystemException(this,SysExceptRepoID::NO_MEMORY,ex);
    return;
  }

  // Note: If this is a one way invocation, we choose to return a 
  // MessageError Message instead of returning a Reply with System Exception
  // message because the other-end says do not send me a reply!

  try {
    // In future, we have to partially decode the object key to
    // determine which object manager it belongs to.
    // For the moment, there is only one object manager- the rootObjectManager.
    if (obj)
      omni::objectDuplicate(obj);

    if (!obj)
      obj = omni::locateObject(omniObjectManager::root(),pd_objkey);

    if (!obj)
      obj = omni::locatePyObject(omniObjectManager::root(),pd_objkey);

    if (obj) {
      if (!obj->dispatch(*this,
			 (const char *)pd_operation,
			 pd_response_expected)) {

	// Try built-in operations...
	if (!obj->omniObject::dispatch(*this,
				       (const char*)pd_operation,
				       pd_response_expected)) {
	  // No luck
	  RequestReceived(1);
	  throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
	}
      }
    }
    else { // !obj

      if (pd_response_expected && MapKeyToObjectFunction) {
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
	CORBA::Object_var newDestination= MapKeyToObjectFunction(pd_objkey);
	if (!CORBA::is_nil(newDestination)) {
	  // Note that we have completed the object request
	  RequestReceived(1); 
	
	  // Build and send the location forward message...
	  size_t msgsize = (size_t) GIOP_S::ReplyHeaderSize();
	  msgsize = CORBA::Object::NP_alignedSize(newDestination, msgsize);
	  InitialiseReply(GIOP::LOCATION_FORWARD,(CORBA::ULong)msgsize);
	  CORBA::Object::marshalObjRef(newDestination, *this);
	
	  // All done...
	  ReplyCompleted();
	  return;
	}
      }

      // No application-defined loader, or the loader returned nil
      RequestReceived(1);
      if (!pd_response_expected) {
	// This is a one way invocation, we choose to return a MessageError
	// Message instead of returning a Reply with System Exception
	// message because the other-end says do not send me a reply!
	SendMsgErrorMessage();
	ReplyCompleted();
      }
      else {
	CORBA::OBJECT_NOT_EXIST ex(0,CORBA::COMPLETED_NO);
	MarshallSystemException(this,SysExceptRepoID::OBJECT_NOT_EXIST,ex);
      }
    }
  }
  catch (const CORBA::OBJECT_NOT_EXIST &ex) {
    if (obj) omni::objectRelease(obj); obj = 0;
    CHECK_AND_MAYBE_MARSHALL_SYSTEM_EXCEPTION (OBJECT_NOT_EXIST,ex);
  }
  catch (const CORBA::UNKNOWN &ex) {
    if (obj) omni::objectRelease(obj); obj = 0;
    CHECK_AND_MAYBE_MARSHALL_SYSTEM_EXCEPTION (UNKNOWN,ex);
  }
  catch (const CORBA::BAD_PARAM &ex) {
    if (obj) omni::objectRelease(obj); obj = 0;
    CHECK_AND_MAYBE_MARSHALL_SYSTEM_EXCEPTION (BAD_PARAM,ex);
  }
  catch (const CORBA::NO_MEMORY &ex) {
    if (obj) omni::objectRelease(obj); obj = 0;
    CHECK_AND_MAYBE_MARSHALL_SYSTEM_EXCEPTION (NO_MEMORY,ex);
  }
  catch (const CORBA::IMP_LIMIT &ex) {
    if (obj) omni::objectRelease(obj); obj = 0;
    CHECK_AND_MAYBE_MARSHALL_SYSTEM_EXCEPTION (IMP_LIMIT,ex);
  }
  catch (const CORBA::COMM_FAILURE &ex) {
    if (obj) omni::objectRelease(obj); obj = 0;
    CHECK_AND_MAYBE_MARSHALL_SYSTEM_EXCEPTION (COMM_FAILURE,ex);
  }
  catch (const CORBA::INV_OBJREF &ex) {
    if (obj) omni::objectRelease(obj); obj = 0;
    CHECK_AND_MAYBE_MARSHALL_SYSTEM_EXCEPTION (INV_OBJREF,ex);
  }
  catch (const CORBA::NO_PERMISSION &ex) {
    if (obj) omni::objectRelease(obj); obj = 0;
    CHECK_AND_MAYBE_MARSHALL_SYSTEM_EXCEPTION (NO_PERMISSION,ex);
  }
  catch (const CORBA::INTERNAL &ex) {
    if (obj) omni::objectRelease(obj); obj = 0;
    CHECK_AND_MAYBE_MARSHALL_SYSTEM_EXCEPTION (INTERNAL,ex);
  }
  catch (const CORBA::MARSHAL &ex) {
    if (obj) omni::objectRelease(obj); obj = 0;
    CHECK_AND_MAYBE_MARSHALL_SYSTEM_EXCEPTION (MARSHAL,ex);
  }
  catch (const CORBA::INITIALIZE &ex) {
    if (obj) omni::objectRelease(obj); obj = 0;
    CHECK_AND_MAYBE_MARSHALL_SYSTEM_EXCEPTION (INITIALIZE,ex);
  }
  catch (const CORBA::NO_IMPLEMENT &ex) {
    if (obj) omni::objectRelease(obj); obj = 0;
    CHECK_AND_MAYBE_MARSHALL_SYSTEM_EXCEPTION (NO_IMPLEMENT,ex);
  }
  catch (const CORBA::BAD_TYPECODE &ex) {
    if (obj) omni::objectRelease(obj); obj = 0;
    CHECK_AND_MAYBE_MARSHALL_SYSTEM_EXCEPTION (BAD_TYPECODE,ex);
  }
  catch (const CORBA::BAD_OPERATION &ex) {
    if (obj) omni::objectRelease(obj); obj = 0;
    CHECK_AND_MAYBE_MARSHALL_SYSTEM_EXCEPTION (BAD_OPERATION,ex);
  }
  catch (const CORBA::NO_RESOURCES &ex) {
    if (obj) omni::objectRelease(obj); obj = 0;
    CHECK_AND_MAYBE_MARSHALL_SYSTEM_EXCEPTION (NO_RESOURCES,ex);
  }
  catch (const CORBA::NO_RESPONSE &ex) {
    if (obj) omni::objectRelease(obj); obj = 0;
    CHECK_AND_MAYBE_MARSHALL_SYSTEM_EXCEPTION (NO_RESPONSE,ex);
  }
  catch (const CORBA::PERSIST_STORE &ex) {
    if (obj) omni::objectRelease(obj); obj = 0;
    CHECK_AND_MAYBE_MARSHALL_SYSTEM_EXCEPTION (PERSIST_STORE,ex);
  }
  catch (const CORBA::BAD_INV_ORDER &ex) {
    if (obj) omni::objectRelease(obj); obj = 0;
    CHECK_AND_MAYBE_MARSHALL_SYSTEM_EXCEPTION (BAD_INV_ORDER,ex);
  }
  catch (const CORBA::TRANSIENT &ex) {
    if (obj) omni::objectRelease(obj); obj = 0;
    CHECK_AND_MAYBE_MARSHALL_SYSTEM_EXCEPTION (TRANSIENT,ex);
  }
  catch (const CORBA::FREE_MEM &ex) {
    if (obj) omni::objectRelease(obj); obj = 0;
    CHECK_AND_MAYBE_MARSHALL_SYSTEM_EXCEPTION (FREE_MEM,ex);
  }
  catch (const CORBA::INV_IDENT &ex) {
    if (obj) omni::objectRelease(obj); obj = 0;
    CHECK_AND_MAYBE_MARSHALL_SYSTEM_EXCEPTION (INV_IDENT,ex);
  }
  catch (const CORBA::INV_FLAG &ex) {
    if (obj) omni::objectRelease(obj); obj = 0;
    CHECK_AND_MAYBE_MARSHALL_SYSTEM_EXCEPTION (INV_FLAG,ex);
  }
  catch (const CORBA::INTF_REPOS &ex) {
    if (obj) omni::objectRelease(obj); obj = 0;
    CHECK_AND_MAYBE_MARSHALL_SYSTEM_EXCEPTION (INTF_REPOS,ex);
  }
  catch (const CORBA::BAD_CONTEXT &ex) {
    if (obj) omni::objectRelease(obj); obj = 0;
    CHECK_AND_MAYBE_MARSHALL_SYSTEM_EXCEPTION (BAD_CONTEXT,ex);
  }
  catch (const CORBA::OBJ_ADAPTER &ex) {
    if (obj) omni::objectRelease(obj); obj = 0;
    CHECK_AND_MAYBE_MARSHALL_SYSTEM_EXCEPTION (OBJ_ADAPTER,ex);
  }
  catch (const CORBA::DATA_CONVERSION &ex) {
    if (obj) omni::objectRelease(obj); obj = 0;
    CHECK_AND_MAYBE_MARSHALL_SYSTEM_EXCEPTION (DATA_CONVERSION,ex);
  }
  catch (const CORBA::TRANSACTION_REQUIRED &ex) {
    if (obj) omni::objectRelease(obj); obj = 0;
    CHECK_AND_MAYBE_MARSHALL_SYSTEM_EXCEPTION (TRANSACTION_REQUIRED,ex);
  }
  catch (const CORBA::TRANSACTION_ROLLEDBACK &ex) {
    if (obj) omni::objectRelease(obj); obj = 0;
    CHECK_AND_MAYBE_MARSHALL_SYSTEM_EXCEPTION (TRANSACTION_ROLLEDBACK,ex);
  }
  catch (const CORBA::INVALID_TRANSACTION &ex) {
    if (obj) omni::objectRelease(obj); obj = 0;
    CHECK_AND_MAYBE_MARSHALL_SYSTEM_EXCEPTION (INVALID_TRANSACTION,ex);
  }
  catch (const CORBA::WRONG_TRANSACTION &ex) {
    if (obj) omni::objectRelease(obj); obj = 0;
    CHECK_AND_MAYBE_MARSHALL_SYSTEM_EXCEPTION (WRONG_TRANSACTION,ex);
  }
  catch (const terminateProcessing &) {
    if (pd_state == GIOP_S::WaitingForReply) {
      pd_state = GIOP_S::Idle;
    }
  }
  catch (...) {
    if (obj) omni::objectRelease(obj); obj = 0;
    CORBA::UNKNOWN ex(0,CORBA::COMPLETED_MAYBE);
    CHECK_AND_MAYBE_MARSHALL_SYSTEM_EXCEPTION (UNKNOWN,ex);
  }
  if (obj) omni::objectRelease(obj);
  return;
}
#undef CHECK_AND_MAYBE_MARSHALL_SYSTEM_EXCEPTION

void
GIOP_S::HandleLocateRequest(CORBA::Boolean byteorder)
{
  CORBA::ULong msgsize;

  try {
    // This try block catches any exception that might raise during
    // the processing of the request header
    msgsize <<= *this;
    if (msgsize > MaxMessageSize()) {
      SendMsgErrorMessage();
      setStrandIsDying();
      throw CORBA::COMM_FAILURE(0,CORBA::COMPLETED_NO);
    }

    RdMessageSize(msgsize,byteorder);  // Set the size of the message body.

    pd_request_id <<= *this;

    CORBA::ULong keysize;
    keysize <<= *this;
    if (keysize != sizeof(omniObjectKey)) {
      // This key did not come from this orb.
      // silently skip the key. Initialise pd_objkey to all zeros and
      // let the call to locateObject() below to raise the proper exception
      pd_objkey = omniORB::nullkey();
      skip(keysize);
    }
    else {
      get_char_array((CORBA::Char*) &pd_objkey, keysize);
    }
    RequestReceived();
  }
  catch (const CORBA::MARSHAL &ex) {
    RequestReceived(1);
    SendMsgErrorMessage();
    pd_state = GIOP_S::Idle;
    return;
  }
  catch (const CORBA::NO_MEMORY &ex) {
    RequestReceived(1);
    MarshallSystemException(this,SysExceptRepoID::NO_MEMORY,ex);
    return;
  }

  omniObject *obj = 0;
  GIOP::LocateStatusType status;

  if ((obj = omni::locateObject(omniObjectManager::root(),pd_objkey))) {
    omni::objectRelease(obj);
    status = GIOP::OBJECT_HERE;
  }
  else if ((obj = omni::locatePyObject(omniObjectManager::root(),pd_objkey))) {
    omni::objectRelease(obj);
    status = GIOP::OBJECT_HERE;
  }
  else if (MapKeyToObjectFunction) {
    // Cannot find the object in the object tables. If the application
    // has registered a loader, do an upcall to locate the object.  If
    // the return value is not a nil object reference, reply with
    // OBJECT_FORWARD and the new object reference.
    status = GIOP::UNKNOWN_OBJECT;

    try {
      CORBA::Object_var newDestination = MapKeyToObjectFunction(pd_objkey);
      if (!CORBA::is_nil(newDestination)) {
	status = GIOP::OBJECT_FORWARD;
	WrLock();
	pd_state = GIOP_S::ReplyIsBeingComposed;

	size_t msgsize = sizeof(MessageHeader::LocateReply) + 4 + 8;
	msgsize = CORBA::Object::NP_alignedSize(newDestination,msgsize);
	msgsize = msgsize - sizeof(MessageHeader::LocateReply) - 4;
	WrMessageSize(0);
	put_char_array((CORBA::Char*) MessageHeader::LocateReply,
		       sizeof(MessageHeader::LocateReply),
		       omni::ALIGN_1, 1, 1);
	operator>>= ((CORBA::ULong)msgsize,*this);
	operator>>= (pd_request_id,*this);
	operator>>= ((CORBA::ULong)status,*this);
	CORBA::Object::marshalObjRef(newDestination,*this);
	flush(1);
	pd_state = GIOP_S::Idle;
	WrUnlock();
      }
    }
    catch (...) {
    }
  }
  else {
    status = GIOP::UNKNOWN_OBJECT;
  }

  if (status != GIOP::OBJECT_FORWARD) {
    WrLock();
    pd_state = GIOP_S::ReplyIsBeingComposed;

    size_t bodysize = 8;
    WrMessageSize(0);
    put_char_array((CORBA::Char*) MessageHeader::LocateReply,
		   sizeof(MessageHeader::LocateReply),
		   omni::ALIGN_1, 1, 1);
    operator>>= ((CORBA::ULong)bodysize,*this);
    operator>>= (pd_request_id,*this);
    operator>>= ((CORBA::ULong)status,*this);

    flush(1);
    pd_state = GIOP_S::Idle;
    WrUnlock();
  }

  return;
}

void
GIOP_S::HandleCancelRequest(CORBA::Boolean byteorder)
{
  // XXX Not supported yet!!!
  // For the moment, silently ignore this message
  
  // Remove the rest of the message from the input as if they have been
  // processed.
  CORBA::ULong msgsize;
  msgsize <<= *this;
  if (msgsize > MaxMessageSize()) {
    SendMsgErrorMessage();
    setStrandIsDying();
    throw CORBA::COMM_FAILURE(0,CORBA::COMPLETED_NO);
  }

  RdMessageSize(msgsize,byteorder);    // set the size of the message body
  RequestReceived(1);
  pd_state = GIOP_S::Idle;
  return;
}

void
GIOP_S::HandleMessageError()
{
  // Hm... a previous message might have gone wrong
  // For the moment, just close down the connection
  HandleCloseConnection();
  return;
}

void
GIOP_S::HandleCloseConnection()
{
  setStrandIsDying();
  throw CORBA::COMM_FAILURE(0,CORBA::COMPLETED_NO);
}

void
GIOP_S::SendMsgErrorMessage()
{
  WrLock();
  WrMessageSize(0);
  put_char_array((CORBA::Char*) MessageHeader::MessageError,
		 sizeof(MessageHeader::MessageError),
		 omni::ALIGN_1, 1, 0);
  operator>>= ((CORBA::ULong)0,*this);
  flush(1);
  WrUnlock();
  return;
}

static
void 
MarshallSystemException(GIOP_S *s,
			const GIOP_Basetypes::_SysExceptRepoID &id,
			const CORBA::SystemException &ex)
{

  CORBA::ULong msgsize = GIOP_S::ReplyHeaderSize();
  CORBA::ULong exsize = id.len + 
    GIOP_Basetypes::SysExceptRepoID::versionLen + 1;
  msgsize =  omni::align_to(msgsize,omni::ALIGN_4);
  msgsize += sizeof(CORBA::ULong) + exsize;
  msgsize =  omni::align_to(msgsize,omni::ALIGN_4);
  msgsize += sizeof(CORBA::ULong) + sizeof(CORBA::ULong);
  
  s->InitialiseReply(GIOP::SYSTEM_EXCEPTION,msgsize);

  exsize >>= *s;
  s->put_char_array(id.id, id.len);
  s->put_char_array((CORBA::Char*)GIOP_Basetypes::SysExceptRepoID::version,
		    GIOP_Basetypes::SysExceptRepoID::versionLen + 1);
  ex.minor() >>= *s;
  operator>>= ((CORBA::ULong)ex.completed(),*s);
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
