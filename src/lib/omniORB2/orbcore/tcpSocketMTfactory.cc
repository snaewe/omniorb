// -*- Mode: C++; -*-
//                            Package   : omniORB
// tcpSocketMTfactory.cc      Created on: 18/3/96
//                            Author    : Sai Lai Lo (sll)
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
//	Implementation of the Strand using TCP/IP and BSD socket interface
//	

/*
  $Log$
  Revision 1.22.6.10.2.3  2000/03/01 17:08:35  djs
  Restructuring of Rendezvouser into:
                     Rendezvouser
                    (abstract base)
                  /               \
                 /                 \
  OneToOneRendezvouser           PoolRendezvouser
    (implements 1-1)              (abstract mplex)
                                 /                \
                                /                  \
                    SelectSignalRendezvouser        ?? another wakeup
                    (uses select() and kill())      mechanism

  SelectSignalRendezvouser still has a concurrency fault leading to a
  SIGSEGV.

  Revision 1.22.6.10.2.2  2000/02/24 11:47:26  djs
  Pieces connected together and tested.

  Revision 1.22.6.10.2.1  2000/02/22 11:04:57  djs
  Rendezvouser blocks in select() rather than accept()
  Rendezvouser maintains a bitmap of interesting FDs (in preparation for
    it monitoring all the network requests)
  Rendezvouser can be woken up from its blocked state by a worker
    sending it a SIGQUIT. This can only be done while the rendezvouser
    is in select() as at all other times it holds an omni_mutex guard.
  Added a fixed length thread safe Queue template to hold pending requests.

  The 1-1 mapping of threads to connections is still active.

  Revision 1.22.6.10  2000/01/28 15:57:10  djr
  Removed superflouous ref counting in Strand_iterator.
  Removed flags to indicate that Ropes and Strands are heap allocated.
  Improved allocation of client requests to strands.

  Revision 1.22.6.9  1999/12/06 14:02:47  djr
  Fixed bug in tcpSocketMTfactory.cc:dumpbuf().

  Revision 1.22.6.8  1999/10/27 18:20:40  sll
  Fixed the ctor of orker so that if thread create fails, the
  exception raised by omnithread does not cause assertion failure in the
  ORB.

  Revision 1.22.6.7  1999/10/16 13:22:55  djr
  Changes to support compiling on MSVC.

  Revision 1.22.6.6  1999/10/14 16:22:18  djr
  Implemented logging when system exceptions are thrown.

  Revision 1.22.6.5  1999/09/30 12:26:19  djr
  Minor change to remove compiler warning.

  Revision 1.22.6.4  1999/09/27 13:26:23  djr
  Updates to loggin to ensure prefix is always omniORB:

  Revision 1.22.6.3  1999/09/27 11:01:13  djr
  Modifications to logging.

  Revision 1.22.6.2  1999/09/24 15:01:39  djr
  Added module initialisers, and sll's new scavenger implementation.

  Revision 1.22.2.3  1999/09/22 19:56:52  sll
  Correct typo.

  Revision 1.22.2.2  1999/09/22 12:10:01  sll
  Merged port to SCO Unixware 7.

  Revision 1.22.2.1  1999/09/21 20:37:18  sll
  -Simplified the scavenger code and the mechanism in which connections
   are shutdown. Now only one scavenger thread scans both incoming
   and outgoing connections. A separate thread do the actual shutdown.
  -omniORB::scanGranularity() now takes only one argument as there is
   only one scan period parameter instead of 2.
  -Trace messages in various modules have been updated to use the logger
   class.
  -ORBscanGranularity replaces -ORBscanOutgoingPeriod and
                                 -ORBscanIncomingPeriod.

  Revision 1.22  1999/08/30 16:54:24  sll
  Wait much less time in tcpSocketStrand::shutdown. Added trace message.

  Revision 1.21  1999/07/09 21:03:58  sll
  removeIncoming() now waits until all tcpSocketWorker threads have exited
  before returning.

  Revision 1.20  1999/06/28 17:38:01  sll
  Added packet dump routines in ll_send and ll_recv. Enabled at traceLevel 25.
  Added openvms change.

  Revision 1.19  1999/06/26 18:10:52  sll
  Use WSAGetLastError() on win32 to get the errno correctly.

  Revision 1.18  1999/05/31 14:03:06  sll
  Remove set_terminate workaround when compiled with SUN C++ 5.0.

  Revision 1.17  1999/05/22 17:40:11  sll
  Added #ifdef for ciao so that CCia would not complain about gnu/linux
  specifics.

  Revision 1.16  1999/03/11 16:25:57  djr
  Updated copyright notice

  Revision 1.15  1999/02/19 12:22:33  djr
  Fixed typo: sunsos -> sunos

  Revision 1.14  1998/11/09 10:56:57  sll
  Removed the use of the reserved keyword "export".

  Revision 1.13  1998/09/23 15:31:15  sll
  Previously, tcpSocketStrand::shutdown sends an incomplete GIOP
  CloseConnection message (the message length field is missing). Fixed.

  Revision 1.12  1998/09/23 08:48:34  sll
  Use config variable omniORB::maxTcpConnectionPerServer to determine the
  maximum number of outgoing per tcpSocketOutgoingRope.

  Revision 1.11  1998/08/14 13:54:38  sll
  Added pragma hdrstop to control pre-compile header if the compiler feature
  is available.

  Revision 1.10  1998/06/29 17:13:30  sll
  Fixed Solaris specific code in realConnect. Now switch the socket back
  to blocking mode after connect() until all circumstance.

  Revision 1.9  1998/04/22 16:39:50  sll
  Added try-catch loop to guard against exception raised by the thread library
  when it cannot create a new thread for tcpSocketWorker.

  Revision 1.8  1998/04/08 16:06:49  sll
  Added support for Reliant UNIX 5.43

  Revision 1.7  1998/04/07 19:39:40  sll
  Replace cerr with omniORB::log.

  Revision 1.6  1998/03/19 19:53:14  sll
  Delay connect to the remote address space until the first send or recv.
  Previously, connect was made inside the ctor of tcpSocketStrand.

  Revision 1.5  1998/03/04 14:44:36  sll
  Updated to use omniORB::giopServerThreadWrapper.

// Revision 1.4  1998/01/20  17:32:38  sll
// Added support for OpenVMS.
//
  Revision 1.3  1997/12/18 17:27:39  sll
  Updated to work under glibc-2.0.

  Revision 1.2  1997/12/12 18:44:11  sll
  Added call to gateKeeper.

  Revision 1.1  1997/12/09 18:43:18  sll
  Initial revision

  */

#include <omniORB3/CORBA.h>
#define TRUE 1
#define FALSE 0

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <ropeFactory.h>
#include <tcpSocket.h>
#include <exception.h>

// the Fixed Length thread safe queue template
#include <queue.h>

// we need semaphores to modify data in a signal handler
// (not on sparc though)
#include <semaphore.h>

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

#undef EXTRADEBUG
#define THREADPOOL

const unsigned int poolSize = 2;
class tcpStrandWorker;
class OneToOneWorker;
class PoolWorker;

class tcpSocketRendezvouser;
class OneToOneRendezvouser;
class PoolRendezvouser;
#ifdef ONETOONE
# define RENDEZVOUSER OneToOneRendezvouser
#endif
#ifdef THREADPOOL
# define RENDEZVOUSER SelectSignalRendezvouser
#endif

////////////////////////////////////////////////////////////////////
// A tcpSocketRendezvouser is a thread which waits for events to
// occur on its socket(s) and then does something appropriate 
//
// On top of this simple abstraction we can build units with
// specific task-allocation policies (eg handle data here, multiplex
// onto thread pool or spawn a worker solely for this event)
class tcpSocketRendezvouser: public omni_thread{
public:
  tcpSocketRendezvouser(tcpSocketIncomingRope *r,
			tcpSocketMTincomingFactory *f);

