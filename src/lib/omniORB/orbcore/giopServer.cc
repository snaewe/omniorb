// -*- Mode: C++; -*-
//                            Package   : omniORB
// giopServer.cc              Created on: 20 Dec 2000
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2000 AT&T Laboratories Cambridge
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
//	*** PROPRIETORY INTERFACE ***
// 

/*
  $Log$
  Revision 1.22.2.8  2001/06/11 18:00:52  sll
  Fixed silly mistake in shutdown multiple endpoints.

  Revision 1.22.2.7  2001/04/18 18:10:49  sll
  Big checkin with the brand new internal APIs.


*/


#include <omniORB4/CORBA.h>
#include <invoker.h>
#include <giopServer.h>
#include <giopWorker.h>
#include <giopRendezvouser.h>
#include <giopStrand.h>

OMNI_NAMESPACE_BEGIN(omni)

////////////////////////////////////////////////////////////////////////////
giopServer::giopServer() : pd_state(IDLE), pd_cond(&pd_lock)
{
}

////////////////////////////////////////////////////////////////////////////
giopServer::~giopServer()
{
}


////////////////////////////////////////////////////////////////////////////
const char*
giopServer::instantiate(const char* uri,
			CORBA::Boolean no_publish,
			CORBA::Boolean no_listen)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(pd_lock,0);
  omni_tracedmutex_lock sync(pd_lock);
    
  ensureNotInFlux();

  if (!no_listen) {

    giopEndpoint* ept = giopEndpoint::str2Endpoint(uri);
    if (!ept) return 0;

    OMNIORB_ASSERT(pd_state != ZOMBIE);
    
    if (ept->bind()) {
      pd_endpoints.push_back(ept);
      if (pd_state == ACTIVE) activate();
      uri =  ept->address();
    }
    else {
      delete ept;
      return 0;
    }
  }
  else if ( !giopEndpoint::strIsValidEndpoint(uri) ) {
    return 0;
  }

  if (!no_publish) {
    (void)giopEndpoint::addToIOR(uri);
  }
  return uri;
}


////////////////////////////////////////////////////////////////////////////
void
giopServer::start()
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(pd_lock,0);
  omni_tracedmutex_lock sync(pd_lock);

  ensureNotInFlux();

  switch (pd_state) {
  case IDLE:
    {
      pd_state = ACTIVE;
      activate();
    }
    break;
  default:
    break;
  }
}

////////////////////////////////////////////////////////////////////////////
void
giopServer::stop()
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(pd_lock,0);
  omni_tracedmutex_lock sync(pd_lock);

  ensureNotInFlux();

  switch (pd_state) {
  case ACTIVE:
    {
      deactivate();
    }
    break;
  default:
    break;
  }
}

////////////////////////////////////////////////////////////////////////////
void
giopServer::remove()
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(pd_lock,0);
  omni_tracedmutex_lock sync(pd_lock);

  ensureNotInFlux();

  switch (pd_state) {
  case ACTIVE:
    {
      deactivate();
    }
  case IDLE:
    {
      pd_state = ZOMBIE;
    } 
    break;
  default:
    return;
  }

  giopEndpointList::iterator i;
  i    = pd_endpoints.begin();
  
  while (i != pd_endpoints.end()) {
    (*i)->shutdown();
    delete *i;
    pd_endpoints.erase(i);
  }
}

////////////////////////////////////////////////////////////////////////////
void
giopServer::activate()
{
  // Caller is holding pd_lock

  giopEndpointList::iterator i;
  i    = pd_endpoints.begin();

  while (i != pd_endpoints.end()) {
    giopRendezvouser* task = new giopRendezvouser(*i,this);

    if (!orbAsyncInvoker->insert(task)) {
      // Cannot start serving this endpoint.
      // Leave it in pd_endpoints.
      // Do not raise an exception. Instead, just moan about it.
      omniORB::logger log;
      log << "Cannot create a rendezvouser for this endpoint: ";
      log << (*i)->address();
      log << "\n";
      delete task;
      i++;
      continue;
    }
    pd_endpoints.erase(i);

    task->insert(pd_rendezvousers);
  }
}

////////////////////////////////////////////////////////////////////////////
void
giopServer::deactivate()
{
  // Caller is holding pd_lock

  OMNIORB_ASSERT(pd_state == ACTIVE);
  
  pd_state = INFLUX;

 again:
  CORBA::Boolean waitforcompletion = 0;
  {
    Link* p = pd_workers.next;
    if (p != &pd_workers) waitforcompletion = 1;

    for (; p != &pd_workers; p = p->next) {
      ((giopWorker*)p)->terminate();
    }
  }
  {
    Link* p = pd_rendezvousers.next;
    if (p != &pd_rendezvousers) waitforcompletion = 1;

    for (; p != &pd_rendezvousers; p = p->next) {
      ((giopRendezvouser*)p)->terminate();
    }
  }

  if (waitforcompletion) {
    // Here we relinquish the mutex to give the rendezvousers and workers
    // a chance to remove themselves from the lists. Notice that the server
    // is now in the INFLUX state. This means that no start, stop, remove,
    // instantiate can progress until we are done here.
    pd_cond.wait();
    goto again;
  }
  else {
    pd_state = IDLE;
  }
}

