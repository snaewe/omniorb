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
  Revision 1.1.6.4  2007/02/26 15:16:31  dgrisby
  New socketSendBuffer parameter, defaulting to 16384 on Windows.
  Avoids a bug in Windows where select() on send waits for all sent data
  to be acknowledged.

  Revision 1.1.6.3  2005/11/17 17:03:26  dgrisby
  Merge from omni4_0_develop.

  Revision 1.1.6.2  2005/09/01 14:52:12  dgrisby
  Merge from omni4_0_develop.

  Revision 1.1.6.1  2003/03/23 21:02:08  dgrisby
  Start of omniORB 4.1.x development branch.

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
#include <SocketCollection.h>

OMNI_NAMESPACE_BEGIN(omni)

omni_tracedmutex* omniTransportLock = 0;


////////////////////////////////////////////////////////////////////////////
// Maximum sizes for socket sends / recvs

#if defined(__WIN32__)
// Windows has a bug that sometimes means large sends fail
size_t orbParameters::maxSocketSend    = 131072;
size_t orbParameters::maxSocketRecv    = 131072;
int    orbParameters::socketSendBuffer = 16384;

#elif defined(__VMS)
// VMS has a hard limit
size_t orbParameters::maxSocketSend    = 65528;
size_t orbParameters::maxSocketRecv    = 65528;
int    orbParameters::socketSendBuffer = -1;

#else
// Other platforms have no limit
size_t orbParameters::maxSocketSend    = 0x7fffffff;
size_t orbParameters::maxSocketRecv    = 0x7fffffff;
int    orbParameters::socketSendBuffer = -1;
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
			"maxSocketSend = n >= 1024",
			1,
			"-ORBmaxSocketSend < n >= 1024 >") {}

  void visit(const char* value,orbOptions::Source) throw (orbOptions::BadParam) {

    CORBA::ULong v;
    if (!orbOptions::getULong(value,v) || v < 1024) {
      throw orbOptions::BadParam(key(),value,
				 "Invalid value, expect n >= 1024");
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
			"maxSocketRecv = n >= 1024",
			1,
			"-ORBmaxSocketRecv < n >= 1024 >") {}

  void visit(const char* value,orbOptions::Source) throw (orbOptions::BadParam) {

    CORBA::ULong v;
    if (!orbOptions::getULong(value,v) || v < 1024) {
      throw orbOptions::BadParam(key(),value,
				 "Invalid value, expect n >= 1024");
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
class socketSendBufferHandler : public orbOptions::Handler {
public:

  socketSendBufferHandler() : 
    orbOptions::Handler("socketSendBuffer",
			"socketSendBuffer = n >= -1",
			1,
			"-ORBsocketSendBuffer < n >= -1 >") {}

  void visit(const char* value,orbOptions::Source) throw (orbOptions::BadParam) {

    CORBA::Long v;
    if (!orbOptions::getLong(value,v) || v < -1) {
      throw orbOptions::BadParam(key(),value,
				 "Invalid value, expect n >= -1");
    }
    orbParameters::socketSendBuffer = v;
  }

  void dump(orbOptions::sequenceString& result) {
    orbOptions::addKVLong(key(),orbParameters::socketSendBuffer,
			  result);
  }

};

static socketSendBufferHandler socketSendBufferHandler_;


/////////////////////////////////////////////////////////////////////////////
class connectionWatchPeriodHandler : public orbOptions::Handler {
public:

  connectionWatchPeriodHandler() : 
    orbOptions::Handler("connectionWatchPeriod",
			"connectionWatchPeriod = n >= 0 in microsecs",
			1,
			"-ORBconnectionWatchPeriod < n >= 0 in microsecs >") {}

  void visit(const char* value,orbOptions::Source) throw (orbOptions::BadParam) {

    CORBA::ULong v;
    if (!orbOptions::getULong(value,v)) {
      throw orbOptions::BadParam(key(),value,
				 "Expect n >= 0 in microsecs");
    }
    SocketCollection::scan_interval_sec  = v / 1000000;
    SocketCollection::scan_interval_nsec = (v % 1000000) * 1000;
  }

  void dump(orbOptions::sequenceString& result) {
    CORBA::ULong v = (SocketCollection::scan_interval_sec * 1000000 +
		      SocketCollection::scan_interval_nsec / 1000);
    orbOptions::addKVULong(key(),v,result);
  }

};

static connectionWatchPeriodHandler connectionWatchPeriodHandler_;


/////////////////////////////////////////////////////////////////////////////
//            Module initialiser                                           //
/////////////////////////////////////////////////////////////////////////////

class omni_omniTransport_initialiser : public omniInitialiser {
public:
  omni_omniTransport_initialiser() {
    orbOptions::singleton().registerHandler(maxSocketSendHandler_);
    orbOptions::singleton().registerHandler(maxSocketRecvHandler_);
    orbOptions::singleton().registerHandler(socketSendBufferHandler_);
    orbOptions::singleton().registerHandler(connectionWatchPeriodHandler_);
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



