// -*- Mode: C++; -*-
//                            Package   : omniORB
// tcpSocketMTImpl.cc         Created on: 19/5/2000
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
//      

#include <omniORB3/CORBA.h>
#include <omniORB3/omniORB.h>
#define TRUE 1
#define FALSE 0

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <ropeFactory.h>
#include <tcpSocket.h>
#include <exception.h>


#if defined(__WIN32__)

#include <winsock.h>
#include <sys/types.h>

#define RC_INADDR_NONE     INADDR_NONE
#define RC_INVALID_SOCKET  INVALID_SOCKET
#define RC_SOCKET_ERROR    SOCKET_ERROR
#define INETSOCKET         PF_INET
#define CLOSESOCKET(sock)  closesocket(sock)
#define SHUTDOWNSOCKET(sock) ::shutdown(sock,2)
#else

#include <sys/time.h>

#if !defined(__linux__) || !defined(__CIAO__)
#include <sys/socket.h>
#else
// This bit of ugly work around is to hide away the non ansi
// syntax in the headers that causes CCia to complain loudly.
#  ifndef __STRICT_ANSI__
#    define __STRICT_ANSI__
#    include <sys/socket.h>
#    undef  __STRICT_ANSI__
#  else
#    include <sys/socket.h>
#  endif
#endif

#if defined (__uw7__)
#ifdef shutdown
#undef shutdown
#endif
#endif

#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#if defined(__sunos__) && defined(__sparc__) && __OSVERSION__ >= 5
#include <sys/types.h>
#include <fcntl.h>
#endif

#define RC_INADDR_NONE     ((CORBA::ULong)-1)
#define RC_INVALID_SOCKET  (-1)
#define RC_SOCKET_ERROR    (-1)
#define INETSOCKET         AF_INET
#define CLOSESOCKET(sock)   close(sock)
#if defined(__sunos__) && defined(__sparc__) && __OSVERSION__ >= 5
#define SHUTDOWNSOCKET(sock)  ::shutdown(sock,2)
#elif defined(__osf1__) && defined(__alpha__)
#define SHUTDOWNSOCKET(sock)  ::shutdown(sock,2)
#else
  // XXX none of the above, calling shutdown() may not have the
  // desired effect.
#define SHUTDOWNSOCKET(sock)  ::shutdown(sock,2)
#endif
#endif

#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <iostream.h>

#include <libcWrapper.h>
#include <gatekeeper.h>


#if defined(NEED_GETHOSTNAME_PROTOTYPE)
extern "C" int gethostname(char *name, int namelen);
#endif

#if defined(__VMS) && defined(USE_tcpSocketVaxRoutines)
#include "tcpSocketVaxRoutines.h"
#undef accept
#undef recv
#undef send
#define accept(a,b,c) tcpSocketVaxAccept(a,b,c)
#define recv(a,b,c,d) tcpSocketVaxRecv(a,b,c,d)
#define send(a,b,c,d) tcpSocketVaxSend(a,b,c,d)
#endif


#define PTRACE(prefix,message)  \
  omniORB::logs(15, "tcpSocketMTfactory " prefix ": " message)

#include "tcpSocketMTInterface.h"

Dispatcher::Dispatcher(tcpSocketIncomingRope *r,
		       tcpSocketMTincomingFactory *f):
  pd_factory(f), pd_rope(r){
  PTRACE("Dispatcher", "Dispatcher");
}

