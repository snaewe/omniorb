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
  Revision 1.2.2.15  2002/07/03 15:49:51  dgrisby
  Correct debug flag, typos, bug report address.

  Revision 1.2.2.14  2001/09/19 17:26:53  dpg1
  Full clean-up after orb->destroy().

  Revision 1.2.2.13  2001/09/03 16:52:05  sll
  New signature for locateRequest. Now accept a calldescriptor argument.

  Revision 1.2.2.12  2001/09/03 13:28:59  sll
  Changed locateRequest to honour the same retry rule as normal invocation.

  Revision 1.2.2.11  2001/08/17 13:42:49  dpg1
  callDescriptor::userException() no longer has to throw an exception.

  Revision 1.2.2.10  2001/08/15 10:26:14  dpg1
  New object table behaviour, correct POA semantics.

  Revision 1.2.2.9  2001/08/03 17:41:24  sll
  System exception minor code overhaul. When a system exeception is raised,
  a meaning minor code is provided.

  Revision 1.2.2.8  2001/05/09 17:02:25  sll
  Throw omniORB::LOCATION_FORWARD with the right permanent flag.

  Revision 1.2.2.7  2001/04/18 18:18:04  sll
  Big checkin with the brand new internal APIs.

  Revision 1.2.2.6  2000/12/05 17:39:31  dpg1
  New cdrStream functions to marshal and unmarshal raw strings.

  Revision 1.2.2.5  2000/11/15 17:25:45  sll
  cdrCountingStream must now be told explicitly what char and wchar
  codeset convertor to use.

  Revision 1.2.2.4  2000/11/09 12:27:59  dpg1
  Huge merge from omni3_develop, plus full long long from omni3_1_develop.

  Revision 1.2.2.3  2000/11/03 19:12:07  sll
  Use new marshalling functions for byte, octet and char. Use get_octet_array
  instead of get_char_array and put_octet_array instead of put_char_array.

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
#include <omniORB4/omniTransport.h>
#include <omniORB4/IOP_C.h>
#include <omniORB4/callDescriptor.h>
#include <dynamicLib.h>
#include <exceptiondefs.h>
#include <giopStream.h>

OMNI_NAMESPACE_BEGIN(omni)

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
    if (--pd_id->pd_refCount == 0) delete pd_id;
    omni::internalLock->unlock();
  }

private:
  omniRemoteIdentity* pd_id;
};

OMNI_NAMESPACE_END(omni)

OMNI_USING_NAMESPACE(omni)

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void
omniRemoteIdentity::dispatch(omniCallDescriptor& call_desc)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);

  omniRemoteIdentity_RefHolder rh(this);
  // omni::internalLock has been released by RefHolder constructor

  if( omniORB::traceInvocations ) {
    omniORB::logger l;
    l << "Invoke '" << call_desc.op() << "' on remote: " << this << '\n';
  }

  IOP_C_Holder iop_client(pd_ior,key(),keysize(),pd_rope,&call_desc);
  cdrStream& s = ((IOP_C&)iop_client).getStream();

 again:
  call_desc.initialiseCall(s);

  iop_client->InitialiseRequest();

  // Wait for the reply.
  GIOP::ReplyStatusType rc;

  switch( (rc = iop_client->ReceiveReply()) ) {
  case GIOP::NO_EXCEPTION:
    // Unmarshal any result and out/inout arguments.
    call_desc.unmarshalReturnedValues(s);
    iop_client->RequestCompleted();
    break;

  case GIOP::USER_EXCEPTION:
    {
      // Retrieve the Interface Repository ID of the exception.
      CORBA::String_var repoId(s.unmarshalRawString());
      call_desc.userException(iop_client->getStream(), &(IOP_C&)iop_client,
			      repoId);
      // Usually, the userException() call throws a user exception or
      // a system exception. In the DII case, it just stores the
      // exception and returns.

      break;
    }

  case GIOP::LOCATION_FORWARD:
  case GIOP::LOCATION_FORWARD_PERM:
    {
      CORBA::Object_var obj(CORBA::Object::_unmarshalObjRef(s));
      iop_client->RequestCompleted();
      throw omniORB::LOCATION_FORWARD(obj._retn(),
			       (rc == GIOP::LOCATION_FORWARD_PERM) ? 1 : 0);
    }

  case GIOP::NEEDS_ADDRESSING_MODE:
    {
      GIOP::AddressingDisposition v;
      v <<= s;
      pd_ior->addr_mode(v);
      iop_client->RequestCompleted();
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
omniRemoteIdentity::gainRef(omniObjRef*)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);

  pd_refCount++;
}


