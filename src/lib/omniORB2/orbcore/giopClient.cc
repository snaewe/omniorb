// -*- Mode: C++; -*-
//                            Package   : omniORB
// giopClient.cc              Created on: 26/3/96
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
//      Client side GIOP
//      
 
/*
  $Log$
  Revision 1.14  2000/07/13 15:25:57  dpg1
  Merge from omni3_develop for 3.0 release.

  Revision 1.12.6.4  2000/06/22 10:37:50  dpg1
  Transport code now throws omniConnectionBroken exception rather than
  CORBA::COMM_FAILURE when things go wrong. This allows the invocation
  code to distinguish between transport problems and COMM_FAILURES
  propagated from the server side.

  exception.h renamed to exceptiondefs.h to avoid name clash on some
  platforms.

  Revision 1.12.6.3  1999/10/18 11:27:39  djr
  Centralised list of system exceptions.

  Revision 1.12.6.2  1999/10/14 16:22:09  djr
  Implemented logging when system exceptions are thrown.

  Revision 1.12.6.1  1999/09/22 14:26:49  djr
  Major rewrite of orbcore to support POA.

  Revision 1.12  1999/08/30 16:50:17  sll
  Added call to Strand::Sync::clicksSet to control how long a call is
  allowed to progress or how long an idle connection is to stay open.

  Revision 1.11  1999/03/11 16:25:53  djr
  Updated copyright notice

  Revision 1.10  1999/01/07 15:51:17  djr
  Changes needed due to changes to Net/MemBufferedStream.

  Revision 1.9  1998/04/07 19:34:11  sll
  Replace cerr with omniORB::log.

  Revision 1.8  1997/12/09 17:37:15  sll
  Updated to use the new rope and strand interface.
  Now unmarshal context properly.

  Revision 1.7  1997/08/21 21:56:40  sll
  Added system exception TRANSACTION_REQUIRED, TRANSACTION_ROLLEDBACK,
  INVALID_TRANSACTION, WRONG_TRANSACTION.

// Revision 1.6  1997/05/06  15:19:49  sll
// Public release.
//
  */

#include <omniORB3/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <scavenger.h>
#include <exceptiondefs.h>


GIOP_C::GIOP_C(Rope *r)
  : NetBufferedStream(r,1,1,0)
{
  pd_state = GIOP_C::Idle;
  pd_max_message_size = NetBufferedStream::MaxMessageSize() - 
                           sizeof(MessageHeader::HeaderType) -
                           sizeof(CORBA::ULong);
  if (pd_max_message_size > omniORB::MaxMessageSize()) {
    pd_max_message_size = omniORB::MaxMessageSize();
  }
  return;
}


GIOP_C::~GIOP_C()
{
  if (pd_state == GIOP_C::Zombie) {
    if (omniORB::traceLevel >= 15) {
      omniORB::log << "GIOP_C dtor re-entered.\n";
      omniORB::log.flush();
    }
    return;
  }
  if (pd_state != GIOP_C::Idle) {
    setStrandIsDying();
  }
  pd_state = GIOP_C::Zombie;
  return;
}

size_t
GIOP_C::RequestHeaderSize(const size_t objkeysize, const size_t opnamesize)
{
  // Compute the header size, this includes the GIOP Message header and
  // the Request message header.

  CORBA::ULong msgsize = sizeof(MessageHeader::Request) + sizeof(CORBA::ULong);

  // RequestHeader.service_context
  msgsize += sizeof(CORBA::ULong); // XXX 0 length service context

  // RequestHeader.request_id
  // msgsize = omni::align_to(msgsize,omni::ALIGN_4);
  msgsize += sizeof(CORBA::ULong);

  // RequestHeader.response_expected
  // msgsize = omni::align_to(msgsize,omni::ALIGN_1);
  msgsize += sizeof(CORBA::Boolean);

  // RequestHeader.object_key
  msgsize = omni::align_to(msgsize,omni::ALIGN_4);
  msgsize += sizeof(CORBA::ULong) + objkeysize;
  
  // RequestHeader.operation
  msgsize = omni::align_to(msgsize,omni::ALIGN_4);
  msgsize += sizeof(CORBA::ULong) + opnamesize;

  // RequestHeader.requesting_principal
  msgsize = omni::align_to(msgsize,omni::ALIGN_4);
  msgsize += sizeof(CORBA::ULong) + omni::myPrincipalID.length();

  return msgsize;
}