  virtual ~tcpSocketRendezvouser() { }
  virtual void* run_undetached(void *arg);

protected:
  // causes the rendezvouser to watch for events on this strand too
  virtual void watchStrand(tcpSocketStrand *s) = 0;

  // called whenever a new connection attempt is made on the listening
  // socket
  virtual void newConnectionAttempted(tcpSocketStrand *s) = 0;

  // called if/whenever a strand being monitored has data available
  virtual void strandIsActive(tcpSocketStrand *s) = 0;

  // the main event loop
  virtual void waitForEvents(tcpSocketIncomingRope *r) = 0;

  tcpSocketMTincomingFactory* pd_factory;

private:
  tcpSocketRendezvouser();
};

#ifdef ONETOONE
class OneToOneRendezvouser: public tcpSocketRendezvouser{
public:
  OneToOneRendezvouser(tcpSocketIncomingRope *r,
		       tcpSocketMTincomingFactory *f):
    tcpSocketRendezvouser(r, f) {
    start_undetached();
  }

  virtual ~OneToOneRendezvouser() { }

protected:
  void watchStrand(tcpSocketStrand *s){
    throw omniORB::fatalException(__FILE__,__LINE__,
	  "OneToOneRendezvouser never watches strands");
  }
  void strandIsActive(tcpSocketStrand *s){
    throw omniORB::fatalException(__FILE__,__LINE__,
	  "OneToOneRendezvouser doesnt care about active strands");
  }
  void newConnectionAttempted(tcpSocketStrand*);
  void waitForEvents(tcpSocketIncomingRope*);

private:
  OneToOneRendezvouser();
};
#endif

#ifdef THREADPOOL
class PoolRendezvouser: public tcpSocketRendezvouser{
public:
  PoolRendezvouser(tcpSocketIncomingRope *r,
		   tcpSocketMTincomingFactory *f);

  virtual ~PoolRendezvouser() { };

  friend PoolWorker;
protected:

  // Fixed length queue of requests to be processed
  const static int queueLength = 5; // relate this to the getdtablesize()?
  FixedQueue<tcpSocketStrand*, queueLength> pd_q;

  // Mapping of filedescriptors to strands
  // (assumes an array is efficient)
  tcpSocketStrand** pd_fdstrandmap;

  // watchStrand and waitForEvents depend on the policy for waking up
  // the rendezvouser thread when a worker has finished with a strand
  virtual void watchStrand(tcpSocketStrand *) = 0;
  virtual void waitForEvents(tcpSocketIncomingRope *) = 0;

  // normal housekeeping functions
  virtual void strandIsActive(tcpSocketStrand *);
  virtual void newConnectionAttempted(tcpSocketStrand *);

  fd_set interestingFDs;
private:
  PoolRendezvouser();
};

class SelectSignalRendezvouser: public PoolRendezvouser{
public:
  SelectSignalRendezvouser(tcpSocketIncomingRope *r,
			   tcpSocketMTincomingFactory *f):
    PoolRendezvouser(r, f) {
    cerr << "SelectSignalRendezvouser::pd_factory = " << pd_factory << endl;

    // initialse the semaphore
    FD_semaphore = (sem_t*) malloc(sizeof(sem_t));
    sem_init(FD_semaphore, 0, /* value = */ 1);
    
    start_undetached(); 
  }
  virtual ~SelectSignalRendezvouser() { }
  virtual void* run_undetached(void *);

  friend PoolWorker;
protected:
  void waitForEvents(tcpSocketIncomingRope *);
  void watchStrand(tcpSocketStrand *);
  void newConnectionAttempted(tcpSocketStrand *);
  void strandIsActive(tcpSocketStrand *);

private:
  omni_mutex allowed_to_signal;
  fd_set readFDs;
  int highestFD;
  sem_t *FD_semaphore;
  SelectSignalRendezvouser();

public:
  // signal handler touches these
  void updateFDs();

};
#endif

static RENDEZVOUSER* rendezvous;

////////////////////////////////////////////////////////////////////
// A tcpStrandWorker is a thread which continually grabs strands
// with pending requests and services them until told to stop.
//
// On top of this simple abstraction we can build workers with
// specific task-allocation policies (eg 1-1 mapping onto strands,
// multiplexed onto worker pool etc)

class tcpStrandWorker: public omni_thread{
public:
  tcpStrandWorker(tcpSocketMTincomingFactory *f):
    omni_thread(this), pd_factory(f){
    start();
  }
  virtual ~tcpStrandWorker(){
    omni_mutex_lock sync(pd_factory->pd_shutdown_lock);
    OMNIORB_ASSERT(pd_factory->pd_shutdown_nthreads != 0);
    if (pd_factory->pd_shutdown_nthreads > 0) {
      pd_factory->pd_shutdown_nthreads--;
    }
    else {
      pd_factory->pd_shutdown_nthreads++;
      pd_factory->pd_shutdown_cond.signal();
    }
    //delete pd_sync;
  }
  virtual void run(void *arg);
  static void _realRun(void* arg);
protected:
  virtual tcpSocketStrand *findStrandWithRequestPending() = 0;
  virtual void dispatchOne(tcpSocketStrand* s);
  virtual CORBA::Boolean strandDied(tcpSocketStrand* s) = 0;
private:
  tcpSocketMTincomingFactory* pd_factory;
  Strand::Sync *pd_sync;
};

#ifdef ONETOONE
class OneToOneWorker: public tcpStrandWorker{
public:
  OneToOneWorker(tcpSocketStrand *s, tcpSocketMTincomingFactory *f):
    tcpStrandWorker(f),  pd_strand(s) {
    // tcpSocketWorker decremented the refcount of the strand here
    // because the Sync object ctor incremented it
    //s->decrRefCount();
    PTRACE("OneToOneWorker", "Yup its me!");
  }
  virtual ~OneToOneWorker() { }
protected:
  virtual tcpSocketStrand *findStrandWithRequestPending(){
    return pd_strand;
  }
  virtual CORBA::Boolean strandDied(tcpSocketStrand* s){
    // to maintain the one to one mapping, the worker shuts down
    PTRACE("OneToOneWorker", "Worker must shut down in sync");
    return FALSE;
  }
private:
  tcpSocketStrand *pd_strand;
};
#endif

#ifdef THREADPOOL
class PoolWorker: public tcpStrandWorker{
public:
  PoolWorker(PoolRendezvouser *r, tcpSocketMTincomingFactory *f):
    tcpStrandWorker(f), pd_q(&(r->pd_q)){
    PTRACE("PoolWorker", "Constructed");
  }
  virtual ~PoolWorker() { }

protected:
  virtual tcpSocketStrand *findStrandWithRequestPending(){
    PTRACE("PoolWorker", "Removing request from Q");
    return pd_q->remove();
  }
  virtual void dispatchOne(tcpSocketStrand *s);
  virtual CORBA::Boolean strandDied(tcpSocketStrand* s){
    // pool workers outlive the individual strands they service
    PTRACE("PoolWorker", "Workers outlive individual strands");
    return TRUE;
  }
private:
  // Not sure if we need to stick a Sync object in here or not

  // make sure all the pool workers die before the Rendezvouser goes
  // and invalidates this pointer....
  Queue<tcpSocketStrand*> *pd_q;
};
#endif

/////////////////////////////////////////////////////////////////////////////

tcpSocketMTincomingFactory::~tcpSocketMTincomingFactory()  {}


tcpSocketMTincomingFactory::tcpSocketMTincomingFactory() 
      : pd_state(IDLE), pd_shutdown_cond(&pd_shutdown_lock),
        pd_shutdown_nthreads(0)
{
  tcpSocketFactoryType::init();
  
}
 
