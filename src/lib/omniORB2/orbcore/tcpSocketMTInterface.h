// -*- Mode: C++; -*-
//                            Package   : omniORB
// tcpSocketMTInterface.h     Created on: 19/5/2000
//                            Author    : David Scott (djs)
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
//	Description of the interfaces between Client facing network
//      connections, controller (glue) logic implementing a threading 
//      policy and entities responsible for executing requests

// Note that no threading policy will be visible in any of these classes

// Network interface communicates with controller by "sending CommsEvents"


#include <tcpSocket.h>

#include "select.h"
#include "queue.h"

class tcpSocketIncomingRope;
class tcpSocketMTincomingFactory;

typedef tcpSocketHandle_t fd_t;

class CommsEventHandler{
public:
  virtual void newConnectionAttempt(fd_t) = 0;
  virtual void dataAvailable(fd_t) = 0;
  virtual void connectionClosed(fd_t) = 0;
};

class CommsEventSource{
protected:
  CommsEventHandler *pd_ceh;
public:
  virtual void register_CommsEventHandler(CommsEventHandler* ceh){
    pd_ceh = ceh;
  }
};


// A Dispatcher is an instance of a CommsEventSource which watches
// socket(s) and sends events to an interested entity

class Dispatcher: public CommsEventSource{
protected:
  tcpSocketMTincomingFactory *pd_factory;
  tcpSocketIncomingRope *pd_rope;

  fd_t acceptConnection();

public:
  Dispatcher(tcpSocketIncomingRope *r,
	       tcpSocketMTincomingFactory *f);
  virtual ~Dispatcher() { }
 
  virtual void watchHandle(fd_t) = 0;
  virtual void ignoreHandle(fd_t) = 0;

  virtual void waitForEvents(CORBA::Boolean loop_forever = TRUE) = 0;
};

// Watches a single socket by accept()ing on it
// (doesn't do any select()ing/ waitForMultipleObject()s)
class AcceptDispatcher: public Dispatcher{
private:
  AcceptDispatcher();  

public:
  AcceptDispatcher(tcpSocketIncomingRope *r,
		   tcpSocketMTincomingFactory *f);
  virtual ~AcceptDispatcher() { }

  void watchHandle(fd_t);
  void ignoreHandle(fd_t);

  void waitForEvents(CORBA::Boolean loop_forever = TRUE);
};

// Watches a set of sockets by select()ing on them
class SelectDispatcher: public Dispatcher{
private:
  SelectDispatcher();

  Poller poller;

public:
  SelectDispatcher(tcpSocketIncomingRope *r,
		   tcpSocketMTincomingFactory *f);
  virtual ~SelectDispatcher() { }

  void watchHandle(fd_t);
  void ignoreHandle(fd_t);

  void waitForEvents(CORBA::Boolean loop_forever = TRUE);
};

// Abstract base class for each worker implementation.
// Says nothing about where the work actually comes from...
class Worker{
protected:
  Worker() { }
  virtual void process(tcpSocketStrand *);
};

// Worker implementation representing a One to One mapping of
// Threads to Strands
class OOWorker: public Worker, public omni_thread{
public:
  OOWorker(tcpSocketMTincomingFactory *f,
	   tcpSocketStrand *s,
	   CORBA::Boolean start_detached = TRUE):
    Worker(), omni_thread(this), pd_factory(f), pd_strand(s){
    if (start_detached)
      start();
    s->decrRefCount();
  }
  virtual ~OOWorker(){
    omni_mutex_lock sync(pd_factory->pd_shutdown_lock);
    OMNIORB_ASSERT(pd_factory->pd_shutdown_nthreads != 0);
    if (pd_factory->pd_shutdown_nthreads > 0) {
      pd_factory->pd_shutdown_nthreads--;
    }
    else {
      pd_factory->pd_shutdown_nthreads++;
      pd_factory->pd_shutdown_cond.signal();
    }
  }
  virtual void run(void *arg);
  static void _realRun(void *arg);

private:
  tcpSocketMTincomingFactory* pd_factory;
  tcpSocketStrand *pd_strand;
  Strand::Sync *pd_sync;
};

