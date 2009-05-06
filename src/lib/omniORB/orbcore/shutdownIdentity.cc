// -*- Mode: C++; -*-
//                            Package   : omniORB
// shutdownIdentity.cc        Created on: 2001/09/17
//                            Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2007 Apasphere Ltd
//    Copyright (C) 2001 AT&T Research Cambridge
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
//    Dummy singleton identity placed into object references when the
//    ORB is shutting down.
//

/*
  $Log$
  Revision 1.1.4.3  2009/05/06 16:14:56  dgrisby
  Update lots of copyright notices.

  Revision 1.1.4.2  2007/04/14 17:56:52  dgrisby
  Identity downcasting mechanism was broken by VC++ 8's
  over-enthusiastic optimiser.

  Revision 1.1.4.1  2003/03/23 21:02:03  dgrisby
  Start of omniORB 4.1.x development branch.

  Revision 1.1.2.1  2001/09/19 17:26:53  dpg1
  Full clean-up after orb->destroy().

*/

#include <omniORB4/CORBA.h>
#include <omniORB4/minorCode.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <shutdownIdentity.h>

OMNI_USING_NAMESPACE(omni)

static omniShutdownIdentity* the_singleton = 0;

void
omniShutdownIdentity::dispatch(omniCallDescriptor&)
{
  omni::internalLock->unlock();
  OMNIORB_THROW(BAD_INV_ORDER,
		BAD_INV_ORDER_ORBHasShutdown,
		CORBA::COMPLETED_NO);
}

void
omniShutdownIdentity::gainRef(omniObjRef*)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);
  pd_refCount++;
}

void
omniShutdownIdentity::loseRef(omniObjRef*)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);

  if (--pd_refCount == 0) {
    delete this;
    the_singleton = 0;
  }
}

void
omniShutdownIdentity::locateRequest(omniCallDescriptor&)
{
  omni::internalLock->unlock();
}

_CORBA_Boolean
omniShutdownIdentity::inThisAddressSpace()
{
  return 0;
}

omniShutdownIdentity*
omniShutdownIdentity::singleton()
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);

  if (!the_singleton)
    the_singleton = new omniShutdownIdentity;

  return the_singleton;
}

omniIdentity::equivalent_fn
omniShutdownIdentity::get_real_is_equivalent() const
{
  return real_is_equivalent;
}

CORBA::Boolean
omniShutdownIdentity::real_is_equivalent(const omniIdentity* id1,
					 const omniIdentity* id2)
{
  return 0;
}


void*
omniShutdownIdentity::ptrToClass(int* cptr)
{
  if (cptr == &omniShutdownIdentity::_classid)
    return (omniShutdownIdentity*)this;

  if (cptr == &omniIdentity::_classid)
    return (omniIdentity*)this;

  return 0;
}

int omniShutdownIdentity::_classid;
