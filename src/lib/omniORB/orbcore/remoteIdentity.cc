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
  Revision 1.2.2.2  2000/09/27 18:05:51  sll
  Use the new giop engine APIs to drive a remote call.

  Revision 1.2.2.1  2000/07/17 10:35:58  sll
  Merged from omni3_develop the diff between omni3_0_0_pre3 and omni3_0_0.

  Revision 1.3  2000/07/13 15:25:55  dpg1
  Merge from omni3_develop for 3.0 release.

  Revision 1.1.2.5  2000/06/22 10:37:50  dpg1
  Transport code now throws omniConnectionBroken exception rather than
  CORBA::COMM_FAILURE when things go wrong. This allows the invocation
  code to distinguish between transport problems and COMM_FAILURES
  propagated from the server side.

  exception.h renamed to exceptiondefs.h to avoid name clash on some
  platforms.

  Revision 1.1.2.4  1999/10/27 17:32:16  djr
  omni::internalLock and objref_rc_lock are now pointers.

  Revision 1.1.2.3  1999/10/14 16:22:16  djr
  Implemented logging when system exceptions are thrown.

  Revision 1.1.2.2  1999/09/28 10:54:35  djr
  Removed pretty-printing of object keys from object adapters.

  Revision 1.1.2.1  1999/09/22 14:27:06  djr
  Major rewrite of orbcore to support POA.

*/

#include <omniORB4/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <remoteIdentity.h>
#include <omniORB4/callDescriptor.h>
#include <dynamicLib.h>
#include <exceptiondefs.h>


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

class omniRemoteIdentity_RefHolder {
public:
  inline omniRemoteIdentity_RefHolder(omniRemoteIdentity* id) : pd_id(id) {
    pd_id->pd_refCount++;
    omni::internalLock->unlock();
  }

  inline ~omniRemoteIdentity_RefHolder() {
    omni::internalLock->lock();
    int done = --pd_id->pd_refCount > 0;
    omni::internalLock->unlock();
    if( done )  return;
    delete pd_id;
  }

private:
  omniRemoteIdentity* pd_id;
};

//////////////////////////////////////////////////////////////////////
//////////////////////// Call Marshaller      ////////////////////////
//////////////////////////////////////////////////////////////////////

class RemoteIdentityCallArgumentsMarshaller : public giopMarshaller {
public:
  RemoteIdentityCallArgumentsMarshaller(cdrStream& s,
					omniCallDescriptor& desc) : 
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
  cdrStream&     pd_s;
  omniCallDescriptor& pd_desc;
};

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void
omniRemoteIdentity::dispatch(omniCallDescriptor& call_desc)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);

  omniRemoteIdentity_RefHolder rh(this);

  if( omniORB::traceInvocations ) {
    omniORB::logger l;
    l << "Invoke '" << call_desc.op() << "' on remote: " << this << '\n';
  }

  GIOP_C giop_client(pd_ior,pd_rope);
  cdrStream& s = (cdrStream&)giop_client;

 again:
  call_desc.initialiseCall(s);

  omniClientCallMarshaller m(call_desc);

  giop_client.InitialiseRequest(call_desc.op(),
				call_desc.op_len(),
				call_desc.is_oneway(),
				!call_desc.is_oneway(),
				m);

  // Wait for the reply.
  GIOP::ReplyStatusType rc;

  switch( (rc = giop_client.ReceiveReply()) ) {
  case GIOP::NO_EXCEPTION:
    // Unmarshal any result and out/inout arguments.
    call_desc.unmarshalReturnedValues(s);
    giop_client.RequestCompleted();
    break;

  case GIOP::USER_EXCEPTION:
    {
      // Retrieve the Interface Repository ID of the exception.
      CORBA::ULong repoIdLen;
      repoIdLen <<= s;
      if (!s.checkInputOverrun(1,repoIdLen))
	OMNIORB_THROW(MARSHAL,0, CORBA::COMPLETED_MAYBE);
      CORBA::String_var repoId(_CORBA_String_helper::alloc(repoIdLen - 1));
      s.get_char_array((CORBA::Char*)(char*) repoId, repoIdLen);

      call_desc.userException(giop_client, repoId);
      // Never get here - this must throw either a user exception
      // or CORBA::MARSHAL.
      OMNIORB_ASSERT(0);
    }

  case GIOP::LOCATION_FORWARD:
  case GIOP::LOCATION_FORWARD_PERM:
    {
      CORBA::Object_var obj(CORBA::Object::_unmarshalObjRef(s));
      giop_client.RequestCompleted();
      throw omniORB::LOCATION_FORWARD(obj._retn(),
			       (rc == GIOP::LOCATION_FORWARD_PERM) ? 0 : 1);
    }

  case GIOP::NEEDS_ADDRESSING_MODE:
    {
      GIOP::AddressingDisposition v;
      v <<= s;
      pd_ior->addr_mode = v;
      giop_client.RequestCompleted();
      if (omniORB::trace(10)) {
	omniORB::logger log;
	log << "Remote invocation: GIOP::NEEDS_ADDRESSING_MODE: "
	    << (int) v << " retry request.\n";
      }
      goto again;
    }

  case GIOP::SYSTEM_EXCEPTION:
    OMNIORB_ASSERT(0);
    break;

  }
}


void
omniRemoteIdentity::gainObjRef(omniObjRef*)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);

  pd_refCount++;
}


void
omniRemoteIdentity::loseObjRef(omniObjRef*)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);

  if( --pd_refCount > 0 )  return;

  delete this;
}


void
omniRemoteIdentity::locateRequest()
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);

  omniRemoteIdentity_RefHolder rh(this);

  if( omniORB::trace(10) ) {
    omniORB::logger l;
    l << "LocateRequest to remote: " << this << '\n';
  }

  GIOP_C giop_client(pd_ior,pd_rope);
  cdrStream& s = (cdrStream&)giop_client;

  GIOP::LocateStatusType rc;

 again:
  switch( (rc = giop_client.IssueLocateRequest()) ) {
  case GIOP::OBJECT_HERE:
    giop_client.RequestCompleted();
    break;

  case GIOP::UNKNOWN_OBJECT:
    giop_client.RequestCompleted();
    OMNIORB_THROW(OBJECT_NOT_EXIST,0,CORBA::COMPLETED_NO);
    break;        // dummy break

  case GIOP::OBJECT_FORWARD:
  case GIOP::OBJECT_FORWARD_PERM:
    {
      CORBA::Object_var obj(CORBA::Object::_unmarshalObjRef((cdrStream&)giop_client));
      giop_client.RequestCompleted();
      throw omniORB::LOCATION_FORWARD(obj._retn(),
			       (rc == GIOP::OBJECT_FORWARD_PERM) ? 0 : 1);
    }

  case GIOP::LOC_NEEDS_ADDRESSING_MODE:
    {
      GIOP::AddressingDisposition v;
      v <<= s;
      pd_ior->addr_mode = v;
      giop_client.RequestCompleted();
      if (omniORB::trace(10)) {
	omniORB::logger log;
	log << "Remote locatRequest: GIOP::NEEDS_ADDRESSING_MODE: "
	    << (int) v << " retry request.\n";
      }
      goto again;
    }

  case GIOP::LOC_SYSTEM_EXCEPTION:
    OMNIORB_ASSERT(0);
    break;
  }
}


omniRemoteIdentity::~omniRemoteIdentity()
{
  omniORB::logs(15, "omniRemoteIdentity deleted.");
  pd_rope->decrRefCount();
  pd_ior->release();
}