// Wrapper around C library accept()
// Will block if no connections are pending
// On a fatal error will perform proper socket and thread closedown
// if the system ::accept() call fails, will throw COMM_FAILURE
handle_t Dispatcher::acceptConnection(){
  PTRACE("Dispatcher", "acceptConnection()");

  struct sockaddr_in raddr;

#if (defined(__GLIBC__) && __GLIBC__ >= 2)
  // GNU C library uses socklen_t * instead of int* in accept ().
  // This is suppose to be compatible with the upcoming POSIX standard.
  socklen_t l;
#elif defined(__aix__) || defined(__VMS) || defined(__SINIX__) || defined(__uw7__)
  size_t l;
#else
  int l;
#endif

  CORBA::Boolean die = 0;

  // If state is normal we return immediately with the new connection
  // Otherwise we loop around closing connections until told to stop
  while (pd_rope->pd_shutdown == tcpSocketIncomingRope::ACTIVE  && !die){
    try{
      l = sizeof(struct sockaddr_in);

      PTRACE("Dispatcher","try accept()");

      tcpSocketHandle_t new_sock;
      if ((new_sock = ::accept(pd_rope->pd_rendezvous,
			       (struct sockaddr *)&raddr,&l)) 
	  == RC_INVALID_SOCKET) {
#ifndef __WIN32__
	OMNIORB_THROW(COMM_FAILURE,errno,CORBA::COMPLETED_NO);
#else
	OMNIORB_THROW(COMM_FAILURE,::WSAGetLastError(),CORBA::COMPLETED_NO);
#endif
      }

      PTRACE("Dispatcher","accept() ok");
    
      {
	omni_mutex_lock sync(pd_rope->pd_lock);

	if (pd_rope->pd_shutdown != tcpSocketIncomingRope::ACTIVE) {
	  // It has been indicated that this thread should stop
	  // accepting connection request.
	  CLOSESOCKET(new_sock);
	  continue;
	}
      }

      return new_sock;

    } catch(CORBA::COMM_FAILURE&) {
      // XXX accepts failed. The probable cause is that the number of
      //     file descriptors opened has exceeded the limit.
      //     On unix, the value of this limit can be set and get using the
      //              ulimit command.
      //     On NT, if this is part of a DLL, the limit is 256(?)
      //            else the limit is 16(?)
      PTRACE("Dispatcher", "accept fails. Too many file descriptors opened?");
      // sleep(1) workaround removed- might conflict with threading policy
      throw;
    } catch (omniORB::fatalException& ex) {
      if (omniORB::trace(0)) {
        omniORB::logger l;
        l << "You have caught an omniORB bug, details are as follows:\n" <<
          " file: " << ex.file() << "\n"
          " line: " << ex.line() << "\n"
          " mesg: " << ex.errmsg() << "\n"
          " tcpSocketMTImp Dispatcher will not accept new connections.\n";
      }
      die = 1;
    } catch(...) {
      omniORB::logs(0,
       "Unexpected exception caught by tcpSocketMTImpl Dispatcher\n"
       " tcpSocketMTImpl Dispatcher thread will not accept new connection.");
      die = 1;
    }

  }
  if (die) {
    // Something very wrong has happened, before we quit, we have to
    // make sure that a future call by another thread to
    // tcpSocketIncomingRope::cancelThreads() would not wait indefinitely
    // on this thread to response to its connect() request to the
    // rendezous socket. 
    // Shutdown (not close) the rendezvous socket to make sure that the
    // connect() in cancelThreads() would fail.
    SHUTDOWNSOCKET(pd_rope->pd_rendezvous);
  }

  while (pd_rope->pd_shutdown != tcpSocketIncomingRope::NO_THREAD) {

    // We keep on accepting connection requests but close the new sockets
    // immediately until the state of pd_shutdown changes to NO_THREAD.
    // This is to make sure that the thread calling cancelThreads()
    // will be unblocked from the connect() call.

    {
      fd_set rdfds;
#ifndef __CIAO__
      FD_ZERO(&rdfds);
      FD_SET(pd_rope->pd_rendezvous,&rdfds);
#endif
      struct timeval t = { 1,0};
      int rc;
      if ((rc = select(pd_rope->pd_rendezvous+1,&rdfds,0,0,&t)) <= 0) {
#ifndef __WIN32__
        if (rc < 0 && errno != EINTR) {
          die = 1;
        }
#else
        if (rc < 0 && ::WSAGetLastError() != WSAEINTR) {
          die = 1;
        }
#endif
        PTRACE("Dispatcher",
	       "waiting on shutdown state to change to NO_THREAD.");
        continue;
      }
    }
    tcpSocketHandle_t new_sock;
    struct sockaddr_in raddr;
#if (defined(__GLIBC__) && __GLIBC__ >= 2)
    // GNU C library uses socklen_t * instead of int* in accept ().
    // This is suppose to be compatible with the upcoming POSIX standard.
    socklen_t l;
#elif defined(__aix__) || defined(__VMS) || defined(__SINIX__) || defined(__uw7__)
    size_t l;
#else
    int l;
#endif

    l = sizeof(struct sockaddr_in);
    if ((new_sock = ::accept(pd_rope->pd_rendezvous,
			     (struct sockaddr *)&raddr,&l)) 
              == RC_INVALID_SOCKET) 
      {
        die = 1;
        continue;
      }
    CLOSESOCKET(new_sock);

  }

  // Shutdown completed
  return -1;
}

