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
  Revision 1.22.2.10  2001/07/13 15:27:42  sll
  Support for the thread-pool as well as the thread-per-connection policy
  Support for serving multiple incoming calls on the same connection
  simultaneously.

  Revision 1.22.2.9  2001/06/20 18:35:17  sll
  Upper case send,recv,connect,shutdown to avoid silly substutition by
  macros defined in socket.h to rename these socket functions
  to something else.

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
  giopServer::giopServer() : pd_state(IDLE), pd_nconnections(0),
			     pd_cond(&pd_lock), pd_n_temporary_workers(0)
{
  pd_thread_per_connection = omniORB::threadPerConnectionPolicy;
  pd_connectionState = new connectionState*[connectionState::hashsize];
  for (CORBA::ULong i=0; i < connectionState::hashsize; i++) {
    pd_connectionState[i] = 0;
  }
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

    if (ept->Bind()) {
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
    (*i)->Shutdown();
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
    for (CORBA::ULong i=0; i < connectionState::hashsize; i++) {
      connectionState** head = &(pd_connectionState[i]);
      while (*head) {
	(*head)->connection->Shutdown();
	head = &((*head)->next);
      }
    }
    if (pd_nconnections) waitforcompletion = 1;
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

CORBA::ULong giopServer::connectionState::hashsize = 103;

////////////////////////////////////////////////////////////////////////////
giopServer::
connectionState::connectionState(giopConnection* c,giopStrand* s) :
  connection(c), strand(s), next(0)
{
  omni_tracedmutex_lock sync(*omniTransportLock);
  c->incrRefCount();
}

////////////////////////////////////////////////////////////////////////////
giopServer::
connectionState::~connectionState()
{
  OMNIORB_ASSERT(Link::is_empty(workers));
  omni_tracedmutex_lock sync(*omniTransportLock);
  strand->deleteStrandAndConnection();
}

////////////////////////////////////////////////////////////////////////////
giopServer::connectionState*
giopServer::csLocate(giopConnection* conn)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(pd_lock,1);

  connectionState** head = &(pd_connectionState[((omni::ptr_arith_t)conn)%
						connectionState::hashsize]);
  while (*head) {
    if ((*head)->connection == conn)
      break;
    else {
      head = &((*head)->next);
    }
  }
  return *head;
}

////////////////////////////////////////////////////////////////////////////
giopServer::connectionState*
giopServer::csRemove(giopConnection* conn)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(pd_lock,1);

  connectionState* cs = 0;

  connectionState** head = &(pd_connectionState[((omni::ptr_arith_t)conn)%
						connectionState::hashsize]);
  while (*head) {
    if ((*head)->connection == conn) {
      cs = *head;
      *head = cs->next;
      pd_nconnections--;
      if (omniORB::threadPerConnectionPolicy) {
	// Check the number of connection and decide if we need to
	// re-enable the one thread per connection policy that has
	// been temporarily suspended.
	if (!pd_thread_per_connection &&
	    pd_nconnections <= omniORB::threadPerConnectionLowerLimit) {
	  pd_thread_per_connection = 1;
	}
      }
      break;
    }
    else {
      head = &((*head)->next);
    }
  }
  return cs;
}

