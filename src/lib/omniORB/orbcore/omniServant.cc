// -*- Mode: C++; -*-
//                            Package   : omniORB
// omniServant.cc             Created on: 26/2/99
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
//    Base class for object implementations (servants).
//

/*
  $Log$
  Revision 1.2.2.2  2000/09/27 18:10:23  sll
  Use calldescriptor to handle upcalls to CORBA::Object operations.

  Revision 1.2.2.1  2000/07/17 10:35:56  sll
  Merged from omni3_develop the diff between omni3_0_0_pre3 and omni3_0_0.

  Revision 1.3  2000/07/13 15:25:56  dpg1
  Merge from omni3_develop for 3.0 release.

  Revision 1.1.2.5  2000/06/22 10:40:16  dpg1
  exception.h renamed to exceptiondefs.h to avoid name clash on some
  platforms.

  Revision 1.1.2.4  1999/10/27 17:32:14  djr
  omni::internalLock and objref_rc_lock are now pointers.

  Revision 1.1.2.3  1999/10/14 16:22:14  djr
  Implemented logging when system exceptions are thrown.

  Revision 1.1.2.2  1999/09/27 11:01:11  djr
  Modifications to logging.

  Revision 1.1.2.1  1999/09/22 14:27:00  djr
  Major rewrite of orbcore to support POA.

*/

#include <omniORB4/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <omniORB4/omniServant.h>
#include <localIdentity.h>
#include <exceptiondefs.h>
#include <omniORB4/callDescriptor.h>
#include <objectStub.h>

omniServant::~omniServant()
{
  if( pd_identities ) {
    omniORB::logger l;
    l << "ERROR -- A servant has been deleted that is still activated.\n"
      " repo id: " << _mostDerivedRepoId() << "\n";

    omni::internalLock->lock();
    omniLocalIdentity* id = pd_identities;
    while( id ) {
      l << "      id: " << id << '\n';
      id = id->servantsNextIdentity();
    }
    omni::internalLock->unlock();
  }
}


void*
omniServant::_ptrToInterface(const char* repoId)
{
  OMNIORB_ASSERT(repoId);

  if( !strcmp(repoId, CORBA::Object::_PD_repoId) )
    return (void*) 1;

  return 0;
}


void*
omniServant::_downcast()
{
  return 0;
}


const char*
omniServant::_mostDerivedRepoId()
{
  return "";
}


CORBA::Boolean
omniServant::_is_a(const char* repoId)
{
  return _ptrToInterface(repoId) ? 1 : 0;
}


CORBA::Boolean
omniServant::_non_existent()
{
  return 0;
}

_CORBA_Boolean
omniServant::_dispatch(GIOP_S& giop_s)
{
  const char* op = giop_s.invokeInfo().operation();

  if( strcmp(op, "_is_a") == 0 ) {
    omni_is_a_CallDesc call_desc("_is_a",sizeof("_is_a"),0,1);
    _upcall(giop_s,call_desc);
    return 1;
  }

  if( strcmp(op, "_non_existent") == 0 ) {
    omni_non_existent_CallDesc call_desc("_non_existent",
					 sizeof("_non_existent"),1);
    _upcall(giop_s,call_desc);
    return 1;
  }

  if( strcmp(op, "_interface") == 0 ) {
    omni_interface_CallDesc call_desc("_interface",
				      sizeof("_interface"),1);
    _upcall(giop_s,call_desc);
    return 1;
  }

  if( strcmp(op, "_implementation") == 0 ) {
    omniORB::logs(2,
     "WARNING -- received GIOP request \'_implementation\'.\n"
     " This operation is not supported.  CORBA::NO_IMPLEMENT was raised.");
    OMNIORB_THROW(NO_IMPLEMENT,0, CORBA::COMPLETED_NO);
  }
  return 0;
}

void
omniServant::_upcall(GIOP_S& giop_s, omniCallDescriptor& desc)
{
  giop_s.invokeInfo().set_user_exceptions(desc.user_excns(),
					  desc.n_user_excns());
  desc.unmarshalArguments((cdrStream&)giop_s);
  giop_s.RequestReceived();
  desc.doLocalCall(this);
  if (!desc.is_oneway()) {
    omniServerCallMarshaller m(desc);
    giop_s.InitialiseReply(GIOP::NO_EXCEPTION,m);
  }
  giop_s.ReplyCompleted();
}


void
omniServant::_addIdentity(omniLocalIdentity* id)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);
  OMNIORB_ASSERT(id);  OMNIORB_ASSERT(id->servant() == this);

  *id->addrOfServantsNextIdentity() = pd_identities;
  pd_identities = id;
}


void
omniServant::_removeIdentity(omniLocalIdentity* id)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);
  OMNIORB_ASSERT(id);

  omniLocalIdentity** p = &pd_identities;

  while( *p != id ) {
    OMNIORB_ASSERT(*p);
    p = (*p)->addrOfServantsNextIdentity();
  }

  *p = id->servantsNextIdentity();
  *(id->addrOfServantsNextIdentity()) = 0;
}
