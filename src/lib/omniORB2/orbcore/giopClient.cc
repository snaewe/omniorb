// -*- Mode: C++; -*-
//                            Package   : omniORB2
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
  Revision 1.12.4.1  1999/09/15 20:18:31  sll
  Updated to use the new cdrStream abstraction.
  Marshalling operators for NetBufferedStream and MemBufferedStream are now
  replaced with just one version for cdrStream.
  Derived class giopStream implements the cdrStream abstraction over a
  network connection whereas the cdrMemoryStream implements the abstraction
  with in memory buffer.

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

#include <omniORB2/CORBA.h>
#include <giopObjectInfo.h>

GIOP_C::GIOP_C(GIOPObjectInfo* f) : pd_invokeInfo(f)
{
  pd_state = GIOP_C::Idle;
  pd_cdrStream = giopStream::acquire(f->rope(),f->giopVersion());
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
    pd_cdrStream->setTerminalError();
  }
  pd_state = GIOP_C::Zombie;
  pd_cdrStream->release();
  pd_invokeInfo->release();
}

void 
GIOP_C::InitialiseRequest(const char     *opname,
			  size_t         opnamesize,
			  CORBA::Boolean oneway,
			  CORBA::Boolean response_expected)
{
  if (pd_state != GIOP_C::Idle)
    throw omniORB::fatalException(__FILE__,__LINE__,
      "GIOP_C::InitialiseRequest() entered with the wrong state.");				  
  pd_state = GIOP_C::RequestInProgress;

  pd_response_expected = response_expected;

  pd_request_id = pd_cdrStream->outputRequestMessageBegin(pd_invokeInfo,
							  opname,
							  opnamesize,
							  oneway,
							  response_expected);
}

GIOP::ReplyStatusType 
GIOP_C::ReceiveReply()
{
  if (pd_state != GIOP_C::RequestInProgress)
    throw omniORB::fatalException(__FILE__,__LINE__,
      "GIOP_C::ReceiveReply() entered with the wrong state.");

  pd_cdrStream->outputMessageEnd();
  pd_state = GIOP_C::WaitingForReply;

  if (!pd_response_expected) {
    pd_state = GIOP_C::ReplyIsBeingProcessed;
    return GIOP::NO_EXCEPTION;
  }
  
  GIOP::ReplyStatusType rc;

  rc = pd_cdrStream->inputReplyMessageBegin(pd_request_id);

  pd_state = GIOP_C::ReplyIsBeingProcessed;


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
    pd_cdrStream->setTerminalError();
    throw CORBA::COMM_FAILURE(0,CORBA::COMPLETED_MAYBE);
  }
  return rc;
}

void
GIOP_C::RequestCompleted(CORBA::Boolean skip_msg)
{
  if (pd_state != GIOP_C::ReplyIsBeingProcessed)
    throw omniORB::fatalException(__FILE__,__LINE__,
      "GIOP_C::RequestCompleted() entered with the wrong state.");

  pd_cdrStream->inputMessageEnd(skip_msg);
  pd_state = GIOP_C::Idle;
}

GIOP::LocateStatusType
GIOP_C::IssueLocateRequest()
{
  if (pd_state != GIOP_C::Idle)
    throw omniORB::fatalException(__FILE__,__LINE__,
      "GIOP_C::IssueLocateRequest() entered with the wrong state.");

  pd_state = GIOP_C::RequestInProgress;

  pd_request_id = pd_cdrStream->outputLocateMessageBegin(pd_invokeInfo);

  pd_cdrStream->outputMessageEnd();

  pd_state = GIOP_C::WaitingForReply;

  GIOP::LocateStatusType rc;

  rc = pd_cdrStream->inputLocateReplyMessageBegin(pd_request_id);

  pd_state = GIOP_C::ReplyIsBeingProcessed;


  switch (rc) {
  case GIOP::UNKNOWN_OBJECT:
  case GIOP::OBJECT_HERE:
  case GIOP::OBJECT_FORWARD:
  case GIOP::OBJECT_FORWARD_PERM:
    break;
  case GIOP::LOC_SYSTEM_EXCEPTION:
    UnMarshallSystemException();
    break;
  case GIOP::LOC_NEEDS_ADDRESSING_MODE:
    RequestCompleted();
    throw CORBA::TRANSIENT(0,CORBA::COMPLETED_NO);
    break; // redundent.
  default:
    // Should never receive anything other that the above
    // Same treatment as wrong header
    pd_cdrStream->setTerminalError();
    throw CORBA::COMM_FAILURE(0,CORBA::COMPLETED_MAYBE);
  }
  return rc;
}



