// -*- Mode: C++; -*-
//                            Package   : omniORB
// orbMultiRequest.cc         Created on: 17/2/1999
//                            Author    : David Riddoch (djr)
//
//    Copyright (C) 1996-1999 AT&T Laboratories Cambridge
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
//   Implementation of CORBA::ORB::send_multiple_requests_... etc.
//

#include <omniORB3/CORBA.h>
#include <request.h>


struct RequestLink {
  inline RequestLink(CORBA::Request_ptr r) : request(r), next(0) {}

  CORBA::Request_ptr request;
  RequestLink* next;
};


static RequestLink* outgoing_q = 0;
static RequestLink* outgoing_q_tail = 0;  // undefined if outgoing_q == 0
// A queue of deferred requests whose results may not yet have
// arrived.

static RequestLink* incoming_q = 0;
static RequestLink* incoming_q_tail = 0;  // undefined if incoming_q == 0
// A queue of deferred requests who are known to have completed.

static unsigned queue_waiters = 0;
// The number of threads waiting for responses.

static omni_mutex q_lock;
// Lock for accessing the above data.

static omni_condition q_cv(&q_lock);
// This is signalled if( queue_waiters > 0 ) and there might be
// something to receive now.                           =====


CORBA::Status
CORBA::ORB::send_multiple_requests_oneway(const RequestSeq& rs)
{
  for( CORBA::ULong i = 0; i < rs.length(); i++ ) {
    try {
      rs[i]->send_oneway();
    }
    catch(CORBA::Exception& ex) {
      ((RequestImpl*) rs[i]._ptr)->storeExceptionInEnv();
    }
  }

  RETURN_CORBA_STATUS;
}


CORBA::Status
CORBA::ORB::send_multiple_requests_deferred(const RequestSeq& rs)
{
  unsigned nwaiters;

  {
    omni_mutex_lock sync(q_lock);

    for( CORBA::ULong i = 0; i < rs.length(); i++ ) {
      rs[i]->send_deferred();

      RequestLink* rl = new RequestLink(CORBA::Request::_duplicate(rs[i]));
      if( outgoing_q ) {
	outgoing_q_tail->next = rl;
	outgoing_q_tail = rl;
      } else
	outgoing_q = outgoing_q_tail = rl;
    }

    nwaiters = queue_waiters;
  }

  if( rs.length() >= nwaiters )  q_cv.broadcast();
  else
    for( CORBA::ULong i = 0; i < rs.length(); i++ )
      q_cv.signal();

  RETURN_CORBA_STATUS;
}


CORBA::Boolean
CORBA::ORB::poll_next_response()
{
  // If there is anything in the incoming queue, return true. Otherwise
  // check to see if anything in the outgoing queue has completed yet.
  // Transfer anything that has completed to the incoming queue, and
  // then return true or false appropriately.

  omni_mutex_lock sync(q_lock);

  if( incoming_q )  return 1;

  RequestLink** rlp = &outgoing_q;
  RequestLink*  rlp_1 = 0;

  while( *rlp ) {
    RequestLink* rl = *rlp;

    if( rl->request->poll_response() ) {
      // Transfer from outgoing to incoming queue.
      RequestLink* next = rl->next;
      rl->next = 0;
      if( incoming_q ) {
	incoming_q_tail->next = rl;
	incoming_q_tail = rl;
      } else
	incoming_q = incoming_q_tail = rl;
      *rlp = next;
      if (outgoing_q_tail == rl)
	outgoing_q_tail = rlp_1;
    } else {
      rlp = &rl->next;
      rlp_1 = rl;
    }
  }

  return incoming_q != 0;
}


static void
internal_get_response(CORBA::Request_ptr req)
{
  try {
    req->get_response();
  }
  catch(CORBA::SystemException& ex) {
    ((RequestImpl*) req)->storeExceptionInEnv();
  }
}


CORBA::Status
CORBA::ORB::get_next_response(Request_out req_out)
{
  {
    omni_mutex_lock sync(q_lock);

    // We need to block until we can grab something off one of the
    // queues.
    while( !(outgoing_q || incoming_q) ) {
      queue_waiters++;
      q_cv.wait();
      queue_waiters--;
    }

    // If we've received any replies, return one of those.
    if( incoming_q ) {
      req_out = incoming_q->request;
      RequestLink* next = incoming_q->next;
      delete incoming_q;
      incoming_q = next;
      internal_get_response(req_out._data);
      RETURN_CORBA_STATUS;
    }

    // Check the outgoing queue to see if any of them have completed.
    RequestLink** rlp = &outgoing_q;
    RequestLink*  rlp_1 = 0;

    while( *rlp ) {
      RequestLink* rl = *rlp;

      if( rl->request->poll_response() ) {
	*rlp = rl->next;
	req_out = rl->request;
	if (outgoing_q_tail == rl)
	  outgoing_q_tail = rlp_1;
	delete rl;
	internal_get_response(req_out._data);
	RETURN_CORBA_STATUS;
      } else {
	rlp = &rl->next;
	rlp_1 = rl;
      }
    }

    // Otherwise just block on the first one in the outgoing queue.
    req_out = outgoing_q->request;
    RequestLink* next = outgoing_q->next;
    delete outgoing_q;
    outgoing_q = next;
  }

  // This blocks until the reply is received for this message.
  internal_get_response(req_out._data);
  RETURN_CORBA_STATUS;
}