CORBA::Boolean
tcpSocketMTincomingFactory::isIncoming(Endpoint* addr) const
{
  Rope_iterator next_rope(&pd_anchor);
  Rope* r;
  while ((r = next_rope()))
    {
      if (r->this_is(addr))
	return 1;
    }
  return 0;
}

void
tcpSocketMTincomingFactory::instantiateIncoming(Endpoint* addr,
						CORBA::Boolean exportflag)
{
  tcpSocketEndpoint* te = tcpSocketEndpoint::castup(addr);
  if (!te)
    throw omniORB::fatalException(__FILE__,__LINE__,
				  "wrong endpoint argument type");

  Rope_iterator rope_lock(&pd_anchor);

  if (pd_state == ZOMBIE) {
    throw omniORB::fatalException(__FILE__,__LINE__,
			    "cannot instantiate incoming in ZOMBIE state");
  }

  tcpSocketIncomingRope* r = new tcpSocketIncomingRope(this,0,te,exportflag);
  r->incrRefCount(1);

  if (pd_state == ACTIVE) {
    r->rendezvouser = new RENDEZVOUSER(r,this);
  }
}

void
tcpSocketMTincomingFactory::startIncoming()
{
  Rope_iterator next_rope(&pd_anchor);
  tcpSocketIncomingRope* r;

  switch (pd_state) {
  case IDLE:
    try {
      pd_state = ACTIVE;
      while ((r = (tcpSocketIncomingRope*)next_rope())) {
	if (r->pd_shutdown == tcpSocketIncomingRope::NO_THREAD) {
	  r->pd_shutdown = tcpSocketIncomingRope::ACTIVE;
	  r->rendezvouser = new RENDEZVOUSER(r, this);
	  //r->rendezvouser = new tcpSocketRendezvouser(r,this);
	}
      }
    }
    catch(...) { }
    break;
  default:
    break;
  }
}

void
tcpSocketMTincomingFactory::stopIncoming()
{
  Rope_iterator next_rope(&pd_anchor);
  tcpSocketIncomingRope* r;

  switch (pd_state) {
  case ACTIVE:
    try {
      while ((r = (tcpSocketIncomingRope*)next_rope())) {
	r->cancelThreads();
      }
      pd_state = IDLE;
    }
    catch(...) {}
    break;
  default:
    break;
  }
}

void
tcpSocketMTincomingFactory::removeIncoming()
{
  {
    Rope_iterator next_rope(&pd_anchor);
    tcpSocketIncomingRope* r;

    switch (pd_state) {
    case ACTIVE:
      try {
	while ((r = (tcpSocketIncomingRope*)next_rope())) {
	  if (pd_state == ACTIVE) {
	    r->cancelThreads();
	  }
	  if (r->pd_shutdown != tcpSocketIncomingRope::NO_THREAD) {
	    // rendezvouser has not been shutdown properly
	    continue;
	  }
	}
      }
      catch(...) {}
    case IDLE:
      pd_state = ZOMBIE;
      break;
    default:
      return;
    }
  }
  {
    omni_mutex_lock sync(pd_shutdown_lock);
    while (pd_shutdown_nthreads != 0) {
      if (pd_shutdown_nthreads > 0) {
	pd_shutdown_nthreads = -pd_shutdown_nthreads;
      }
      PTRACE("removeIncoming","blocks waiting for worker threads to exit");
      pd_shutdown_cond.wait();
    }
  }
  {
    Rope_iterator next_rope(&pd_anchor);
    tcpSocketIncomingRope* r;

    try {
      while ((r = (tcpSocketIncomingRope*)next_rope())) {
	if (r->pd_shutdown != tcpSocketIncomingRope::NO_THREAD) {
	  // rendezvouser has not been shutdown properly
	  continue;
	}
	if (r->pd_rendezvous != RC_INVALID_SOCKET) {
	  CLOSESOCKET(r->pd_rendezvous);
	  r->pd_rendezvous = RC_INVALID_SOCKET;
	  r->decrRefCount(1);
	}
      }
    }
    catch(...) {}
  }
  PTRACE("removeIncoming","Done");
}

Rope*
tcpSocketMTincomingFactory::findIncoming(Endpoint* addr) const
{
  tcpSocketEndpoint* te = tcpSocketEndpoint::castup(addr);
  if (!te) return 0;

  Rope_iterator next_rope(&pd_anchor);
  Rope* r;
  while ((r = next_rope()))
    {
      if (r->this_is(addr)) {
	r->incrRefCount(1);
	return r;
      }
    }
  return 0;
}

void 
tcpSocketMTincomingFactory::getIncomingIOPprofiles(const CORBA::Octet* objkey,
						   const size_t objkeysize,
			      IOP::TaggedProfileList& profilelist) const
{
  Rope_iterator next_rope(&pd_anchor);
  tcpSocketIncomingRope* r;
  while ((r = (tcpSocketIncomingRope*) next_rope()) && r->pd_export) {
    CORBA::ULong index = profilelist.length();
    profilelist.length(index+1);
    tcpSocketFactoryType::singleton->encodeIOPprofile(r->me,objkey,objkeysize,
						      profilelist[index]);
  }
}

