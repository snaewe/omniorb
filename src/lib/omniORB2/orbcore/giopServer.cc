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
  Revision 1.22  2000/07/04 15:22:57  dpg1
  Merge from omni3_develop.

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

#include <omniORB3/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <scavenger.h>
#include <ropeFactory.h>
#include <objectAdapter.h>
#include <omniORB3/omniServant.h>
#include <localIdentity.h>
#include <initRefs.h>
#include <exceptiondefs.h>


size_t  GIOP_Basetypes::max_giop_message_size = 2048 * 1024;


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

  pd_user_exns = 0;
  pd_n_user_exns = 0;
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
	  omniORB::logs(15, "GIOP_S::RequestReceived: garbage left at the"
			" end of message.");
	  if (!omniORB::strictIIOP) {
	    skip(RdMessageUnRead(),1);
	  }
	  else {
	    setStrandIsDying();
	    OMNIORB_THROW_CONNECTION_BROKEN(0,CORBA::COMPLETED_NO);
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
    OMNIORB_THROW_CONNECTION_BROKEN(0,CORBA::COMPLETED_NO);
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
GIOP_S::InitialiseReply(const GIOP::ReplyStatusType status, size_t msgsize)
{
  if (!pd_response_expected)
    throw terminateProcessing();

  if (pd_state != GIOP_S::WaitingForReply)
    throw omniORB::fatalException(__FILE__,__LINE__,
				  "GIOP_S::InitialiseReply() entered with the wrong state.");

  size_t bodysize = msgsize-sizeof(MessageHeader::Reply)-sizeof(CORBA::ULong);
  if (bodysize > MaxMessageSize())
    OMNIORB_THROW(MARSHAL,0,CORBA::COMPLETED_YES);

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
	    OMNIORB_THROW_CONNECTION_BROKEN(0,CORBA::COMPLETED_MAYBE);
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
	    OMNIORB_THROW_CONNECTION_BROKEN(0,CORBA::COMPLETED_MAYBE);
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
	    OMNIORB_THROW_CONNECTION_BROKEN(0,CORBA::COMPLETED_MAYBE);
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
	    OMNIORB_THROW_CONNECTION_BROKEN(0,CORBA::COMPLETED_MAYBE);
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
	    OMNIORB_THROW_CONNECTION_BROKEN(0,CORBA::COMPLETED_MAYBE);
	  }
	gs.HandleCloseConnection();
	break;
      }
    default:
      {
	// Wrong header or invalid message type
	gs.SendMsgErrorMessage();
	gs.setStrandIsDying();
	OMNIORB_THROW_CONNECTION_BROKEN(0,CORBA::COMPLETED_NO);
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

  try {
    // This try block catches any exception that might raise during
    // the processing of the request header.

    msgsize <<= *this;
    if (msgsize > MaxMessageSize()) {
      SendMsgErrorMessage();
      setStrandIsDying();
      OMNIORB_THROW_CONNECTION_BROKEN(0,CORBA::COMPLETED_NO);
    }

    RdMessageSize(msgsize,byteorder);  // Set the size of the message body.

    // XXX Do not support any service context yet.
    // XXX For the moment, skips the service context.
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

    pd_key <<= *this;

    CORBA::ULong octetlen;
    octetlen <<= *this;
    if (octetlen > OMNIORB_GIOPDRIVER_GIOP_S_INLINE_BUF_SIZE)
      {
	// Do not blindly allocate a buffer for the operation string
	// a poison packet might set this to a huge value.
	if (octetlen > RdMessageUnRead()) {
	  OMNIORB_THROW(MARSHAL,0,CORBA::COMPLETED_NO);
	}
	CORBA::Octet* p = new CORBA::Octet[octetlen];
	if (!p)  OMNIORB_THROW(NO_MEMORY,0,CORBA::COMPLETED_NO);
	pd_operation = p;
      }
    get_char_array((CORBA::Octet*) pd_operation, octetlen);
    if( !octetlen || pd_operation[octetlen - 1] != '\0' )
      OMNIORB_THROW(MARSHAL,0, CORBA::COMPLETED_NO);

    octetlen <<= *this;
    if (octetlen > OMNIORB_GIOPDRIVER_GIOP_S_INLINE_BUF_SIZE)
      {
	// Do not blindly allocate a buffer for the principal octet vector
	// a poison packet might set this to a huge value
	if (octetlen > RdMessageUnRead()) {
	  OMNIORB_THROW(MARSHAL,0,CORBA::COMPLETED_NO);
	}
	CORBA::Octet* p = new CORBA::Octet[octetlen];
	if (!p)  OMNIORB_THROW(NO_MEMORY,0,CORBA::COMPLETED_NO);
	pd_principal = p;
      }
    get_char_array((CORBA::Octet*) pd_principal, octetlen);
  }
  catch (CORBA::MARSHAL& ex) {
    RequestReceived(1);
    SendMsgErrorMessage();
    pd_state = GIOP_S::Idle;
    return;
  }
  catch (CORBA::NO_MEMORY& ex) {
    RequestReceived(1);
    MarshallSystemException(this, SysExceptRepoID::NO_MEMORY, ex);
    return;
  }

  try {

    // Can we find the object in the local object table?

    CORBA::ULong hash = omni::hash(pd_key.key(), pd_key.size());
    omni::internalLock->lock();
    omniLocalIdentity* id;
    id = omni::locateIdentity(pd_key.key(), pd_key.size(), hash);

    if( id && id->servant() ) {
      id->dispatch(*this);
      return;
    }

    omni::internalLock->unlock();

    // Can we create a suitable object on demand?

    omniObjAdapter_var adapter(
	  omniObjAdapter::getAdapter(pd_key.key(), pd_key.size()));

    if( adapter ) {
      adapter->dispatch(*this, pd_key.key(), pd_key.size());
      return;
    }

    // Or is it the bootstrap agent?

    if( pd_key.size() == 4 && !memcmp(pd_key.key(), "INIT", 4) &&
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
    if (!strandIsDying() && (pd_state == RequestIsBeingProcessed ||
			     pd_state == WaitingForReply)) {
      if( pd_state == RequestIsBeingProcessed )
	RequestReceived(1);
      if( !pd_response_expected ) {
	SendMsgErrorMessage();
	ReplyCompleted();
      }
      else {
	int repoid_size;
	const char* repoid = ex._NP_repoId(&repoid_size);
	size_t msgsize = GIOP_S::ReplyHeaderSize();
	msgsize = omni::align_to(msgsize, omni::ALIGN_4) + 4 + repoid_size;
	msgsize = omni::align_to(msgsize, omni::ALIGN_4) + 8;

	InitialiseReply(GIOP::SYSTEM_EXCEPTION, msgsize);

	CORBA::ULong(repoid_size) >>= *this;
	put_char_array((const CORBA::Char*) repoid, repoid_size);
	ex.minor() >>= *this;
	CORBA::ULong(ex.completed()) >>= *this;
	ReplyCompleted();
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
		    operation());

    CORBA::Object_var release_it(ex.get_obj());

    if( !strandIsDying() && pd_response_expected &&
	(pd_state == RequestIsBeingProcessed ||
	 pd_state == WaitingForReply) ) {

      if( CORBA::is_nil(ex.get_obj()) ) {
	CORBA::UNKNOWN ex(0, CORBA::COMPLETED_MAYBE);
	CHECK_AND_MAYBE_MARSHALL_SYSTEM_EXCEPTION (UNKNOWN,ex);
      }
      else {
	if( pd_state == RequestIsBeingProcessed )  RequestReceived(1);
	// Marshal the location forward message.
	size_t msgsize = (size_t) GIOP_S::ReplyHeaderSize();
	msgsize = CORBA::Object::_NP_alignedSize(ex.get_obj(), msgsize);
	InitialiseReply(GIOP::LOCATION_FORWARD, CORBA::ULong(msgsize));
	CORBA::Object::_marshalObjRef(ex.get_obj(), *this);
	ReplyCompleted();
      }
    }
    else {
      CORBA::UNKNOWN ex(0, CORBA::COMPLETED_MAYBE);
      CHECK_AND_MAYBE_MARSHALL_SYSTEM_EXCEPTION (UNKNOWN,ex);
    }
  }

  catch(...) {
    if( omniORB::traceLevel > 1 ) {
      omniORB::logger l;
      l << "WARNING -- method \'" << operation() << "\' raised an unexpected\n"
	" exception (not a CORBA exception).\n";
    }
    CORBA::UNKNOWN ex(0, CORBA::COMPLETED_MAYBE);
    CHECK_AND_MAYBE_MARSHALL_SYSTEM_EXCEPTION (UNKNOWN,ex);
  }
}


void
GIOP_S::HandleLocateRequest(CORBA::Boolean byteorder)
{
  CORBA::ULong msgsize;

  try {
    // This try block catches any exception that might raise during
    // the processing of the request header.
    msgsize <<= *this;
    if (msgsize > MaxMessageSize()) {
      SendMsgErrorMessage();
      setStrandIsDying();
      OMNIORB_THROW_CONNECTION_BROKEN(0,CORBA::COMPLETED_NO);
    }

    RdMessageSize(msgsize, byteorder);  // Set the size of the message body.

    pd_request_id <<= *this;

    pd_key <<= *this;

    RequestReceived();
  }
  catch (CORBA::MARSHAL& ex) {
    RequestReceived(1);
    SendMsgErrorMessage();
    pd_state = GIOP_S::Idle;
    return;
  }
  catch (CORBA::NO_MEMORY& ex) {
    RequestReceived(1);
    MarshallSystemException(this,SysExceptRepoID::NO_MEMORY,ex);
    return;
  }

  omniORB::logs(10, "Handling a GIOP LOCATE_REQUEST.");

  GIOP::LocateStatusType status = GIOP::UNKNOWN_OBJECT;

  CORBA::ULong hash = omni::hash(pd_key.key(), pd_key.size());
  omni::internalLock->lock();
  omniLocalIdentity* id;
  id = omni::locateIdentity(pd_key.key(), pd_key.size(), hash);
  if( id && id->servant() )  status = GIOP::OBJECT_HERE;
  omni::internalLock->unlock();

  if( status == GIOP::UNKNOWN_OBJECT ) {

    // We attempt to find the object adapter (activate it if necassary)
    // and ask it if the object exists, or if it has the *capability*
    // to activate such an object.  ie. is it able to do object loading
    // on demand?

    omniObjAdapter_var adapter(
	  omniObjAdapter::getAdapter(pd_key.key(), pd_key.size()));

    if( adapter ) {
      try {
	if( adapter->objectExists(pd_key.key(), pd_key.size()) )
	  status = GIOP::OBJECT_HERE;
      }
      catch(omniORB::LOCATION_FORWARD& lf) {
	status = GIOP::OBJECT_FORWARD;
	WrLock();
	pd_state = ReplyIsBeingComposed;

	int msgsize = sizeof(MessageHeader::LocateReply) + 4 + 8;
	msgsize = CORBA::Object::_NP_alignedSize(lf.get_obj(), msgsize);
	msgsize = msgsize - sizeof(MessageHeader::LocateReply) - 4;
	WrMessageSize(0);
	put_char_array((CORBA::Char*) MessageHeader::LocateReply,
		       sizeof(MessageHeader::LocateReply),
		       omni::ALIGN_1, 1, 1);
	CORBA::ULong(msgsize) >>= *this;
	pd_request_id         >>= *this;
	CORBA::ULong(status)  >>= *this;
	CORBA::Object::_marshalObjRef(lf.get_obj(), *this);
	flush(1);
	pd_state = GIOP_S::Idle;
	WrUnlock();
      }
      catch(...) {
	// status == GIOP::UNKNOWN_OBJECT
      }
    }
    else if( pd_key.size() == 4 && !memcmp(pd_key.key(), "INIT", 4) &&
	     omniInitialReferences::is_bootstrap_agentImpl_initialised() )
      status = GIOP::OBJECT_HERE;
  }

  if( status != GIOP::OBJECT_FORWARD ) {
    WrLock();
    pd_state = GIOP_S::ReplyIsBeingComposed;

    size_t bodysize = 8;
    WrMessageSize(0);
    put_char_array((CORBA::Char*) MessageHeader::LocateReply,
		   sizeof(MessageHeader::LocateReply),
		   omni::ALIGN_1, 1, 1);
    CORBA::ULong(bodysize) >>= *this;
    pd_request_id          >>= *this;
    CORBA::ULong(status)   >>= *this;

    flush(1);
    pd_state = GIOP_S::Idle;
    WrUnlock();
  }
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
    OMNIORB_THROW_CONNECTION_BROKEN(0,CORBA::COMPLETED_NO);
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
  OMNIORB_THROW_CONNECTION_BROKEN(0,CORBA::COMPLETED_NO);
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


void
GIOP_S::MaybeMarshalUserException(void* pex)
{
  CORBA::UserException& ex = * (CORBA::UserException*) pex;

  int i, repoid_size;
  const char* repoid = ex._NP_repoId(&repoid_size);

  // Could turn this into a binary search (list is sorted).
  // Usually a short list though -- probably not worth it.
  for( i = 0; i < pd_n_user_exns; i++ )
    if( !strcmp(pd_user_exns[i], repoid) ) {
      size_t msgsize = ReplyHeaderSize();
      msgsize = omni::align_to(msgsize, omni::ALIGN_4) + 4 + repoid_size;
      msgsize = ex._NP_alignedSize(msgsize);
      InitialiseReply(GIOP::USER_EXCEPTION, msgsize);
      CORBA::ULong(repoid_size) >>= *this;
      put_char_array((const CORBA::Char*) repoid, repoid_size);
      ex._NP_marshal(*this);
      ReplyCompleted();
      break;
    }

  if( i == pd_n_user_exns ) {
    if( omniORB::trace(1) ) {
      omniORB::logger l;
      l << "WARNING -- method \'" << operation() << "\' on: " << pd_key
	<< "\n raised the exception: " << repoid << '\n';
    }
    CORBA::UNKNOWN ex(0, CORBA::COMPLETED_MAYBE);
    CHECK_AND_MAYBE_MARSHALL_SYSTEM_EXCEPTION (UNKNOWN, ex);
  }
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