void 
GIOP_C::InitialiseRequest(const void          *objkey,
			  const size_t         objkeysize,
			  const char          *opname,
			  const size_t         opnamesize,
			  const size_t         msgsize,
			  const CORBA::Boolean oneway)
{
  if (pd_state != GIOP_C::Idle)
    throw omniORB::fatalException(__FILE__,__LINE__,
      "GIOP_C::InitialiseRequest() entered with the wrong state.");
 
  clicksSet(StrandScavenger::clientCallTimeLimit());

  size_t bodysize =msgsize-sizeof(MessageHeader::Request)-sizeof(CORBA::ULong);
  if (bodysize > MaxMessageSize()) {
    OMNIORB_THROW(MARSHAL,0,CORBA::COMPLETED_NO);
  }

  pd_state = GIOP_C::RequestInProgress;
  pd_request_id = newRequestID();


  // Marshall the GIOP Message Header
  WrMessageSize(msgsize);
  put_char_array((CORBA::Char *)MessageHeader::Request,
		 sizeof(MessageHeader::Request),
		 omni::ALIGN_1, 1, !oneway);

  operator>>= ((CORBA::ULong)bodysize,*this);

  // Marshall the Request Header
  // XXX should marshall service context here,
  // XXX for the moment, just marshall a 0 length service context
  operator>>= ((CORBA::ULong)0,*this);

  operator>>= (pd_request_id,*this);

  if (oneway)
    pd_response_expected = 0;
  else
    pd_response_expected = 1;
  operator>>= (pd_response_expected,*this);

  operator>>= ((CORBA::ULong) objkeysize,*this);
  put_char_array((CORBA::Char*) objkey, objkeysize);

  operator>>= ((CORBA::ULong) opnamesize,*this);

  put_char_array((CORBA::Char*) opname, opnamesize);

  CORBA::ULong a = omni::myPrincipalID.length();
  operator>>= (a,*this);

  put_char_array((CORBA::Char*) omni::myPrincipalID.NP_data(),
		 omni::myPrincipalID.length());
}