tcpSocketIncomingRope::tcpSocketIncomingRope(tcpSocketMTincomingFactory* f,
					     unsigned int maxStrands,
					     tcpSocketEndpoint *e,
					     CORBA::Boolean exportflag)
  : Rope(f->anchor(),maxStrands), pd_export(exportflag), 
    pd_shutdown(NO_THREAD), rendezvouser(0)
{
  struct sockaddr_in myaddr;

  // For the moment, we do not impose a restriction on the maximum
  // no. of strands that can be accepted. In other words, <maxStrands> is 
  // ignored.

  if ((pd_rendezvous = socket(INETSOCKET,SOCK_STREAM,0)) == RC_INVALID_SOCKET) {
#ifndef __WIN32__
    OMNIORB_THROW(COMM_FAILURE,errno,CORBA::COMPLETED_NO);
#else
    OMNIORB_THROW(COMM_FAILURE,::WSAGetLastError(),CORBA::COMPLETED_NO);
#endif
  }
  myaddr.sin_family = INETSOCKET;
  myaddr.sin_addr.s_addr = INADDR_ANY;
  myaddr.sin_port = htons(e->port());
  
  if (e->port()) {
    int valtrue = 1;
    if (setsockopt(pd_rendezvous,SOL_SOCKET,
		   SO_REUSEADDR,(char*)&valtrue,sizeof(int)) == RC_SOCKET_ERROR)
      {
	CLOSESOCKET(pd_rendezvous);
#ifndef __WIN32__
	OMNIORB_THROW(COMM_FAILURE,errno,CORBA::COMPLETED_NO);
#else
	OMNIORB_THROW(COMM_FAILURE,::WSAGetLastError(),CORBA::COMPLETED_NO);
#endif
      }
  }

  if (bind(pd_rendezvous,(struct sockaddr *)&myaddr,
	   sizeof(struct sockaddr_in)) == RC_SOCKET_ERROR) 
  {
    CLOSESOCKET(pd_rendezvous);
#ifndef __WIN32__
    OMNIORB_THROW(COMM_FAILURE,errno,CORBA::COMPLETED_NO);
#else
    OMNIORB_THROW(COMM_FAILURE,::WSAGetLastError(),CORBA::COMPLETED_NO);
#endif
  }

  // Make it a passive socket
  if (listen(pd_rendezvous,5) == RC_SOCKET_ERROR) {
    CLOSESOCKET(pd_rendezvous);
#ifndef __WIN32__
    OMNIORB_THROW(COMM_FAILURE,errno,CORBA::COMPLETED_NO);
#else
    OMNIORB_THROW(COMM_FAILURE,::WSAGetLastError(),CORBA::COMPLETED_NO);
#endif
  }
  
  {
#if (defined(__GLIBC__) && __GLIBC__ >= 2)
    // GNU C library uses socklen_t * instead of int* in getsockname().
    // This is suppose to be compatible with the upcoming POSIX standard.
    socklen_t l;
#elif defined(__aix__) || defined(__VMS) || defined(__SINIX__) || defined(__uw7__)
    size_t l;
# else
    int l;
# endif



    l = sizeof(struct sockaddr_in);
    if (getsockname(pd_rendezvous,
		    (struct sockaddr *)&myaddr,&l) == RC_SOCKET_ERROR) {
      CLOSESOCKET(pd_rendezvous);
#ifndef __WIN32__
      OMNIORB_THROW(COMM_FAILURE,errno,CORBA::COMPLETED_NO);
#else
      OMNIORB_THROW(COMM_FAILURE,::WSAGetLastError(),CORBA::COMPLETED_NO);
#endif
    }

    e->port(ntohs(myaddr.sin_port));

    if (e->host() == 0 || strlen((const char*)e->host()) == 0) {

      char self[64];
      if (gethostname(&self[0],64) == RC_SOCKET_ERROR) {
	throw omniORB::fatalException(__FILE__,__LINE__,
				      "Cannot get the name of this host");
      }

      LibcWrapper::hostent_var h;
      int rc;

      if (LibcWrapper::gethostbyname(self,h,rc) < 0) {
	throw omniORB::fatalException(__FILE__,__LINE__,
				      "Cannot get the address of this host");
      }
      memcpy((void *)&myaddr.sin_addr,
	     (void *)h.hostent()->h_addr_list[0],
	     sizeof(myaddr.sin_addr));
      char ipaddr[16];
      // To prevent purify from generating UMR warnings, use the following temp
      // variables to store the IP address fields.
      int ip1 = (int)((ntohl(myaddr.sin_addr.s_addr) & 0xff000000) >> 24);
      int ip2 = (int)((ntohl(myaddr.sin_addr.s_addr) & 0x00ff0000) >> 16);
      int ip3 = (int)((ntohl(myaddr.sin_addr.s_addr) & 0x0000ff00) >> 8);
      int ip4 = (int)(ntohl(myaddr.sin_addr.s_addr) & 0x000000ff);
      sprintf(ipaddr,"%d.%d.%d.%d",ip1,ip2,ip3,ip4);
      e->host((const CORBA::Char *) ipaddr);

    }
    else {
      // The caller has already specified the host name, we are not going to
      // override it here. However, it may be possible that the host name does
      // not resolve to one of the IP addresses that identified the network
      // interfaces of this machine. There is no way to guard against this
      // mistake.
      // Do nothing here.
    }
  }

  me = new tcpSocketEndpoint(e);
}

tcpSocketIncomingRope::~tcpSocketIncomingRope()
{
  PTRACE("~tcpSocketIncomingRope","called");
  if (me) {
    delete me;
    me = 0;
  }
  if (pd_rendezvous != RC_INVALID_SOCKET) {
    CLOSESOCKET(pd_rendezvous);
    pd_rendezvous = RC_INVALID_SOCKET;
  }
}

void
tcpSocketIncomingRope::cancelThreads()
{
  if (rendezvouser) {
      pd_lock.lock();
      pd_shutdown = SHUTDOWN;
      pd_lock.unlock();
  }

  CutStrands();
  
  if (rendezvouser) {
    // Unblock the rendezvouser from its accept() call.
    // Cannot shutdown the rendezvous socket because we want to keep it open
    // and could be served later by another rendezvouser.
    // 
    // Instead, unblock the rendezvouser by making a connect() call.

    struct sockaddr_in myaddr;

    {
      Endpoint* e = 0;
      this_is(e);
      tcpSocketEndpoint* te = (tcpSocketEndpoint*)e;
      myaddr.sin_family = INETSOCKET;
      myaddr.sin_port   = htons(te->port());
      if (LibcWrapper::isipaddr((char*)te->host())) 
	{
	  CORBA::ULong ip_p = inet_addr((char*) te->host());
	  memcpy((void*) &myaddr.sin_addr, (void*) &ip_p, sizeof(myaddr.sin_addr));
	}
      else
	{
	  LibcWrapper::hostent_var h;
	  int  rc;
	  LibcWrapper::gethostbyname((char*)te->host(),h,rc);
	  memcpy((void*)&myaddr.sin_addr,
		 (void*)h.hostent()->h_addr_list[0],
		 sizeof(myaddr.sin_addr));
	}
      delete te;
    }

    tcpSocketHandle_t  tmp_sock;

    if ((tmp_sock = socket(INETSOCKET,SOCK_STREAM,0)) == RC_INVALID_SOCKET) 
      {
	// If we cannot create a socket, we cannot shutdown the rendezvouser
	omniORB::logs(1,
	      "tcpSocketIncomingRope::cancelThreads() cannot create\n"
	      " a socket to connect to the rendezvous socket.  The\n"
	      " rendezvous thread may or may not have exited.  If this is\n"
	      " a temporary resource exhaustion, try again later.");
	return;
      }
    if (connect(tmp_sock,(struct sockaddr *)&myaddr,
		sizeof(struct sockaddr_in)) == RC_SOCKET_ERROR) 
      {
	// OK, so we cannot connect to the rendezvouser, it would have
	// unblock from accept() anyway. That is fine as well.
      }
    else 
      {
	// Now we have unblock the rendezvouser, just close the socket.
	CLOSESOCKET(tmp_sock);
      }

    // Now we have got the rendezvouser's attention. We acknowlege that
    // we have seen it and instructs the rendezvouser to exit.
    pd_lock.lock();
    pd_shutdown = NO_THREAD;
    pd_lock.unlock();

    PTRACE("stopIncoming","Waiting for tcpSocketMT Rendezvouser to exit...");
    rendezvouser->join(0); // Wait till the rendezvouser to come back
    PTRACE("stopIncoming","tcpSocketMT Rendezvouser has exited");
    rendezvouser = 0;
  }
}

Strand *
tcpSocketIncomingRope::newStrand()
{
  throw omniORB::fatalException(__FILE__,__LINE__,
				"newStrand should not be called.");
#ifdef NEED_DUMMY_RETURN
  return 0; // dummy return to keep some compilers happy
#endif
}

/////////////////////////////////////////////////////////////////////////////

tcpSocketMToutgoingFactory::~tcpSocketMToutgoingFactory()  {}


tcpSocketMToutgoingFactory::tcpSocketMToutgoingFactory()
{
  tcpSocketFactoryType::init();
}

CORBA::Boolean
tcpSocketMToutgoingFactory::isOutgoing(Endpoint* addr) const
{
  Rope_iterator next_rope(&pd_anchor);
  Rope* r;
  while ((r = next_rope()))
    {
      if (r->remote_is(addr))
	return 1;
    }
  return 0;
}

Rope*
tcpSocketMToutgoingFactory::findOrCreateOutgoing(Endpoint* addr)
{
  tcpSocketEndpoint* te = tcpSocketEndpoint::castup(addr);
  if (!te) return 0;

  Rope_iterator next_rope(&pd_anchor);
  Rope* r;
  while ((r = next_rope()))
    {
      if (r->remote_is(addr)) {
	r->incrRefCount(1);
	return r;
      }
    }
  r = new tcpSocketOutgoingRope(this,omniORB::maxTcpConnectionPerServer,te);
  r->incrRefCount(1);
  return r;
}


tcpSocketOutgoingRope::tcpSocketOutgoingRope(tcpSocketMToutgoingFactory* f,
					     unsigned int maxStrands,
					     tcpSocketEndpoint *e)
  : Rope(f->anchor(),maxStrands)
{
  remote = new tcpSocketEndpoint(e);
}