////////////////////////////////////////////////////////////////////////////
giopServer::connectionState*
giopServer::csInsert(giopConnection* conn)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(pd_lock,1);
  

  giopStrand* s = new giopStrand(conn,this);
  {
    ASSERT_OMNI_TRACEDMUTEX_HELD(*omniTransportLock,0);
    omni_tracedmutex_lock sync(*omniTransportLock);
    s->StrandList::insert(giopStrand::passive);
  }

  connectionState* cs =  new connectionState(conn,s);

  connectionState** head = &(pd_connectionState[((omni::ptr_arith_t)conn)%
						connectionState::hashsize]);
  cs->next = *head;
  *head = cs;

  pd_nconnections++;

  if (omniORB::threadPerConnectionPolicy) {
    // Check the number of connection and decide if we need to
    // turn off the one thread per connection policy temporarily.
    if (pd_thread_per_connection && 
	pd_nconnections >= omniORB::threadPerConnectionUpperLimit) {
      pd_thread_per_connection = 0;
    }
  }

  conn->pd_has_dedicated_thread = pd_thread_per_connection;

  return cs;
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
      connectionState* cs = csInsert(conn);

      if (conn->pd_has_dedicated_thread) {
	giopWorker* task = new giopWorker(cs->strand,this,
					  !conn->pd_has_dedicated_thread);
	if (!orbAsyncInvoker->insert(task)) {
	  // Cannot start serving this new connection.
	  omniORB::logger log;
	  log << "Cannot create a worker for this endpoint: "
	      << conn->myaddress()
	      << " from "
	      << conn->peeraddress()
	      << "\n";
	  delete task;
	  {
	    omni_tracedmutex_lock sync(*omniTransportLock);
	    cs->strand->safeDelete();
	  }
	  csRemove(conn);
	  delete cs;

	  throw outOfResource();
	}
	task->insert(cs->workers);
	conn->pd_n_workers++;
      }
      else {
	conn->setSelectable(1);
      }
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
    ept->Shutdown();
    delete ept;
  }
  else {
    pd_endpoints.push_back(ept);
    // A new giopRendezvouser will be instantiated for this endpoint next
    // time activate() is called.
  }

  if (pd_state == INFLUX) {
    if (Link::is_empty(pd_rendezvousers) && pd_nconnections == 0) {
      pd_cond.broadcast();
    }
  }
}

////////////////////////////////////////////////////////////////////////////
void
giopServer::notifyRzReadable(giopConnection* conn,
			     CORBA::Boolean force_create)
{
  // Theory of operation: read the state diagrams at the end of this file.

  ASSERT_OMNI_TRACEDMUTEX_HELD(pd_lock,0);
  omni_tracedmutex_lock sync(pd_lock);

  switch (pd_state) {
  case ACTIVE:
    {
      if (conn->pd_dying) return;

      if (!force_create &&
	  conn->pd_n_workers >= (int)omniORB::maxServerThreadPerConnection) {
	conn->pd_has_hit_n_workers_limit = 1;
	return;
      }

      connectionState* cs = csLocate(conn);
      if (!cs) return;

      giopWorker* task = new giopWorker(cs->strand,this,1);
      if (!orbAsyncInvoker->insert(task)) {
	// Cannot start serving this new connection.
	// Should never happen
	OMNIORB_ASSERT(0);
      }
      task->insert(cs->workers);
      conn->pd_n_workers++;
      pd_n_temporary_workers++;
    }
  default:
    break;
  }
}

////////////////////////////////////////////////////////////////////////////
void
giopServer::notifyCallFullyBuffered(giopConnection* conn)
{
  notifyRzReadable(conn,1);
}

////////////////////////////////////////////////////////////////////////////
void
giopServer::removeConnectionAndWorker(giopWorker* w) {

  giopConnection* conn = w->strand()->connection;

  conn->pd_dying = 1; // From now on, the giopServer will not create
                      // any more workers to serve this connection.

  connectionState* cs = csLocate(conn);
  // We remove the lock on pd_lock before calling the connection's
  // clearSelectable(). This is necessary so that a simultaneous
  // callback from the Rendezvouser thread will have a chance to
  // look at the connectionState table.
  pd_lock.unlock();

  conn->clearSelectable();

  // Once we reach here, it is certain that the rendezvouser thread
  // would not take any interest in this connection anymore. It
  // is therefore safe to delete this record.
  pd_lock.lock();

  if (w->singleshot()) pd_n_temporary_workers--;

  w->remove();
  delete w;

  conn->pd_n_workers--;

  if (Link::is_empty(cs->workers)) {
    csRemove(conn);
    delete cs;
  }

  if (pd_state == INFLUX) {
    if (Link::is_empty(pd_rendezvousers) && pd_nconnections == 0) {
      pd_cond.broadcast();
    }
  }
}