// Worker implementation which takes requests of a request queue
class QWorker: public Worker, public omni_thread{
public:
  QWorker(tcpSocketMTincomingFactory *f,
	  FixedQueue<tcpSocketStrand*> *q,
	  Dispatcher *d,
	  CORBA::Boolean start_detached = TRUE):
    Worker(), omni_thread(this), pd_factory(f), pd_q(q), pd_d(d){
    if (start_detached)
      start();
  }
  virtual ~QWorker(){
    omni_mutex_lock sync(pd_factory->pd_shutdown_lock);
    OMNIORB_ASSERT(pd_factory->pd_shutdown_nthreads != 0);
    if (pd_factory->pd_shutdown_nthreads > 0) {
      pd_factory->pd_shutdown_nthreads--;
    }
    else {
      pd_factory->pd_shutdown_nthreads++;
      pd_factory->pd_shutdown_cond.signal();
    }
  }
  virtual void run(void *arg);
  static void _realRun(void *arg);
private:
  tcpSocketMTincomingFactory *pd_factory;
  FixedQueue<tcpSocketStrand*> *pd_q;
  Dispatcher *pd_d;
  Strand::Sync *pd_sync;
};

class nobody;

class Controller{
protected:
  Controller() {}
  virtual ~Controller() {}
public:
  virtual void join(int){}

  static Controller* instance(tcpSocketIncomingRope *r,
			      tcpSocketMTincomingFactory *f);

  friend nobody;
};

class OOPolicyController: public Controller,
			  public CommsEventHandler, 
			  public omni_thread{
  tcpSocketIncomingRope* pd_rope;
  tcpSocketMTincomingFactory* pd_factory;

  AcceptDispatcher pd_dispatcher;
  
public:
  OOPolicyController(tcpSocketIncomingRope *r,
		     tcpSocketMTincomingFactory *f);
  virtual ~OOPolicyController();

  virtual void *run_undetached(void *arg);

  virtual void newConnectionAttempt(fd_t);

  virtual void dataAvailable(fd_t);
  virtual void connectionClosed(fd_t);
};

class QPolicyController: public Controller,
			 public CommsEventHandler, 
			 public omni_thread{
  tcpSocketIncomingRope *pd_rope;
  tcpSocketMTincomingFactory *pd_factory;

  SelectDispatcher pd_dispatcher;

  FixedQueue<tcpSocketStrand*> pd_queue;
  unsigned long int pd_maxstrands;
  tcpSocketStrand **pd_strandmap;

public:
  QPolicyController(tcpSocketIncomingRope *r,
		    tcpSocketMTincomingFactory *f,
		    unsigned int queueLength = 1024,
		    int nWorkers = -1);
  virtual ~QPolicyController();

  virtual void *run_undetached(void *arg);

  virtual void newConnectionAttempt(fd_t);

  virtual void dataAvailable(fd_t);
  virtual void connectionClosed(fd_t);
};

class LeaderFollower: public Controller,
		      public Worker, 
		      public CommsEventHandler,
		      public omni_thread{

  tcpSocketIncomingRope *pd_rope;
  tcpSocketMTincomingFactory *pd_factory;

  tcpSocketStrand *pd_strand;
  CORBA::Boolean pd_amspecial;


  static SelectDispatcher *pd_dispatcher;
  static unsigned int pd_maxstrands;
  static tcpSocketStrand **pd_strandmap;

public:
  LeaderFollower(tcpSocketIncomingRope *r,
		 tcpSocketMTincomingFactory *f,
		 int nFollowers = -1,
		 CORBA::Boolean special = TRUE);
  virtual ~LeaderFollower();

  static omni_mutex pd_leader;

  virtual void *run_undetached(void *arg);
  
  virtual void newConnectionAttempt(fd_t);
  virtual void dataAvailable(fd_t);
  virtual void connectionClosed(fd_t);

};

#if 0
class ControllerDelegate{

  omni_thread *delegate;

public:
  ControllerDelegate(tcpSocketIncomingRope *r,
		     tcpSocketMTincomingFactory *f);
  ~ControllerDelegate();

  void join(void **x) { delegate->join(x); }

}
#endif