void
omniRemoteIdentity::loseRef(omniObjRef*)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);

  if( --pd_refCount > 0 )  return;

  delete this;
}


void
omniRemoteIdentity::locateRequest(omniCallDescriptor& call_desc)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);

  omniRemoteIdentity_RefHolder rh(this);

  if( omniORB::trace(10) ) {
    omniORB::logger l;
    l << "LocateRequest to remote: " << this << '\n';
  }

  IOP_C_Holder iop_client(pd_ior,key(),keysize(),pd_rope,&call_desc);
  cdrStream& s = ((IOP_C&)iop_client).getStream();

  GIOP::LocateStatusType rc;

 again:
  switch( (rc = iop_client->IssueLocateRequest()) ) {
  case GIOP::OBJECT_HERE:
    iop_client->RequestCompleted();
    break;

  case GIOP::UNKNOWN_OBJECT:
    iop_client->RequestCompleted();
    OMNIORB_THROW(OBJECT_NOT_EXIST,OBJECT_NOT_EXIST_NoMatch,
		  CORBA::COMPLETED_NO);
    break;        // dummy break

  case GIOP::OBJECT_FORWARD:
  case GIOP::OBJECT_FORWARD_PERM:
    {
      CORBA::Object_var obj(CORBA::Object::_unmarshalObjRef(s));
      iop_client->RequestCompleted();
      throw omniORB::LOCATION_FORWARD(obj._retn(),
				      (rc == GIOP::OBJECT_FORWARD_PERM) ? 1 : 0);
    }

  case GIOP::LOC_NEEDS_ADDRESSING_MODE:
    {
      GIOP::AddressingDisposition v;
      v <<= s;
      pd_ior->addr_mode(v);
      iop_client->RequestCompleted();
      if (omniORB::trace(10)) {
	omniORB::logger log;
	log << "Remote locateRequest: GIOP::NEEDS_ADDRESSING_MODE: "
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
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);

  omniORB::logs(15, "omniRemoteIdentity deleted.");
  pd_rope->decrRefCount();
  pd_ior->release();

  if (--identity_count == 0)
    lastIdentityHasBeenDeleted();
}

omniIdentity::equivalent_fn
omniRemoteIdentity::get_real_is_equivalent() const {
  return real_is_equivalent;
}

CORBA::Boolean
omniRemoteIdentity::real_is_equivalent(const omniIdentity* id1,
				       const omniIdentity* id2) {

  omniRemoteIdentity* rid1 = (omniRemoteIdentity*)id1;
  omniRemoteIdentity* rid2 = (omniRemoteIdentity*)id2;

  if (rid1->pd_rope != rid2->pd_rope) return 0;

  const CORBA::Octet* key1 = rid1->key();
  int keysize1             = rid1->keysize();

  const CORBA::Octet* key2 = rid2->key();
  int keysize2             = rid2->keysize();

  if (keysize1 != keysize2 || memcmp((void*)key1,(void*)key2,keysize1) != 0)
      // Object keys do not match
      return 0;

  return 1;
}

_CORBA_Boolean
omniRemoteIdentity::inThisAddressSpace()
{
  return 0;
}

void*
omniRemoteIdentity::thisClassCompare(omniIdentity* id, void* vfn)
{
  classCompare_fn fn = (classCompare_fn)vfn;

  if (fn == omniRemoteIdentity::thisClassCompare)
    return (omniRemoteIdentity*)id;

  return 0;
}