tcpSocketOutgoingRope::~tcpSocketOutgoingRope()
{
  PTRACE("~tcpSocketOutgoingRope","called");
  if (remote) {
    delete remote;
    remote = 0;
  }
}

Strand *
tcpSocketOutgoingRope::newStrand()
{
  return new tcpSocketStrand(this,remote);
}


/////////////////////////////////////////////////////////////////////////////



// Size of transmit and receive buffers
const 
unsigned int 
tcpSocketStrand::buffer_size = 8192 + (int)omni::max_alignment;


static tcpSocketHandle_t realConnect(tcpSocketEndpoint* r);


tcpSocketStrand::tcpSocketStrand(tcpSocketOutgoingRope *rope,
				 tcpSocketEndpoint   *r)
  : reliableStreamStrand(tcpSocketStrand::buffer_size,rope),
    pd_send_giop_closeConnection(0), pd_delay_connect(0)
{
  // Do not try to connect to the remote host in this ctor.
  // This is to avoid holding the mutex on rope->pd_lock while the connect
  // is in progress. Holding the mutex for an extended period is bad as this 
  // can have ***serious*** side effect. 
  // One immediate consequence of holding the rope->pd_lock is that the
  // outScavenger will be blocked on rope->pd_lock when it is scanning
  // for idle strands. This in turn blockout any thread trying to lock
  // rope->pd_anchor->pd_lock. This is really bad because no new rope
  // can be added to the anchor.


  pd_socket = RC_INVALID_SOCKET;
  pd_delay_connect = new tcpSocketEndpoint(r);
  // Do the connect on first call to ll_recv or ll_send.
}

tcpSocketStrand::tcpSocketStrand(tcpSocketIncomingRope *r,
				 tcpSocketHandle_t sock)
  : reliableStreamStrand(tcpSocketStrand::buffer_size,r),
    pd_socket(sock), pd_send_giop_closeConnection(1), pd_delay_connect(0)
{
}


tcpSocketStrand::~tcpSocketStrand() 
{
  if (omniORB::trace(5)) {
    omniORB::logger l;
    l << "tcpSocketStrand::~Strand() close socket no. " << pd_socket << '\n';
  }
  if (pd_socket != RC_INVALID_SOCKET)
    CLOSESOCKET(pd_socket);
  pd_socket = RC_INVALID_SOCKET;
  if (pd_delay_connect)
    delete pd_delay_connect;
  pd_delay_connect = 0;
}


static inline char printable_char(char c) {
  return (c < 32 || c > 126) ? '.' : c;
}


static void dumpbuf(unsigned char* buf, size_t sz)
{
  static omni_mutex lock;
  omni_mutex_lock sync(lock);
  unsigned int i;
  char row[80];

  // Do complete rows of 16 octets.
  while( sz >= 16 ) {
    sprintf(row, "%02x%02x %02x%02x %02x%02x %02x%02x "
	         "%02x%02x %02x%02x %02x%02x %02x%02x ",
            (int) buf[0], (int) buf[1], (int) buf[2], (int) buf[3],
            (int) buf[4], (int) buf[5], (int) buf[6], (int) buf[7],
            (int) buf[8], (int) buf[9], (int) buf[10], (int) buf[11],
            (int) buf[12], (int) buf[13], (int) buf[14], (int) buf[15]);
    fprintf(stderr, "%s", row);
    char* p = row;
    for( i = 0; i < 16; i++ )  *p++ = printable_char(*buf++);
    *p++ = '\0';
    fprintf(stderr,"%s\n", row);
    sz -= 16;
  }

  if( sz ) {
    // The final part-row.
    for( i = 0; i < sz; i++ )
      fprintf(stderr, (i & 1) ? "%02x ":"%02x", (int) buf[i]);
    for( ; i < 16; i++ )
      fprintf(stderr, (i & 1) ? "   ":"  ");
    for( i = 0; i < sz; i++ )
      fprintf(stderr, "%c", printable_char(buf[i]));
    fprintf(stderr,"\n");
  }
}


size_t
tcpSocketStrand::ll_recv(void* buf, size_t sz)
{
  if (pd_delay_connect) {
    // We have not connect to the remote host yet. Do the connect now.
    // Note: May block on connect for sometime if the remote host is down
    //
    if ((pd_socket = realConnect(pd_delay_connect)) == RC_INVALID_SOCKET) {
      _setStrandIsDying();
#ifndef __WIN32__
      OMNIORB_THROW(COMM_FAILURE,errno,CORBA::COMPLETED_NO);
#else
      OMNIORB_THROW(COMM_FAILURE,::WSAGetLastError(),CORBA::COMPLETED_NO);
#endif
    }
    delete pd_delay_connect;
    pd_delay_connect = 0;
  }

  int rx;
  while (1) {
    if ((rx = ::recv(pd_socket,(char*)buf,sz,0)) == RC_SOCKET_ERROR) {
      if (errno == EINTR)
	continue;
      else
	{
	  _setStrandIsDying();
#ifndef __WIN32__
	  OMNIORB_THROW(COMM_FAILURE,errno,CORBA::COMPLETED_NO);
#else
	  OMNIORB_THROW(COMM_FAILURE,::WSAGetLastError(),CORBA::COMPLETED_NO);
#endif
	}
    }
    else
      if (rx == 0) {
	_setStrandIsDying();
#ifndef __WIN32__
	OMNIORB_THROW(COMM_FAILURE,errno,CORBA::COMPLETED_NO);
#else
	OMNIORB_THROW(COMM_FAILURE,::WSAGetLastError(),CORBA::COMPLETED_NO);
#endif
      }
    break;
  }
  if (omniORB::trace(30)) {
      fprintf(stderr,"ll_recv: %d bytes\n",rx);
      ::dumpbuf((unsigned char*)buf,rx);
  }
  return (size_t)rx;
}

void
tcpSocketStrand::ll_send(void* buf,size_t sz) 
{
  if (pd_delay_connect) {
    // We have not connect to the remote host yet. Do the connect now.
    // Note: May block on connect for sometime if the remote host is down
    //
    if ((pd_socket = realConnect(pd_delay_connect)) == RC_INVALID_SOCKET) {
      _setStrandIsDying();
#ifndef __WIN32__
      OMNIORB_THROW(COMM_FAILURE,errno,CORBA::COMPLETED_NO);
#else
      OMNIORB_THROW(COMM_FAILURE,::WSAGetLastError(),CORBA::COMPLETED_NO);
#endif
    }
    delete pd_delay_connect;
    pd_delay_connect = 0;
  }

  int tx;
  char *p = (char *)buf;

  if (omniORB::trace(30)) {
      fprintf(stderr,"ll_send: %d bytes\n",sz);
      ::dumpbuf((unsigned char*)buf,sz);
  }

  while (sz) {
    if ((tx = ::send(pd_socket,p,sz,0)) == RC_SOCKET_ERROR) {
#ifndef __WIN32__
      if (errno == EINTR)
	continue;
      else {
	_setStrandIsDying();
	OMNIORB_THROW(COMM_FAILURE,errno,CORBA::COMPLETED_NO);
      }
#else
      if (::WSAGetLastError() == WSAEINTR)
 	continue;
      else {
 	_setStrandIsDying();
	OMNIORB_THROW(COMM_FAILURE,::WSAGetLastError(),CORBA::COMPLETED_MAYBE);
      }
#endif
    }
    else
      if (tx == 0) {
	_setStrandIsDying();
#ifndef __WIN32__
	OMNIORB_THROW(COMM_FAILURE,errno,CORBA::COMPLETED_NO);
#else
	OMNIORB_THROW(COMM_FAILURE,::WSAGetLastError(),CORBA::COMPLETED_NO);
#endif
      }
    sz -= tx;
    p += tx;
  }
  return;
}