////////////////////////////////////////////////////////////////////////////
void
giopServer::notifyWkDone(giopWorker* w, CORBA::Boolean exit_on_error)
{
  // Theory of operation: read the state diagrams at the end of this file.

  ASSERT_OMNI_TRACEDMUTEX_HELD(pd_lock,0);

  if (exit_on_error) {
    omni_tracedmutex_lock sync(pd_lock);
    removeConnectionAndWorker(w);
    return;
  }

  giopConnection* conn = w->strand()->connection;

  if (conn->pd_has_dedicated_thread) {

    // This connection is managed with the thread-per-connection policy
    if (!w->singleshot()) {
      // This is the dedicated thread
      conn->clearSelectable();
      omni_tracedmutex_lock sync(pd_lock);
      conn->pd_dedicated_thread_in_upcall = 0;
      conn->pd_has_hit_n_workers_limit = 0;
    }
    else {
      // This is a temporary worker thread
      omni_tracedmutex_lock sync(pd_lock);
      if (conn->pd_has_hit_n_workers_limit) {
	// Previously we have hit the limit on the no. of worker threads,
	// There is definitely a request pending. We re-cycle this
	// worker to deal with it *UNLESS* the dedicated thread is already
	// doing so.
	conn->pd_has_hit_n_workers_limit = 0;
	if (conn->pd_dedicated_thread_in_upcall ) {
	  if (!orbAsyncInvoker->insert(w)) {
	    OMNIORB_ASSERT(0);
	  }
	  return;
	}
      }
      w->remove();
      delete w;
      conn->pd_n_workers--;
      pd_n_temporary_workers--;
    }
  }
  else {

    // This connection is managed with the thread-pool policy
    // setSelectable if this is the only worker thread.
    OMNIORB_ASSERT(w->singleshot() == 1); // Never called by a dedicated thread
    CORBA::Boolean doselect;
    {
      omni_tracedmutex_lock sync(pd_lock);

      if (conn->pd_has_hit_n_workers_limit) {
	// Previously we have hit the limit on the no. of worker threads,
	// There is definitely a request pending. We re-cycle this
	// worker to deal with it.
	conn->pd_has_hit_n_workers_limit = 0;
	if (!orbAsyncInvoker->insert(w)) {
	  OMNIORB_ASSERT(0);
	}
	return;
      }

      w->remove();
      delete w;
      conn->pd_n_workers--;
      pd_n_temporary_workers--;
      // Set doselect if this is the last worker for this connection and
      // there is no worker tasks that cannot be allocated to a thread from
      // the pool.
      doselect = (conn->pd_n_workers == 0 && 
		  pd_n_temporary_workers < omniORB::maxServerThreadPoolSize);
    }
    if (doselect) {
      // We could call conn->setSelectable(1). 
      // Instead we call Peek(). This thread will be used for a short time to
      // monitor the connection. If the connection is available for reading,
      // the callback function peekCallBack is called. We can afford to
      // call Peek() here because this thread is otherwise idle.
      conn->Peek(peekCallBack,(void*)this);
    }
  }
}

////////////////////////////////////////////////////////////////////////////
// Callback function used by giopConnection::Peek(). Called when the
// connection indeed has something to read immediately.
void
giopServer::peekCallBack(void* cookie, giopConnection* conn) {
  giopServer* this_ = (giopServer*)cookie;
  this_->notifyRzReadable(conn);
}