GIOP::ReplyStatusType 
GIOP_C::ReceiveReply()
{
  if (pd_state != GIOP_C::RequestInProgress)
    throw omniORB::fatalException(__FILE__,__LINE__,
      "GIOP_C::ReceiveReply() entered with the wrong state.");

  if (WrMessageSpaceLeft())
    throw omniORB::fatalException(__FILE__,__LINE__,
       "GIOP_C::ReceiveReply() reported wrong request message size.");

  pd_state = GIOP_C::WaitingForReply;
  flush(1);

 if (!pd_response_expected) {
    pd_state = GIOP_C::ReplyIsBeingProcessed;
    return GIOP::NO_EXCEPTION;
  }
  
  RdMessageSize(0,omni::myByteOrder);

  MessageHeader::HeaderType hdr;
  get_char_array((CORBA::Char*) hdr, sizeof(MessageHeader::HeaderType),
		 omni::ALIGN_1, 1);

  pd_state = GIOP_C::ReplyIsBeingProcessed;

  if (hdr[0] != MessageHeader::Reply[0] ||
      hdr[1] != MessageHeader::Reply[1] ||
      hdr[2] != MessageHeader::Reply[2] ||
      hdr[3] != MessageHeader::Reply[3] ||
      hdr[4] != MessageHeader::Reply[4] ||
      hdr[5] != MessageHeader::Reply[5] ||
      hdr[7] != MessageHeader::Reply[7])
    {
      // Wrong header
      setStrandIsDying();
      OMNIORB_THROW_CONNECTION_BROKEN(0,CORBA::COMPLETED_MAYBE);
    }

  CORBA::ULong msgsize;
  msgsize <<= *this;
  if (hdr[6] != omni::myByteOrder) {
    msgsize =  ((((msgsize) & 0xff000000) >> 24) | 
		(((msgsize) & 0x00ff0000) >> 8)  | 
		(((msgsize) & 0x0000ff00) << 8)  | 
		(((msgsize) & 0x000000ff) << 24));
  }

  if (msgsize > MaxMessageSize()) {
    // message size has exceeded the limit
    setStrandIsDying();
    OMNIORB_THROW_CONNECTION_BROKEN(0,CORBA::COMPLETED_MAYBE);
  }

  RdMessageSize(msgsize,hdr[6]);

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

  CORBA::ULong req_id;
  req_id <<= *this;
  if (req_id != pd_request_id) {
    // Not the expected reply, skip the entire message
    skip(RdMessageUnRead(),1);
    pd_state = GIOP_C::RequestInProgress;
    return this->ReceiveReply();
  }

  CORBA::ULong rc;
  rc <<= *this;
  switch (rc) {
  case GIOP::SYSTEM_EXCEPTION:
    {
      UnMarshallSystemException();
      // never reaches here
    }
    break;
  case GIOP::NO_EXCEPTION:
  case GIOP::USER_EXCEPTION:
  case GIOP::LOCATION_FORWARD:
    break;
  default:
    // Should never receive anything other that the above
    // Same treatment as wrong header
    setStrandIsDying();
    OMNIORB_THROW_CONNECTION_BROKEN(0,CORBA::COMPLETED_MAYBE);
  }
  return (GIOP::ReplyStatusType)rc;
}

void
GIOP_C::RequestCompleted(CORBA::Boolean skip_msg)
{
  if (pd_state != GIOP_C::ReplyIsBeingProcessed)
    throw omniORB::fatalException(__FILE__,__LINE__,
      "GIOP_C::RequestCompleted() entered with the wrong state.");
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
	    omniORB::log << "GIOP_C::RequestCompleted: garbage left at the end of message.\n";
	    omniORB::log.flush();
	  }
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
  clicksSet(StrandScavenger::outIdleTimeLimit());
  pd_state = GIOP_C::Idle;
  return;
}