void
tcpSocketStrand::real_shutdown()
{
  if (pd_send_giop_closeConnection)
    {
      // XXX This is a bit ugly. GIOP requires us to send a CloseConnection 
      // message before shutdown. Ideally, we do not want this abstraction to 
      // do any GIOP dependent stuff. If this a problem in future, we should 
      // perhap make it a parameter to decide whether or what to send on
      // shutdown.

      static char closeConnectionMessage[12] = {
	   'G','I','O','P',
	   1,0,
	   _OMNIORB_HOST_BYTE_ORDER_,
	   GIOP::CloseConnection,
	   0,0,0,0
      };

      size_t sz = sizeof(closeConnectionMessage);
      char* p = closeConnectionMessage;
      while (sz) {
	fd_set wrfds;
#       ifndef __CIAO__
	FD_ZERO(&wrfds);
	FD_SET(pd_socket,&wrfds);
#       endif
	struct timeval t = { 0,100000};
	int rc;
	if ((rc = select(pd_socket+1,0,&wrfds,0,&t)) <= 0) {
	  // Any timeout or error, we just don't border sending the message.
	  break;
	}
	int tx;
	if ((tx = ::send(pd_socket,p,sz,0)) == RC_SOCKET_ERROR) {
	  // Any error we just don't border sending anymore.
	  break;
	}
	sz -= tx;
	p += tx;
      }
    }
  _setStrandIsDying();
  SHUTDOWNSOCKET(pd_socket);
  if (omniORB::trace(15)) {
    omniORB::logger l;
    l << "tcpSocketStrand::real_shutdown() fd no. " << pd_socket << " Done\n";
  }
  return;
}

static
tcpSocketHandle_t
realConnect(tcpSocketEndpoint* r)
{
  struct sockaddr_in raddr;
  LibcWrapper::hostent_var h;
  int  rc;
  tcpSocketHandle_t sock;

  if (! LibcWrapper::isipaddr( (char*) r->host()))
    {
      if (LibcWrapper::gethostbyname((char *)r->host(),h,rc) < 0) 
	{
	  // XXX look at rc to decide what to do or if to give up what errno
	  // XXX to return EINVAL.
	  //
	  return RC_INVALID_SOCKET;
	}
      // We just pick the first address in the list, may be we should go
      // through the list and if possible pick the one that is on the same
      // subnet.
      memcpy((void*)&raddr.sin_addr,
	     (void*)h.hostent()->h_addr_list[0],
	     sizeof(raddr.sin_addr));
    }
  else
    {
      // The machine name is already an IP address
      CORBA::ULong ip_p;
      if ( (ip_p = inet_addr( (char*) r->host() )) == RC_INADDR_NONE)
	{
	  return RC_INVALID_SOCKET;
	}
      memcpy((void*) &raddr.sin_addr, (void*) &ip_p, sizeof(raddr.sin_addr));
    }

  raddr.sin_family = INETSOCKET;
  raddr.sin_port   = htons(r->port());

  if ((sock = socket(INETSOCKET,SOCK_STREAM,0)) == RC_INVALID_SOCKET) {
    return RC_INVALID_SOCKET;
  }

#if defined(__sunos__) && defined(__sparc__) && __OSVERSION__ >= 5
  // Use non-blocking connect.
  int fl = O_NONBLOCK;
  if (fcntl(sock,F_SETFL,fl) < RC_SOCKET_ERROR) {
    CLOSESOCKET(sock);
    return RC_INVALID_SOCKET;
  }
  if (connect(sock,(struct sockaddr *)&raddr,
	      sizeof(struct sockaddr_in)) == RC_SOCKET_ERROR) 
  {
    if (errno != EINPROGRESS) {
      CLOSESOCKET(sock);
      return RC_INVALID_SOCKET;
    }
    fd_set wrfds;
#   ifndef __CIAO__
    FD_ZERO(&wrfds);
    FD_SET(sock,&wrfds);
#   endif
    struct timeval t = { 30,0 };
    int rc;
    if ((rc = select(sock+1,0,&wrfds,0,&t)) <= 0) {
      // Timeout, do not bother trying again.
      CLOSESOCKET(sock);
      return RC_INVALID_SOCKET;
    }
  }
  // Set the socket back to blocking
  fl = 0;
  if (fcntl(sock,F_SETFL,fl) == RC_SOCKET_ERROR) {
    CLOSESOCKET(sock);
    return RC_INVALID_SOCKET;
  }

#else
  if (connect(sock,(struct sockaddr *)&raddr,
	      sizeof(struct sockaddr_in)) == RC_SOCKET_ERROR) 
  {
    CLOSESOCKET(sock);
    return RC_INVALID_SOCKET;
  }
#endif


  return sock;
}

/////////////////////////////////////////////////////////////////////////////


#if defined(__sunos__) && defined(__sparc__) && __OSVERSION__ >= 5
#if defined(__SUNPRO_CC) && __SUNPRO_CC <= 0x420
#include <signal.h>
static void abort()
{
  kill (getpid(),SIGABRT);
  while (1) {
    sleep(1000000);
  }
}

typedef void (*PFV)();
extern PFV set_terminate(PFV);
#endif
#endif

////////////////////////////////////////////////////////////////////////////
// Implementation of the abstract base class tcpSocketRendezvouser
tcpSocketRendezvouser::tcpSocketRendezvouser(tcpSocketIncomingRope *r,
					     tcpSocketMTincomingFactory *f):
  omni_thread(r), pd_factory(f){
  cerr << "tcpSocketRendezvouser::pd_factory = " << pd_factory << endl;
  // the actual thread should be started last, by the most derived 
  // constructor
}