////////////////////////////////////////////////////////////////////////////
void
giopServer::notifyWkPreUpCall(giopWorker* w, CORBA::Boolean data_in_buffer) {

  // Theory of operation: read the state diagrams at the end of this file.

  ASSERT_OMNI_TRACEDMUTEX_HELD(pd_lock,0);

  giopConnection* conn = w->strand()->connection;

  if (conn->pd_has_dedicated_thread) {
    // This connection is managed with the thread-per-connection policy
    if (!w->singleshot()) {
      // This is the dedicated thread.
      // setSelectable unless another temporary worker thread is already
      // present.
      int n;
      {
	omni_tracedmutex_lock sync(pd_lock);
	n = conn->pd_n_workers;
	conn->pd_dedicated_thread_in_upcall = 1;
      }
      if (n == 1) {
	conn->setSelectable(0,data_in_buffer);
      }
    }
    else {
      // This is a temporary worker thread
      // setSelectable only if the dedicated thread is in upcall.
      CORBA::Boolean n;
      {
	omni_tracedmutex_lock sync(pd_lock);
	n = conn->pd_dedicated_thread_in_upcall;
      }
      if (n) {
	conn->setSelectable(0,data_in_buffer);
      }
    }
  }
  else {
    // This connection is managed with the thread-pool policy
    conn->setSelectable(0,data_in_buffer);
  }
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


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
//   The following diagram illustrates the state transition of a connection
//   when it is managed under the thread pool policy.
//
//   Under this policy, the connection is watched by the rendezvouser
//   thread. When data arrive, the rendezvouser calls notifyRzReadable and
//   a worker thread is dispatched.  The worker unmarshal data from the
//   connection. When it finishes unmarshaling all the arguments, worker
//   calls notifyWkPreUpCall before doing the upcall into the
//   implementation.  In notifyWkPreUpCall, the connection is again
//   registered to be watched by the rendezvouser. When the worker has
//   finished the upcall, it calls notifyWkDone. If data for another call
//   arrives, the whole cycle repeats again.
//
//   The state diagram below illustrates what happens:
//
//        Legend:
//               +----------+     x = 1 : the connection is watched
//               +  (x,y,z) +         0 : the connection is not watched
//               +----------+     y = 1 : rendezvouser detected data arrival
//                                    0 : no new data have arrived
//                                z = n >= 0 : no. of threads serving the
//                                             connection
//

//
//                +---------+
//                | (1,0,0) |<----------------------+
//	          +----+----+                       |
//		       |		   	    |
//           	       | DataArrive	   	    |
//		       |		   	    |
//		       V		   	    |
//                +---------+	   	            |notifyWkDone**
//     	          | (0,1,0) |	   	            |  **-tell the rendezvouser
//	          +----+----+	   	            |     to watch immediately
//		       |		   	    |
//		       | notifyRzReadable	    |
//		       |		   	    |
//     	       	       V		   	    |
//                 +---------+       	          +-+-------+
//     	       +-->| (0,0,1) +------------------->| (1,0,1) |
//	       |   +---------+ notifyWkPreUpCall  +--+------+
//	       | 	     	 	   	     | ^
//     	       |     	  +--------------------------+ |
//	       |	  |    	    DataArrive 	       |
//             |	  V  		      	       |
//             |   +---------+	      	       	       |
//notifyWkDone |   | (0,1,1) |		       	       |
//   	       |   +----+----+		       	       |notifyWkDone
//   	       |	  |    			       |
//   	       |	  | notifyRzReadable	       |
//     	       |    	  |  		    	       |
//	       |    	  V  		    	       |
//             |   +---------+       	          +----+----+
//     	       +---+ (0,0,2) +------------------->| (1,0,2) |
//	       +-->+---------+ notifyWkPreUpCall  +--+------+
//	       | 	     	 	   	     | ^
//     	       |     	  +--------------------------+ |
//	       |	  |    	    DataArrive 	       |
//             |	  V  		      	       |
//             |   +---------+	      	       	       |
//notifyWkDone |   | (0,1,2) |		       	       |
//   	       |   +----+----+		       	       |notifyWkDone
//   	       |	  |    			       |
//   	       |	  | notifyRzReadable	       |
//     	       |    	  |  		    	       |
//	       |    	  V  		    	       |
//             |   +---------+       	          +----+----+
//     	       +---+ (0,0,3) +------------------->| (1,0,3) |
//	           +---------+ notifyWkPreUpCall  +--+------+
//  .... And so on...
//
//

//   To summarise, here are what the transition function should do:
//     notifyRzReadable() 
//        - Always create a temporary worker
//
//     notifyWkPreUpCall()
//        - setSelectable
//
//     notifyWkDone()
//        - setSelectable(1) if n worker == 1.
//

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
//   The following diagram illustrates the state transition of a connection
//   when it is managed under the thread-per-connection policy.
//
//   Under this policy, each connection has a dedicated worker thread.
//   When a call arrives, the worker unmarshalls all the arguments and do
//   an upcall. Before it does the upcall, it calls notifyWkPreUpCall.  In
//   notifyWkPreUpCall, the connection is registered to be watched by
//   the rendezvouser. When the worker has finished the upcall, it calls
//   notifyWkDone, the connnection is un-registered with the rendezvouser.
//   The worker then waits for another call to arrive and the whole cycle
//   repeats again.
//   If while the worker is performing the upcall another call has arrived,
//   the rendezvouser calls notifyRzReadable and a temporary worker thread
//   is dispatched. This thread is 'temporary' in the sense that it is only
//   dispatched to serve one call.
//   The interaction of the dedicated worker, the temporary worker and
//   the rendezvouser is summarised by the state diagram below:
//
//        Legend:
//               +----------+     x = 1 : the connection is watched
//               +  (x,y,z) +         0 : the connection is not watched
//               +----------+     y = 1 : rendezvouser detected data arrival
//                                    0 : no new data have arrived
//                                z = n >= 0 : no. of threads serving the
//                                             connection. The dedicated thread
//                                             is not watching the connection.
//                                z* = same as z except that the dedicated
//                                     thread is watching the connection.
//
//               notifyWkDone - transition executed by the temporary workers
//
//               notifyWkDone - transition executed by the dedicated worker
//               ^^^^^^^^^^^^
//

//
//                 +---------+
//     	      +--->| (0,0,1*)|<--------------------+
//     	      |    +----+----+			   |
//     	      |	       	| notifyWkPreUpCall	   |
//     	      |	    	| ^^^^^^^^^^^^^^^^^	   |
//notifyWkDone|	       	|     			   |
//^^^^^^^^^^^^|	    	|     			   |
//	      |	    	V     			   |
//            |    +---------+			   |
//     	      +----+ (1,0,1) +			   |notifyWkDone
//	      +--->+----+----+			   |
//     	      |	       	|     			   |
//	      |	   	|DataArrive		   |
//	      |	   	|     	   		   |
//	      |	   	V     	   		   |
//            |    +---------+			   |
//     	      |    | (0,1,1) +			   |
//	      |    +----+----+			   |
//	      |	       	|      		      	   |
//	      |	       	|notifyRzReadable     	   |	notifyWkPreUpCall
//notifyWkDone|	   	|      		      	   |   +--+
//	      |	   	| +-----------------------+|   |  |
//	      |	   	| | notifyWkPreUpCall(1)  ||   |  |
//	      |	   	V V ^^^^^^^^^^^^^^^^^  	  ||   V  |
//            |    +---------+         	          ++------+-+
//     	    +-|--->| (0,0,2) +------------------->| (0,0,2*)|
//	    | |    +----+----+    notifyWkDone    +---------+
//	    | |	   	|      	  ^^^^^^^^^^^^	     ^ ^
//	    | |	   	|  			     | |
//	    | |	   	|notifyWkPreUpCall	     | |
//	    | |	   	|  		  	     | |
//	    | |	   	|      	       	  	     | |
//	    | |	   	V  			     | |
//          | |    +---------+			     | |
//     	    | +----+ (1,0,2) +-----------------------+ |
//	    |   +->+----+----+	   notifyWkDone	       |notifyWkDone
//     	    +-+	|      	|  	   ^^^^^^^^^^^^	       |
//     	      |	|      	|  			       |
//	      |	|  	|DataArrive		       |
//	      |	|  	|  			       |
//	      |	|  	V  			       |
//            | |  +---------+			       |
//     	      |	|  | (0,1,2) +			       |
//	      | |  +----+----+			   +---+
//	      |	|      	|      		      	   |
//	      |	|      	|notifyRzReadable     	   |	notifyWkPreUpCall
//notifyWkDone|	|  	|      		      	   |   +--+
//	      |	|  	|     		      	   |   |  |
//	      |	|  	|      		      	   |   |  |
//	      |	|  	V      		      	   |   V  |
//            | |  +---------+       	          ++------+-+
//     	      +----+ (0,0,3) +------------------->| (0,0,3*)|
//	        |  +----+----+    notifyWkDone    +---------+
//     	       	|      	|      	  ^^^^^^^^^^^^	   ^
//	       	|      	|  		  	   |
//  notifyWkDone|  	|notifyWkPreUpCall	   |
//	       	|  	|  		  	   |
//	       	|  	|      	       	  	   |
//	       	|  	V  		  	   |
//              |  +---------+		  	   |
//     	        +--+ (1,0,3) +---------------------+
//     	           +----+----+	   notifyWkDone
//     	       	       	|      	   ^^^^^^^^^^^^
//
//   Note: (1) Race condition. Happen if dedicated thread gets the read lock
//             before the temporary worker.
//
//   And So On...
//
//

//   To summarise, here are what the transition function should do:
//     notifyRzReadable() 
//        - Always create a temporary worker
//
//     notifyWkPreUpCall()
//        dedicated thread - setSelectable unless n workers != 1
//        temporary thread - setSelectable only if the dedicated thread is
//                           in the upcall
//
//     notifyWkDone()
//        dedicated thread - clearSelectable
//        temporary thread - Do nothing
//