AcceptDispatcher::AcceptDispatcher(tcpSocketIncomingRope *r,
				   tcpSocketMTincomingFactory *f):
  Dispatcher(r, f) {
  PTRACE("AcceptDispatcher", "AcceptDispatcher");
}

void AcceptDispatcher::watchHandle(handle_t){
  OMNIORB_ASSERT(NULL);
}

void AcceptDispatcher::ignoreHandle(handle_t){
  OMNIORB_ASSERT(NULL);
}

void AcceptDispatcher::waitForEvents(CORBA::Boolean loop_forever){
  PTRACE("AcceptDispatcher", "waitForEvents");

  CORBA::Boolean alive = TRUE;
  while (alive){
    alive = loop_forever;

    try{
      tcpSocketHandle_t new_sock = acceptConnection();

      //cerr << "new_sock = " << new_sock << endl;
      if (new_sock == -1){
	// shutting down
	PTRACE("AcceptDispatcher", "shutting down");
	return;
      }

      if (pd_ceh) pd_ceh->newConnectionAttempt(new_sock);

    } catch (CORBA::COMM_FAILURE&){
      // accept call failed - too many open file descriptors?
      PTRACE("AcceptDispatcher",
	     "accept fails. Too many file descriptors opened?");
      //omni_thread::sleep(1,0);
      continue;
    }

  }
}

SelectDispatcher::SelectDispatcher(tcpSocketIncomingRope *r,
				   tcpSocketMTincomingFactory *f):
  Dispatcher(r, f){
  PTRACE("SelectDispatcher", "SelectDispatcher");

  poller.add(pd_rope->pd_rendezvous);
}

void SelectDispatcher::watchHandle(handle_t x){
  PTRACE("SelectDispatcher", "watchHandle");
  //cerr << "fd = " << x << endl;
  poller.add(x);
}

void SelectDispatcher::ignoreHandle(handle_t x){
  PTRACE("SelectDispatcher", "ignoreHandle");
  //cerr << "fd = " << x << endl;
  poller.remove(x);
}

void SelectDispatcher::waitForEvents(CORBA::Boolean loop_forever){
  PTRACE("SelectDispatcher", "waitForEvents");

  CORBA::Boolean alive = TRUE;
  while (alive){
    alive = loop_forever;

    /*
    {
      PollSet_Active_Iterator set = poller.wait();
      cerr << "Active fds = {";
      socket_t fd;
      while ((fd = set()) != SOCKET_UNDEFINED)
	cerr << fd << " ";
      cerr << "}" << endl;
    }
    */
    //cerr << "Waiting {\n" << poller << "\n}\n";
    PollSet_Active_Iterator set = poller.wait();
    socket_t fd;
    while ((fd = set()) != SOCKET_UNDEFINED){
      //cerr << "active fd = " << fd << endl;
      if (fd == pd_rope->pd_rendezvous){
	PTRACE("SelectDispatcher", "new connection attempted");
	socket_t new_sock = acceptConnection();

	if (new_sock == -1){
	  // shutting down
	  PTRACE("SelectDispatcher", "shutting down");
	  return;
	}
	
	if (pd_ceh) pd_ceh->newConnectionAttempt(new_sock);
      }else{
	PTRACE("SelectDispatcher", "new data available on socket");
	
	if (pd_ceh) pd_ceh->dataAvailable(fd);
      }
    }
  }
}

void Worker::process(tcpSocketStrand *s){
  PTRACE("Worker", "process()");
  //cerr << "  fd = " << s->handle() << endl;

  try {
    GIOP_S::dispatcher(s);
  } catch (CORBA::COMM_FAILURE&) {
    PTRACE("Worker","#### Communication failure. Connection closed.");

    throw;
  } catch(const omniORB::fatalException& ex) {
    if( omniORB::trace(0) ) {
      omniORB::logger l;
      l << "You have caught an omniORB bug, details are as follows:\n"
	" file: " << ex.file() << "\n"
	" line: " << ex.line() << "\n"
	" mesg: " << ex.errmsg() << "\n";
    }
    throw;
  } catch (...) {
    omniORB::logs(0, "An exception has occured and was caught by"
		  " tcpSocketMTImpl Worker thread.");
    throw;
  }
  PTRACE("Worker", "process finished");
}

