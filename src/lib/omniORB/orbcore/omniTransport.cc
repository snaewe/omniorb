// -*- Mode: C++; -*-
//                            Package   : omniORB
// omniTransport.cc           Created on: 16/01/2001
//                            Author    : Sai Lai Lo (sll)
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

/*
  $Log$
  Revision 1.1.4.4  2001/09/19 17:26:51  dpg1
  Full clean-up after orb->destroy().

  Revision 1.1.4.3  2001/08/17 17:12:41  sll
  Modularise ORB configuration parameters.

  Revision 1.1.4.2  2001/06/13 20:13:15  sll
  Minor updates to make the ORB compiles with MSVC++.

  Revision 1.1.4.1  2001/04/18 18:10:48  sll
  Big checkin with the brand new internal APIs.


  */

#include <omniORB4/CORBA.h>
#include <omniORB4/omniTransport.h>
#include <omniORB4/IOP_C.h>
#include <initialiser.h>

OMNI_NAMESPACE_BEGIN(omni)

omni_tracedmutex* omniTransportLock     = 0;

////////////////////////////////////////////////////////////////////////////
IOP_C_Holder::IOP_C_Holder(const omniIOR* ior,
			   const CORBA::Octet* key,
			   CORBA::ULong keysize,
			   Rope* rope,
			   omniCallDescriptor* calldesc) : pd_rope(rope) {

  OMNIORB_ASSERT(calldesc);
  pd_iop_c = rope->acquireClient(ior,key,keysize,calldesc);
}

////////////////////////////////////////////////////////////////////////////
IOP_C_Holder::~IOP_C_Holder() {
  pd_rope->releaseClient(pd_iop_c);
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
void
RopeLink::insert(RopeLink& head)
{
  next = head.prev->next;
  head.prev->next = this;
  prev = head.prev;
  head.prev = this;
}

////////////////////////////////////////////////////////////////////////////
void
RopeLink::remove()
{
  prev->next = next;
  next->prev = prev;
}

////////////////////////////////////////////////////////////////////////////
CORBA::Boolean
RopeLink::is_empty(RopeLink& head)
{
  return (head.next == &head);
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
void
StrandList::insert(StrandList& head)
{
  next = head.prev->next;
  head.prev->next = this;
  prev = head.prev;
  head.prev = this;
}

////////////////////////////////////////////////////////////////////////////
void
StrandList::remove()
{
  prev->next = next;
  next->prev = prev;
}

////////////////////////////////////////////////////////////////////////////
CORBA::Boolean
StrandList::is_empty(StrandList& head)
{
  return (head.next == &head);
}


/////////////////////////////////////////////////////////////////////////////
//            Module initialiser                                           //
/////////////////////////////////////////////////////////////////////////////

class omni_omniTransport_initialiser : public omniInitialiser {
public:

  void attach() {
    OMNIORB_ASSERT(!omniTransportLock);

    if (!omniTransportLock) omniTransportLock = new omni_tracedmutex;
  }

  void detach() {
    // omniTransportLock is deleted by the final clean-up in omniInternal.cc
  }
};

static omni_omniTransport_initialiser initialiser;

omniInitialiser& omni_omniTransport_initialiser_ = initialiser;

OMNI_NAMESPACE_END(omni)



