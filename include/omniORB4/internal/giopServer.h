// -*- Mode: C++; -*-
//                            Package   : omniORB
// giopServer.h                 Created on: 20 Dec 2000
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
  Revision 1.1.2.1  2001/02/23 16:47:04  sll
  Added new files.

*/

#ifndef __GIOPSERVER_H__
#define __GIOPSERVER_H__

#include <omniORB4/omniutilities.h>
#include <omniORB4/giopEndpoint.h>

OMNI_NAMESPACE_BEGIN(omni)

class giopRendezvouser;
class giopWorker;
class giopStrand;

class giopServer {
public:
  giopServer();
  ~giopServer();

  CORBA::Boolean instantiate(giopEndpoint*);
  // Accept to serve requests coming in from this endpoint .
  // If start() has already been called, incoming
  // requests from this endpoint will be served immediately.
  //
  // Returns TRUE(1) if the instantiation is successful,
  // otherwise returns FALSE(0).
  //
  // If return TRUE(1), the argument endpoint is consumed
  // by the server. Otherwise, the caller should free the endpoint's storage.
  //
  // This function does not raise an exception.
  // 
  // Thread Safety preconditions:
  //    None. Concurrent calls to methods of this object is synchronised 
  //    internally.

  void start();
  // When this function returns, the server will service requests.
  //
  // This function does not raise an exception.
  //
  // Thread Safety preconditions:
  //    None. Concurrent calls to methods of this object is synchronised 
  //    internally.

  void stop();
  // When this function returns, the server will no longer serve the requests.
  // Existing strands will be shutdown. 
  // However, the endpoints will stay.
  // This server will serve incoming requests again when start() is called.
  //
  // This function does not raise an exception.
  //
  // Thread Safety preconditions:
  //    None. Concurrent calls to methods of this object is synchronised 
  //    internally.

  void remove();
  // When this function returns, all endpoints will be removed.
  //
  // This function does not raise an exception.
  //
  // Thread Safety preconditions:
  //    None. Concurrent calls to methods of this object is synchronised 
  //    internally.

  friend class giopRendezvouser;
  friend class giopWorker;

  class Link {
  public:
    Link* next;
    Link* prev;

    Link() : next(this), prev(this) {}

    void insert(Link& head);
    void remove();
    static CORBA::Boolean is_empty(Link& head);

  private:
    Link(const Link&);
    Link& operator=(const Link&);
  };


private:
  enum { IDLE, ACTIVE, ZOMBIE, INFLUX }  pd_state;
  giopEndpointList                       pd_endpoints;
  Link                                   pd_rendezvousers;
  Link                                   pd_workers;
  omni_tracedmutex                       pd_lock;
  omni_tracedcondition                   pd_cond;

  void activate();
  // Activate all endpoints in pd_endpoints. This involves instantiating a
  // giopRendezvouser for each of the endpoints.
  //
  // This function does not raise an exception.
  //
  // Thread Safety preconditions:
  //    Caller must hold pd_lock.

  void deactivate();
  // deactivate all endpoints in pd_endpoints. This involves terminating
  // all giopRendezvousers and giopWorkers. For each giopRendezvouser, 
  // its endpoint is reentered into pd_endpoints.
  //
  // This function does not raise an exception.
  //
  // Thread Safety preconditions:
  //    Caller must hold pd_lock.

  void ensureNotInFlux();
  // ensure that the state of the server is not INFLUX.
  // If pd_state == INFLUX, block waiting until it changes out of that state.
  //
  // This function does not raise an exception.
  //
  // Thread Safety preconditions:
  //    Caller must hold pd_lock.

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////
  //
  // Callback functions used by giopRendezvouser
  //
  void notifyRzNewConnection(giopRendezvouser*,giopConnection*);
  // Callback by giopRendezvouser when a new connection is accepted.
  // If no exception is raised by the call, the connection is consumed.
  // Otherwise the caller must free the connection.
  //
  // notifyRzNewConnection could throw:
  //    outOfResource to indicate it cannot accept the new strand.
  //    Terminate     to indicate that the caller should stop serving
  //                     this connection immediately
  //
  // Thread Safety preconditions:
  //    Caller MUST NOT hold pd_lock. The lock is acquired by this method.
  //
  class outOfResource {};
  class Terminate {};

  void notifyRzDone(giopRendezvouser*,CORBA::Boolean exit_on_error);
  // Callback by giopRendezvouser when the task is about to end.
  // The flag exit_on_error indicates whether the task ends because it
  // was told or trigged by an error.
  //
  // Thread Safety preconditions:
  //    Caller MUST NOT hold pd_lock. The lock is acquired by this method.
  //


  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////
  //
  // Callback functions used by giopWorker
  //
  void notifyWkDone(giopWorker*,CORBA::Boolean exit_on_error);
  // Callback by giopWorker when the task is about to end.
  //
  // The flag exit_on_error indicates whether the task ends because it
  // was told or trigged by an error.
  //
  // Thread Safety preconditions:
  //    Caller MUST NOT hold pd_lock. The lock is acquired by this method.
  //

public:
  void notifyWkPreUpCall(giopStrand*);
  // Callback by the thread performing the giopWorker task when it
  // is about to perform an upcall to the applicaton level code.
  // This is an indication that from this point onwards, the thread will not
  // be reading from the strand. The server may want to start watching the
  // strand for any new request coming in.
  //
  // Thread Safety preconditions:
  //    Caller MUST NOT hold pd_lock. The lock may be acquired by this method.
  //

};

OMNI_NAMESPACE_END(omni)

#endif // __GIOPSERVER_H__
