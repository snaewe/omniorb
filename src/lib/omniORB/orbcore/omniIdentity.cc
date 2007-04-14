// -*- Mode: C++; -*-
//                            Package   : omniORB
// omniIdentity.cc            Created on: 2001/09/17
//                            Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2001 AT&T Laboratories Cambridge
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
  Revision 1.1.4.2  2007/04/14 17:56:52  dgrisby
  Identity downcasting mechanism was broken by VC++ 8's
  over-enthusiastic optimiser.

  Revision 1.1.4.1  2003/03/23 21:02:09  dgrisby
  Start of omniORB 4.1.x development branch.

  Revision 1.1.2.1  2001/09/19 17:26:50  dpg1
  Full clean-up after orb->destroy().

*/

#include <omniORB4/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <omniIdentity.h>

OMNI_USING_NAMESPACE(omni)

int omniIdentity::identity_count = 0;

static omni_tracedcondition* cond = 0;

void
omniIdentity::waitForLastIdentity()
{
  omni_tracedmutex_lock sync(*omni::internalLock);

  if (identity_count == 0)
    return;

  omniORB::logs(15, "Waiting for client invocations to complete");

  cond = new omni_tracedcondition(omni::internalLock);

  while (identity_count) cond->wait();

  delete cond;
  cond = 0;
}

void
omniIdentity::lastIdentityHasBeenDeleted()
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);

  if (cond)
    cond->signal();
}

void*
omniIdentity::ptrToClass(int* cptr)
{
  if (cptr == &omniIdentity::_classid) return (omniIdentity*)this;
  return 0;
}

int omniIdentity::_classid;