GIOP::LocateStatusType
GIOP_C::IssueLocateRequest(const void   *objkey,
			   const size_t  objkeysize)
{
  if (pd_state != GIOP_C::Idle)
    throw omniORB::fatalException(__FILE__,__LINE__,
      "GIOP_C::IssueLocateRequest() entered with the wrong state.");

  clicksSet(StrandScavenger::clientCallTimeLimit());

  CORBA::ULong msgsize = sizeof(MessageHeader::LocateRequest) + 
                         sizeof(CORBA::ULong);
  CORBA::ULong bodysize = omni::align_to(msgsize,omni::ALIGN_4) - msgsize;

  bodysize += sizeof (CORBA::ULong) + sizeof(CORBA::ULong) + objkeysize;
  msgsize += bodysize;

  pd_state = GIOP_C::RequestInProgress;
  pd_request_id = newRequestID();

  WrMessageSize(msgsize);
  put_char_array((CORBA::Char*) MessageHeader::LocateRequest,
		 sizeof(MessageHeader::LocateRequest),
		 omni::ALIGN_1, 1, 1);
  operator>>= ((CORBA::ULong)bodysize,*this);

  operator>>= (pd_request_id,*this);

  operator>>= ((CORBA::ULong) objkeysize,*this);
  put_char_array((CORBA::Char*) objkey, objkeysize);

  pd_state = GIOP_C::WaitingForReply;
  flush(1);

  do {
    RdMessageSize(0,omni::myByteOrder);

    MessageHeader::HeaderType hdr;
    get_char_array((CORBA::Char*)hdr, sizeof(MessageHeader::HeaderType),
		   omni::ALIGN_1, 1);

    pd_state = GIOP_C::ReplyIsBeingProcessed;

    if (hdr[0] != MessageHeader::LocateReply[0] ||
	hdr[1] != MessageHeader::LocateReply[1] ||
	hdr[2] != MessageHeader::LocateReply[2] ||
	hdr[3] != MessageHeader::LocateReply[3] ||
	hdr[4] != MessageHeader::LocateReply[4] ||
	hdr[5] != MessageHeader::LocateReply[5] ||
	hdr[7] != MessageHeader::LocateReply[7])
      {
	// Wrong header
	setStrandIsDying();
	OMNIORB_THROW_CONNECTION_BROKEN(0,CORBA::COMPLETED_MAYBE);
      }

    CORBA::ULong msgsize;
    msgsize <<= *this;
    if (hdr[6] != omni::myByteOrder) {
      msgsize =  ((((msgsize) & 0xff000000) >> 24) | 
		  (((msgsize) & 0x00ff0000) >> 8)  | 
		  (((msgsize) & 0x0000ff00) << 8)  | 
		  (((msgsize) & 0x000000ff) << 24));
    }

    if (msgsize > MaxMessageSize()) {
      // message size has exceeded the limit
      setStrandIsDying();
      OMNIORB_THROW_CONNECTION_BROKEN(0,CORBA::COMPLETED_MAYBE);
    }

    RdMessageSize(msgsize,hdr[6]);

    CORBA::ULong req_id;
    req_id <<= *this;
    if (req_id != pd_request_id) {
      // Not the expected reply, skip the entire message
      skip(RdMessageUnRead(),1);
      pd_state = GIOP_C::RequestInProgress;
      continue;
    }
  } while (0);
  
  CORBA::ULong rc;
  rc <<= *this;

  switch (rc) {
  case GIOP::UNKNOWN_OBJECT:
  case GIOP::OBJECT_HERE:
  case GIOP::OBJECT_FORWARD:
    break;
  default:
    // Should never receive anything other that the above
    // Same treatment as wrong header
    setStrandIsDying();
    OMNIORB_THROW_CONNECTION_BROKEN(0,CORBA::COMPLETED_MAYBE);
  }
  return (GIOP::LocateStatusType)rc;
}



void
GIOP_C::UnMarshallSystemException()
{

#define CHECK_AND_IF_MATCH_THROW_SYSTEM_EXCEPTION(_ex) \
  if (strncmp((const char *)repoid,(const char *) \
	      GIOP_Basetypes::SysExceptRepoID:: _ex .id, \
	      GIOP_Basetypes::SysExceptRepoID:: _ex .len)==0) \
    OMNIORB_THROW(_ex, m, (CORBA::CompletionStatus) s);

  // Real code begins here
  CORBA::ULong len;
  len <<= *this;
  if (len > omniORB_GIOP_Basetypes_SysExceptRepoID_maxIDLen)
    OMNIORB_THROW(UNKNOWN,0,CORBA::COMPLETED_MAYBE);

  CORBA::Char repoid[omniORB_GIOP_Basetypes_SysExceptRepoID_maxIDLen];

  get_char_array(repoid, len);
  CORBA::ULong m;
  CORBA::ULong s;
  m <<= *this;
  s <<= *this;
  RequestCompleted();
  switch (s) {
  case CORBA::COMPLETED_YES:
  case CORBA::COMPLETED_NO:
  case CORBA::COMPLETED_MAYBE:
    break;
  default:
    OMNIORB_THROW(UNKNOWN,0,CORBA::COMPLETED_MAYBE);
  };

  OMNIORB_FOR_EACH_SYS_EXCEPTION(CHECK_AND_IF_MATCH_THROW_SYSTEM_EXCEPTION)

  // If none of the above matched
  OMNIORB_THROW(UNKNOWN,0,CORBA::COMPLETED_MAYBE);

#undef CHECK_AND_IF_MATCH_THROW_SYSTEM_EXCEPTION
}

