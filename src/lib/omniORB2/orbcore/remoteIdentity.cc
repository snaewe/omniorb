// -*- Mode: C++; -*-
//                            Package   : omniORB
// remoteIdentity.cc          Created on: 16/6/99
//                            Author    : David Riddoch (djr)
//
//    Copyright (C) 1996-1999 AT&T Research Cambridge
//
//    This file is part of the omniORB library.
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

/*
  $Log$
  Revision 1.1.2.3  1999/10/14 16:22:16  djr
  Implemented logging when system exceptions are thrown.

  Revision 1.1.2.2  1999/09/28 10:54:35  djr
  Removed pretty-printing of object keys from object adapters.

  Revision 1.1.2.1  1999/09/22 14:27:06  djr
  Major rewrite of orbcore to support POA.

*/

#include <omniORB3/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <remoteIdentity.h>
#include <omniORB3/callDescriptor.h>
#include <dynamicLib.h>
#include <exception.h>


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

class omniRemoteIdentity_RefHolder {
public:
  inline omniRemoteIdentity_RefHolder(omniRemoteIdentity* id) : pd_id(id) {
    pd_id->pd_refCount++;
    omni::internalLock.unlock();
  }

  inline ~omniRemoteIdentity_RefHolder() {
    omni::internalLock.lock();
    int done = --pd_id->pd_refCount > 0;
    omni::internalLock.unlock();
    if( done )  return;
    delete pd_id;
  }

private:
  omniRemoteIdentity* pd_id;
};

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void
omniRemoteIdentity::dispatch(omniCallDescriptor& call_desc)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(omni::internalLock, 1);

  omniRemoteIdentity_RefHolder rh(this);

  if( omniORB::traceInvocations ) {
    omniORB::logger l;
    l << "Invoke '" << call_desc.op() << "' on remote: " << this << '\n';
  }

  CORBA::Boolean reuse = 0; //?? move this up into transport
  const omniCallDescriptor::ContextInfo* ctxt_info = call_desc.context_info();

  try {
    GIOP_C giop_c(rope());
    reuse = giop_c.isReUsingExistingConnection();

    // Calculate the size of the message.
    CORBA::ULong msgsize =
      GIOP_C::RequestHeaderSize(keysize(), call_desc.op_len());

    msgsize = call_desc.alignedSize(msgsize);
    if( ctxt_info )
      msgsize = omniDynamicLib::ops->context_aligned_size(msgsize,
						  ctxt_info->context,
						  ctxt_info->expected,
						  ctxt_info->num_expected);

    giop_c.InitialiseRequest(key(), keysize(), call_desc.op(),
			     call_desc.op_len(), msgsize,
			     call_desc.is_oneway());

    // Marshal the arguments to the operation.
    call_desc.marshalArguments(giop_c);
    if( ctxt_info )
      omniDynamicLib::ops->marshal_context(giop_c, ctxt_info->context,
					   ctxt_info->expected,
					   ctxt_info->num_expected);

    // Wait for the reply.

    switch( giop_c.ReceiveReply() ) {
    case GIOP::NO_EXCEPTION:
      // Unmarshal any result and out/inout arguments.
      call_desc.unmarshalReturnedValues(giop_c);
      giop_c.RequestCompleted();
      break;

    case GIOP::USER_EXCEPTION:
      {
	// Retrieve the Interface Repository ID of the exception.
	CORBA::ULong repoIdLen;
	repoIdLen <<= giop_c;
	if( repoIdLen > giop_c.RdMessageUnRead() )
	  OMNIORB_THROW(MARSHAL,0, CORBA::COMPLETED_MAYBE);
	CORBA::String_var repoId(omni::allocString(repoIdLen - 1));
	giop_c.get_char_array((CORBA::Char*)(char*) repoId, repoIdLen);

	call_desc.userException(giop_c, repoId);
	// Never get here - this must throw either a user exception
	// or CORBA::MARSHAL.
	OMNIORB_ASSERT(0);
      }

    case GIOP::LOCATION_FORWARD:
      {
	CORBA::Object_var obj(CORBA::Object::_unmarshalObjRef(giop_c));
	giop_c.RequestCompleted();
	throw omniORB::LOCATION_FORWARD(obj._retn());
      }

    case GIOP::SYSTEM_EXCEPTION:
      OMNIORB_ASSERT(0);
      break;

    }
  }
  catch(CORBA::COMM_FAILURE& ex) {
    if( reuse ){
      CORBA::TRANSIENT ex2(ex.minor(), ex.completed());
      throw ex2;
    }
    else throw;
  }
}


void
omniRemoteIdentity::gainObjRef(omniObjRef*)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(omni::internalLock, 1);

  pd_refCount++;
}


void
omniRemoteIdentity::loseObjRef(omniObjRef*)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(omni::internalLock, 1);

  if( --pd_refCount > 0 )  return;

  delete this;
}


void
omniRemoteIdentity::locateRequest()
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(omni::internalLock, 1);

  omniRemoteIdentity_RefHolder rh(this);

  if( omniORB::trace(10) ) {
    omniORB::logger l;
    l << "LocateRequest to remote: " << this << '\n';
  }

  CORBA::Boolean reuse = 0; //?? move this up into transport

  try {
    GIOP_C giop_c(rope());
    reuse = giop_c.isReUsingExistingConnection();

    CORBA::ULong msgsize = GIOP_C::RequestHeaderSize(keysize(), 14);

    switch( giop_c.IssueLocateRequest(key(), keysize()) ) {
    case GIOP::OBJECT_HERE:
      giop_c.RequestCompleted();
      break;

    case GIOP::UNKNOWN_OBJECT:
      giop_c.RequestCompleted();
      OMNIORB_THROW(OBJECT_NOT_EXIST,0,CORBA::COMPLETED_NO);
      break;        // dummy break

    case GIOP::OBJECT_FORWARD:
      {
	CORBA::Object_var obj(CORBA::Object::_unmarshalObjRef(giop_c));
	giop_c.RequestCompleted();
	throw omniORB::LOCATION_FORWARD(obj._retn());
      }

    }
  }
  catch(CORBA::COMM_FAILURE& ex) {
    if( reuse ){
      CORBA::TRANSIENT ex2(ex.minor(), ex.completed());
      throw ex2;
    }
    else throw;
  }
}


omniRemoteIdentity::~omniRemoteIdentity()
{
  omniORB::logs(15, "omniRemoteIdentity deleted.");
  pd_rope->decrRefCount();
}