void OOWorker::run(void *arg){
  omniORB::giopServerThreadWrapper::
    getGiopServerThreadWrapper()->run(OOWorker::_realRun,arg);
  // the wrapper run() method will pass back control to OOWorker
  // by calling  _realRun(arg) when it is ready.
}

void OOWorker::_realRun(void *arg){
  OOWorker* me = (OOWorker*)arg;
  PTRACE("OOWorker", "start");

  CORBA::Boolean alive = TRUE;
  while (alive){
    try{
      me->process(me->pd_strand);
    }catch (CORBA::COMM_FAILURE&){
      // strand has died, worker should too
      PTRACE("OOWorker", "strand death detected");

      alive = FALSE;
    }catch(...){
      omniORB::logs(0, "Exception caught by OOWorker thread.");
      break;
    }
  }

  PTRACE("OOWorker", "end");
}

void QWorker::run(void *arg){
  omniORB::giopServerThreadWrapper::
    getGiopServerThreadWrapper()->run(QWorker::_realRun,arg);
  // the wrapper run() method will pass back control to OOWorker
  // by calling  _realRun(arg) when it is ready.
}

void QWorker::_realRun(void *arg){
  QWorker* me = (QWorker*)arg;
  PTRACE("QWorker", "start");

  CORBA::Boolean alive = TRUE;
  while (alive){
    try{
      tcpSocketStrand *s = me->pd_q->remove();
      me->process(s);

      me->pd_d->watchHandle(s->handle());
    }catch (CORBA::COMM_FAILURE&){
      // FIXME: how does the worker shut down?
      PTRACE("QWorker", "strand death detected");
    }catch(...){
      omniORB::logs(0, "Exception caught by QWorker thread.");
      break;
    }
  }

  PTRACE("QWorker", "end");
}


Controller *Controller::instance(tcpSocketIncomingRope *r,
				 tcpSocketMTincomingFactory *f){
  switch (omniORB::concurrencyModel){
  case omniORB::PerConnection:
    return new OOPolicyController(r, f);
  case omniORB::QueueBased:
    return new QPolicyController(r, f);
  case omniORB::LeaderFollower:
    return new LeaderFollower(r, f);
  default:
    throw omniORB::fatalException(__FILE__, __LINE__,
				  "Unknown concurrency model selected");
  };
}

OOPolicyController::OOPolicyController(tcpSocketIncomingRope *r,
				       tcpSocketMTincomingFactory *f):
  omni_thread(this), pd_rope(r), pd_factory(f), pd_dispatcher(r, f){ 
  PTRACE("OOPolicyController", "created");
  
  pd_dispatcher.register_CommsEventHandler(this);
  
  start_undetached();
}

OOPolicyController::~OOPolicyController(){
  PTRACE("OOPolicyController", "destroyed");

}

void *OOPolicyController::run_undetached(void *arg){

#if defined(__sunos__) && defined(__sparc__) && __OSVERSION__ >= 5
#if defined(__SUNPRO_CC) && __SUNPRO_CC <= 0x420
  set_terminate(abort);
#endif
#endif
  PTRACE("OOPolicyController","thread start.");

  pd_dispatcher.waitForEvents();

  PTRACE("OOPolicyController", "thread exiting");
  return NULL;
}

