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
  Revision 1.1.4.7  2005/08/23 11:45:06  dgrisby
  New maxSocketSend and maxSocketRecv parameters.

  Revision 1.1.4.6  2002/09/04 23:29:30  dgrisby
  Avoid memory corruption with multiple list removals.

  Revision 1.1.4.5  2001/09/20 13:26:15  dpg1
  Allow ORB_init() after orb->destroy().

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
#include <orbOptions.h>
#include <orbParameters.h>

OMNI_NAMESPACE_BEGIN(omni)

omni_tracedmutex* omniTransportLock     = 0;

////////////////////////////////////////////////////////////////////////////
// Maximum sizes for socket sends / recvs

#if defined(__WIN32__)
// Windows has a bug that sometimes means large sends fail
size_t orbParameters::maxSocketSend = 131072;
size_t orbParameters::maxSocketRecv = 131072;

#elif defined(__VMS)
// VMS has a hard limit
size_t orbParameters::maxSocketSend = 65528;
size_t orbParameters::maxSocketRecv = 65528;

#else
// Other platforms have no limit
size_t orbParameters::maxSocketSend = 0x7fffffff;
size_t orbParameters::maxSocketRecv = 0x7fffffff;
#endif


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

  // When a connection is scavenged, remove() is called by the
  // scavenger to remove the connection from the scavenger's list.
  // Later, the thread looking after the strand calls safeDelete()
  // which attempts to remove() it again. Setting next and prev to
  // this means the second remove has no effect.
  next = prev = this;
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
  next = prev = this;
}

////////////////////////////////////////////////////////////////////////////
CORBA::Boolean
StrandList::is_empty(StrandList& head)
{
  return (head.next == &head);
}


/////////////////////////////////////////////////////////////////////////////
class maxSocketSendHandler : public orbOptions::Handler {
public:

  maxSocketSendHandler() : 
    orbOptions::Handler("maxSocketSend",
			"maxSocketSend = n >= 8192",
			1,
			"-ORBmaxSocketSend < n >= 8192 >") {}

  void visit(const char* value,orbOptions::Source) throw (orbOptions::BadParam) {

    CORBA::ULong v;
    if (!orbOptions::getULong(value,v) || v < 8192) {
      throw orbOptions::BadParam(key(),value,
				 "Invalid value, expect n >= 8192");
    }
    orbParameters::maxSocketSend = v;
  }

  void dump(orbOptions::sequenceString& result) {
    orbOptions::addKVULong(key(),orbParameters::maxSocketSend,
			   result);
  }

};

static maxSocketSendHandler maxSocketSendHandler_;


/////////////////////////////////////////////////////////////////////////////
class maxSocketRecvHandler : public orbOptions::Handler {
public:

  maxSocketRecvHandler() : 
    orbOptions::Handler("maxSocketRecv",
			"maxSocketRecv = n >= 8192",
			1,
			"-ORBmaxSocketRecv < n >= 8192 >") {}

  void visit(const char* value,orbOptions::Source) throw (orbOptions::BadParam) {

    CORBA::ULong v;
    if (!orbOptions::getULong(value,v) || v < 8192) {
      throw orbOptions::BadParam(key(),value,
				 "Invalid value, expect n >= 8192");
    }
    orbParameters::maxSocketRecv = v;
  }

  void dump(orbOptions::sequenceString& result) {
    orbOptions::addKVULong(key(),orbParameters::maxSocketRecv,
			   result);
  }

};

static maxSocketRecvHandler maxSocketRecvHandler_;


/////////////////////////////////////////////////////////////////////////////
//            Module initialiser                                           //
/////////////////////////////////////////////////////////////////////////////

class omni_omniTransport_initialiser : public omniInitialiser {
public:
  omni_omniTransport_initialiser() {
    orbOptions::singleton().registerHandler(maxSocketSendHandler_);
    orbOptions::singleton().registerHandler(maxSocketRecvHandler_);
  }

  void attach() {
    if (!omniTransportLock) omniTransportLock = new omni_tracedmutex;
  }

  void detach() {
    // omniTransportLock is deleted by the final clean-up in omniInternal.cc
  }
};

static omni_omniTransport_initialiser initialiser;

omniInitialiser& omni_omniTransport_initialiser_ = initialiser;

OMNI_NAMESPACE_END(omni)