////////////////////////////////////////////////////////////////////////////
void
giopServer::ensureNotInFlux()
{
  while (pd_state == INFLUX) {
    pd_cond.wait();
    // Note: we could have more than one thread blocking here so must
    //       be wake up by a broadcast. Or else we have to keep a count
    //       on how many is waiting.
  }
}


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
void
giopServer::notifyRzNewConnection(giopRendezvouser* r, giopConnection* conn)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(pd_lock,0);
  omni_tracedmutex_lock sync(pd_lock);

  switch (pd_state) {
  case ACTIVE:
    {
      giopStrand* s = new giopStrand(conn);
      {
	ASSERT_OMNI_TRACEDMUTEX_HELD(*omniTransportLock,0);
	omni_tracedmutex_lock sync(*omniTransportLock);
	s->StrandList::insert(giopStrand::passive);
      }
      giopWorker* task = new giopWorker(s,this);
      if (!orbAsyncInvoker->insert(task)) {
	// Cannot start serving this new connection.
	omniORB::logger log;
	log << "Cannot create a worker for this endpoint: "
	    << conn->myaddress()
	    << " from " 
	    << conn->peeraddress()
	    << "\n";
	delete task;

	omni_tracedmutex_lock sync(*omniTransportLock);
	s->StrandList::remove();
	delete s;
	throw outOfResource();
      }
      task->insert(pd_workers);
      break;
    }
  default:
    throw Terminate();
  }
}

////////////////////////////////////////////////////////////////////////////
void
giopServer::notifyRzDone(giopRendezvouser* r, CORBA::Boolean exit_on_error)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(pd_lock,0);
  omni_tracedmutex_lock sync(pd_lock);

  if (!exit_on_error) {
    OMNIORB_ASSERT(pd_state == INFLUX);
    // For the moment, we do not instantiate giopRendezvouser to do single shot.
    // Therefore, this function will *NEVER* be called until:
    //  1. We have called deactivate()
    //  2. giopRendezvouser have encountered a non-recoverable error. In which
    //     case exit_on_error == 1.
  }

  giopEndpoint* ept = r->endpoint();

  r->remove();

  delete r;
  
  if (exit_on_error) {
    omniORB::logger log;
    log << "Unrecoverable error for this endpoint: ";
    log << ept->address();
    log << ", it will no longer be serviced.\n";
    ept->shutdown();
    delete ept;
  }
  else {
    pd_endpoints.push_back(ept);
    // A new giopRendezvouser will be instantiated for this endpoint next
    // time activate() is called.
  }

  if (pd_state == INFLUX) {
    if (Link::is_empty(pd_rendezvousers) && Link::is_empty(pd_workers)) {
      pd_cond.broadcast();
    }
  }
}


////////////////////////////////////////////////////////////////////////////
void
giopServer::notifyWkDone(giopWorker* w, CORBA::Boolean exit_on_error)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(pd_lock,0);
  omni_tracedmutex_lock sync(pd_lock);

  OMNIORB_ASSERT(exit_on_error == 1);
  // For the moment, we do not instantiate giopWorker to do single shot.
  // Therefore, this function will *NEVER* be called until giopWorker
  // encounters some error condition and it would have set exit_on_error to
  // 1.

  w->remove();
  delete w;
  // Do not do anything to the strand! It will be garbage collected
  // in due course.

  if (pd_state == INFLUX) {
    if (Link::is_empty(pd_rendezvousers) && Link::is_empty(pd_workers)) {
      pd_cond.broadcast();
    }
  }
}

////////////////////////////////////////////////////////////////////////////
void
giopServer::notifyWkPreUpCall(giopStrand*) {
  // Do nothing at the moment.
  return;
}

////////////////////////////////////////////////////////////////////////////
void
giopServer::Link::insert(giopServer::Link& head)
{
  next = head.prev->next;
  head.prev->next = this;
  prev = head.prev;
  head.prev = this;
}

////////////////////////////////////////////////////////////////////////////
void
giopServer::Link::remove()
{
  prev->next = next;
  next->prev = prev;
}

////////////////////////////////////////////////////////////////////////////
CORBA::Boolean
giopServer::Link::is_empty(giopServer::Link& head)
{
  return (head.next == &head);
}


OMNI_NAMESPACE_END(omni)
