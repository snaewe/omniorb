// -*- Mode: C++; -*-
//                            Package   : omniORB
// callHandle.cc              Created on: 16/05/2001
//                            Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2001 AT&T Laboratories Cambridge
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
//
//   Call handle used during remote or in-process operation dispatch.

/*
 $Log$
 Revision 1.1.2.1  2001/05/29 17:03:50  dpg1
 In process identity.

*/

#include <omniORB4/CORBA.h>
#include <omniORB4/omniORB.h>
#include <omniORB4/callHandle.h>
#include <omniORB4/callDescriptor.h>
#include <omniORB4/omniServant.h>
#include <omniORB4/IOP_C.h>


static void
dealWithUserException(cdrMemoryStream& stream,
		      omniCallDescriptor& desc,
		      CORBA::UserException& ex);


void
omniCallHandle::upcall(omniServant* servant, omniCallDescriptor& desc)
{
  if (pd_iop_s) { // Remote call
    pd_iop_s->ReceiveRequest(desc);
  
    if (pd_upcall_hook)
      pd_upcall_hook->upcall(servant, desc);
    else
      desc.doLocalCall(servant);

    pd_iop_s->SendReply();
  }
  else { // In process call

    if (pd_call_desc == &desc) {
      // Fast case -- call descriptor can invoke directly on the servant
      if (pd_upcall_hook)
	pd_upcall_hook->upcall(servant, desc);
      else
	desc.doLocalCall(servant);
    }
    else {
      // Cannot call directly -- use a memory stream for now
      if (omniORB::traceInvocations) {
	omniORB::logger l;
	l << "In process indirect call '" << desc.op() << "'\n";
      }
      cdrMemoryStream stream;
      pd_call_desc->initialiseCall(stream);
      pd_call_desc->marshalArguments(stream);
      desc.unmarshalArguments(stream);

      try {
	if (pd_upcall_hook)
	  pd_upcall_hook->upcall(servant, desc);
	else
	  desc.doLocalCall(servant);
      }
#ifdef HAS_Cplusplus_catch_exception_by_base
      catch (CORBA::UserException& ex) {
	stream.rewindPtrs();
	dealWithUserException(stream, desc, ex);
	OMNIORB_ASSERT(0);
      }
#else
      catch (omniORB::StubUserException& uex) {
	try {
	  CORBA::UserException& ex = *((CORBA::UserException*)uex.ex());
	  stream.rewindPtrs();
	  dealWithUserException(stream, desc, ex);
	}
	catch (...) {
	  delete uex.ex();  // ?? Possible memory leak?
	  throw;
	}
	OMNIORB_ASSERT(0);
      }
#endif
      stream.rewindPtrs();
      desc.marshalReturnedValues(stream);
      pd_call_desc->unmarshalReturnedValues(stream);
    }
  }
}


OMNI_NAMESPACE_BEGIN(omni)
class inProcessIOP_C : public IOP_C {
public:
  inProcessIOP_C(cdrStream& stream) : pd_stream(stream) {}

  virtual cdrStream& getStream();
  virtual void InitialiseRequest();
  virtual GIOP::ReplyStatusType ReceiveReply();
  virtual void RequestCompleted(_CORBA_Boolean skip=0);
  virtual GIOP::LocateStatusType IssueLocateRequest();
private:
  cdrStream& pd_stream;
};

cdrStream& inProcessIOP_C::getStream() { return pd_stream; }

void inProcessIOP_C::RequestCompleted(_CORBA_Boolean skip) {}

void inProcessIOP_C::InitialiseRequest() {
  OMNIORB_ASSERT(0);
}
GIOP::ReplyStatusType inProcessIOP_C::ReceiveReply() {
  OMNIORB_ASSERT(0);
  return GIOP::NO_EXCEPTION; // To silence paranoid compilers
}
GIOP::LocateStatusType inProcessIOP_C::IssueLocateRequest() {
  OMNIORB_ASSERT(0);
  return GIOP::UNKNOWN_OBJECT; // To silence paranoid compilers
}
OMNI_NAMESPACE_END(omni)


static void
dealWithUserException(cdrMemoryStream& stream,
		      omniCallDescriptor& desc,
		      CORBA::UserException& ex)
{
  int size;
  const char* repoId = ex._NP_repoId(&size);

  if (omniORB::trace(25)) {
    omniORB::logger l;
    l << "Handling in-process user exception '" << repoId << "'\n";
  }

  ex._NP_marshal(stream);

  // omniCallDescriptor::userException() requires an IOP_C object as
  // its argument, so it can call RequestCompleted() on it. We use a
  // dummy IOP_C.
  _OMNI_NS(inProcessIOP_C) iop_c(stream);

  desc.userException(iop_c, repoId);

  // userException() _must_ throw an exception
  OMNIORB_ASSERT(0);
}

void 
omniCallHandle::SkipRequestBody()
{
  if (pd_iop_s)
    pd_iop_s->SkipRequestBody();
}