void
GIOP_C::UnMarshallSystemException()
{

#define CHECK_AND_IF_MATCH_THROW_SYSTEM_EXCEPTION(_ex) \
  if (strncmp((const char *)repoid,(const char *) \
	      GIOP_Basetypes::SysExceptRepoID:: _ex .id, \
	      GIOP_Basetypes::SysExceptRepoID:: _ex .len)==0) \
    { \
      CORBA:: _ex ex(m,(CORBA::CompletionStatus)s); \
      throw ex; \
    }

  // Real code begins here
  CORBA::ULong len;
  len <<= *pd_cdrStream;
  if (len > omniORB_GIOP_Basetypes_SysExceptRepoID_maxIDLen)
    throw CORBA::UNKNOWN(0,CORBA::COMPLETED_MAYBE);

  CORBA::Char repoid[omniORB_GIOP_Basetypes_SysExceptRepoID_maxIDLen];

  pd_cdrStream->get_char_array(repoid, len);
  CORBA::ULong m;
  CORBA::ULong s;
  m <<= *pd_cdrStream;
  s <<= *pd_cdrStream;
  RequestCompleted();
  switch (s) {
  case CORBA::COMPLETED_YES:
  case CORBA::COMPLETED_NO:
  case CORBA::COMPLETED_MAYBE:
    break;
  default:
    throw CORBA::UNKNOWN(0,CORBA::COMPLETED_MAYBE);
  };

  CHECK_AND_IF_MATCH_THROW_SYSTEM_EXCEPTION (UNKNOWN);
  CHECK_AND_IF_MATCH_THROW_SYSTEM_EXCEPTION (BAD_PARAM);
  CHECK_AND_IF_MATCH_THROW_SYSTEM_EXCEPTION (NO_MEMORY);
  CHECK_AND_IF_MATCH_THROW_SYSTEM_EXCEPTION (IMP_LIMIT);
  CHECK_AND_IF_MATCH_THROW_SYSTEM_EXCEPTION (COMM_FAILURE);
  CHECK_AND_IF_MATCH_THROW_SYSTEM_EXCEPTION (INV_OBJREF);
  CHECK_AND_IF_MATCH_THROW_SYSTEM_EXCEPTION (OBJECT_NOT_EXIST);
  CHECK_AND_IF_MATCH_THROW_SYSTEM_EXCEPTION (NO_PERMISSION);
  CHECK_AND_IF_MATCH_THROW_SYSTEM_EXCEPTION (INTERNAL);
  CHECK_AND_IF_MATCH_THROW_SYSTEM_EXCEPTION (MARSHAL);
  CHECK_AND_IF_MATCH_THROW_SYSTEM_EXCEPTION (INITIALIZE);
  CHECK_AND_IF_MATCH_THROW_SYSTEM_EXCEPTION (NO_IMPLEMENT);
  CHECK_AND_IF_MATCH_THROW_SYSTEM_EXCEPTION (BAD_TYPECODE);
  CHECK_AND_IF_MATCH_THROW_SYSTEM_EXCEPTION (BAD_OPERATION);
  CHECK_AND_IF_MATCH_THROW_SYSTEM_EXCEPTION (NO_RESOURCES);
  CHECK_AND_IF_MATCH_THROW_SYSTEM_EXCEPTION (NO_RESPONSE);
  CHECK_AND_IF_MATCH_THROW_SYSTEM_EXCEPTION (PERSIST_STORE);
  CHECK_AND_IF_MATCH_THROW_SYSTEM_EXCEPTION (BAD_INV_ORDER);
  CHECK_AND_IF_MATCH_THROW_SYSTEM_EXCEPTION (TRANSIENT);
  CHECK_AND_IF_MATCH_THROW_SYSTEM_EXCEPTION (FREE_MEM);
  CHECK_AND_IF_MATCH_THROW_SYSTEM_EXCEPTION (INV_IDENT);
  CHECK_AND_IF_MATCH_THROW_SYSTEM_EXCEPTION (INV_FLAG);
  CHECK_AND_IF_MATCH_THROW_SYSTEM_EXCEPTION (INTF_REPOS);
  CHECK_AND_IF_MATCH_THROW_SYSTEM_EXCEPTION (BAD_CONTEXT);
  CHECK_AND_IF_MATCH_THROW_SYSTEM_EXCEPTION (OBJ_ADAPTER);
  CHECK_AND_IF_MATCH_THROW_SYSTEM_EXCEPTION (DATA_CONVERSION);
  CHECK_AND_IF_MATCH_THROW_SYSTEM_EXCEPTION (TRANSACTION_REQUIRED);
  CHECK_AND_IF_MATCH_THROW_SYSTEM_EXCEPTION (TRANSACTION_ROLLEDBACK);
  CHECK_AND_IF_MATCH_THROW_SYSTEM_EXCEPTION (INVALID_TRANSACTION);
  CHECK_AND_IF_MATCH_THROW_SYSTEM_EXCEPTION (WRONG_TRANSACTION);

  // If none of the above matched
  throw CORBA::UNKNOWN(0,CORBA::COMPLETED_MAYBE);

#undef CHECK_AND_IF_MATCH_THROW_SYSTEM_EXCEPTION
}