void*
tcpSocketRendezvouser::run_undetached(void *arg)
{
  tcpSocketIncomingRope* r = (tcpSocketIncomingRope*) arg;

#if defined(__sunos__) && defined(__sparc__) && __OSVERSION__ >= 5
#if defined(__SUNPRO_CC) && __SUNPRO_CC <= 0x420
  set_terminate(abort);
#endif
#endif
  PTRACE("Rendezvouser","start.");

  CORBA::Boolean die = 0;

  while (r->pd_shutdown == tcpSocketIncomingRope::ACTIVE  && !die) {
    try {
      waitForEvents(r);
    }
    catch(CORBA::COMM_FAILURE&) {
      // XXX accepts failed. The probable cause is that the number of
      //     file descriptors opened has exceeded the limit.
      //     On unix, the value of this limit can be set and get using the
      //              ulimit command.
      //     On NT, if this is part of a DLL, the limit is 256(?)
      //            else the limit is 16(?)
      // The following is a temporary fix, this thread just wait for a while
      // and tries again. Hopfully, some connections might be freed by then.
      PTRACE("Rendezvouser","accept fails. Too many file descriptors opened?");
      omni_thread::sleep(1,0);
      continue;
    }
    catch(omniORB::fatalException& ex) {
      if (omniORB::trace(0)) {
	omniORB::logger l;
	l << "You have caught an omniORB bug, details are as follows:\n" <<
	  " file: " << ex.file() << "\n"
	  " line: " << ex.line() << "\n"
	  " mesg: " << ex.errmsg() << "\n"
	  " tcpSocketMT Rendezvouser thread will not accept new connection.\n";
      }
      die = 1;
    }
    catch(...) {
      omniORB::logs(0,
       "Unexpected exception caught by tcpSocketMT Rendezvouser\n"
       " tcpSocketMT Rendezvouser thread will not accept new connection.");
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
    SHUTDOWNSOCKET(r->pd_rendezvous);
  }

  while (r->pd_shutdown != tcpSocketIncomingRope::NO_THREAD) {

    // We keep on accepting connection requests but close the new sockets
    // immediately until the state of pd_shutdown changes to NO_THREAD.
    // This is to make sure that the thread calling cancelThreads()
    // will be unblocked from the connect() call.

    {
      fd_set rdfds;
#     ifndef __CIAO__
      FD_ZERO(&rdfds);
      FD_SET(r->pd_rendezvous,&rdfds);
#     endif
      struct timeval t = { 1,0};
      int rc;
      if ((rc = select(r->pd_rendezvous+1,&rdfds,0,0,&t)) <= 0) {
#ifndef __WIN32__
	if (rc < 0 && errno != EINTR) {
	  die = 1;
	}
#else
 	if (rc < 0 && ::WSAGetLastError() != WSAEINTR) {
	  die = 1;
	}
#endif
	PTRACE("Rendezvouser","waiting on shutdown state to change to NO_THREAD.");
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
    if ((new_sock = ::accept(r->pd_rendezvous,(struct sockaddr *)&raddr,&l)) 
	      == RC_INVALID_SOCKET) 
      {
	die = 1;
	continue;
      }
    CLOSESOCKET(new_sock);
  }

  PTRACE("Rendezvouser","exit.");
  return 0;
}

////////////////////////////////////////////////////////////////////////////
// Implementation of the class OneToOneRendezvouser
#ifdef ONETOONE
void OneToOneRendezvouser::waitForEvents(tcpSocketIncomingRope *r){
  tcpSocketHandle_t new_sock;
  struct sockaddr_in raddr;
  tcpSocketStrand *newSt = 0;

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

  PTRACE("Rendezvouser","block on accept()");

  if ((new_sock = ::accept(r->pd_rendezvous,(struct sockaddr *)&raddr,&l)) 
      == RC_INVALID_SOCKET) {
#ifndef __WIN32__
    OMNIORB_THROW(COMM_FAILURE,errno,CORBA::COMPLETED_NO);
#else
    OMNIORB_THROW(COMM_FAILURE,::WSAGetLastError(),CORBA::COMPLETED_NO);
#endif
  }
  PTRACE("Rendezvouser","unblock from accept()");

  {
    omni_mutex_lock sync(r->pd_lock);

    if (r->pd_shutdown != tcpSocketIncomingRope::ACTIVE) {
      // It has been indicated that this thread should stop
      // accepting connection request.
      CLOSESOCKET(new_sock);
      return;
    }
    
    newSt = new tcpSocketStrand(r,new_sock);
    newSt->incrRefCount(1);
  }

  newConnectionAttempted(newSt);
}

void OneToOneRendezvouser::newConnectionAttempted(tcpSocketStrand *newSt){
  tcpStrandWorker *newthr = 0;

  PTRACE("Rendezvouser","accept new strand.");
  omni_mutex_lock sync(pd_factory->pd_shutdown_lock);
  if (pd_factory->pd_shutdown_nthreads >= 0) {
    pd_factory->pd_shutdown_nthreads++;
  }
  else {
    pd_factory->pd_shutdown_nthreads--;
  }

  try {
    newthr = new OneToOneWorker(newSt,pd_factory);
  }
  catch(...) {
    newthr = 0;
  }
  if (!newthr) {
    // Cannot create a new thread to serve the strand
    // We have no choice but to shutdown the strand.
    // The long term solutions are:  start multiplexing the new strand
    // and the rendezvous; close down idle connections; reasign
    // threads to strands; etc.
    newSt->decrRefCount();
    newSt->real_shutdown();

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
#endif

#ifdef THREADPOOL
PoolRendezvouser::PoolRendezvouser(tcpSocketIncomingRope *r,
				   tcpSocketMTincomingFactory *f):
  tcpSocketRendezvouser(r, f){
  cerr << "PoolRendezvouser::pd_factory = " << pd_factory << endl;

  { // Build the fd -> tcpSocketStrand* table
    int maxstrands = getdtablesize();
    pd_fdstrandmap = new tcpSocketStrand*[maxstrands];
    // Clean table entries
    for (int i=0;i<maxstrands;i++) pd_fdstrandmap[i] = NULL;
  }

  // build the thread pool
  for (unsigned int i=0;i<poolSize;i++) new PoolWorker(this, f);
}

void PoolRendezvouser::strandIsActive(tcpSocketStrand *s){
  // add this strand to the Queue of things to process
  pd_q.add(s);
  // mark this strand as uninteresting
  FD_CLR(s->handle(), &interestingFDs);
}

void PoolRendezvouser::newConnectionAttempted(tcpSocketStrand *s){
  int new_sock = s->handle();

  // mark this strand as interesting
  FD_SET(new_sock, &interestingFDs);

  // add the strand to the lookup table
  pd_fdstrandmap[new_sock] = s;
}

/////////////////////////////////////////////////////////////////////////////
// Blocks with select(), wakes up with a signal()
#include <signal.h>

// HACK the rendezvouser thread (for killing purposes)
static pthread_t rendez_thread;
static int wake_signal = SIGQUIT;

void handle_signal(int signalno){
  rendezvous->updateFDs();
  PTRACE("Rendezvouser signal handler", "Updated select FDs");
}

void SelectSignalRendezvouser::updateFDs(){
  // this bit is shared
  sem_wait(rendezvous->FD_semaphore);
  {
    memcpy(&rendezvous->readFDs, &rendezvous->interestingFDs, sizeof(fd_set));
  }
  sem_post(rendezvous->FD_semaphore);
}

void SelectSignalRendezvouser::newConnectionAttempted(tcpSocketStrand *s){
  int new_sock = s->handle();

  sem_wait(FD_semaphore);
  {
    FD_SET(new_sock, &interestingFDs);
  }
  sem_post(FD_semaphore);

  if (new_sock > highestFD) highestFD = new_sock;
  // is it important to reset that after removing a FD?

  // add the strand to the lookup table
  pd_fdstrandmap[new_sock] = s;
}

void SelectSignalRendezvouser::strandIsActive(tcpSocketStrand *s){
  // add this strand to the Queue of things to process
  pd_q.add(s);
  sem_wait(FD_semaphore);
  {
    // mark this strand as uninteresting
    FD_CLR(s->handle(), &interestingFDs);
  }
  sem_post(FD_semaphore);
}

// Updates the state of the rendezvouser thread to make it watch
// a strand again for more data. Send a signal to the thread to
// force it to rescan its filedescriptor set.
void SelectSignalRendezvouser::watchStrand(tcpSocketStrand *s){

  PTRACE("Rendezvouser", "wakeUp called [acquiring lock]");
  {
    omni_mutex_lock lock(rendezvous->allowed_to_signal);

    // mark the strand as interesting again
#ifdef EXTRADEBUG
    cerr << "Watching strand fd = " << s->handle() << " again" << endl;
    fflush(stderr);
#endif

    sem_wait(rendezvous->FD_semaphore);
    {
      FD_SET(s->handle(), &rendezvous->interestingFDs);
    }
    sem_post(rendezvous->FD_semaphore);

    // this handle is actually a strand isn't it?
    OMNIORB_ASSERT(rendezvous->pd_fdstrandmap[s->handle()]);
    pthread_kill(rendez_thread, wake_signal);

    PTRACE("Rendezvouser", "wakeUp finished [released lock]");
  }

}

void dump_fdset(fd_set *f, int highestFD){
  cerr << "[";
  for (int i=0;i<=highestFD;i++)
    if (FD_ISSET(i, f)) cerr << i << "  ";
  cerr << "]" << endl;
}

void*
SelectSignalRendezvouser::run_undetached(void *arg)
{
  tcpSocketIncomingRope* r = (tcpSocketIncomingRope*) arg;

  PTRACE("SelectSignalRendezvouser","start.");

  // HACK - get the pthreads thread ID
  rendez_thread = pthread_self();
  rendezvous = this;
  signal(wake_signal, &handle_signal);
  // HACK
  
  // build the interesting bitmap of file descriptors
  FD_ZERO(&interestingFDs);
  // we're always interested in new connections
  FD_SET(r->pd_rendezvous, &interestingFDs);
  highestFD = r->pd_rendezvous;

  // noone is allowed to signal us unless we know it is safe
  allowed_to_signal.lock();

  return tcpSocketRendezvouser::run_undetached(arg);
}

void SelectSignalRendezvouser::waitForEvents(tcpSocketIncomingRope *r){
  tcpSocketHandle_t new_sock;
  struct sockaddr_in raddr;

  // select() modifies the bitmap in place so we need to copy it
  // Worker threads are also able to modify the interestingFD set
  // while we aren't looking and then cause us to take a fresh new
  // local copy.
  
  // readFDs are shared- semaphores are the only async-signal safe 
  sem_wait(FD_semaphore);
  {
    memcpy(&readFDs, &interestingFDs, sizeof(fd_set));
  }
  sem_post(FD_semaphore);
#ifdef EXTRADEBUG
  cerr << "interestingFDs = "; dump_fdset(&interestingFDs, highestFD);
  cerr << "readFDs        = "; dump_fdset(&readFDs, highestFD);
#endif EXTRADEBUG 

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

  int signal_result;
      
  // ----------------------------------------------------------
  // A worker thread can signal us safely while we are in here
  // (safe == does not screw up important other syscalls) 
  allowed_to_signal.unlock();
  {
    PTRACE("Rendezvouser","blocking on select()");
#ifdef EXTRADEBUG
    cerr << "Read FDs = ";
    dump_fdset(&readFDs, highestFD);
#endif
    signal_result = ::select(highestFD + 1, &readFDs, NULL, NULL, NULL);
  } 
  allowed_to_signal.lock();
  // ----------------------------------------------------------

  // dodgy hack- in case readFDs got clobbered after select
  sem_wait(FD_semaphore);
  {
    memcpy(&readFDs, &interestingFDs, sizeof(fd_set));
  }
  sem_post(FD_semaphore);
      
  signal_result = ::select(highestFD + 1, &readFDs, NULL, NULL, NULL);
	
#ifdef EXTRADEBUG
  cerr << "signal_result == " << signal_result << endl;
  cerr << "readFDs == ";
  dump_fdset(&readFDs, highestFD);
  fflush(stderr);
#endif

  if (signal_result == -1){
    // Received a signal while blocked in select()
    PTRACE("Rendezvouser", "SIGNAL unblocked select");
    // This means either that:
    //  * A worker thread has finished reading a request on
    //    a strand and wishes that strand to be monitored for
    //    new incoming requests again
    //  * A signal in some other context indicating something
    //    really bad has happened
    // Assuming that noone else will send this particular signal
    // type, assume the former.
    
    // Just go round the loop again refreshing the bitmap and
    // reblocking
  } else {

    // is it a new connection?
#ifdef EXTRADEBUG
    cerr << "r->pd_rendezvous == " << r->pd_rendezvous << endl;
    fflush(stderr);
#endif
    if (FD_ISSET(r->pd_rendezvous, &readFDs)){
      PTRACE("Rendezvouser", "new connection attempted");
#ifdef EXTRADEBUG
      cerr << "accept?" << endl;
      fflush(stderr);
#endif
      if ((new_sock = ::accept(r->pd_rendezvous,(struct sockaddr *)&raddr,&l)) 
	  == RC_INVALID_SOCKET) {
#ifndef __WIN32__
	OMNIORB_THROW(COMM_FAILURE,errno,CORBA::COMPLETED_NO);
#else
	OMNIORB_THROW(COMM_FAILURE,::WSAGetLastError(),CORBA::COMPLETED_NO);
#endif
      }
#ifdef EXTRADEBUG
      cerr << "accepted." << endl;
      fflush(stderr);
#endif
      PTRACE("Rendezvouser","unblock from select(), accept() successful");

      {
	omni_mutex_lock sync(r->pd_lock);
	
	if (r->pd_shutdown != tcpSocketIncomingRope::ACTIVE) {
	  // It has been indicated that this thread should stop
	  // accepting connection request.
	  CLOSESOCKET(new_sock);
	  return;
	}
	
	tcpSocketStrand *newSt = new tcpSocketStrand(r,new_sock);
	newSt->incrRefCount(1);

	newConnectionAttempted(newSt);

      }

      PTRACE("Rendezvouser","accept new strand.");

      OMNIORB_ASSERT(pd_factory != NULL);
      omni_mutex_lock sync(pd_factory->pd_shutdown_lock);
      if (pd_factory->pd_shutdown_nthreads >= 0) {
	pd_factory->pd_shutdown_nthreads++;
      } else {
	pd_factory->pd_shutdown_nthreads--;
      }
	
    } else {
      // (code is only ever reached iff the rendezvouser is looking
      // for incoming GIOP data)

      // which strands were the requests on?
      int nactive = signal_result;
      int fd = 0;
      while (nactive){
	OMNIORB_ASSERT(fd <= highestFD);
	if (FD_ISSET(fd, &readFDs)){
	  tcpSocketStrand *s = pd_fdstrandmap[fd];
	  OMNIORB_ASSERT(s != NULL);
	  strandIsActive(s);

	  // need to make this thread higher priority than any activated
	  // worker because it is still holding the signal mutex?
	  nactive--;
	}
	fd++;
      }
    }
  }
}
#endif

///////////////////////////////////////////////////////////////////
// Worker implementations

void tcpStrandWorker::run(void *arg){
  omniORB::giopServerThreadWrapper::
    getGiopServerThreadWrapper()->run(tcpStrandWorker::_realRun,arg);
  // the wrapper run() method will pass back control to tcpSocketWorker
  // by calling  _realRun(arg) when it is ready.
}

void tcpStrandWorker::_realRun(void *arg){
  tcpStrandWorker* me = (tcpStrandWorker*)arg;

#if defined(__sunos__) && defined(__sparc__) && __OSVERSION__ >= 5
#if defined(__SUNPRO_CC) && __SUNPRO_CC <= 0x420
  set_terminate(abort);
#endif
#endif
  
  PTRACE("Worker","start.");

  CORBA::Boolean alive = TRUE;
  // fetch strand, service and repeat
  try {
    while (alive) {
      tcpSocketStrand *s = me->findStrandWithRequestPending();
#ifdef EXTRADEBUG
      cerr << "strand from Q == " << s << endl;
      cerr << "fd # on strand == " << s->handle() << endl;
      fflush(stderr);
#endif
      try{
	me->dispatchOne(s);
      } catch (CORBA::COMM_FAILURE&){
	// Should the worker thread live on?
	PTRACE("Worker", "Strand has died");
	alive = me->strandDied(s);
      }
    }
  } catch (...) {
    // error handling is done in the other function
  }

  PTRACE("Worker","exit.");
}

void tcpStrandWorker::dispatchOne(tcpSocketStrand *s){
  PTRACE("Worker", "in worker dispatch function");

  // Why does the old tcpSocketWorker engage a read lock via a
  // Strand::Sync object when 
  //    GIOP_S <: NetBufferedStream <: Strand_Sync
   
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
		  " tcpSocketMT Worker thread.");
    throw;
  }
  
  PTRACE("Worker", "worker finishing request dispatch");
}

#ifdef THREADPOOL
void PoolWorker::dispatchOne(tcpSocketStrand *s){
  tcpStrandWorker::dispatchOne(s);
  // wake up the rendezvous thread
  // this _should_ be done after the GIOP_S drops the read lock
  // (for the last time for that request)
  rendezvous->watchStrand(s);
}
#endif