void OOPolicyController::newConnectionAttempt(handle_t socket){
  PTRACE("OOPolicyController", "newConnectionAttempt");

  tcpSocketStrand *s = new tcpSocketStrand(pd_rope, socket);
  s->incrRefCount(1); // about to hand over to worker

  OOWorker *newthr;

  omni_mutex_lock sync(pd_factory->pd_shutdown_lock);
  if (pd_factory->pd_shutdown_nthreads >= 0) {
    pd_factory->pd_shutdown_nthreads++;
  }
  else {
    pd_factory->pd_shutdown_nthreads--;
  }

  try{
    newthr = new OOWorker(pd_factory, s);
  }catch(...){
    newthr = NULL;
  }
  if (!newthr){
    // Cannot create a new thread to serve the strand
    // We have no choice but to shutdown the strand.
    // The long term solutions are:  start multiplexing the new strand
    // and the rendezvous; close down idle connections; reasign
    // threads to strands; etc.
    s->decrRefCount();
    s->real_shutdown();

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

}

void OOPolicyController::dataAvailable(handle_t){ 
  OMNIORB_ASSERT(NULL); 
}
void OOPolicyController::connectionClosed(handle_t){ 
  OMNIORB_ASSERT(NULL); 
}


QPolicyController::QPolicyController(tcpSocketIncomingRope *r,
				     tcpSocketMTincomingFactory *f,
				     unsigned int queueLength,
				     int nWorkers):
  omni_thread(this), pd_rope(r), pd_factory(f), pd_dispatcher(r, f),
  pd_queue(queueLength){ 
  PTRACE("QPolicyController", "created");
  
  pd_dispatcher.register_CommsEventHandler(this);

  pd_maxstrands = queueLength;

  pd_strandmap = new (tcpSocketStrand*)[pd_maxstrands];

  if (pd_strandmap == NULL)
    throw omniORB::fatalException(__FILE__, __LINE__,
				  "Cannot allocate a socket handle -> Strand map");
  
  for (unsigned int i=0;i<pd_maxstrands;i++)
    *(pd_strandmap + i) = NULL;

  if (nWorkers == -1)
    nWorkers = omniORB::threadPoolSize;
  
  PTRACE("QPolicyController", "building pool of workers");
  for (int i=0;i<nWorkers;i++)
    new QWorker(pd_factory, &pd_queue, &pd_dispatcher);

  start_undetached();
}

QPolicyController::~QPolicyController(){
  PTRACE("QPolicyController", "destroyed");

  delete[] pd_strandmap;
}

void *QPolicyController::run_undetached(void *arg){

#if defined(__sunos__) && defined(__sparc__) && __OSVERSION__ >= 5
#if defined(__SUNPRO_CC) && __SUNPRO_CC <= 0x420
  set_terminate(abort);
#endif
#endif
  PTRACE("QPolicyController","thread start.");

  pd_dispatcher.waitForEvents();

  PTRACE("QPolicyController", "thread exiting");
  return NULL;
}

void QPolicyController::newConnectionAttempt(handle_t socket){
  PTRACE("QPolicyController", "newConnectionAttempt");

  pd_dispatcher.watchHandle(socket);

  tcpSocketStrand *newSt = new tcpSocketStrand(pd_rope, socket);
  newSt->incrRefCount(1);

  OMNIORB_ASSERT(socket < (signed int)pd_maxstrands);

  pd_strandmap[socket] = newSt;
}

void QPolicyController::dataAvailable(handle_t socket){
  PTRACE("QPolicyController", "dataAvailable");

  pd_dispatcher.ignoreHandle(socket);

  OMNIORB_ASSERT(socket < (signed int)pd_maxstrands);

  pd_queue.add(pd_strandmap[socket]);

}

void QPolicyController::connectionClosed(handle_t socket){
  PTRACE("QPolicyController", "connectionClosed");

  OMNIORB_ASSERT(NULL);
}

SelectDispatcher *LeaderFollower::pd_dispatcher;

LeaderFollower::LeaderFollower(tcpSocketIncomingRope *r,
			       tcpSocketMTincomingFactory *f,
			       int nFollowers,
			       CORBA::Boolean special):
  Worker(), omni_thread(this), pd_rope(r), pd_factory(f), 
  pd_strand(NULL), pd_amspecial(special){
  PTRACE("LeaderFollower", "LeaderFollower");


  pd_maxstrands = getdtablesize();

  pd_strandmap = new (tcpSocketStrand*)[pd_maxstrands];

  if (pd_strandmap == NULL)
    throw omniORB::fatalException(__FILE__, __LINE__,
				  "Cannot allocate a socket handle -> Strand map");
  
  for (unsigned int i=0;i<pd_maxstrands;i++)
    *(pd_strandmap + i) = NULL;

  // special == looks after the static data
  if (pd_amspecial)
    pd_dispatcher = new SelectDispatcher(r, f);


  if (nFollowers == -1)
    nFollowers = omniORB::threadPoolSize;

  // create a group of peers
  for (int i=0;i<(nFollowers-1);i++)
    new LeaderFollower(r, f, 0, FALSE);

  start_undetached();
}

unsigned int LeaderFollower::pd_maxstrands;
tcpSocketStrand **LeaderFollower::pd_strandmap;

LeaderFollower::~LeaderFollower(){
  PTRACE("LeaderFollower", "~LeaderFollower");

  if (pd_amspecial)
    delete pd_dispatcher;
}


omni_mutex LeaderFollower::pd_leader;

void *LeaderFollower::run_undetached(void *arg){

#if defined(__sunos__) && defined(__sparc__) && __OSVERSION__ >= 5
#if defined(__SUNPRO_CC) && __SUNPRO_CC <= 0x420
  set_terminate(abort);
#endif
#endif
  PTRACE("LeaderFollower","thread start.");

  // need to consider shutdown
  while (1){
    while (pd_strand == NULL){
      PTRACE("LeaderFollower", "becoming leader");
      {
	omni_mutex_lock lock(LeaderFollower::pd_leader);

	// tell me about the events
	pd_dispatcher->register_CommsEventHandler(this);
	
	pd_dispatcher->waitForEvents(FALSE);
	
      }
    }
    // we have work to do on pd_strand
    try{
      process(pd_strand);
      pd_dispatcher->watchHandle(pd_strand->handle());
      pd_strand->decrRefCount();
      pd_strand = NULL;
    }catch (CORBA::COMM_FAILURE&){
      // FIXME: how does the worker shut down?
      PTRACE("LeaderFollower", "strand death detected");
      *(pd_strandmap + (pd_strand->handle())) = NULL;

      pd_strand->decrRefCount();
      pd_strand->real_shutdown();

      //cerr << "Strand died, fd = " << pd_strand->handle() << endl;
      // ?
      //OMNIORB_ASSERT(close(pd_strand->handle()) != 0);

      pd_strand = NULL;

    }catch(...){
      omniORB::logs(0, "Exception caught by QWorker thread.");
      pd_strand->decrRefCount();
      pd_strand->real_shutdown();

      OMNIORB_ASSERT(close(pd_strand->handle()) != 0);

      pd_strand = NULL;
      break;
    }
  }
  PTRACE("LeaderFollower", "thread exiting");
  return NULL;

}

void LeaderFollower::newConnectionAttempt(handle_t socket){
  PTRACE("LeaderFollower", "newConnectionAttempt");

  //cerr << "newConnection socket = " << socket << endl;

  tcpSocketStrand *newSt = new tcpSocketStrand(pd_rope, socket);
  OMNIORB_ASSERT((unsigned int)socket < pd_maxstrands);
  *(pd_strandmap + socket) = newSt;

  pd_dispatcher->watchHandle(socket);
}

void LeaderFollower::dataAvailable(handle_t socket){
  PTRACE("LeaderFollower", "dataAvailable");

  //cerr << "dataAvailable socket = " << socket << endl;

  // if we don't already have a task in hand
  if (pd_strand == NULL){
    pd_dispatcher->ignoreHandle(socket);
  
    OMNIORB_ASSERT((unsigned int)socket < pd_maxstrands);
    pd_strand = *(pd_strandmap + socket);
    OMNIORB_ASSERT(pd_strand != NULL);
    
    pd_strand->incrRefCount(1);
  }
}

void LeaderFollower::connectionClosed(handle_t socket){
  OMNIORB_ASSERT(NULL);

}

#if 0
ControllerDelegate::ControllerDelegate(tcpSocketIncomingRope *r,
				       tcpSocketMTincomingFactory *f){
  switch (omniORB::concurrencyModel){
  case PerConnection:
    delegate = new OOPolicyController(r, f);
    return;
  case QueueBased:
    delegate = new QPolicyController(r, f);
    return;
  case LeaderFollower:
    delegate = new LeaderFollower(r, f);
    return;
  default:
    throw omniORB::fatalError(__FILE__, __LINE__,
			      "Unknown concurrency model selected");
  };
}

ControllerDelegate::~ControllerDelegate(){
  delete delegate;
}
#endif
